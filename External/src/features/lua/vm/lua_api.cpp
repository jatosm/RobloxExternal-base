#include "lua_api.h"
#include "lua_core.h"
#include "lua_vm.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <Windows.h>
#include <winhttp.h>

#include <cctype>
#include <string>

#pragma comment(lib, "winhttp.lib")

namespace thenwefuckin::lua
{
namespace
{
int l_print(lua_State* L)
{
    const int n = lua_gettop(L);
    std::string line;
    lua_getglobal(L, "tostring");
    for (int i = 1; i <= n; ++i)
    {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK)
        {
            log(OutLevel::Error, lua_tostring(L, -1) ? lua_tostring(L, -1) : "?");
            lua_pop(L, 1);
            continue;
        }
        const char* s = lua_tostring(L, -1);
        if (i > 1)
            line.push_back('\t');
        line += s ? s : "nil";
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    log(OutLevel::Print, line);
    return 0;
}

int l_warn(lua_State* L)
{
    const int n = lua_gettop(L);
    std::string line;
    for (int i = 1; i <= n; ++i)
    {
        const char* s = luaL_tolstring(L, i, nullptr);
        if (i > 1)
            line.push_back('\t');
        line += s ? s : "nil";
        lua_pop(L, 1);
    }
    log(OutLevel::Warn, line);
    return 0;
}

int l_identifyexecutor(lua_State* L)
{
    lua_pushstring(L, "thenwefuckin");
    lua_pushstring(L, "1.0");
    return 2;
}

int l_wait(lua_State* L)
{
    const float t = static_cast<float>(luaL_optnumber(L, 1, 0.03));
    if (!lua_isyieldable(L))
        return 0;
    detail::schedule_wait(L, t);
    return lua_yield(L, 0);
}

int resume_new_thread(lua_State* L, int fn_idx)
{
    lua_State* co = lua_newthread(L);
    lua_pushvalue(L, fn_idx);
    lua_xmove(L, co, 1);
    int nres = 0;
    const int st = lua_resume(co, L, 0, &nres);
    if (st != LUA_OK && st != LUA_YIELD)
    {
        log(OutLevel::Error, lua_tostring(co, -1) ? lua_tostring(co, -1) : "?");
        lua_pop(co, 1);
    }
    lua_pushthread(co);
    lua_xmove(co, L, 1);
    return 1;
}

int l_spawn(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    return resume_new_thread(L, 1);
}

void push_delay_token(lua_State* L, int idx, unsigned seq)
{
    auto* ud = static_cast<detail::DelayToken*>(lua_newuserdata(L, sizeof(detail::DelayToken)));
    ud->idx = idx;
    ud->seq = seq;
    luaL_getmetatable(L, detail::kTaskMt);
    lua_setmetatable(L, -2);
}

int l_delay(lua_State* L)
{
    const float t = static_cast<float>(luaL_checknumber(L, 1));
    luaL_checktype(L, 2, LUA_TFUNCTION);
    const int idx = detail::alloc_delay(L, t, 2);
    push_delay_token(L, idx, detail::delay_seq(idx));
    return 1;
}

int l_defer(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    const int idx = detail::alloc_delay(L, 0.0f, 1);
    push_delay_token(L, idx, detail::delay_seq(idx));
    return 1;
}
}

int l_cancel(lua_State* L)
{
    auto* ud = static_cast<detail::DelayToken*>(luaL_testudata(L, 1, detail::kTaskMt));
    if (!ud)
        return 0;
    detail::cancel_delay(L, ud->idx, ud->seq);
    return 0;
}

namespace
{
int l_iskeypressed(lua_State* L)
{
    int vk = 0;
    if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char* s = lua_tostring(L, 1);
        if (!s || !s[0])
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        if (_stricmp(s, "space") == 0) vk = VK_SPACE;
        else if (_stricmp(s, "shift") == 0) vk = VK_SHIFT;
        else if (_stricmp(s, "ctrl") == 0) vk = VK_CONTROL;
        else if (_stricmp(s, "alt") == 0) vk = VK_MENU;
        else if (_stricmp(s, "tab") == 0) vk = VK_TAB;
        else if (_stricmp(s, "escape") == 0 || _stricmp(s, "esc") == 0) vk = VK_ESCAPE;
        else vk = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(s[0])));
    }
    else
        vk = static_cast<int>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, (GetAsyncKeyState(vk) & 0x8000) != 0);
    return 1;
}

int l_getmousepos(lua_State* L)
{
    POINT pt{};
    GetCursorPos(&pt);
    lua_pushnumber(L, static_cast<lua_Number>(pt.x));
    lua_pushnumber(L, static_cast<lua_Number>(pt.y));
    return 2;
}

