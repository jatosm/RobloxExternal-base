#include "lua_draw.h"
#include "lua_core.h"
#include "lua_vm.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "../../../utility/cache/cache.h"

#include <cmath>
#include <mutex>
#include <string>
#include <vector>

namespace thenwefuckin::lua
{
namespace
{
std::mutex g_plr_mu;
std::vector<PlayerSnap> g_plr;

std::mutex g_draw_mu;
std::vector<DrawCmd> g_draws;

std::vector<int> g_render_cbs;
std::atomic<bool> g_has_render{ false };

unsigned draw_color(lua_State* L, int i)
{
    auto clamp255 = [](double v) -> unsigned {
        if (v < 0.0) return 0u;
        if (v > 255.0) return 255u;
        return static_cast<unsigned>(v);
    };
    const unsigned r = clamp255(luaL_checknumber(L, i));
    const unsigned g = clamp255(luaL_checknumber(L, i + 1));
    const unsigned b = clamp255(luaL_checknumber(L, i + 2));
    const unsigned a = lua_gettop(L) >= i + 3 ? clamp255(luaL_optnumber(L, i + 3, 255.0)) : 255u;
    return (a << 24) | (b << 16) | (g << 8) | r;
}

int l_draw_box(lua_State* L)
{
    const float x = static_cast<float>(luaL_checknumber(L, 1));
    const float y = static_cast<float>(luaL_checknumber(L, 2));
    const float w = static_cast<float>(luaL_checknumber(L, 3));
    const float h = static_cast<float>(luaL_checknumber(L, 4));
    DrawCmd d;
    d.kind = 0;
    d.x0 = x;
    d.y0 = y;
    d.x1 = x + w;
    d.y1 = y + h;
    d.color = draw_color(L, 5);
    push_draw(d);
    return 0;
}

int l_draw_fill(lua_State* L)
{
    const float x = static_cast<float>(luaL_checknumber(L, 1));
    const float y = static_cast<float>(luaL_checknumber(L, 2));
    const float w = static_cast<float>(luaL_checknumber(L, 3));
    const float h = static_cast<float>(luaL_checknumber(L, 4));
    DrawCmd d;
    d.kind = 1;
    d.x0 = x;
    d.y0 = y;
    d.x1 = x + w;
    d.y1 = y + h;
    d.color = draw_color(L, 5);
    push_draw(d);
    return 0;
}

int l_draw_text(lua_State* L)
{
    const float x = static_cast<float>(luaL_checknumber(L, 1));
    const float y = static_cast<float>(luaL_checknumber(L, 2));
    size_t n = 0;
    const char* s = luaL_checklstring(L, 3, &n);
    DrawCmd d;
    d.kind = 2;
    d.x0 = x;
    d.y0 = y;
    d.text.assign(s, n);
    if (lua_gettop(L) >= 4)
        d.color = draw_color(L, 4);
    push_draw(d);
    return 0;
}

int l_draw_line(lua_State* L)
{
    DrawCmd d;
    d.kind = 3;
    d.x0 = static_cast<float>(luaL_checknumber(L, 1));
    d.y0 = static_cast<float>(luaL_checknumber(L, 2));
    d.x1 = static_cast<float>(luaL_checknumber(L, 3));
    d.y1 = static_cast<float>(luaL_checknumber(L, 4));
    d.color = draw_color(L, 5);
    push_draw(d);
    return 0;
}

int l_players(lua_State* L)
{
    std::vector<PlayerSnap> list;
    {
        std::lock_guard<std::mutex> lk(g_plr_mu);
        list = g_plr;
    }
    lua_newtable(L);
    for (size_t i = 0; i < list.size(); ++i)
    {
        const PlayerSnap& p = list[i];
        lua_newtable(L);
        lua_pushlstring(L, p.name.data(), p.name.size());
        lua_setfield(L, -2, "name");
        lua_pushnumber(L, static_cast<lua_Number>(p.x));
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, static_cast<lua_Number>(p.y));
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, static_cast<lua_Number>(p.w));
        lua_setfield(L, -2, "w");
        lua_pushnumber(L, static_cast<lua_Number>(p.h));
        lua_setfield(L, -2, "h");
        lua_pushnumber(L, static_cast<lua_Number>(p.hp));
        lua_setfield(L, -2, "hp");
        lua_pushnumber(L, static_cast<lua_Number>(p.dist));
        lua_setfield(L, -2, "dist");
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    return 1;
}

int l_on_render(lua_State* L)
{
    if (lua_isnoneornil(L, 1))
    {
        for (int ref : g_render_cbs)
        {
            if (ref != LUA_NOREF)
                luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
        g_render_cbs.clear();
        g_has_render.store(false);
        return 0;
    }
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    g_render_cbs.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
    g_has_render.store(true);
    return 0;
}
}

void register_draw_api(lua_State* L)
{
    lua_newtable(L);
    lua_pushcfunction(L, l_draw_box);
    lua_setfield(L, -2, "box");
    lua_pushcfunction(L, l_draw_fill);
    lua_setfield(L, -2, "fill");
    lua_pushcfunction(L, l_draw_text);
    lua_setfield(L, -2, "text");
    lua_pushcfunction(L, l_draw_line);
    lua_setfield(L, -2, "line");
    lua_setglobal(L, "draw");
    lua_pushcfunction(L, l_players);
    lua_setglobal(L, "players");
    lua_getglobal(L, "cheat");
    if (lua_istable(L, -1))
    {
        lua_pushcfunction(L, l_on_render);
        lua_setfield(L, -2, "on_render");
    }
    lua_pop(L, 1);
}

void snapshot_players(float fw, float fh)
{
    using namespace thenwefuckin;
    std::vector<PlayerSnap> out;
    if (g_ctx.ready && visual::wcache::viewValid)
    {
        const auto& view = visual::wcache::view;
        const auto& camPos = visual::wcache::camPos;
        for (auto& wc : visual::wcache::entries)
        {
            if (!wc.valid)
                continue;
            if ((wc.head - camPos).length() < 2.5f || (wc.root - camPos).length() < 2.5f)
                continue;
            math::vec3 top3{ wc.head.x, wc.head.y + 0.6f, wc.head.z };
            const float drop = wc.isR6 ? 3.0f : 2.5f;
            math::vec3 bot3{ wc.root.x, wc.root.y - drop, wc.root.z };
            math::vec2 top{};
            math::vec2 bot{};
            if (!projection::to_screen(top3, view, fw, fh, top))
                continue;
            if (!projection::to_screen(bot3, view, fw, fh, bot))
                continue;
            float hh = fabsf(bot.y - top.y) + 10.0f;
            if (hh < 4.0f)
                hh = 4.0f;
            float ww = hh * 0.72f;
            if (ww < 3.0f)
                ww = 3.0f;
            const float midX = (top.x + bot.x) * 0.5f;
            const float minY = (std::min)(top.y, bot.y) - 5.0f;
            if (midX < -12.0f || midX > fw + 12.0f || minY + hh * 0.5f < -12.0f || minY + hh * 0.5f > fh + 12.0f)
                continue;
            float dist = (wc.root - g_ctx.local_pos).length();
            if (dist < 0.1f)
                dist = (wc.head - g_ctx.local_pos).length();
            PlayerSnap ps;
            ps.name = wc.name;
            ps.x = midX - ww * 0.5f;
            ps.y = minY;
            ps.w = ww;
            ps.h = hh;
            ps.hp = wc.hp;
            ps.dist = dist * 0.28f;
            out.push_back(std::move(ps));
            if (out.size() >= 64)
                break;
        }
    }
    std::lock_guard<std::mutex> lk(g_plr_mu);
    g_plr.swap(out);
}

std::vector<PlayerSnap> players_snapshot()
{
    std::lock_guard<std::mutex> lk(g_plr_mu);
    return g_plr;
}

bool has_render()
{
    return g_has_render.load();
}

void fire_render()
{
    std::lock_guard<std::recursive_mutex> lk(detail::mutex());
    lua_State* G = detail::state();
    if (!G || g_render_cbs.empty())
        return;
    detail::set_busy(true);
    const int base = lua_gettop(G);
    for (size_t i = 0; i < g_render_cbs.size(); ++i)
    {
        lua_rawgeti(G, LUA_REGISTRYINDEX, g_render_cbs[i]);
        if (!lua_isfunction(G, -1))
        {
            lua_pop(G, 1);
            continue;
        }
        lua_State* co = lua_newthread(G);
        lua_pushvalue(G, -2);
        lua_remove(G, -3);
        lua_xmove(G, co, 1);
        int nres = 0;
        const int st = lua_resume(co, G, 0, &nres);
        if (st != LUA_OK && st != LUA_YIELD)
        {
            log(OutLevel::Error, lua_tostring(co, -1) ? lua_tostring(co, -1) : "?");
            lua_pop(co, 1);
        }
        lua_settop(G, base);
    }
    detail::set_busy(false);
}

void push_draw(const DrawCmd& d)
{
    std::lock_guard<std::mutex> lk(g_draw_mu);
    if (g_draws.size() >= 2048)
        return;
    g_draws.push_back(d);
}

std::vector<DrawCmd> take_draws()
{
    std::lock_guard<std::mutex> lk(g_draw_mu);
    std::vector<DrawCmd> out;
    out.swap(g_draws);
    return out;
}

namespace detail
{
void reset_render()
{
    std::lock_guard<std::recursive_mutex> lk(mutex());
    lua_State* G = state();
    if (G)
    {
        for (int ref : g_render_cbs)
        {
            if (ref != LUA_NOREF)
                luaL_unref(G, LUA_REGISTRYINDEX, ref);
        }
    }
    g_render_cbs.clear();
    g_has_render.store(false);
    std::lock_guard<std::mutex> dk(g_draw_mu);
    g_draws.clear();
}

void clear_render_callbacks()
{
    std::lock_guard<std::recursive_mutex> lk(mutex());
    lua_State* G = state();
    if (G)
    {
        for (int ref : g_render_cbs)
        {
            if (ref != LUA_NOREF)
                luaL_unref(G, LUA_REGISTRYINDEX, ref);
        }
    }
    g_render_cbs.clear();
    g_has_render.store(false);
}
}
}
