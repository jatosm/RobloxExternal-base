#include "game_gc.h"
#include "lua_vm.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "../../../utility/cache/cache.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace thenwefuckin::lua
{
namespace
{
inline bool valid(uint64_t a)
{
    return a >= 0x10000ull && a < 0x7FFFFFFFFFFFull;
}

template <typename T>
T rd(uint64_t a)
{
    if (!valid(a) || !g_proc || !g_proc->connected())
        return T{};
    return g_proc->read<T>(a);
}

inline bool raw(uint64_t a, void* dst, size_t n)
{
    if (!valid(a) || !g_proc || !g_proc->connected())
        return false;
    return g_proc->read_buf(a, dst, n);
}

template <typename T>
void wr(uint64_t a, const T& v)
{
    if (!valid(a) || !g_proc || !g_proc->connected())
        return;
    g_proc->write<T>(a, v);
}

inline uint64_t game_base()
{
    if (!g_proc || !g_proc->connected())
        return 0;
    return g_proc->base();
}

uint64_t game_datamodel()
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

struct StateLayout
{
    int top = 0;
    int base = 0;
    int global = 0;
};

bool looks_like_layout(uint64_t L, const StateLayout& lo, uint64_t* outG = nullptr)
{
    if (!valid(L) || (L & 0xF))
        return false;
    const uint64_t top = rd<uint64_t>(L + static_cast<uint64_t>(lo.top));
    const uint64_t base = rd<uint64_t>(L + static_cast<uint64_t>(lo.base));
    const uint64_t G = rd<uint64_t>(L + static_cast<uint64_t>(lo.global));
    if (!valid(top) || !valid(base) || !valid(G))
        return false;
    if ((top | base) & 0xF)
        return false;
    if (top < base || (top - base) > 0x40000)
        return false;
    const uint64_t tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
    const uint8_t st = rd<uint8_t>(G + Offsets::LuauGlobal::gcstate);
    if (tb < 1024 || tb > (512ull << 20))
        return false;
    if (st > 4)
        return false;
    if (outG)
        *outG = G;
    return true;
}

bool looks_like_lua_state(uint64_t L)
{
    const StateLayout cur{ static_cast<int>(Offsets::LuaState::Top),
        static_cast<int>(Offsets::LuaState::Base),
        static_cast<int>(Offsets::LuaState::Global) };
    return looks_like_layout(L, cur);
}

bool looks_like_any(uint64_t L, uint64_t* outG = nullptr)
{
    static const StateLayout layouts[3] = { { 0x20, 0x28, 0x38 }, { 0x20, 0x28, 0x78 }, { 0x8, 0x28, 0x20 } };
    for (auto& lo : layouts)
    {
        if (looks_like_layout(L, lo, outG))
            return true;
    }
    return false;
}

bool resolve_slot(uint64_t wrapper, uintptr_t slotOff, uint64_t* outL, uint64_t* outG)
{
    const uint64_t slotAddr = wrapper + slotOff;
    const uint64_t enc = rd<uint64_t>(slotAddr);
    const uint64_t cands[3] = { enc, enc ^ slotAddr, enc ^ wrapper };
    for (uint64_t c : cands)
    {
        uint64_t G = 0;
        if (!looks_like_any(c, &G))
            continue;
        if (outL)
            *outL = c;
        if (outG)
            *outG = G;
        return true;
    }
    return false;
}

uint64_t s_cached_L = 0;
uint64_t s_cached_G = 0;

bool cached_state(uint64_t* outL, uint64_t* outG)
{
    if (!s_cached_L)
        return false;
    uint64_t G = 0;
    if (!looks_like_any(s_cached_L, &G))
    {
        s_cached_L = 0;
        s_cached_G = 0;
        return false;
    }
    if (outL)
        *outL = s_cached_L;
    if (outG)
        *outG = G;
    s_cached_G = G;
    return true;
}

bool scan_wrapper(uint64_t wrapper, uint64_t* outL, uint64_t* outG, uint64_t* outHit)
{
    uint64_t chunk[128];
    for (uint64_t base = wrapper & ~0xFFFULL; base < wrapper + 0x1000; base += sizeof(chunk))
    {
        if (!g_proc || !g_proc->connected())
            return false;
        if (!g_proc->read_buf(base, chunk, sizeof(chunk)))
        {
            if (base > wrapper)
                break;
            continue;
        }
        for (size_t i = 0; i < 128; ++i)
        {
            const uint64_t addr = base + static_cast<uint64_t>(i) * 8ull;
            const uint64_t enc = chunk[i];
            const uint64_t cands[3] = { enc, enc ^ addr, enc ^ wrapper };
            for (uint64_t c : cands)
            {
                uint64_t G = 0;
                if (!looks_like_any(c, &G))
                    continue;
                if (outL)
                    *outL = c;
                if (outG)
                    *outG = G;
                if (outHit)
                    *outHit = addr;
                s_cached_L = c;
                s_cached_G = G;
                return true;
            }
        }
    }
    return false;
}

struct GameVm
{
    uint64_t L = 0;
    uint64_t G = 0;
    uint64_t tb = 0;
};

uint64_t find_script_context()
{
    const uint64_t dm = game_datamodel();
    if (!dm)
        return 0;
    for (auto& c : node{ dm }.children())
    {
        if (!c.addr || !valid(c.addr))
            continue;
        if (c.class_name() == "ScriptContext")
            return c.addr;
    }
    return 0;
}

void collect_game_vms(std::vector<GameVm>& out)
{
    out.clear();
    const uint64_t sc = find_script_context();
    if (!sc)
        return;
    const uintptr_t wraps[3] = {
        Offsets::ScriptContext::VmWrapper,
        Offsets::ScriptContext::VmWrapperBig,
        Offsets::ScriptContext::VmWrapper2,
    };
    const uintptr_t loffs[2] = {
        Offsets::ScriptContext::LuaState,
        Offsets::ScriptContext::LuaStateAlt,
    };
    std::unordered_set<uint64_t> seen_g;
    uint64_t cL = 0;
    uint64_t cG = 0;
    if (cached_state(&cL, &cG) && seen_g.insert(cG).second)
    {
        GameVm vm{};
        vm.L = cL;
        vm.G = cG;
        vm.tb = rd<uint64_t>(cG + Offsets::LuauGlobal::totalbytes);
        out.push_back(vm);
        return;
    }
    for (int wi = 0; wi < 3; ++wi)
    {
        const uint64_t w = rd<uint64_t>(sc + wraps[wi]);
        if (!valid(w))
            continue;
        for (int li = 0; li < 2; ++li)
        {
            uint64_t L = 0;
            uint64_t G = 0;
            if (!resolve_slot(w, loffs[li], &L, &G))
                continue;
            if (!valid(G) || !seen_g.insert(G).second)
                continue;
            GameVm vm{};
            vm.L = L;
            vm.G = G;
            vm.tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
            out.push_back(vm);
        }
    }
    if (!out.empty())
        return;
    for (int wi = 0; wi < 3; ++wi)
    {
        const uint64_t w = rd<uint64_t>(sc + wraps[wi]);
        if (!valid(w))
            continue;
        uint64_t L = 0;
        uint64_t G = 0;
        uint64_t hit = 0;
        if (!scan_wrapper(w, &L, &G, &hit))
            continue;
        if (!valid(G) || !seen_g.insert(G).second)
            continue;
        GameVm vm{};
        vm.L = L;
        vm.G = G;
        vm.tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
        out.push_back(vm);
        return;
    }
    if (!out.empty())
        return;
    uint64_t chunk[256];
    for (uint64_t base = sc & ~0xFFFULL; base < sc + 0x2000; base += sizeof(chunk))
    {
        if (!g_proc || !g_proc->connected())
            return;
        if (!g_proc->read_buf(base, chunk, sizeof(chunk)))
            break;
        for (size_t i = 0; i < 256; ++i)
        {
            const uint64_t cand = chunk[i];
            uint64_t G = 0;
            if (!looks_like_any(cand, &G))
                continue;
            if (!valid(G) || !seen_g.insert(G).second)
                continue;
            GameVm vm{};
            vm.L = cand;
            vm.G = G;
            vm.tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
            out.push_back(vm);
            if (out.size() >= 4)
                return;
        }
    }
}

uint64_t resolve_game_state()
{
    std::vector<GameVm> vms;
    collect_game_vms(vms);
    uint64_t best = 0;
    uint64_t best_tb = 0;
    for (auto& vm : vms)
    {
        if (vm.tb > best_tb)
        {
            best_tb = vm.tb;
            best = vm.L;
        }
    }
    return best;
}

uint64_t game_global()
{
    const uint64_t L = resolve_game_state();
    if (!L)
        return 0;
    const uint64_t G = rd<uint64_t>(L + Offsets::LuaState::Global);
    return valid(G) ? G : 0;
}

int l_setgc(lua_State* L)
{
    const char* opt = luaL_checkstring(L, 1);
    if (!opt)
        return 0;
    const uint64_t G = game_global();
    if (!G)
    {
        lua_pushnil(L);
        lua_pushstring(L, "no game lua state");
        return 2;
    }
    if (std::strcmp(opt, "stop") == 0)
    {
        wr<int64_t>(G + Offsets::LuauGlobal::GCthreshold, -1);
        lua_pushboolean(L, 1);
        return 1;
    }
    if (std::strcmp(opt, "restart") == 0)
    {
        const uint64_t tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
        wr<uint64_t>(G + Offsets::LuauGlobal::GCthreshold, tb);
        lua_pushboolean(L, 1);
        return 1;
    }
    if (std::strcmp(opt, "count") == 0)
    {
        const uint64_t tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
        lua_pushnumber(L, static_cast<lua_Number>(tb >> 10));
        return 1;
    }
    if (std::strcmp(opt, "isrunning") == 0)
    {
        const int64_t thr = rd<int64_t>(G + Offsets::LuauGlobal::GCthreshold);
        lua_pushboolean(L, thr != -1);
        return 1;
    }
    if (std::strcmp(opt, "pause") == 0 || std::strcmp(opt, "setpause") == 0)
    {
        const int old = rd<int32_t>(G + Offsets::LuauGlobal::gcpause);
        if (lua_gettop(L) >= 2 && !lua_isnil(L, 2))
        {
            int v = static_cast<int>(luaL_checkinteger(L, 2));
            if (v < 0) v = 0;
            if (v > 1000) v = 1000;
            wr<int32_t>(G + Offsets::LuauGlobal::gcpause, v);
        }
        lua_pushinteger(L, old);
        return 1;
    }
    if (std::strcmp(opt, "stepmul") == 0 || std::strcmp(opt, "setstepmul") == 0)
    {
        const int old = rd<int32_t>(G + Offsets::LuauGlobal::gcstepmul);
        if (lua_gettop(L) >= 2 && !lua_isnil(L, 2))
        {
            int v = static_cast<int>(luaL_checkinteger(L, 2));
            if (v < 0) v = 0;
            if (v > 10000) v = 10000;
            wr<int32_t>(G + Offsets::LuauGlobal::gcstepmul, v);
        }
        lua_pushinteger(L, old);
        return 1;
    }
    if (std::strcmp(opt, "collect") == 0 || std::strcmp(opt, "step") == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "collect/step need ingame call");
        return 2;
    }
    lua_pushnil(L);
    lua_pushstring(L, "unknown setgc opt");
    return 2;
}

const char* tt_name(int tt)
{
    if (tt == 0) return "nil";
    if (tt == 1) return "bool";
    if (tt == 2) return "lightud";
    if (tt == 3) return "number";
    if (tt == 5) return "vector";
    if (tt == 6) return "string";
    if (tt == 7) return "table";
    if (tt == 9) return "userdata";
    if (tt == 10) return "thread";
    if (tt == 11) return "buffer";
    return "other";
}

uint64_t gco_next(uint64_t obj, int tt)
{
    if (tt == 7)
        return rd<uint64_t>(obj + 40);
    return rd<uint64_t>(obj + 8);
}

uint64_t g_hdr_tbl = 0;
uint8_t g_hdr_lsz = 0;
uint64_t g_hdr_node = 0;

bool looks_like_table(uint64_t o)
{
    if (!valid(o) || (o & 0x7))
        return false;
    if (rd<uint8_t>(o + 1) != 7)
        return false;
    const uint8_t lsz = rd<uint8_t>(o + 3);
    if (lsz == 0 || lsz > 18)
        return false;
    const int32_t sz = rd<int32_t>(o + 8);
    if (sz < 0 || sz > 1000000)
        return false;
    const uint64_t arr = rd<uint64_t>(o + 32);
    const uint64_t n = rd<uint64_t>(o + 24);
    if (arr && !valid(arr))
        return false;
    if (!valid(n))
        return false;
    const uint64_t empty = game_base() + Offsets::LuauGlobal::dummynode;
    if (n == empty)
        return false;
    g_hdr_tbl = o;
    g_hdr_lsz = lsz;
    g_hdr_node = n;
    return true;
}

bool ts_eq(uint64_t ts, const char* key, size_t klen)
{
    if (!valid(ts) || (ts & 7))
        return false;
    if (rd<uint8_t>(ts + 1) != 6)
        return false;
    const uint32_t len = rd<uint32_t>(ts + 20);
    if (len != static_cast<uint32_t>(klen) || len > 512)
        return false;
    char buf[512];
    if (!raw(ts + 24, buf, len))
        return false;
    return std::memcmp(buf, key, klen) == 0;
}

bool ts_eq(uint64_t ts, const char* key, size_t klen);

bool table_val_str_eq(uint64_t val_node, const char* s, size_t len)
{
    const int tt = rd<int32_t>(val_node + 12) & 0xF;
    if (tt != 6)
        return false;
    const uint64_t ts = rd<uint64_t>(val_node);
    return ts_eq(ts, s, len);
}

bool table_has_str_val(uint64_t tbl, const char* needle, size_t nlen)
{
    if (!looks_like_table(tbl))
        return false;
    const uint8_t lsz = rd<uint8_t>(tbl + 3);
    const uint64_t n = rd<uint64_t>(tbl + 24);
    const int count = 1 << lsz;
    for (int i = 0; i < count; ++i)
    {
        if (table_val_str_eq(n + static_cast<uint64_t>(i) * 32ull, needle, nlen))
            return true;
    }
    return false;
}

bool table_find_str(uint64_t tbl, const char* key, uint64_t& val_node)
{
    if (!valid(tbl))
        return false;
    uint8_t lsz = 0;
    uint64_t n = 0;
    if (g_hdr_tbl == tbl)
    {
        lsz = g_hdr_lsz;
        n = g_hdr_node;
    }
    else
    {
        lsz = rd<uint8_t>(tbl + 3);
        if (lsz == 0 || lsz > 18)
            return false;
        n = rd<uint64_t>(tbl + 24);
        if (!valid(n))
            return false;
        const uint64_t empty = game_base() + Offsets::LuauGlobal::dummynode;
        if (n == empty)
            return false;
        g_hdr_tbl = tbl;
        g_hdr_lsz = lsz;
        g_hdr_node = n;
    }
    const int count = 1 << lsz;
    const size_t klen = key ? std::strlen(key) : 0;
    unsigned char buf[32 * 64];
    int off = 0;
    while (off < count)
    {
        int chunk = count - off;
        if (chunk > 64)
            chunk = 64;
        const size_t bytes = static_cast<size_t>(chunk) * 32ull;
        if (!raw(n + static_cast<uint64_t>(off) * 32ull, buf, bytes))
            return false;
        for (int i = 0; i < chunk; ++i)
        {
            const unsigned char* nd = buf + static_cast<size_t>(i) * 32ull;
            if ((nd[28] & 0xF) != 6)
                continue;
            uint64_t ts = 0;
            std::memcpy(&ts, nd + 16, 8);
            if (!key || !ts_eq(ts, key, klen))
                continue;
            val_node = n + static_cast<uint64_t>(off + i) * 32ull;
            return true;
        }
        off += chunk;
    }
    return false;
}

uint64_t proxy_addr(lua_State* L, int idx)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "tw_gtables");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return 0;
    }
    lua_pushvalue(L, idx);
    lua_rawget(L, -2);
    const uint64_t a = static_cast<uint64_t>(lua_tointeger(L, -1));
    lua_pop(L, 2);
    return a;
}