unsigned bit_arg(lua_State* L, int idx)
{
    return static_cast<unsigned>(static_cast<long long>(luaL_checknumber(L, idx)));
}

int l_bit_band(lua_State* L)
{
    const int n = lua_gettop(L);
    unsigned r = 0xFFFFFFFFu;
    for (int i = 1; i <= n; ++i)
        r &= bit_arg(L, i);
    lua_pushinteger(L, static_cast<lua_Integer>(r));
    return 1;
}

int l_bit_bor(lua_State* L)
{
    const int n = lua_gettop(L);
    unsigned r = 0;
    for (int i = 1; i <= n; ++i)
        r |= bit_arg(L, i);
    lua_pushinteger(L, static_cast<lua_Integer>(r));
    return 1;
}

int l_bit_bxor(lua_State* L)
{
    const int n = lua_gettop(L);
    unsigned r = 0;
    for (int i = 1; i <= n; ++i)
        r ^= bit_arg(L, i);
    lua_pushinteger(L, static_cast<lua_Integer>(r));
    return 1;
}

int l_bit_bnot(lua_State* L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(~bit_arg(L, 1)));
    return 1;
}

int l_bit_shift(lua_State* L, bool left)
{
    const unsigned x = bit_arg(L, 1);
    const int d = static_cast<int>(luaL_checkinteger(L, 2)) & 31;
    lua_pushinteger(L, static_cast<lua_Integer>(left ? (x << d) : (x >> d)));
    return 1;
}

int l_bit_lshift(lua_State* L) { return l_bit_shift(L, true); }
int l_bit_rshift(lua_State* L) { return l_bit_shift(L, false); }

void register_bit32(lua_State* L)
{
    lua_newtable(L);
    lua_pushcfunction(L, l_bit_band);
    lua_setfield(L, -2, "band");
    lua_pushcfunction(L, l_bit_bor);
    lua_setfield(L, -2, "bor");
    lua_pushcfunction(L, l_bit_bxor);
    lua_setfield(L, -2, "bxor");
    lua_pushcfunction(L, l_bit_bnot);
    lua_setfield(L, -2, "bnot");
    lua_pushcfunction(L, l_bit_lshift);
    lua_setfield(L, -2, "lshift");
    lua_pushcfunction(L, l_bit_rshift);
    lua_setfield(L, -2, "rshift");
    lua_setglobal(L, "bit32");
}

bool parse_url(const std::string& url, bool& https, std::wstring& host, INTERNET_PORT& port, std::wstring& path)
{
    https = false;
    port = 80;
    path = L"/";
    std::string u = url;
    if (u.rfind("https://", 0) == 0)
    {
        https = true;
        port = 443;
        u = u.substr(8);
    }
    else if (u.rfind("http://", 0) == 0)
        u = u.substr(7);
    else
        return false;
    const size_t slash = u.find('/');
    std::string h = (slash == std::string::npos) ? u : u.substr(0, slash);
    if (slash != std::string::npos)
        path.assign(u.substr(slash).begin(), u.substr(slash).end());
    const size_t colon = h.find(':');
    if (colon != std::string::npos)
    {
        port = static_cast<INTERNET_PORT>(std::atoi(h.c_str() + colon + 1));
        h = h.substr(0, colon);
    }
    host.assign(h.begin(), h.end());
    return !host.empty();
}

bool http_get_raw(const std::string& url, std::string& out, int& status)
{
    out.clear();
    status = 0;
    bool https = false;
    std::wstring host, path;
    INTERNET_PORT port = 80;
    if (!parse_url(url, https, host, port, path))
        return false;
    HINTERNET ses = WinHttpOpen(L"thenwefuckin/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!ses)
        return false;
    HINTERNET con = WinHttpConnect(ses, host.c_str(), port, 0);
    if (!con)
    {
        WinHttpCloseHandle(ses);
        return false;
    }
    std::wstring meth = L"GET";
    HINTERNET req = WinHttpOpenRequest(con, meth.c_str(), path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, https ? WINHTTP_FLAG_SECURE : 0);
    if (!req)
    {
        WinHttpCloseHandle(con);
        WinHttpCloseHandle(ses);
        return false;
    }
    WinHttpSetTimeouts(req, 5000, 5000, 15000, 15000);
    const bool sent = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    const bool received = sent && WinHttpReceiveResponse(req, nullptr);
    if (received)
    {
        DWORD st = 0;
        DWORD sz = sizeof(st);
        WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &st, &sz, WINHTTP_NO_HEADER_INDEX);
        status = static_cast<int>(st);
        for (;;)
        {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0)
                break;
            const size_t old = out.size();
            out.resize(old + avail);
            DWORD read = 0;
            if (!WinHttpReadData(req, out.data() + old, avail, &read))
            {
                out.resize(old);
                break;
            }
            out.resize(old + read);
            if (out.size() > (8u << 20))
                break;
        }
    }
    WinHttpCloseHandle(req);
    WinHttpCloseHandle(con);
    WinHttpCloseHandle(ses);
    return received;
}

