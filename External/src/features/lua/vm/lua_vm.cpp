#include "lua_vm.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_core.h"
#include "lua_api.h"
#include "lua_draw.h"
#include "lua_game.h"
#include "game_gc.h"

#include <Windows.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace thenwefuckin::lua
{
namespace
{
lua_State* g_L = nullptr;
std::recursive_mutex g_mu;
std::atomic<bool> g_busy{ false };
std::atomic<bool> g_tick_run{ false };
std::thread g_tick_th;

struct WaitEntry
{
    int ref = LUA_NOREF;
    float left = 0.0f;
};
std::vector<WaitEntry> g_waits;

struct DelayEntry
{
    int fn_ref = LUA_NOREF;
    float left = 0.0f;
    bool alive = false;
    unsigned seq = 0;
};
std::vector<DelayEntry> g_delays;
unsigned g_token_seq = 0;

std::mutex g_out_mu;
std::vector<OutLine> g_output;

std::mutex g_act_mu;
std::vector<std::function<void()>> g_actions;

std::mutex g_snap_mu;
Snapshot g_snap;

struct Budget
{
    long long blocks = 0;
};

void hook(lua_State* L, lua_Debug*)
{
    auto* b = static_cast<Budget*>(lua_getextraspace(L));
    if (++b->blocks > 200)
        luaL_error(L, "instruction limit exceeded");
}

}

namespace detail
{
std::recursive_mutex& mutex() { return g_mu; }
lua_State* state() { return g_L; }
void set_busy(bool b) { g_busy.store(b); }
void schedule_wait(lua_State* L, float sec)
{
    if (!lua_isyieldable(L))
        return;
    if (sec < 0.0f)
        sec = 0.0f;
    lua_pushthread(L);
    g_waits.push_back({ luaL_ref(L, LUA_REGISTRYINDEX), sec });
}
int alloc_delay(lua_State* L, float sec, int fn_idx)
{
    if (sec < 0.0f)
        sec = 0.0f;
    int idx = -1;
    for (int i = 0; i < static_cast<int>(g_delays.size()); ++i)
    {
        if (!g_delays[static_cast<size_t>(i)].alive && g_delays[static_cast<size_t>(i)].fn_ref == LUA_NOREF)
        {
            idx = i;
            break;
        }
    }
    if (idx < 0)
    {
        idx = static_cast<int>(g_delays.size());
        g_delays.push_back({});
    }
    lua_pushvalue(L, fn_idx);
    DelayEntry& d = g_delays[static_cast<size_t>(idx)];
    d.fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    d.left = sec;
    d.alive = true;
    d.seq = ++g_token_seq;
    return idx;
}
unsigned delay_seq(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(g_delays.size()))
        return 0;
    return g_delays[static_cast<size_t>(idx)].seq;
}
bool cancel_delay(lua_State* L, int idx, unsigned seq)
{
    if (idx < 0 || idx >= static_cast<int>(g_delays.size()))
        return false;
    DelayEntry& d = g_delays[static_cast<size_t>(idx)];
    if (!d.alive || d.seq != seq)
        return false;
    d.alive = false;
    if (d.fn_ref != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, d.fn_ref);
        d.fn_ref = LUA_NOREF;
    }
    return true;
}
}