void push_game_tvalue(lua_State* L, uint64_t tv);

void push_table_proxy(lua_State* L, uint64_t addr)
{
    lua_newtable(L);
    luaL_getmetatable(L, "tw.luau_table");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        return;
    }
    lua_setmetatable(L, -2);
    lua_getfield(L, LUA_REGISTRYINDEX, "tw_gtables");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return;
    }
    lua_pushvalue(L, -2);
    lua_pushinteger(L, static_cast<lua_Integer>(addr));
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

void push_game_tvalue(lua_State* L, uint64_t tv)
{
    const int tt = rd<int32_t>(tv + 12);
    if (tt == 0)
    {
        lua_pushnil(L);
        return;
    }
    if (tt == 1)
    {
        lua_pushboolean(L, rd<int32_t>(tv) != 0);
        return;
    }
    if (tt == 3)
    {
        lua_pushnumber(L, rd<double>(tv));
        return;
    }
    if (tt == 6)
    {
        const uint64_t ts = rd<uint64_t>(tv);
        if (!valid(ts))
        {
            lua_pushnil(L);
            return;
        }
        const uint32_t len = rd<uint32_t>(ts + 20);
        if (len > 0x10000)
        {
            lua_pushnil(L);
            return;
        }
        if (len == 0)
        {
            lua_pushlstring(L, "", 0);
            return;
        }
        char stack[256];
        char* buf = stack;
        std::string heap;
        if (len > sizeof(stack))
        {
            heap.resize(len);
            buf = heap.data();
        }
        if (!raw(ts + 24, buf, len))
        {
            lua_pushnil(L);
            return;
        }
        lua_pushlstring(L, buf, len);
        return;
    }
    if (tt == 7)
    {
        const uint64_t t = rd<uint64_t>(tv);
        if (looks_like_table(t))
            push_table_proxy(L, t);
        else
            lua_pushnil(L);
        return;
    }
    lua_pushnil(L);
}