int l_httpget(lua_State* L)
{
    const char* url = luaL_checkstring(L, 1);
    std::string out;
    int status = 0;
    {
        std::string body;
        if (http_get_raw(url, body, status) && status >= 200 && status < 300)
        {
            lua_pushlstring(L, body.data(), body.size());
            return 1;
        }
        out = body;
    }
    return luaL_error(L, "httpget failed (%d)", status);
}

int l_cheat_version(lua_State* L)
{
    lua_pushstring(L, "1.0");
    return 1;
}

int l_cheat_fps(lua_State* L)
{
    lua_pushnumber(L, static_cast<lua_Number>(snapshot().fps));
    return 1;
}

int l_cheat_players(lua_State* L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(snapshot().players));
    return 1;
}

int l_cheat_time(lua_State* L)
{
    lua_pushnumber(L, static_cast<lua_Number>(GetTickCount64() / 1000.0));
    return 1;
}

int l_getgenv(lua_State* L)
{
    lua_pushglobaltable(L);
    return 1;
}

int l_checkcaller(lua_State* L)
{
    lua_pushboolean(L, 1);
    return 1;
}

int l_newcclosure(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushvalue(L, 1);
    return 1;
}

struct CloneBuf
{
    std::string data;
};

int l_clone_writer(lua_State*, const void* p, size_t sz, void* ud)
{
    static_cast<CloneBuf*>(ud)->data.append(static_cast<const char*>(p), sz);
    return 0;
}

int l_clonefunction(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (lua_iscfunction(L, 1))
    {
        lua_pushvalue(L, 1);
        return 1;
    }
    CloneBuf b;
    if (lua_dump(L, l_clone_writer, &b, 0) != 0)
        return luaL_error(L, "clone failed");
    if (luaL_loadbufferx(L, b.data.data(), b.data.size(), "clone", nullptr) != LUA_OK)
        return lua_error(L);
    lua_remove(L, -2);
    return 1;
}
}

void register_base_api(lua_State* L)
{
    lua_pushcfunction(L, l_print);
    lua_setglobal(L, "print");
    lua_pushcfunction(L, l_warn);
    lua_setglobal(L, "warn");
    lua_pushcfunction(L, l_identifyexecutor);
    lua_setglobal(L, "identifyexecutor");
    lua_pushcfunction(L, l_wait);
    lua_setglobal(L, "wait");
    lua_pushcfunction(L, l_spawn);
    lua_setglobal(L, "spawn");
    lua_pushcfunction(L, l_iskeypressed);
    lua_setglobal(L, "iskeypressed");
    lua_pushcfunction(L, l_getmousepos);
    lua_setglobal(L, "getmousepos");
    lua_pushcfunction(L, l_httpget);
    lua_setglobal(L, "httpget");
    lua_pushcfunction(L, l_getgenv);
    lua_setglobal(L, "getgenv");
    lua_pushcfunction(L, l_checkcaller);
    lua_setglobal(L, "checkcaller");
    lua_pushcfunction(L, l_newcclosure);
    lua_setglobal(L, "newcclosure");
    lua_pushcfunction(L, l_clonefunction);
    lua_setglobal(L, "clonefunction");
    lua_pushcfunction(L, l_cheat_time);
    lua_setglobal(L, "tick");
    lua_getglobal(L, "load");
    lua_setglobal(L, "loadstring");
    luaL_newmetatable(L, detail::kTaskMt);
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushcfunction(L, l_wait);
    lua_setfield(L, -2, "wait");
    lua_pushcfunction(L, l_spawn);
    lua_setfield(L, -2, "spawn");
    lua_pushcfunction(L, l_delay);
    lua_setfield(L, -2, "delay");
    lua_pushcfunction(L, l_defer);
    lua_setfield(L, -2, "defer");
    lua_pushcfunction(L, l_cancel);
    lua_setfield(L, -2, "cancel");
    lua_setglobal(L, "task");
    lua_newtable(L);
    lua_pushcfunction(L, l_cheat_version);
    lua_setfield(L, -2, "version");
    lua_pushcfunction(L, l_cheat_fps);
    lua_setfield(L, -2, "fps");
    lua_pushcfunction(L, l_cheat_players);
    lua_setfield(L, -2, "players");
    lua_pushcfunction(L, l_cheat_time);
    lua_setfield(L, -2, "time");
    lua_setglobal(L, "cheat");
    register_bit32(L);
}
}