void open_safe_libs(lua_State* L)
{
    luaL_requiref(L, LUA_GNAME, luaopen_base, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    lua_pop(L, 1);
    luaL_requiref(L, LUA_COLIBNAME, luaopen_coroutine, 1);
    lua_pop(L, 1);
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    lua_pushnil(L);
    lua_setglobal(L, "loadfile");
}

void register_api(lua_State* L)
{
    register_base_api(L);
    register_draw_api(L);
    register_game_api(L);
    register_gc(L);
}

void resume_wait(size_t i)
{
    const int ref = g_waits[i].ref;
    g_waits.erase(g_waits.begin() + static_cast<std::ptrdiff_t>(i));
    lua_rawgeti(g_L, LUA_REGISTRYINDEX, ref);
    lua_State* co = lua_tothread(g_L, -1);
    luaL_unref(g_L, LUA_REGISTRYINDEX, ref);
    if (!co)
    {
        lua_pop(g_L, 1);
        return;
    }
    int nres = 0;
    const int st = lua_resume(co, g_L, 0, &nres);
    if (st != LUA_OK && st != LUA_YIELD)
    {
        log(OutLevel::Error, lua_tostring(co, -1) ? lua_tostring(co, -1) : "?");
        lua_pop(co, 1);
    }
    lua_pop(g_L, 1);
}

void run_delay(size_t i)
{
    const int ref = g_delays[i].fn_ref;
    g_delays[i].alive = false;
    g_delays[i].fn_ref = LUA_NOREF;
    lua_rawgeti(g_L, LUA_REGISTRYINDEX, ref);
    luaL_unref(g_L, LUA_REGISTRYINDEX, ref);
    if (!lua_checkstack(g_L, 2))
    {
        lua_pop(g_L, 1);
        return;
    }
    lua_State* co = lua_newthread(g_L);
    lua_pushvalue(g_L, -2);
    lua_remove(g_L, -3);
    lua_xmove(g_L, co, 1);
    int nres = 0;
    const int st = lua_resume(co, g_L, 0, &nres);
    if (st != LUA_OK && st != LUA_YIELD)
    {
        log(OutLevel::Error, lua_tostring(co, -1) ? lua_tostring(co, -1) : "?");
        lua_pop(co, 1);
    }
    lua_pop(g_L, 1);
}

void tick_loop()
{
    auto last = std::chrono::steady_clock::now();
    while (g_tick_run.load())
    {
        const auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (dt < 0.0f)
            dt = 0.0f;
        if (dt > 0.1f)
            dt = 0.1f;
        tick(dt);
        Sleep(4);
    }
}

bool initialize()
{
    if (g_L)
        return true;
    g_L = luaL_newstate();
    if (!g_L)
        return false;
    *static_cast<Budget*>(lua_getextraspace(g_L)) = Budget{};
    lua_sethook(g_L, hook, LUA_MASKCOUNT, 100000);
    open_safe_libs(g_L);
    register_api(g_L);
    if (!g_tick_run.load())
    {
        g_tick_run.store(true);
        g_tick_th = std::thread(tick_loop);
    }
    return true;
}

void shutdown()
{
    g_tick_run.store(false);
    if (g_tick_th.joinable())
        g_tick_th.join();
    for (int i = 0; i < 200 && g_busy.load(); ++i)
        Sleep(10);
    std::lock_guard<std::recursive_mutex> lk(g_mu);
    if (!g_L)
        return;
    for (auto& w : g_waits)
    {
        if (w.ref != LUA_NOREF)
            luaL_unref(g_L, LUA_REGISTRYINDEX, w.ref);
    }
    g_waits.clear();
    for (auto& d : g_delays)
    {
        if (d.fn_ref != LUA_NOREF)
            luaL_unref(g_L, LUA_REGISTRYINDEX, d.fn_ref);
        d.fn_ref = LUA_NOREF;
        d.alive = false;
    }
    g_delays.clear();
    detail::clear_render_callbacks();
    lua_close(g_L);
    g_L = nullptr;
}

bool ready()
{
    return g_L != nullptr;
}

lua_State* state()
{
    return g_L;
}

bool execute(const std::string& source, const char* chunk_name)
{
    if (!g_L)
        return false;
    g_busy.store(true);
    std::lock_guard<std::recursive_mutex> lk(g_mu);
    if (!g_L)
    {
        g_busy.store(false);
        return false;
    }
    clear_output();
    detail::reset_render();
    const int base = lua_gettop(g_L);
    if (luaL_loadstring(g_L, source.c_str()) != LUA_OK)
    {
        log(OutLevel::Error, lua_tostring(g_L, -1) ? lua_tostring(g_L, -1) : "?");
        lua_settop(g_L, base);
        g_busy.store(false);
        return true;
    }
    lua_State* co = lua_newthread(g_L);
    lua_pushvalue(g_L, -2);
    lua_remove(g_L, -3);
    lua_xmove(g_L, co, 1);
    if (chunk_name && chunk_name[0])
    {
        lua_pushstring(co, chunk_name);
        lua_setglobal(co, "_CHUNK");
    }
    int nres = 0;
    const int st = lua_resume(co, g_L, 0, &nres);
    if (st != LUA_OK && st != LUA_YIELD)
    {
        log(OutLevel::Error, lua_tostring(co, -1) ? lua_tostring(co, -1) : "?");
        lua_pop(co, 1);
    }
    lua_settop(g_L, base);
    g_busy.store(false);
    log(OutLevel::Info, "executed");
    return true;
}

void tick(float dt)
{
    std::lock_guard<std::recursive_mutex> lk(g_mu);
    if (!g_L)
        return;
    g_busy.store(true);
    for (size_t i = 0; i < g_waits.size();)
    {
        g_waits[i].left -= dt;
        if (g_waits[i].left > 0.0f)
        {
            ++i;
            continue;
        }
        resume_wait(i);
    }
    for (size_t i = 0; i < g_delays.size(); ++i)
    {
        if (!g_delays[i].alive)
            continue;
        g_delays[i].left -= dt;
        if (g_delays[i].left > 0.0f)
            continue;
        run_delay(i);
    }
    g_busy.store(false);
}

void log(OutLevel level, const std::string& text)
{
    std::lock_guard<std::mutex> lk(g_out_mu);
    g_output.push_back({ level, text });
    while (g_output.size() > 300)
        g_output.erase(g_output.begin());
}

void clear_output()
{
    std::lock_guard<std::mutex> lk(g_out_mu);
    g_output.clear();
}

std::vector<OutLine> output_snapshot()
{
    std::lock_guard<std::mutex> lk(g_out_mu);
    return g_output;
}

void push_action(std::function<void()> fn)
{
    if (!fn)
        return;
    std::lock_guard<std::mutex> lk(g_act_mu);
    if (g_actions.size() < 256)
        g_actions.push_back(std::move(fn));
}

void run_actions()
{
    std::vector<std::function<void()>> local;
    {
        std::lock_guard<std::mutex> lk(g_act_mu);
        local.swap(g_actions);
    }
    for (auto& fn : local)
    {
        try
        {
            fn();
        }
        catch (...)
        {
        }
    }
}

void set_snapshot(const Snapshot& s)
{
    std::lock_guard<std::mutex> lk(g_snap_mu);
    g_snap = s;
}

Snapshot snapshot()
{
    std::lock_guard<std::mutex> lk(g_snap_mu);
    return g_snap;
}
}