int proxy_index(lua_State* L)
{
    const uint64_t tbl = proxy_addr(L, 1);
    if (!tbl)
    {
        lua_pushnil(L);
        return 1;
    }
    const char* key = lua_tostring(L, 2);
    if (!key)
    {
        lua_pushnil(L);
        return 1;
    }
    uint64_t nd = 0;
    if (!table_find_str(tbl, key, nd))
    {
        lua_pushnil(L);
        return 1;
    }
    push_game_tvalue(L, nd);
    return 1;
}

int proxy_newindex(lua_State* L)
{
    const uint64_t tbl = proxy_addr(L, 1);
    if (!tbl)
        return 0;
    const char* key = lua_tostring(L, 2);
    if (!key)
        return 0;
    uint64_t nd = 0;
    if (!table_find_str(tbl, key, nd))
        return 0;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        const double n = lua_tonumber(L, 3);
        if (!valid(nd))
            return 0;
        if (!g_proc || !g_proc->connected())
            return 0;
        g_proc->write<double>(nd, n);
        g_proc->write<int32_t>(nd + 12, 3);
        return 0;
    }
    if (lua_type(L, 3) == LUA_TBOOLEAN)
    {
        if (!valid(nd))
            return 0;
        if (!g_proc || !g_proc->connected())
            return 0;
        g_proc->write<int32_t>(nd, lua_toboolean(L, 3) ? 1 : 0);
        g_proc->write<int32_t>(nd + 12, 1);
        return 0;
    }
    return 0;
}

