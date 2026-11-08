#include "lua_game.h"
#include "lua_vm.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "../../../utility/cache/cache.h"

#include <cstdint>
#include <string>
#include <vector>

namespace thenwefuckin::lua
{
namespace
{
uintptr_t resolve_datamodel()
{
    if (!g_proc || !g_proc->connected())
        return 0;
    if (g_ctx.ready && g_ctx.datamodel.addr)
        return g_ctx.datamodel.addr;
    const uint64_t base = g_proc->base();
    if (!base)
        return 0;
    const uint64_t fake = g_proc->read<uint64_t>(base + Offsets::FakeDataModel::Pointer);
    if (!fake)
        return 0;
    return g_proc->read<uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
}

void push_instance(lua_State* L, uintptr_t addr)
{
    *static_cast<uintptr_t*>(lua_newuserdata(L, sizeof(uintptr_t))) = addr;
    luaL_getmetatable(L, "thenwefuckin.Instance");
    lua_setmetatable(L, -2);
}

uintptr_t check_instance(lua_State* L, int idx)
{
    auto* ud = static_cast<uintptr_t*>(luaL_checkudata(L, idx, "thenwefuckin.Instance"));
    return ud ? *ud : 0;
}

int l_inst_name(lua_State* L)
{
    const uintptr_t addr = check_instance(L, 1);
    if (!addr || !g_proc || !g_proc->connected())
    {
        lua_pushnil(L);
        return 1;
    }
    const std::string s = node{ addr }.name();
    if (s.empty())
        lua_pushnil(L);
    else
        lua_pushlstring(L, s.data(), s.size());
    return 1;
}

int l_inst_class(lua_State* L)
{
    const uintptr_t addr = check_instance(L, 1);
    if (!addr || !g_proc || !g_proc->connected())
    {
        lua_pushnil(L);
        return 1;
    }
    const std::string s = node{ addr }.class_name();
    if (s.empty())
        lua_pushnil(L);
    else
        lua_pushlstring(L, s.data(), s.size());
    return 1;
}

int l_inst_parent(lua_State* L)
{
    const uintptr_t addr = check_instance(L, 1);
    if (!addr || !g_proc || !g_proc->connected())
    {
        lua_pushnil(L);
        return 1;
    }
    node p = node{ addr }.parent();
    if (!p)
    {
        lua_pushnil(L);
        return 1;
    }
    push_instance(L, p.addr);
    return 1;
}

int l_inst_children(lua_State* L)
{
    const uintptr_t addr = check_instance(L, 1);
    lua_newtable(L);
    if (!addr || !g_proc || !g_proc->connected())
        return 1;
    std::vector<node> kids = node{ addr }.children();
    int n = 0;
    for (auto& k : kids)
    {
        if (!k.addr)
            continue;
        push_instance(L, k.addr);
        lua_rawseti(L, -2, ++n);
        if (n >= 500)
            break;
    }
    return 1;
}

int l_inst_find(lua_State* L)
{
    const uintptr_t addr = check_instance(L, 1);
    const char* name = luaL_checkstring(L, 2);
    if (!addr || !name || !g_proc || !g_proc->connected())
    {
        lua_pushnil(L);
        return 1;
    }
    node hit = node{ addr }.find_child(name);
    if (!hit)
    {
        lua_pushnil(L);
        return 1;
    }
    push_instance(L, hit.addr);
    return 1;
}

int l_inst_isa(lua_State* L)
{
    const uintptr_t addr = check_instance(L, 1);
    const char* cls = luaL_checkstring(L, 2);
    bool ok = false;
    if (addr && cls && g_proc && g_proc->connected())
        ok = node{ addr }.class_name() == cls;
    lua_pushboolean(L, ok ? 1 : 0);
    return 1;
}

int l_game_root(lua_State* L)
{
    const uintptr_t dm = resolve_datamodel();
    if (!dm)
    {
        lua_pushnil(L);
        return 1;
    }
    push_instance(L, dm);
    return 1;
}

int l_game_workspace(lua_State* L)
{
    const uintptr_t dm = resolve_datamodel();
    if (!dm)
    {
        lua_pushnil(L);
        return 1;
    }
    node ws = node{ dm }.find_child("Workspace");
    if (!ws)
    {
        lua_pushnil(L);
        return 1;
    }
    push_instance(L, ws.addr);
    return 1;
}

int l_game_find(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    const uintptr_t dm = resolve_datamodel();
    if (!dm || !name || !name[0])
    {
        lua_pushnil(L);
        return 1;
    }
    std::vector<uintptr_t> stack;
    stack.push_back(dm);
    size_t visited = 0;
    while (!stack.empty() && visited < 20000)
    {
        const uintptr_t cur = stack.back();
        stack.pop_back();
        ++visited;
        node n{ cur };
        if (n.name() == name)
        {
            push_instance(L, cur);
            return 1;
        }
        std::vector<node> kids = n.children();
        for (auto it = kids.rbegin(); it != kids.rend(); ++it)
        {
            if (it->addr)
                stack.push_back(it->addr);
        }
    }
    lua_pushnil(L);
    return 1;
}
}

void register_game_api(lua_State* L)
{
    if (luaL_newmetatable(L, "thenwefuckin.Instance"))
    {
        lua_newtable(L);
        lua_pushcfunction(L, l_inst_name);
        lua_setfield(L, -2, "Name");
        lua_pushcfunction(L, l_inst_class);
        lua_setfield(L, -2, "ClassName");
        lua_pushcfunction(L, l_inst_parent);
        lua_setfield(L, -2, "Parent");
        lua_pushcfunction(L, l_inst_children);
        lua_setfield(L, -2, "Children");
        lua_pushcfunction(L, l_inst_find);
        lua_setfield(L, -2, "FindFirstChild");
        lua_pushcfunction(L, l_inst_isa);
        lua_setfield(L, -2, "IsA");
        lua_setfield(L, -2, "__index");
    }
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushcfunction(L, l_game_root);
    lua_setfield(L, -2, "root");
    lua_pushcfunction(L, l_game_workspace);
    lua_setfield(L, -2, "workspace");
    lua_pushcfunction(L, l_game_find);
    lua_setfield(L, -2, "find");
    lua_setglobal(L, "game");
}
}