int l_rawget_proxy(lua_State* L)
{
    luaL_checkany(L, 1);
    luaL_checkany(L, 2);
    if (lua_istable(L, 1) && proxy_addr(L, 1) != 0 && lua_type(L, 2) == LUA_TSTRING)
    {
        const uint64_t tbl = proxy_addr(L, 1);
        const char* key = lua_tostring(L, 2);
        uint64_t nd = 0;
        if (key && table_find_str(tbl, key, nd))
        {
            push_game_tvalue(L, nd);
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_call(L, 2, 1);
    return 1;
}

int getgc_iter(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const lua_Integer i = luaL_checkinteger(L, 2) + 1;
    lua_rawgeti(L, 1, i);
    if (lua_isnil(L, -1))
        return 0;
    lua_pushinteger(L, i);
    lua_insert(L, -2);
    return 2;
}

const char* g_fkey = nullptr;
const char* g_fval = nullptr;
bool g_fval_only = false;
uint64_t g_cached_find = 0;

bool try_add_table(lua_State* L, uint64_t obj, int& n, int maxn, std::unordered_set<uint64_t>& seen)
{
    if (n >= maxn || !looks_like_table(obj) || !seen.insert(obj).second)
        return false;
    if (g_fval_only && g_fval)
    {
        if (!table_has_str_val(obj, g_fval, std::strlen(g_fval)))
            return false;
    }
    else if (g_fkey)
    {
        uint64_t nd = 0;
        if (!table_find_str(obj, g_fkey, nd))
            return false;
        if (g_fval && !table_val_str_eq(nd, g_fval, std::strlen(g_fval)))
            return false;
    }
    if (g_fkey || g_fval_only)
        g_cached_find = obj;
    push_table_proxy(L, obj);
    lua_rawseti(L, -2, ++n);
    return true;
}

void collect_from_gco(lua_State* L, uint64_t head, int& n, int maxn, std::unordered_set<uint64_t>& seen)
{
    uint64_t cur = head;
    for (int i = 0; i < 80000 && n < maxn; ++i)
    {
        if (!valid(cur) || (cur & 0x7))
            break;
        const int tt = static_cast<int>(rd<uint8_t>(cur + 1));
        if (tt < 0 || tt > 15)
            break;
        if (tt == 7)
            try_add_table(L, cur, n, maxn, seen);
        const uint64_t nxt = gco_next(cur, tt);
        if (nxt == cur)
            break;
        cur = nxt;
    }
}

void collect_from_pages(lua_State* L, uint64_t head, int& n, int maxn,
    std::unordered_set<uint64_t>& seen, int lim, int next_off)
{
    uint64_t p = head;
    for (int pi = 0; pi < lim && n < maxn; ++pi)
    {
        if (!p || !valid(p))
            break;
        const int32_t pageSize = rd<int32_t>(p + 32);
        const int32_t blockSize = rd<int32_t>(p + 36);
        if (blockSize >= 40 && blockSize <= 512 && pageSize >= 128 && pageSize <= 65536)
        {
            const int cap = (pageSize - 64) / blockSize;
            if (cap > 0 && cap < 20000)
            {
                for (int i = 0; i < cap && n < maxn; ++i)
                    try_add_table(L, p + 64ull + static_cast<uint64_t>(i) * static_cast<uint64_t>(blockSize), n, maxn, seen);
            }
        }
        const uint64_t nxt = rd<uint64_t>(p + static_cast<uint64_t>(next_off));
        if (nxt == p)
            break;
        p = nxt;
    }
}

void visit_all_tables(uint64_t G, const std::function<void(uint64_t)>& fn)
{
    auto walk_gco = [&](uint64_t head) {
        uint64_t cur = head;
        for (int i = 0; i < 80000; ++i)
        {
            if (!valid(cur) || (cur & 0x7))
                break;
            const int tt = static_cast<int>(rd<uint8_t>(cur + 1));
            if (tt < 0 || tt > 15)
                break;
            if (tt == 7)
                fn(cur);
            const uint64_t nxt = gco_next(cur, tt);
            if (nxt == cur)
                break;
            cur = nxt;
        }
    };
    walk_gco(rd<uint64_t>(G + Offsets::LuauGlobal::gray));
    walk_gco(rd<uint64_t>(G + Offsets::LuauGlobal::grayagain));
    walk_gco(rd<uint64_t>(G + Offsets::LuauGlobal::weak));
    auto walk_pages = [&](uint64_t head, int lim, int next_off) {
        uint64_t p = head;
        for (int pi = 0; pi < lim; ++pi)
        {
            if (!p || !valid(p))
                break;
            const int32_t pageSize = rd<int32_t>(p + 32);
            const int32_t blockSize = rd<int32_t>(p + 36);
            if (blockSize >= 40 && blockSize <= 512 && pageSize >= 128 && pageSize <= 65536)
            {
                const int cap = (pageSize - 64) / blockSize;
                if (cap > 0 && cap < 20000)
                {
                    for (int i = 0; i < cap; ++i)
                        fn(p + 64ull + static_cast<uint64_t>(i) * static_cast<uint64_t>(blockSize));
                }
            }
            const uint64_t nxt = rd<uint64_t>(p + static_cast<uint64_t>(next_off));
            if (nxt == p)
                break;
            p = nxt;
        }
    };
    walk_pages(rd<uint64_t>(G + Offsets::LuauGlobal::gcopages), 200000, static_cast<int>(Offsets::LuauGlobal::page_next_free));
    walk_pages(rd<uint64_t>(G + Offsets::LuauGlobal::gcopages_large), 200000, static_cast<int>(Offsets::LuauGlobal::page_next_all));
}

int collect_game_tables(lua_State* L, uint64_t G, int maxn, int& n, std::unordered_set<uint64_t>& seen)
{
    if (g_cached_find && n < maxn)
        try_add_table(L, g_cached_find, n, maxn, seen);
    collect_from_gco(L, rd<uint64_t>(G + Offsets::LuauGlobal::gray), n, maxn, seen);
    collect_from_gco(L, rd<uint64_t>(G + Offsets::LuauGlobal::grayagain), n, maxn, seen);
    collect_from_gco(L, rd<uint64_t>(G + Offsets::LuauGlobal::weak), n, maxn, seen);
    if (n >= maxn)
        return n;
    collect_from_pages(L, rd<uint64_t>(G + Offsets::LuauGlobal::gcopages_large),
        n, maxn, seen, 200000, static_cast<int>(Offsets::LuauGlobal::page_next_all));
    return n;
}

int l_getgc(lua_State* L)
{
    const int top = lua_gettop(L);
    if (top < 1 || !lua_toboolean(L, 1))
    {
        lua_pushnil(L);
        lua_pushstring(L, "getgc() with no args reports info; pass true for tables");
        return 2;
    }
    std::vector<GameVm> vms;
    collect_game_vms(vms);
    if (vms.empty())
    {
        lua_pushnil(L);
        lua_pushstring(L, "no game lua state");
        return 2;
    }
    g_fkey = nullptr;
    g_fval = nullptr;
    lua_createtable(L, 256, 0);
    int n = 0;
    std::unordered_set<uint64_t> seen;
    seen.reserve(8192);
    for (auto& vm : vms)
        collect_game_tables(L, vm.G, 120000, n, seen);
    lua_pushcfunction(L, getgc_iter);
    lua_insert(L, -2);
    lua_pushinteger(L, 0);
    return 3;
}

int l_getgc_info(lua_State* L)
{
    std::vector<GameVm> vms;
    collect_game_vms(vms);
    const uint64_t G = game_global();
    if (!G || vms.empty())
    {
        lua_pushnil(L);
        lua_pushstring(L, "no game lua state");
        return 2;
    }
    int by_tt[16] = {};
    int total = 0;
    auto count_list = [&](uint64_t head) {
        uint64_t cur = head;
        for (int i = 0; i < 80000; ++i)
        {
            if (!valid(cur) || (cur & 0x7))
                break;
            const int tt = static_cast<int>(rd<uint8_t>(cur + 1));
            if (tt < 0 || tt >= 16)
                break;
            ++by_tt[tt];
            ++total;
            const uint64_t nxt = gco_next(cur, tt);
            if (nxt == cur)
                break;
            cur = nxt;
        }
    };
    count_list(rd<uint64_t>(G + Offsets::LuauGlobal::gray));
    count_list(rd<uint64_t>(G + Offsets::LuauGlobal::grayagain));
    count_list(rd<uint64_t>(G + Offsets::LuauGlobal::weak));
    const uint64_t tb = rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes);
    const int64_t thr = rd<int64_t>(G + Offsets::LuauGlobal::GCthreshold);
    lua_createtable(L, 0, 16);
    lua_pushnumber(L, static_cast<lua_Number>(tb >> 10));
    lua_setfield(L, -2, "kb");
    lua_pushinteger(L, static_cast<lua_Integer>(rd<uint8_t>(G + Offsets::LuauGlobal::gcstate)));
    lua_setfield(L, -2, "gcstate");
    lua_pushboolean(L, thr != -1);
    lua_setfield(L, -2, "isrunning");
    lua_pushinteger(L, static_cast<lua_Integer>(rd<int32_t>(G + Offsets::LuauGlobal::gcgoal)));
    lua_setfield(L, -2, "goal");
    lua_pushinteger(L, static_cast<lua_Integer>(rd<int32_t>(G + Offsets::LuauGlobal::gcpause)));
    lua_setfield(L, -2, "pause");
    lua_pushinteger(L, static_cast<lua_Integer>(rd<int32_t>(G + Offsets::LuauGlobal::gcstepmul)));
    lua_setfield(L, -2, "stepmul");
    lua_pushinteger(L, total);
    lua_setfield(L, -2, "total");
    char buf[32];
    std::snprintf(buf, sizeof(buf), "0x%llX", static_cast<unsigned long long>(vms[0].L));
    lua_pushstring(L, buf);
    lua_setfield(L, -2, "L");
    std::snprintf(buf, sizeof(buf), "0x%llX", static_cast<unsigned long long>(G));
    lua_pushstring(L, buf);
    lua_setfield(L, -2, "G");
    lua_createtable(L, 0, 16);
    for (int t = 0; t < 16; ++t)
    {
        if (by_tt[t] <= 0)
            continue;
        lua_pushinteger(L, by_tt[t]);
        lua_setfield(L, -2, tt_name(t));
    }
    lua_setfield(L, -2, "by_type");
    return 1;
}

int l_findgc(lua_State* L)
{
    const char* key = nullptr;
    const char* val = nullptr;
    bool val_only = false;
    if (lua_istable(L, 1))
    {
        lua_pushnil(L);
        lua_pushstring(L, "multi-key form needs strt_hash, still missing");
        return 2;
    }
    if (lua_type(L, 1) == LUA_TSTRING)
        key = lua_tostring(L, 1);
    else if (lua_isnoneornil(L, 1) && lua_type(L, 2) == LUA_TSTRING)
    {
        val = lua_tostring(L, 2);
        val_only = true;
    }
    else
        return luaL_error(L, "findgc(key) or findgc(key,val) or findgc(nil,val)");
    if (!val_only && lua_gettop(L) >= 2 && lua_type(L, 2) == LUA_TSTRING)
        val = lua_tostring(L, 2);
    std::vector<GameVm> vms;
    collect_game_vms(vms);
    if (vms.empty())
    {
        lua_pushnil(L);
        lua_pushstring(L, "no game lua state");
        return 2;
    }
    g_fkey = key;
    g_fval = val;
    g_fval_only = val_only;
    lua_createtable(L, 8, 0);
    int n = 0;
    std::unordered_set<uint64_t> seen;
    const int cap = 8000;
    for (auto& vm : vms)
    {
        collect_game_tables(L, vm.G, n + 4000 > cap ? cap : n + 4000, n, seen);
        if (n >= cap)
            break;
    }
    g_fkey = nullptr;
    g_fval = nullptr;
    g_fval_only = false;
    lua_pushcfunction(L, getgc_iter);
    lua_insert(L, -2);
    lua_pushinteger(L, 0);
    return 3;
}

int l_getrawkeys(lua_State* L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "tw_gtables");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_createtable(L, 0, 0);
        return 1;
    }
    lua_pop(L, 1);
    luaL_checkany(L, 1);
    uint64_t tbl = 0;
    if (lua_istable(L, 1))
    {
        lua_getfield(L, LUA_REGISTRYINDEX, "tw_gtables");
        if (lua_istable(L, -1))
        {
            lua_pushvalue(L, 1);
            lua_rawget(L, -2);
            tbl = static_cast<uint64_t>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    if (!tbl || !looks_like_table(tbl))
    {
        lua_createtable(L, 0, 0);
        return 1;
    }
    const uint8_t lsz = rd<uint8_t>(tbl + 3);
    const uint64_t n = rd<uint64_t>(tbl + 24);
    const int count = 1 << lsz;
    lua_createtable(L, 0, count > 0 ? count : 1);
    for (int i = 0; i < count; ++i)
    {
        const uint64_t nd = n + static_cast<uint64_t>(i) * 32ull;
        if ((rd<uint8_t>(nd + 28) & 0xF) != 6)
            continue;
        const uint64_t ts = rd<uint64_t>(nd + 16);
        if (!valid(ts) || rd<uint8_t>(ts + 1) != 6)
            continue;
        const uint32_t len = rd<uint32_t>(ts + 20);
        if (len == 0 || len > 128)
            continue;
        char buf[129]{};
        if (!raw(ts + 24, buf, len))
            continue;
        const int vtt = rd<int32_t>(nd + 12) & 0xF;
        if (vtt == 0)
            continue;
        lua_pushlstring(L, buf, len);
        push_game_tvalue(L, nd);
        lua_settable(L, -3);
    }
    return 1;
}

void verify_report(const std::string& s)
{
    log(OutLevel::Info, s);
}

int l_gcverify(lua_State* L)
{
    if (!g_proc || !g_proc->connected())
    {
        lua_pushnil(L);
        lua_pushstring(L, "no game");
        return 2;
    }
    const uint64_t base = game_base();
    char b[160];
    std::snprintf(b, sizeof(b), "base=0x%llX", static_cast<unsigned long long>(base));
    verify_report(b);
    const uint64_t dm = game_datamodel();
    {
        std::snprintf(b, sizeof(b), "datamodel=%s", dm ? "ok" : "MISSING");
        verify_report(b);
        if (!dm)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    uint64_t sc = 0;
    for (auto& c : node{ dm }.children())
    {
        if (!c.addr || !valid(c.addr))
            continue;
        if (c.class_name() == "ScriptContext")
        {
            sc = c.addr;
            break;
        }
    }
    {
        std::snprintf(b, sizeof(b), "scriptcontext=%s", sc ? "ok" : "MISSING");
        verify_report(b);
    }
    const uintptr_t wraps[3] = {
        Offsets::ScriptContext::VmWrapper,
        Offsets::ScriptContext::VmWrapperBig,
        Offsets::ScriptContext::VmWrapper2,
    };
    const char* wnames[3] = { "VmWrapper", "VmWrapperBig", "VmWrapper2" };
    int good = 0;
    auto dump_candidate = [&](const char* tag, uint64_t L) {
        const uint64_t top = rd<uint64_t>(L + Offsets::LuaState::Top);
        const uint64_t bs = rd<uint64_t>(L + Offsets::LuaState::Base);
        const uint64_t G = rd<uint64_t>(L + Offsets::LuaState::Global);
        const uint64_t gt = rd<uint64_t>(L + 0x68);
        std::snprintf(b, sizeof(b), "  %s raw: top=0x%llX base=0x%llX G=0x%llX gt=0x%llX", tag,
            static_cast<unsigned long long>(top), static_cast<unsigned long long>(bs),
            static_cast<unsigned long long>(G), static_cast<unsigned long long>(gt));
        verify_report(b);
        if (valid(G))
        {
            std::snprintf(b, sizeof(b), "  %s G: kb=%llu state=%u", tag,
                static_cast<unsigned long long>(rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes) >> 10),
                static_cast<unsigned>(rd<uint8_t>(G + Offsets::LuauGlobal::gcstate)));
            verify_report(b);
        }
    };
    for (int wi = 0; wi < 3; ++wi)
    {
        const uint64_t w = rd<uint64_t>(sc + wraps[wi]);
        if (!valid(w))
        {
            std::snprintf(b, sizeof(b), "%s: no wrapper", wnames[wi]);
            verify_report(b);
            continue;
        }
        std::snprintf(b, sizeof(b), "%s: wrapper=0x%llX", wnames[wi],
            static_cast<unsigned long long>(w));
        verify_report(b);
        const uint64_t l0 = rd<uint64_t>(w + Offsets::ScriptContext::LuaState);
        const uint64_t l1 = rd<uint64_t>(w + Offsets::ScriptContext::LuaStateAlt);
        std::snprintf(b, sizeof(b), "%s: slots=0x%llX/0x%llX", wnames[wi],
            static_cast<unsigned long long>(l0), static_cast<unsigned long long>(l1));
        verify_report(b);
        for (int li = 0; li < 2; ++li)
        {
            const uintptr_t off = li == 0 ? Offsets::ScriptContext::LuaState : Offsets::ScriptContext::LuaStateAlt;
            const uint64_t enc = rd<uint64_t>(w + off);
            uint64_t dL = 0;
            const bool hit = resolve_slot(w, off, &dL, nullptr);
            std::snprintf(b, sizeof(b), "  slot+%X decode: %s", static_cast<unsigned>(off),
                hit ? "HIT" : "miss");
            verify_report(b);
            (void)enc;
        }
        {
            uint64_t sL = 0;
            uint64_t sG = 0;
            uint64_t hit = 0;
            const bool found = scan_wrapper(w, &sL, &sG, &hit);
            std::snprintf(b, sizeof(b), "  wrapper scan: %s", found ? "HIT" : "nothing");
            verify_report(b);
            if (found)
            {
                std::snprintf(b, sizeof(b), "  scan hit at +0x%llX L=0x%llX",
                    static_cast<unsigned long long>(hit - w),
                    static_cast<unsigned long long>(sL));
                verify_report(b);
            }
        }
        static const StateLayout layouts[3] = { { 0x20, 0x28, 0x38 }, { 0x20, 0x28, 0x78 }, { 0x8, 0x28, 0x20 } };
        static const char* lnames[3] = { "A38", "A78", "C20" };
        char res[64]{};
        bool any = false;
        for (int k = 0; k < 3; ++k)
        {
            const bool p0 = looks_like_layout(l0, layouts[k]);
            const bool p1 = looks_like_layout(l1, layouts[k]);
            if (p0 || p1)
                any = true;
            char tmp[16];
            std::snprintf(tmp, sizeof(tmp), "%s:%c%c ", lnames[k], p0 ? 'Y' : '-', p1 ? 'Y' : '-');
            strcat_s(res, tmp);
        }
        if (any)
            ++good;
        std::snprintf(b, sizeof(b), "%s: layouts %s", wnames[wi], res);
        verify_report(b);
        if (valid(l0) && !looks_like_any(l0))
            dump_candidate("LuaState", l0);
        if (valid(l1) && !looks_like_any(l1))
            dump_candidate("LuaStateAlt", l1);
    }
    {
        const uint64_t G = game_global();
        if (G)
        {
            std::snprintf(b, sizeof(b), "global: kb=%llu state=%u pause=%d goal=%d mul=%d",
                static_cast<unsigned long long>(rd<uint64_t>(G + Offsets::LuauGlobal::totalbytes) >> 10),
                static_cast<unsigned>(rd<uint8_t>(G + Offsets::LuauGlobal::gcstate)),
                static_cast<int>(rd<int32_t>(G + Offsets::LuauGlobal::gcpause)),
                static_cast<int>(rd<int32_t>(G + Offsets::LuauGlobal::gcgoal)),
                static_cast<int>(rd<int32_t>(G + Offsets::LuauGlobal::gcstepmul)));
            verify_report(b);
        }
        else
            verify_report("global: MISSING");
    }
    {
        const uint64_t reg = rd<uint64_t>(base + Offsets::Reflection::NameRegistry);
        std::snprintf(b, sizeof(b), "reflection registry: %s", valid(reg) ? "present" : "MISSING");
        verify_report(b);
    }
    {
        const uint64_t desc = rd<uint64_t>(base + Offsets::WorldRoot::RaycastDescriptorRva);
        const uint64_t img = Offsets::ClientBuild::ImageSize;
        auto in_text = [&](uint64_t fn) {
            return fn >= base && fn < base + img;
        };
        if (valid(desc))
        {
            const uint64_t f80 = rd<uint64_t>(desc + 0x80);
            const uint64_t f90 = rd<uint64_t>(desc + 0x90);
            std::snprintf(b, sizeof(b), "boundfn +0x80: %s, +0x90: %s",
                in_text(f80) ? "code" : "not-code", in_text(f90) ? "code" : "not-code");
            verify_report(b);
        }
        else
            verify_report("raycast descriptor: MISSING");
    }
    lua_pushboolean(L, good > 0 ? 1 : 0);
    return 1;
}

void ensure_proxy_mt(lua_State* L)
{
    luaL_newmetatable(L, "tw.luau_table");
    lua_pushcfunction(L, proxy_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, proxy_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "tw_gtables");
}
}

void register_gc(lua_State* L)
{
    ensure_proxy_mt(L);
    lua_pushcfunction(L, l_setgc);
    lua_setglobal(L, "setgc");
    lua_pushcfunction(L, l_getgc);
    lua_setglobal(L, "getgc");
    lua_pushcfunction(L, l_getgc_info);
    lua_setglobal(L, "getgc_info");
    lua_pushcfunction(L, l_findgc);
    lua_setglobal(L, "findgc");
    lua_pushcfunction(L, l_getrawkeys);
    lua_setglobal(L, "getrawkeys");
    lua_pushcfunction(L, l_gcverify);
    lua_setglobal(L, "gcverify");
    lua_getglobal(L, "rawget");
    if (lua_isfunction(L, -1))
    {
        lua_pushcclosure(L, l_rawget_proxy, 1);
        lua_setglobal(L, "rawget");
    }
    else
    {
        lua_pop(L, 1);
    }
}
}
