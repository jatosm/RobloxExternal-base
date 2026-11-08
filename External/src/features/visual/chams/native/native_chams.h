#pragma once
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "../../../../utility/cache/cache.h"
#include "../../../../utility/logger/logger.h"
#include "../../../../menu/overlay/settings.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cwchar>
#include <initializer_list>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#ifndef CFG_CALL_TARGET_VALID
#define CFG_CALL_TARGET_VALID 0x00000001
#endif
#pragma comment(lib, "d3dcompiler.lib")
namespace thenwefuckin::visual::native_chams
{
    namespace pending
    {
        inline constexpr std::uintptr_t FastClusterBindingSubobject = 0;
        inline constexpr std::uintptr_t FastClusterEntityBegin = 0;
        inline constexpr std::uintptr_t FastClusterEntityEnd = 0;
        inline constexpr std::uintptr_t FastClusterBindingOwner = 0;
        inline constexpr std::uintptr_t FastClusterEntityMaterialPtr = 0;
        inline constexpr std::uintptr_t FastClusterEntityDecalMaterialPtr = 0;
    }

    constexpr int k_slot_create_buf = 3;
    constexpr int k_slot_create_il = 11;
    constexpr int k_slot_create_vs = 12;
    constexpr int k_slot_create_ps = 15;
    constexpr int k_slot_create_blend = 20;
    constexpr int k_slot_create_dss = 21;
    constexpr int k_slot_create_raster = 22;
    constexpr int k_slot_vscb = 7;
    constexpr int k_slot_psset = 9;
    constexpr int k_slot_vsset = 11;
    constexpr int k_slot_drawidx = 12;
    constexpr int k_slot_pscb = 16;
    constexpr int k_slot_ilset = 17;
    constexpr int k_slot_vbset = 18;
    constexpr int k_slot_ibset = 19;
    constexpr int k_slot_topo = 24;
    constexpr int k_slot_omset = 33;
    constexpr int k_slot_blendset = 35;
    constexpr int k_slot_dssset = 36;
    constexpr int k_slot_rsset = 43;
    constexpr int k_slot_update = 48;
    constexpr int k_slot_psget = 74;
    constexpr int k_slot_blendget = 91;
    constexpr int k_slot_dssget = 92;
    constexpr int k_slot_present = 8;
    constexpr int k_slot_dc_draw = 28;
    constexpr int k_vt_cap = 510;

    constexpr std::uint32_t k_cmd_vs = 1;
    constexpr std::uint32_t k_cmd_ps = 2;
    constexpr std::uint32_t k_cmd_il = 3;
    constexpr std::uint32_t k_cmd_buf = 4;
    constexpr std::uint32_t k_cmd_blend = 5;
    constexpr std::uint32_t k_cmd_raster = 6;
    constexpr std::uint32_t k_cmd_dss = 7;

    constexpr int k_max_targets = 512;
    constexpr int k_max_ents = 256;
    constexpr int k_ent_budget = 2048;

    inline std::uint32_t Draws();
    inline std::uint32_t Items();
    inline std::uint32_t Entered();
    inline std::uint32_t Creates();
    inline std::uint32_t Ready();
    inline int ShaderNameCount();

    inline constexpr char k_hlsl[] = R"HLSL(
cbuffer Cham : register(b13)
{
    float4 color;
    float4 glow;
    float time;
    int mode;
    float depth_scale;
    float depth_bias;
};

struct Out
{
    float4 c : SV_TARGET;
    float z : SV_DEPTH;
};

float3 screen_normal(float4 pos)
{
    float2 g = float2(ddx(pos.w), ddy(pos.w)) / max(abs(pos.w), 1e-6) * 480.0;
    return normalize(float3(g, 1.0));
}

float hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float vnoise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + float2(1.0, 0.0));
    float c = hash21(i + float2(0.0, 1.0));
    float d = hash21(i + float2(1.0, 1.0));
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

float fbm2(float2 p)
{
    float a = 0.0;
    float w = 0.5;
    [unroll] for (int k = 0; k < 4; ++k)
    {
        a += w * vnoise(p);
        p = p * 2.02 + 7.3;
        w *= 0.5;
    }
    return a;
}

float3 hue_rot(float3 c, float h)
{
    float3 k = float3(0.57735, 0.57735, 0.57735);
    return c * cos(h) + cross(k, c) * sin(h) + k * dot(k, c) * (1.0 - cos(h));
}

float3 lit(float3 fill, float3 N, float amb, float k)
{
    return fill * (amb + saturate(dot(N, normalize(float3(0.45, -0.70, 0.55)))) * k);
}

float3 spec(float3 fill, float3 N, float amb, float k, float p)
{
    float3 L = normalize(float3(0.50, -0.80, 0.35));
    float3 H = normalize(L + float3(0.0, 0.0, 1.0));
    return fill * (amb + saturate(dot(N, L)) * k) + pow(saturate(dot(N, H)), p);
}

void add_glow(inout float3 c, inout float a, float mask)
{
    float w = saturate(mask) * glow.a;
    float t = w / (1.0 + w);
    c = lerp(c, glow.rgb, t);
    c += glow.rgb * t * t * 0.75;
    a = saturate(a + t * (1.0 - a) * 0.9);
}

Out ps_main(float4 pos : SV_POSITION)
{
    float3 fill = color.rgb;
    float a = color.a;
    float3 N = screen_normal(pos);
    float rim = saturate(1.0 - saturate(N.z));
    float rp = pow(abs(rim), 2.4);
    float edge = saturate(length(fwidth(N)) * 2.2);
    float3 N5 = normalize(round(N * 5.0) / 5.0);
    float fe5 = saturate(length(fwidth(N5)) * 3.2);
    float2 uv = pos.xy;
    float3 c = fill;
    float gmask = rp * 0.55 + edge * 0.75;

    switch (mode)
    {
    case 0:
        gmask = rp * 0.55 + edge * 0.8;
        break;
    case 1:
        c = lit(fill, N, 0.32, 0.68);
        gmask = pow(abs(rim), 2.0) * 0.7 + edge * 0.55;
        break;
    case 2:
        c = fill * (0.38 + saturate(N.y) * 0.35);
        gmask = edge * 0.7;
        break;
    case 3:
        c = fill * (0.18 + pow(abs(rim), 1.2) * 0.55);
        a *= saturate(0.10 + pow(abs(rim), 1.2) * 0.9);
        gmask = pow(abs(rim), 1.2) * 0.85 + edge * 0.55;
        break;
    case 4:
        c = spec(fill, N, 0.16, 0.45, 64.0);
        a *= saturate(0.18 + rim * 0.7);
        gmask = pow(abs(rim), 1.5) * 0.9 + edge * 0.6;
        break;
    case 5:
        c = fill * (0.22 + saturate(N.y * 0.4 + 0.2));
        a *= saturate(0.30 + rim * 0.55);
        gmask = pow(abs(rim), 1.35) * 0.8 + fe5 * 0.75;
        break;
    case 6:
        c = hue_rot(fill, rim * 4.0 + edge * 2.0);
        a *= saturate(0.20 + rim * 0.65);
        gmask = pow(abs(rim), 1.4) * 0.8;
        break;
    case 7:
    {
        float n = fbm2(uv * 0.010 + float2(0.0, -time * 0.40));
        n = fbm2(uv * 0.016 + n * 1.4 + time * 0.12);
        float flame = smoothstep(0.30, 0.75, n);
        c = fill * (0.14 + flame * 0.22);
        gmask = flame * 0.9 + rp * 0.35;
    }
        break;
    default:
        c = fill * (0.12 + pow(abs(rim), 0.8) * 0.2);
        gmask = pow(abs(rim), 0.9) * 0.95 + edge * 0.4;
        break;
    }

    add_glow(c, a, gmask);
    Out o;
    o.c = float4(saturate(c), saturate(a));
    o.z = saturate(pos.z * depth_scale + depth_bias);
    return o;
}
)HLSL";

#pragma pack(push, 8)
    struct FrameCB
    {
        float vp[16];
        float cam[3];
        float time;
        int mode;
        float pad[3];
    };
    static_assert(sizeof(FrameCB) == 96);

    struct RemoteState
    {
        std::uint32_t enabled;
        std::uint32_t cmd;
        std::uint32_t ready;
        std::uint32_t ngeom;
        std::uint64_t device;
        std::uint64_t ctx;
        std::uint64_t orig_present;
        std::uint64_t orig_draw;
        std::uint64_t vs;
        std::uint64_t ps;
        std::uint64_t il;
        std::uint64_t cb;
        std::uint64_t cb1;
        std::uint64_t blend;
        std::uint64_t raster;
        std::uint64_t dss;
        std::uint64_t last_rtv;
        std::uint64_t last_dsv;
        std::uint64_t bytecode;
        std::uint64_t bytecode_len;
        std::uint64_t out_res;
        std::uint64_t init_sys;
        std::uint64_t geoms;
        std::uint32_t entered;
        std::uint32_t draws;
        std::uint32_t creates;
        std::uint32_t busy;
        std::uint32_t stage;
        std::uint32_t il_count;
        FrameCB frame;
        std::uint64_t fn_create_vs;
        std::uint64_t fn_create_ps;
        std::uint64_t fn_create_il;
        std::uint64_t fn_create_buf;
        std::uint64_t fn_create_blend;
        std::uint64_t fn_create_raster;
        std::uint64_t fn_create_dss;
        std::uint64_t fn_vsset;
        std::uint64_t fn_psset;
        std::uint64_t fn_ilset;
        std::uint64_t fn_vbset;
        std::uint64_t fn_ibset;
        std::uint64_t fn_topo;
        std::uint64_t fn_omset;
        std::uint64_t fn_omget;
        std::uint64_t fn_blendset;
        std::uint64_t fn_dssset;
        std::uint64_t fn_rsset;
        std::uint64_t fn_vscb;
        std::uint64_t fn_pscb;
        std::uint64_t fn_update;
        std::uint64_t fn_drawidx;
        std::uint64_t dc;
        std::uint64_t fn_psget;
        std::uint64_t fn_dssget;
        std::uint64_t fn_blendget;
    };
#pragma pack(pop)

    static_assert(offsetof(RemoteState, cmd) == 0x04);
    static_assert(offsetof(RemoteState, device) == 0x10);
    static_assert(offsetof(RemoteState, orig_present) == 0x20);
    static_assert(offsetof(RemoteState, orig_draw) == 0x28);
    static_assert(offsetof(RemoteState, vs) == 0x30);
    static_assert(offsetof(RemoteState, last_rtv) == 0x70);
    static_assert(offsetof(RemoteState, bytecode) == 0x80);
    static_assert(offsetof(RemoteState, geoms) == 0xA0);
    static_assert(offsetof(RemoteState, entered) == 0xA8);
    static_assert(offsetof(RemoteState, busy) == 0xB4);
    static_assert(offsetof(RemoteState, frame) == 0xC0);
    static_assert(offsetof(RemoteState, fn_create_vs) == 0x120);
    static_assert(offsetof(RemoteState, fn_omget) == 0x190);
    static_assert(offsetof(RemoteState, fn_vscb) == 0x1B0);
    static_assert(offsetof(RemoteState, fn_pscb) == 0x1B8);
    static_assert(offsetof(RemoteState, fn_update) == 0x1C0);
    static_assert(offsetof(RemoteState, fn_drawidx) == 0x1C8);
    static_assert(offsetof(RemoteState, dc) == 0x1D0);
    static_assert(offsetof(RemoteState, fn_psget) == 0x1D8);
    static_assert(offsetof(RemoteState, fn_dssget) == 0x1E0);
    static_assert(offsetof(RemoteState, fn_blendget) == 0x1E8);
    static_assert(sizeof(RemoteState) == 0x1F0);

    struct Patch
    {
        std::uintptr_t at = 0;
        std::uint64_t old = 0;
    };

    struct Target
    {
        std::uint64_t geom;
        float color[4];
        float glow[4];
        float time;
        int mode;
        float depth_scale;
        float depth_bias;
    };
    static_assert(sizeof(Target) == 56);

    struct Hook
    {
        std::uintptr_t state = 0;
        std::uintptr_t geoms = 0;
        std::uintptr_t cave = 0;
        std::size_t cave_n = 0;
        std::uintptr_t present_thunk = 0;
        std::uintptr_t create_stub = 0;
        std::uintptr_t draw_thunk = 0;
        std::uintptr_t device = 0;
        std::uintptr_t ctx = 0;
        std::uintptr_t dc = 0;
        std::uintptr_t dc_vt = 0;
        std::uintptr_t swap = 0;
        std::uintptr_t swap_vt = 0;
        std::uintptr_t vt_mem = 0;
        std::uintptr_t orig_present = 0;
        std::uintptr_t orig_draw = 0;
        std::uintptr_t module = 0;
        std::uintptr_t ps_dxbc = 0;
        std::size_t ps_len = 0;
        Patch patches[16]{};
        int npatch = 0;
        int stage = 0;
        int ntarget = 0;
        int slot = 0;
        int nnode = 0;
        int nent = 0;
        int ndrop = 0;
        int nbind = 0;
        int nswvt = 0;
        int ndcvt = 0;
        bool hooked = false;
        bool depth_rev = false;
        bool cmd_wait = false;
        bool logged = false;
        bool warned = false;
        std::uint8_t occl = 0;
    };

    inline Hook g{};
    inline std::chrono::steady_clock::time_point g_fail{};
    inline const char* g_why = nullptr;
    inline int g_nunk = 0;
    inline int g_nfc = 0;
    inline int g_npick = 0;
    inline std::uint64_t g_unk_vt = 0;

    inline bool addr_ok(std::uintptr_t a)
    {
        return a >= 0x10000ull && a < 0x00007FFFFFFFFFFFull;
    }
    inline bool live()
    {
        return g_proc && g_proc->connected() && g_proc->alive();
    }
    inline bool is_valid(std::uint64_t p)
    {
        if (!live() || !p)
            return false;
        MEMORY_BASIC_INFORMATION mbi{};
        return VirtualQueryEx(g_proc->raw(), (void*)(std::uintptr_t)p, &mbi, sizeof(mbi))
            && mbi.State == MEM_COMMIT;
    }
    inline bool rd(std::uint64_t at, void* dst, std::size_t n)
    {
        return live() && g_proc->read_buf(at, dst, n);
    }
    inline std::uint64_t rd64(std::uint64_t at)
    {
        return live() ? g_proc->read<std::uint64_t>(at) : 0ull;
    }
    inline bool wr(std::uint64_t at, const void* src, std::size_t n)
    {
        return live() && g_proc->write_buf(at, src, n);
    }
    inline void log_line(const char* fmt, ...)
    {
        char buf[512];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        Logger::log("chams", buf);
    }

    inline std::uintptr_t module_base()
    {
        return (std::uintptr_t)g_proc->base();
    }

    inline std::uintptr_t module_by_name(const wchar_t* name)
    {
        if (!live() || !name)
            return 0;
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, g_proc->pid());
        if (snap == INVALID_HANDLE_VALUE)
            return 0;
        MODULEENTRY32W me{};
        me.dwSize = sizeof(me);
        std::uintptr_t found = 0;
        if (Module32FirstW(snap, &me))
        {
            do
            {
                if (_wcsicmp(me.szModule, name) == 0)
                {
                    found = (std::uintptr_t)me.modBaseAddr;
                    break;
                }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
        return found;
    }

    inline std::uintptr_t visual_engine()
    {
        const std::uintptr_t base = module_base();
        if (!base)
            return 0;
        const std::uint64_t ve = rd64(base + Offsets::VisualEngine::Pointer);
        return is_valid(ve) ? (std::uintptr_t)ve : 0;
    }

    inline std::uintptr_t render_view()
    {
        const std::uintptr_t ve = visual_engine();
        if (!ve)
            return 0;
        const std::uint64_t rv = rd64(ve + Offsets::VisualEngine::RenderView);
        return is_valid(rv) ? (std::uintptr_t)rv : 0;
    }

    inline bool want_reverse()
    {
        const std::uintptr_t ve = visual_engine();
        if (!ve)
            return g.depth_rev;
        float m[16]{};
        if (!rd(ve + Offsets::VisualEngine::ViewMatrix, m, sizeof(m)))
            return g.depth_rev;
        const float zl = std::sqrt(m[8] * m[8] + m[9] * m[9] + m[10] * m[10]);
        const float wl = std::sqrt(m[12] * m[12] + m[13] * m[13] + m[14] * m[14]);
        if (wl < 1e-4f)
            return g.depth_rev;
        return zl < wl * 0.5f;
    }

    inline bool find_d3d()
    {
        if (g.device && is_valid(g.device) && g.ctx && is_valid(g.ctx) &&
            g.swap && is_valid(g.swap))
            return true;
        const std::uintptr_t base = module_base();
        const std::uintptr_t rv = render_view();
        if (!base || !rv)
            return false;
        const std::uintptr_t wrap = (std::uintptr_t)rd64(rv + Offsets::RenderView::DeviceD3D11);
        if (!is_valid(wrap))
            return false;
        if (rd64(wrap) != base + Offsets::DeviceD3D11Gfx::VTableRva)
            return false;
        const std::uintptr_t dev = (std::uintptr_t)rd64(wrap + Offsets::DeviceD3D11Gfx::DevicePtr);
        const std::uintptr_t ctx = (std::uintptr_t)rd64(wrap + Offsets::DeviceD3D11Gfx::ContextPtr);
        const std::uintptr_t swap = (std::uintptr_t)rd64(wrap + Offsets::DeviceD3D11Gfx::SwapChainPtr);
        const std::uintptr_t dc = (std::uintptr_t)rd64(wrap + Offsets::DeviceD3D11Gfx::ContextObj);
        if (!addr_ok(dev) || !addr_ok(ctx) || !addr_ok(swap) || !addr_ok(dc))
            return false;
        if (rd64(dc + Offsets::DeviceD3D11Gfx::ContextObjToContext) != ctx)
            return false;
        g.device = dev;
        g.ctx = ctx;
        g.swap = swap;
        g.dc = dc;
        return true;
    }

    inline std::uintptr_t vt_fn(std::uintptr_t obj, int slot)
    {
        const std::uintptr_t vt = (std::uintptr_t)rd64(obj);
        if (!addr_ok(vt))
            return 0;
        const std::uintptr_t fn = (std::uintptr_t)rd64(vt + (std::size_t)slot * 8);
        return addr_ok(fn) ? fn : 0;
    }

    inline void append_u64(std::vector<std::uint8_t>& c, std::uint64_t v)
    {
        const auto* b = (const std::uint8_t*)&v;
        c.insert(c.end(), b, b + 8);
    }
    inline void append_call_rax(std::vector<std::uint8_t>& c)
    {
        c.insert(c.end(), { 0xFF, 0xD0 });
    }
    inline std::size_t append_jcc32(std::vector<std::uint8_t>& c, std::uint8_t cc)
    {
        c.push_back(0x0F);
        c.push_back((std::uint8_t)(0x80 | cc));
        c.insert(c.end(), 4, 0);
        return c.size() - 4;
    }
    inline void patch_rel32(std::vector<std::uint8_t>& c, std::size_t at, std::size_t dest)
    {
        const std::int32_t rel = (std::int32_t)((std::ptrdiff_t)dest - (std::ptrdiff_t)(at + 4));
        std::memcpy(c.data() + at, &rel, 4);
    }

    inline bool mark_cfg(std::uintptr_t t)
    {
        static FARPROC proc = nullptr;
        if (!proc)
        {
            const char* mods[] = {
                "kernelbase.dll", "kernel32.dll",
                "api-ms-win-core-memory-l1-1-3.dll"
            };
            for (auto* m : mods)
            {
                HMODULE h = GetModuleHandleA(m);
                if (!h)
                    h = LoadLibraryA(m);
                if (!h)
                    continue;
                proc = GetProcAddress(h, "SetProcessValidCallTargets");
                if (proc)
                    break;
            }
        }
        if (!proc || !live())
            return false;
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        const std::size_t page = si.dwPageSize ? (std::size_t)si.dwPageSize : 0x1000u;
        struct Info
        {
            ULONG_PTR Offset;
            ULONG Flags;
        } info{};
        info.Offset = t & (page - 1);
        info.Flags = CFG_CALL_TARGET_VALID;
        using Fn = BOOL(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, void*);
        return ((Fn)proc)(
            g_proc->raw(),
            (void*)(t & ~((std::uintptr_t)page - 1)),
            page, 1, &info) != 0;
    }

    inline bool write_exec(std::uintptr_t at, const void* data, std::size_t n)
    {
        if (!addr_ok(at) || !data || !n)
            return false;
        DWORD old = 0;
        const bool changed = g_proc->protect_remote(at, n, PAGE_EXECUTE_READWRITE, &old);
        const bool ok = g_proc->write_buf(at, data, n);
        if (changed)
            g_proc->protect_remote(at, n, old, nullptr);
        return ok;
    }

    inline bool exec_prot(DWORD p)
    {
        const DWORD b = p & 0xFF;
        return b == PAGE_EXECUTE || b == PAGE_EXECUTE_READ ||
            b == PAGE_EXECUTE_READWRITE || b == PAGE_EXECUTE_WRITECOPY;
    }

    inline std::uintptr_t find_dll_cave(std::size_t need, std::uintptr_t ignore)
    {
        static const wchar_t* pref[] = {
            L"winsta.dll", L"win32u.dll", L"uxtheme.dll", L"dwmapi.dll",
            L"msctf.dll", L"TextInputFramework.dll", L"CoreMessaging.dll",
            L"user32.dll", L"imm32.dll", L"gdi32.dll", L"ole32.dll", L"combase.dll",
        };
        for (auto* name : pref)
        {
            const std::uintptr_t mb = module_by_name(name);
            if (!mb)
                continue;
            IMAGE_DOS_HEADER dos{};
            if (!rd(mb, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE)
                continue;
            IMAGE_NT_HEADERS64 nt{};
            if (!rd(mb + (std::uintptr_t)dos.e_lfanew, &nt, sizeof(nt))
                || nt.Signature != IMAGE_NT_SIGNATURE)
                continue;
            const std::size_t ms = nt.OptionalHeader.SizeOfImage;
            if (ms < 0x2000)
                continue;
            std::uintptr_t addr = mb + 0x1000;
            const std::uintptr_t to = mb + ms;
            MEMORY_BASIC_INFORMATION mbi{};
            while (addr < to &&
                VirtualQueryEx(g_proc->raw(), (void*)addr, &mbi, sizeof(mbi)))
            {
                const auto rb = (std::uintptr_t)mbi.BaseAddress;
                const auto rs = (std::size_t)mbi.RegionSize;
                const std::uintptr_t next = rb + rs;
                if (next <= addr)
                    break;
                const bool usable = mbi.State == MEM_COMMIT
                    && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
                    && exec_prot(mbi.Protect);
                if (usable && rs >= need)
                {
                    std::vector<std::uint8_t> buf(rs);
                    if (rd(rb, buf.data(), rs))
                    {
                        std::size_t run = 0;
                        for (std::size_t i = 0; i < rs; ++i)
                        {
                            if (buf[i] == 0x00 || buf[i] == 0xCC)
                                ++run;
                            else
                                run = 0;
                            if (run >= need)
                            {
                                const std::uintptr_t start = rb + i + 1 - run;
                                const std::uintptr_t aligned = (start + 0x0F) & ~std::uintptr_t(0x0F);
                                if (ignore && aligned == ignore)
                                    continue;
                                if (aligned + need > rb + rs)
                                    continue;
                                auto hit = [&](std::uintptr_t p, std::size_t n)
                                {
                                    return p && aligned < p + n && p < aligned + need;
                                };
                                if (hit(g.cave, g.cave_n))
                                    continue;
                                return aligned;
                            }
                        }
                    }
                }
                addr = next;
            }
        }
        return 0;
    }

    inline void fail(const char* why)
    {
        if (g_why == why)
            return;
        g_why = why;
        log_line("nativechams: %s", why);
    }

    inline bool compile(const char* entry, const char* model, std::vector<std::uint8_t>& out)
    {
        ID3DBlob* blob = nullptr;
        ID3DBlob* err = nullptr;
        const HRESULT hr = D3DCompile(
            k_hlsl, sizeof(k_hlsl) - 1, nullptr, nullptr, nullptr,
            entry, model, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &blob, &err);
        if (FAILED(hr) || !blob)
        {
            if (err)
            {
                log_line("nativechams compile: %s", (const char*)err->GetBufferPointer());
                err->Release();
            }
            return false;
        }
        if (err)
            err->Release();
        const auto* p = (const std::uint8_t*)blob->GetBufferPointer();
        out.assign(p, p + blob->GetBufferSize());
        blob->Release();
        return !out.empty();
    }

    inline std::vector<std::uint8_t> make_present_thunk(std::uintptr_t state, std::uintptr_t create)
    {
        std::vector<std::uint8_t> c;
        c.insert(c.end(), { 0x51, 0x52, 0x41, 0x50, 0x41, 0x51 });
        c.insert(c.end(), { 0x48, 0x83, 0xEC, 0x28 });
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0xFF, 0x80, 0xA8, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x83, 0xB8, 0xB4, 0x00, 0x00, 0x00, 0x00 });
        const std::size_t j_busy = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0xC7, 0x80, 0xB4, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x83, 0x78, 0x04, 0x00 });
        const std::size_t j_nocmd = append_jcc32(c, 0x04);
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, create);
        append_call_rax(c);
        const std::size_t nocmd = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0xC7, 0x80, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
        const std::size_t out = c.size();
        c.insert(c.end(), { 0x48, 0x83, 0xC4, 0x28 });
        c.insert(c.end(), { 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59 });
        c.insert(c.end(), { 0x49, 0xBA });
        append_u64(c, state);
        c.insert(c.end(), { 0x4D, 0x8B, 0x5A, 0x20 });
        c.insert(c.end(), { 0x41, 0xFF, 0xE3 });
        patch_rel32(c, j_busy, out);
        patch_rel32(c, j_nocmd, nocmd);
        return c;
    }

    inline std::vector<std::uint8_t> make_create_stub(std::uintptr_t state)
    {
        std::vector<std::uint8_t> c;
        c.insert(c.end(), { 0x48, 0x83, 0xEC, 0x48 });
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x8B, 0x48, 0x04 });
        c.insert(c.end(), { 0x85, 0xC9 });
        const std::size_t j_none = append_jcc32(c, 0x04);
        c.insert(c.end(), { 0x48, 0xC7, 0x80, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x83, 0xF9, 0x01 });
        const std::size_t j_nvs = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8B, 0x90, 0x80, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8B, 0x80, 0x88, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4D, 0x31, 0xC9 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x90, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x89, 0x54, 0x24, 0x20 });
        c.insert(c.end(), { 0xFF, 0x90, 0x20, 0x01, 0x00, 0x00 });
        c.insert(c.end(), { 0xE9, 0, 0, 0, 0 });
        const std::size_t j_fin0 = c.size() - 4;
        const std::size_t nvs = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x83, 0xF9, 0x02 });
        const std::size_t j_nps = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8B, 0x90, 0x80, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8B, 0x80, 0x88, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4D, 0x31, 0xC9 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x90, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x89, 0x54, 0x24, 0x20 });
        c.insert(c.end(), { 0xFF, 0x90, 0x28, 0x01, 0x00, 0x00 });
        c.insert(c.end(), { 0xE9, 0, 0, 0, 0 });
        const std::size_t j_fin1 = c.size() - 4;
        const std::size_t nps = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x83, 0xF9, 0x03 });
        const std::size_t j_nil = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8D, 0x90, 0x10, 0x03, 0x00, 0x00 });
        c.insert(c.end(), { 0x44, 0x8B, 0x80, 0xBC, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8B, 0x88, 0x80, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x48, 0x8B, 0x80, 0x88, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x48, 0x89, 0x44, 0x24, 0x20 });
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x4C, 0x8D, 0x90, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x89, 0x54, 0x24, 0x28 });
        c.insert(c.end(), { 0xFF, 0x90, 0x30, 0x01, 0x00, 0x00 });
        c.insert(c.end(), { 0xE9, 0, 0, 0, 0 });
        const std::size_t j_fin2 = c.size() - 4;
        const std::size_t nil_ = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x83, 0xF9, 0x04 });
        const std::size_t j_nbuf = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8D, 0x90, 0x00, 0x02, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8B, 0x80, 0x98, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4D, 0x85, 0xC0 });
        c.insert(c.end(), { 0x74, 0x24 });
        c.insert(c.end(), { 0x4C, 0x89, 0x80, 0xD8, 0x03, 0x00, 0x00 });
        c.insert(c.end(), { 0x48, 0xC7, 0x80, 0xE0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x48, 0xC7, 0x80, 0xE8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x80, 0xD8, 0x03, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x88, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0xFF, 0x90, 0x38, 0x01, 0x00, 0x00 });
        c.insert(c.end(), { 0xE9, 0, 0, 0, 0 });
        const std::size_t j_fin3 = c.size() - 4;
        const std::size_t nbuf = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x83, 0xF9, 0x05 });
        const std::size_t j_nblend = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8D, 0x90, 0x00, 0x02, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x80, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00 });
        c.insert(c.end(), { 0xE9, 0, 0, 0, 0 });
        const std::size_t j_fin4 = c.size() - 4;
        const std::size_t nblend = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x83, 0xF9, 0x06 });
        const std::size_t j_nrast = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8D, 0x90, 0x00, 0x02, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x80, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0xFF, 0x90, 0x48, 0x01, 0x00, 0x00 });
        c.insert(c.end(), { 0xE9, 0, 0, 0, 0 });
        const std::size_t j_fin5 = c.size() - 4;
        const std::size_t nrast = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0x83, 0xF9, 0x07 });
        const std::size_t j_ndss = append_jcc32(c, 0x05);
        c.insert(c.end(), { 0x48, 0x8B, 0x48, 0x10 });
        c.insert(c.end(), { 0x48, 0x8D, 0x90, 0x00, 0x02, 0x00, 0x00 });
        c.insert(c.end(), { 0x4C, 0x8D, 0x80, 0x90, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0xFF, 0x90, 0x50, 0x01, 0x00, 0x00 });
        const std::size_t fin = c.size();
        c.insert(c.end(), { 0x48, 0xB8 });
        append_u64(c, state);
        c.insert(c.end(), { 0xC7, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00 });
        c.insert(c.end(), { 0xFF, 0x80, 0xB0, 0x00, 0x00, 0x00 });
        const std::size_t none = c.size();
        c.insert(c.end(), { 0x48, 0x83, 0xC4, 0x48 });
        c.push_back(0xC3);
        patch_rel32(c, j_none, none);
        patch_rel32(c, j_nvs, nvs);
        patch_rel32(c, j_nps, nps);
        patch_rel32(c, j_nil, nil_);
        patch_rel32(c, j_nbuf, nbuf);
        patch_rel32(c, j_nblend, nblend);
        patch_rel32(c, j_nrast, nrast);
        patch_rel32(c, j_ndss, fin);
        patch_rel32(c, j_fin0, fin);
        patch_rel32(c, j_fin1, fin);
        patch_rel32(c, j_fin2, fin);
        patch_rel32(c, j_fin3, fin);
        patch_rel32(c, j_fin4, fin);
        patch_rel32(c, j_fin5, fin);
        return c;
    }

    inline std::vector<std::uint8_t> make_draw_thunk(std::uintptr_t state)
    {
        std::vector<std::uint8_t> c;
        auto bytes = [&](std::initializer_list<std::uint8_t> v) { c.insert(c.end(), v); };
        auto disp32 = [&](std::uint32_t v)
        {
            const auto* b = (const std::uint8_t*)&v;
            c.insert(c.end(), b, b + 4);
        };
        auto ld = [&](std::uint8_t modrm, std::uint32_t off)
        {
            bytes({ 0x49, 0x8B, modrm });
            disp32(off);
        };
        auto ld_rax = [&](std::uint32_t off) { ld(0x86, off); };
        auto ld_rdx = [&](std::uint32_t off) { ld(0x96, off); };
        auto ctx = [&] { bytes({ 0x49, 0x8B, 0x4E, 0x18 }); };
        auto call_state = [&](std::uint32_t fn)
        {
            ld_rax(fn);
            append_call_rax(c);
        };
        auto jmp32 = [&]
        {
            c.push_back(0xE9);
            c.insert(c.end(), 4, 0);
            return c.size() - 4;
        };

        bytes({ 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56 });
        bytes({ 0x48, 0x81, 0xEC, 0x98, 0x00, 0x00, 0x00 });
        bytes({ 0x48, 0x89, 0xCE });
        bytes({ 0x48, 0x89, 0xD3 });
        bytes({ 0x4D, 0x89, 0xC4 });
        bytes({ 0x4D, 0x89, 0xCD });
        bytes({ 0x49, 0xBE });
        append_u64(c, state);
        bytes({ 0x48, 0xC7, 0x44, 0x24, 0x40, 0x00, 0x00, 0x00, 0x00 });
        bytes({ 0x48, 0xC7, 0x44, 0x24, 0x48, 0x00, 0x00, 0x00, 0x00 });
        bytes({ 0xC7, 0x44, 0x24, 0x50, 0x00, 0x00, 0x00, 0x00 });
        bytes({ 0x48, 0xC7, 0x44, 0x24, 0x58, 0x00, 0x00, 0x00, 0x00 });
        bytes({ 0x48, 0xC7, 0x44, 0x24, 0x60, 0x00, 0x00, 0x00, 0x00 });

        std::vector<std::size_t> to_call;
        bytes({ 0x49, 0x83, 0x7E, 0x70, 0x00 });
        to_call.push_back(append_jcc32(c, 0x05));
        bytes({ 0x41, 0x83, 0x3E, 0x00 });
        to_call.push_back(append_jcc32(c, 0x04));
        bytes({ 0x41, 0x83, 0x7E, 0x08, 0x00 });
        to_call.push_back(append_jcc32(c, 0x04));
        bytes({ 0x49, 0x83, 0x7E, 0x38, 0x00 });
        to_call.push_back(append_jcc32(c, 0x04));
        bytes({ 0x41, 0x8B, 0x4E, 0x0C });
        bytes({ 0x85, 0xC9 });
        to_call.push_back(append_jcc32(c, 0x04));
        ld_rdx(offsetof(RemoteState, geoms));
        bytes({ 0x48, 0x85, 0xD2 });
        to_call.push_back(append_jcc32(c, 0x04));
        bytes({ 0x31, 0xC0 });
        const std::size_t scan = c.size();
        bytes({ 0x48, 0x39, 0x1A });
        const std::size_t j_found = append_jcc32(c, 0x04);
        bytes({ 0x48, 0x83, 0xC2, 0x38 });
        bytes({ 0xFF, 0xC0 });
        bytes({ 0x39, 0xC8 });
        patch_rel32(c, append_jcc32(c, 0x02), scan);
        to_call.push_back(jmp32());

        patch_rel32(c, j_found, c.size());
        bytes({ 0x49, 0xC7, 0x46, 0x70, 0x01, 0x00, 0x00, 0x00 });
        bytes({ 0x48, 0x89, 0x54, 0x24, 0x58 });

        ctx();
        bytes({ 0x48, 0x8D, 0x54, 0x24, 0x40 });
        bytes({ 0x45, 0x31, 0xC0, 0x45, 0x31, 0xC9 });
        call_state(offsetof(RemoteState, fn_psget));

        ctx();
        bytes({ 0x48, 0x8D, 0x54, 0x24, 0x48 });
        bytes({ 0x4C, 0x8D, 0x44, 0x24, 0x50 });
        call_state(offsetof(RemoteState, fn_dssget));

        ctx();
        bytes({ 0x48, 0x8D, 0x54, 0x24, 0x60 });
        bytes({ 0x4C, 0x8D, 0x44, 0x24, 0x68 });
        bytes({ 0x4C, 0x8D, 0x4C, 0x24, 0x78 });
        call_state(offsetof(RemoteState, fn_blendget));

        ctx();
        bytes({ 0x49, 0x8B, 0x56, 0x48 });
        bytes({ 0x45, 0x31, 0xC0, 0x45, 0x31, 0xC9 });
        bytes({ 0x48, 0x8B, 0x44, 0x24, 0x58 });
        bytes({ 0x48, 0x83, 0xC0, 0x08 });
        bytes({ 0x48, 0x89, 0x44, 0x24, 0x20 });
        bytes({ 0x48, 0xC7, 0x44, 0x24, 0x28, 0x00, 0x00, 0x00, 0x00 });
        bytes({ 0x48, 0xC7, 0x44, 0x24, 0x30, 0x00, 0x00, 0x00, 0x00 });
        call_state(offsetof(RemoteState, fn_update));

        ctx();
        bytes({ 0x49, 0x8B, 0x56, 0x38 });
        bytes({ 0x45, 0x31, 0xC0, 0x45, 0x31, 0xC9 });
        call_state(offsetof(RemoteState, fn_psset));

        ctx();
        bytes({ 0xBA, 0x0D, 0x00, 0x00, 0x00 });
        bytes({ 0x41, 0xB8, 0x01, 0x00, 0x00, 0x00 });
        bytes({ 0x4D, 0x8D, 0x4E, 0x48 });
        call_state(offsetof(RemoteState, fn_pscb));

        ctx();
        bytes({ 0x49, 0x8B, 0x56, 0x68 });
        bytes({ 0x45, 0x31, 0xC0 });
        call_state(offsetof(RemoteState, fn_dssset));

        for (std::uint8_t i = 0; i < 4; ++i)
        {
            bytes({ 0xC7, 0x84, 0x24 });
            disp32(0x80u + i * 4u);
            bytes({ 0x00, 0x00, 0x80, 0x3F });
        }
        ctx();
        bytes({ 0x49, 0x8B, 0x56, 0x58 });
        bytes({ 0x4C, 0x8D, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00 });
        bytes({ 0x41, 0xB9, 0xFF, 0xFF, 0xFF, 0xFF });
        call_state(offsetof(RemoteState, fn_blendset));

        bytes({ 0x41, 0xFF, 0x86 });
        disp32(offsetof(RemoteState, draws));

        const std::size_t call_site = c.size();
        for (auto at : to_call)
            patch_rel32(c, at, call_site);

        for (std::uint8_t i = 0; i < 4; ++i)
        {
            bytes({ 0x48, 0x8B, 0x84, 0x24 });
            disp32(0xF0u + i * 8u);
            bytes({ 0x48, 0x89, 0x44, 0x24, (std::uint8_t)(0x20 + i * 8) });
        }
        bytes({ 0x48, 0x89, 0xF1 });
        bytes({ 0x48, 0x89, 0xDA });
        bytes({ 0x4D, 0x89, 0xE0 });
        bytes({ 0x4D, 0x89, 0xE9 });
        bytes({ 0x49, 0x8B, 0x46, 0x28 });
        append_call_rax(c);
        bytes({ 0x48, 0x89, 0xC7 });

        bytes({ 0x48, 0x83, 0x7C, 0x24, 0x58, 0x00 });
        const std::size_t j_out = append_jcc32(c, 0x04);

        ctx();
        bytes({ 0x48, 0x8B, 0x54, 0x24, 0x40 });
        bytes({ 0x45, 0x31, 0xC0, 0x45, 0x31, 0xC9 });
        call_state(offsetof(RemoteState, fn_psset));

        ctx();
        bytes({ 0x48, 0x8B, 0x54, 0x24, 0x48 });
        bytes({ 0x44, 0x8B, 0x44, 0x24, 0x50 });
        call_state(offsetof(RemoteState, fn_dssset));

        ctx();
        bytes({ 0x48, 0x8B, 0x54, 0x24, 0x60 });
        bytes({ 0x4C, 0x8D, 0x44, 0x24, 0x68 });
        bytes({ 0x44, 0x8B, 0x4C, 0x24, 0x78 });
        call_state(offsetof(RemoteState, fn_blendset));

        for (std::uint8_t off : { 0x40, 0x48, 0x60 })
        {
            bytes({ 0x48, 0x8B, 0x4C, 0x24, off });
            bytes({ 0x48, 0x85, 0xC9 });
            bytes({ 0x74, 0x06 });
            bytes({ 0x48, 0x8B, 0x01 });
            bytes({ 0xFF, 0x50, 0x10 });
        }

        bytes({ 0x49, 0xC7, 0x46, 0x70, 0x00, 0x00, 0x00, 0x00 });
        patch_rel32(c, j_out, c.size());
        bytes({ 0x48, 0x89, 0xF8 });
        bytes({ 0x48, 0x81, 0xC4, 0x98, 0x00, 0x00, 0x00 });
        bytes({ 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x5F, 0x5E, 0x5B });
        c.push_back(0xC3);
        return c;
    }

    inline bool add_patch(std::uintptr_t at, std::uintptr_t thunk, std::uintptr_t* orig)
    {
        if (!addr_ok(at) || !addr_ok(thunk) || g.npatch >= 16)
            return false;
        const std::uint64_t old = rd64(at);
        if (!old || old == thunk)
            return old == thunk;
        for (int i = 0; i < g.npatch; ++i)
        {
            if (g.patches[i].at == at)
                return true;
        }
        DWORD prot = 0;
        if (!g_proc->protect_remote(at, 8, PAGE_READWRITE, &prot))
            return false;
        g_proc->write<std::uint64_t>(at, thunk);
        g_proc->protect_remote(at, 8, prot, nullptr);
        g.patches[g.npatch].at = at;
        g.patches[g.npatch].old = old;
        ++g.npatch;
        if (orig && !*orig)
            *orig = (std::uintptr_t)old;
        return true;
    }

    inline void unpatch()
    {
        for (int i = 0; i < g.npatch; ++i)
        {
            if (!g.patches[i].at)
                continue;
            DWORD prot = 0;
            if (g_proc->protect_remote(g.patches[i].at, 8, PAGE_READWRITE, &prot))
            {
                g_proc->write<std::uint64_t>(g.patches[i].at, g.patches[i].old);
                g_proc->protect_remote(g.patches[i].at, 8, prot, nullptr);
            }
            g.patches[i] = {};
        }
        g.npatch = 0;
    }

    inline int vt_len(std::uintptr_t vt)
    {
        std::uint64_t v[k_vt_cap]{};
        if (!rd(vt, v, sizeof(v)))
            return 0;
        int n = 0;
        for (; n < k_vt_cap; ++n)
        {
            const auto fn = (std::uintptr_t)v[n];
            if (!addr_ok(fn))
                break;
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQueryEx(g_proc->raw(), (void*)fn, &mbi, sizeof(mbi))
                || mbi.State != MEM_COMMIT || !exec_prot(mbi.Protect))
                break;
        }
        return n;
    }

    inline int clone_vt(std::uintptr_t src, std::uintptr_t dst, int slot, std::uintptr_t thunk, std::uintptr_t* orig)
    {
        const int n = vt_len(src);
        if (n <= slot)
            return 0;
        const std::size_t bytes = (std::size_t)(n + 1) * 8;
        std::vector<std::uint64_t> v((std::size_t)n + 1);
        if (!rd(src - 8, v.data(), bytes))
            return 0;
        *orig = (std::uintptr_t)v[(std::size_t)slot + 1];
        if (!addr_ok(*orig))
            return 0;
        v[(std::size_t)slot + 1] = thunk;
        if (!wr(dst - 8, v.data(), bytes))
            return 0;
        return n;
    }

    inline bool patch_vtables()
    {
        if (!g.present_thunk || !g.draw_thunk || !g.dc || !g.swap || !g.vt_mem)
            return false;
        const std::uintptr_t dc_vt = (std::uintptr_t)rd64(g.dc);
        const std::uintptr_t swap_vt = (std::uintptr_t)rd64(g.swap);
        if (!addr_ok(dc_vt) || !addr_ok(swap_vt) || dc_vt == g.dc_vt || swap_vt == g.swap_vt)
            return false;
        g.swap_vt = g.vt_mem + 8;
        g.dc_vt = g.vt_mem + 0x1000 + 8;
        g.nswvt = clone_vt(swap_vt, g.swap_vt, k_slot_present, g.present_thunk, &g.orig_present);
        g.ndcvt = clone_vt(dc_vt, g.dc_vt, k_slot_dc_draw, g.draw_thunk, &g.orig_draw);
        if (!g.nswvt || !g.ndcvt)
            return false;
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, orig_present), g.orig_present);
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, orig_draw), g.orig_draw);
        if (!add_patch(g.swap, g.swap_vt, nullptr))
            return false;
        return add_patch(g.dc, g.dc_vt, nullptr);
    }

    inline bool setup_meta()
    {
        if (!g.device || !g.state || !g.ctx || !g.dc)
            return false;
        std::vector<std::uint8_t> ps;
        if (!compile("ps_main", "ps_5_0", ps))
            return false;
        if (g.ps_dxbc)
        {
            g_proc->free_remote(g.ps_dxbc);
            g.ps_dxbc = 0;
        }
        g.ps_dxbc = g_proc->alloc_remote(ps.size() + 16, PAGE_READWRITE);
        if (!g.ps_dxbc)
            return false;
        if (!wr(g.ps_dxbc, ps.data(), ps.size()))
            return false;
        g.ps_len = ps.size();

        if (!g.geoms)
            g.geoms = g_proc->alloc_remote((std::size_t)k_max_targets * sizeof(Target) * 2 + 16, PAGE_READWRITE);
        if (!g.geoms)
            return false;

        RemoteState st{};
        rd(g.state, &st, sizeof(st));
        st.device = g.device;
        st.ctx = g.ctx;
        st.dc = g.dc;
        st.geoms = 0;
        st.ngeom = 0;
        st.fn_create_ps = vt_fn(g.device, k_slot_create_ps);
        st.fn_create_buf = vt_fn(g.device, k_slot_create_buf);
        st.fn_create_dss = vt_fn(g.device, k_slot_create_dss);
        st.fn_psset = vt_fn(g.ctx, k_slot_psset);
        st.fn_psget = vt_fn(g.ctx, k_slot_psget);
        st.fn_pscb = vt_fn(g.ctx, k_slot_pscb);
        st.fn_dssset = vt_fn(g.ctx, k_slot_dssset);
        st.fn_dssget = vt_fn(g.ctx, k_slot_dssget);
        st.fn_blendset = vt_fn(g.ctx, k_slot_blendset);
        st.fn_blendget = vt_fn(g.ctx, k_slot_blendget);
        st.fn_create_blend = vt_fn(g.device, k_slot_create_blend);
        st.fn_update = vt_fn(g.ctx, k_slot_update);
        if (!addr_ok((std::uintptr_t)st.fn_create_ps) || !addr_ok((std::uintptr_t)st.fn_create_dss) ||
            !addr_ok((std::uintptr_t)st.fn_create_buf) || !addr_ok((std::uintptr_t)st.fn_psset) ||
            !addr_ok((std::uintptr_t)st.fn_psget) || !addr_ok((std::uintptr_t)st.fn_pscb) ||
            !addr_ok((std::uintptr_t)st.fn_dssset) || !addr_ok((std::uintptr_t)st.fn_dssget) ||
            !addr_ok((std::uintptr_t)st.fn_blendset) || !addr_ok((std::uintptr_t)st.fn_blendget) ||
            !addr_ok((std::uintptr_t)st.fn_create_blend) || !addr_ok((std::uintptr_t)st.fn_update))
            return false;
        st.busy = 0;
        st.cmd = 0;
        st.ready = 0;
        wr(g.state, &st, sizeof(st));
        return true;
    }

    inline void issue_cmd(std::uint32_t cmd, std::uintptr_t code, std::size_t len, std::uintptr_t init)
    {
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, out_res), 0);
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, bytecode), code);
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, bytecode_len), len);
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, init_sys), init);
        g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, cmd), cmd);
        g.cmd_wait = true;
    }

    inline void issue_desc_cmd(std::uint32_t cmd, const void* desc, std::size_t n, std::uintptr_t init)
    {
        wr(g.state + 0x200, desc, n);
        issue_cmd(cmd, 0, 0, init);
    }

    inline std::uint64_t take_out()
    {
        const std::uint32_t cmd = g_proc->read<std::uint32_t>(g.state + offsetof(RemoteState, cmd));
        if (cmd)
            return 0;
        g.cmd_wait = false;
        return rd64(g.state + offsetof(RemoteState, out_res));
    }

    inline void store_res(std::uint64_t off, std::uint64_t v)
    {
        g_proc->write<std::uint64_t>(g.state + off, v);
    }

    inline bool pump_creates()
    {
        if (!g.state)
            return false;
        if (g.cmd_wait)
        {
            const std::uint64_t out = take_out();
            if (g.cmd_wait)
                return false;
            if (!out)
            {
                fail("create");
                g.stage = 0;
                return false;
            }
            switch (g.stage)
            {
            case 1: store_res(offsetof(RemoteState, ps), out); g.stage = 2; break;
            case 2:
                store_res(offsetof(RemoteState, dss), out);
                g.stage = 3;
                break;
            case 3: store_res(offsetof(RemoteState, cb), out); g.stage = 4; break;
            case 4:
                store_res(offsetof(RemoteState, blend), out);
                g.stage = 11;
                g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, ready), 1);
                break;
            default:
                break;
            }
        }
        if (g.cmd_wait)
            return g.stage >= 11;

        if (g.stage == 0)
        {
            g.stage = 1;
            issue_cmd(k_cmd_ps, g.ps_dxbc, g.ps_len, 0);
            return false;
        }
        if (g.stage == 2)
        {
            D3D11_DEPTH_STENCIL_DESC d{};
            d.DepthEnable = TRUE;
            d.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            d.DepthFunc = g.depth_rev ? D3D11_COMPARISON_GREATER_EQUAL : D3D11_COMPARISON_LESS_EQUAL;
            d.StencilEnable = FALSE;
            issue_desc_cmd(k_cmd_dss, &d, sizeof(d), 0);
            return false;
        }
        if (g.stage == 3)
        {
            D3D11_BUFFER_DESC d{};
            d.ByteWidth = 48;
            d.Usage = D3D11_USAGE_DEFAULT;
            d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            issue_desc_cmd(k_cmd_buf, &d, sizeof(d), 0);
            return false;
        }
        if (g.stage == 4)
        {
            D3D11_BLEND_DESC d{};
            auto& rt = d.RenderTarget[0];
            rt.BlendEnable = TRUE;
            rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
            rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            rt.BlendOp = D3D11_BLEND_OP_ADD;
            rt.SrcBlendAlpha = D3D11_BLEND_ONE;
            rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            issue_desc_cmd(k_cmd_blend, &d, sizeof(d), 0);
            return false;
        }
        return g.stage >= 11;
    }

    inline bool cluster_ready()
    {
        return pending::FastClusterBindingSubobject && pending::FastClusterEntityBegin
            && pending::FastClusterEntityEnd && pending::FastClusterBindingOwner
            && pending::FastClusterEntityMaterialPtr && pending::FastClusterEntityDecalMaterialPtr;
    }

    struct snapshot_entry
    {
        std::uintptr_t player = 0;
        std::uintptr_t character = 0;
        float hp = 0.0f;
        float max_hp = 0.0f;
    };

    inline bool read_targets(std::vector<snapshot_entry>& out)
    {
        out.clear();
        if (!g_ctx.ready || !live())
            return false;
        const std::uintptr_t local = g_ctx.local_player.addr;
        std::uintptr_t local_team = 0;
        if (local)
            local_team = g_proc->read<std::uintptr_t>(local + Offsets::Player::Team);
        for (const auto& p : g_ctx.player_list())
        {
            if (!p.addr)
                continue;
            const bool is_local = local && p.addr == local;
            if (!is_local && settings::vis_team_check && local_team)
            {
                const std::uintptr_t team = g_proc->read<std::uintptr_t>(p.addr + Offsets::Player::Team);
                if (team && team == local_team)
                    continue;
            }
            const node ch = g_ctx.get_character(p);
            if (!ch.addr)
                continue;
            snapshot_entry e{};
            e.player = p.addr;
            e.character = ch.addr;
            if (!find_live_humanoid(ch, &e.hp, &e.max_hp))
                continue;
            if (e.max_hp > 0.0f && e.hp <= 0.0f)
                continue;
            out.push_back(e);
        }
        return !out.empty();
    }

    inline void collect_parts(std::uintptr_t character, std::vector<std::uintptr_t>& out)
    {
        out.clear();
        const node ch{ character };
        if (!ch)
            return;
        for (const auto& c : ch.children())
        {
            if (!c.addr)
                continue;
            const std::string cls = c.class_name();
            if (cls != "Part" && cls != "MeshPart" && cls != "WedgePart"
                && cls != "CornerWedgePart" && cls != "UnionOperation")
                continue;
            if (c.name() == "HumanoidRootPart")
                continue;
            out.push_back(c.addr);
        }
    }

    inline bool is_vt(std::uint64_t obj, std::uint64_t rva)
    {
        if (!addr_ok((std::uintptr_t)obj) || (obj & 7) || !g.module)
            return false;
        return rd64(obj) == g.module + rva;
    }

    inline void add_geom(std::uint64_t q, std::vector<std::uint64_t>& out)
    {
        if (!is_vt(q, Offsets::GeometryD3D11::VTableRva))
            return;
        if (std::find(out.begin(), out.end(), q) == out.end())
            out.push_back(q);
    }

    inline std::uint64_t resolve_cluster(std::uint64_t node_addr)
    {
        if (is_vt(node_addr, Offsets::FastClusterBinding::VTableRva))
        {
            ++g.nbind;
            node_addr = rd64(node_addr + pending::FastClusterBindingOwner)
                + pending::FastClusterBindingSubobject;
        }
        if (is_vt(node_addr, Offsets::FastCluster::VTableRva))
            node_addr += pending::FastClusterBindingSubobject;
        return is_vt(node_addr, Offsets::FastCluster::VTableRvaSub) ? node_addr : 0;
    }

    inline void collect_cluster(std::uint64_t cluster, std::vector<std::uint64_t>& out)
    {
        if (g.nent >= k_ent_budget)
            return;
        const std::uint64_t base = cluster - pending::FastClusterBindingSubobject;
        const std::uint64_t begin = rd64(base + pending::FastClusterEntityBegin);
        const std::uint64_t end = rd64(base + pending::FastClusterEntityEnd);
        if (!addr_ok((std::uintptr_t)begin) || end <= begin || ((end - begin) & 7))
            return;
        if ((end - begin) > k_max_ents * 8ull)
        {
            ++g.ndrop;
            return;
        }
        const int n = (int)((end - begin) / 8);
        g.nent += n;
        std::uint64_t ents[k_max_ents]{};
        if (!rd(begin, ents, (std::size_t)n * 8))
            return;
        for (int i = 0; i < n; ++i)
        {
            if (!addr_ok((std::uintptr_t)ents[i]) || (ents[i] & 7))
                continue;
            std::uint64_t hdr[10]{};
            if (!rd(ents[i], hdr, sizeof(hdr)))
                continue;
            if (hdr[0] != g.module + Offsets::FastClusterEntity::VTableRva)
                continue;
            const std::size_t mat = pending::FastClusterEntityMaterialPtr / 8;
            const std::size_t decal = pending::FastClusterEntityDecalMaterialPtr / 8;
            if (mat < 10)
                add_geom(hdr[mat], out);
            if (decal < 10)
                add_geom(hdr[decal], out);
        }
    }

    inline int shader_mode()
    {
        int mode = settings::chams_style;
        if (mode < 0)
            mode = 0;
        if (mode >= ShaderNameCount())
            mode = ShaderNameCount() - 1;
        return mode;
    }

    inline void fill_geoms()
    {
        if (!g.state || !g.geoms || g.stage < 11)
            return;
        auto off = [&]
        {
            g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, enabled), 0);
            g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, ngeom), 0);
            g.ntarget = 0;
        };
        if (!settings::chams_enabled)
        {
            off();
            return;
        }
        std::vector<snapshot_entry> snap;
        if (!read_targets(snap))
        {
            off();
            return;
        }

        static const auto t0 = std::chrono::steady_clock::now();
        const float now_s = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - t0).count();
        const float dscale = 0.052f;
        const float dbias = g.depth_rev ? 1.f - dscale : 0.f;
        const int mode = shader_mode();
        const float glow_strength = settings::chams_outline ? 1.0f : 0.0f;

        std::vector<Target> targets;
        targets.reserve(64);
        std::vector<std::uint64_t> seen;
        seen.reserve(32);
        std::vector<std::uint64_t> taken;
        taken.reserve(64);
        std::vector<std::uintptr_t> parts;
        g.nnode = 0;
        g.nent = 0;
        g.ndrop = 0;
        g.nbind = 0;
        g_nunk = 0;
        g_nfc = 0;
        g_npick = 0;
        g_unk_vt = 0;

        for (const auto& e : snap)
        {
            ++g_npick;
            collect_parts(e.character, parts);
            for (std::uintptr_t part : parts)
            {
                const std::uint64_t cn = rd64(part + Offsets::BasePart::ClusterNode);
                if (!cn || std::find(seen.begin(), seen.end(), cn) != seen.end())
                    continue;
                seen.push_back(cn);
                if (is_vt(cn, Offsets::FastCluster::VTableRva)
                    || is_vt(cn, Offsets::FastCluster::VTableRvaSub))
                    ++g_nfc;
                else if (!is_vt(cn, Offsets::FastClusterBinding::VTableRva))
                {
                    ++g_nunk;
                    if (!g_unk_vt)
                        g_unk_vt = rd64(cn);
                }
                const std::uint64_t cluster = resolve_cluster(cn);
                if (!cluster)
                    continue;
                ++g.nnode;
                std::vector<std::uint64_t> geoms;
                collect_cluster(cluster, geoms);
                for (std::uint64_t q : geoms)
                {
                    if ((int)targets.size() >= k_max_targets)
                        break;
                    if (std::find(taken.begin(), taken.end(), q) != taken.end())
                        continue;
                    taken.push_back(q);
                    Target t{};
                    t.geom = q;
                    std::memcpy(t.color, settings::chams_fill_color, sizeof(t.color));
                    t.glow[0] = settings::chams_color[0];
                    t.glow[1] = settings::chams_color[1];
                    t.glow[2] = settings::chams_color[2];
                    t.glow[3] = glow_strength;
                    t.mode = mode;
                    t.time = now_s;
                    t.depth_scale = dscale;
                    t.depth_bias = dbias;
                    targets.push_back(t);
                }
            }
        }

        g.ntarget = (int)targets.size();
        if (targets.empty())
        {
            off();
            return;
        }
        const std::uintptr_t dst = g.geoms + (std::size_t)g.slot * k_max_targets * sizeof(Target);
        wr(dst, targets.data(), targets.size() * sizeof(Target));
        g_proc->write<std::uint64_t>(g.state + offsetof(RemoteState, geoms), dst);
        g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, ngeom), (std::uint32_t)targets.size());
        g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, enabled), 1);
        g.slot ^= 1;
    }

    inline bool install()
    {
        if (g.hooked)
            return true;
        const auto now = std::chrono::steady_clock::now();
        if (g_fail.time_since_epoch().count() != 0 &&
            now - g_fail < std::chrono::milliseconds(800))
            return false;
        const std::uintptr_t base = module_base();
        if (!base || !live())
        {
            g_fail = now;
            return false;
        }
        if (!g.state)
            g.state = g_proc->alloc_remote(0x1000, PAGE_READWRITE);
        if (!g.state)
        {
            fail("alloc");
            g_fail = now;
            return false;
        }
        if (!find_d3d())
        {
            fail("no device");
            g_fail = now;
            return false;
        }
        if (!setup_meta())
        {
            fail("gpu meta");
            g_fail = now;
            return false;
        }

        auto cr = make_create_stub(g.state);
        auto dr = make_draw_thunk(g.state);
        auto ps = make_present_thunk(g.state, 0x10000);
        if (cr.empty() || dr.empty() || ps.empty())
        {
            fail("thunk");
            g_fail = now;
            return false;
        }
        const auto align16 = [](std::size_t n) { return (n + 15u) & ~std::size_t(15); };
        const std::size_t nps = align16(ps.size());
        const std::size_t ncr = align16(cr.size());
        const std::size_t ndr = align16(dr.size());
        const std::size_t nall = nps + ncr + ndr;
        const std::uintptr_t cave = find_dll_cave(nall, 0);
        if (!cave)
        {
            fail("cave");
            g_fail = now;
            return false;
        }
        g.cave = cave;
        g.cave_n = nall;
        g.present_thunk = cave;
        g.create_stub = cave + nps;
        g.draw_thunk = cave + nps + ncr;
        if (!g.vt_mem)
            g.vt_mem = g_proc->alloc_remote(0x2000, PAGE_READWRITE);
        if (!g.vt_mem)
        {
            fail("alloc vt");
            g.cave = 0;
            g_fail = now;
            return false;
        }
        ps = make_present_thunk(g.state, g.create_stub);
        if (ps.size() > nps)
        {
            fail("thunk jmp");
            g.cave = 0;
            g_fail = now;
            return false;
        }
        if (!write_exec(g.create_stub, cr.data(), cr.size())
            || !write_exec(g.draw_thunk, dr.data(), dr.size())
            || !write_exec(g.present_thunk, ps.data(), ps.size()))
        {
            fail("write stub");
            g.cave = 0;
            g_fail = now;
            return false;
        }
        FlushInstructionCache(g_proc->raw(), (void*)g.cave, nall);
        mark_cfg(g.present_thunk);
        mark_cfg(g.create_stub);
        mark_cfg(g.draw_thunk);
        if (!patch_vtables())
        {
            fail("vt");
            unpatch();
            g.cave = 0;
            g_fail = now;
            return false;
        }
        g.module = base;
        g.hooked = true;
        g.stage = 0;
        if (!g.logged)
        {
            g.logged = true;
            log_line("nativechams on patches=%d cave=0x%llX swapvt=%d dcvt=%d swap=0x%llX dc=0x%llX",
                g.npatch, (unsigned long long)g.cave, g.nswvt, g.ndcvt,
                (unsigned long long)g.swap, (unsigned long long)g.dc);
        }
        return true;
    }

    inline void teardown()
    {
        if (g.state)
            g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, enabled), 0);
        if (!g.hooked)
            return;
        unpatch();
        g.hooked = false;
    }

    inline void Tick()
    {
        if (!live())
        {
            teardown();
            g.device = 0;
            g.ctx = 0;
            g.swap = 0;
            g.dc = 0;
            g.orig_present = 0;
            g.orig_draw = 0;
            return;
        }

        const std::uintptr_t base = module_base();
        if (g.module && base && g.module != base)
        {
            teardown();
            if (g.state) { g_proc->free_remote(g.state); g.state = 0; }
            if (g.geoms) { g_proc->free_remote(g.geoms); g.geoms = 0; }
            if (g.vt_mem) { g_proc->free_remote(g.vt_mem); g.vt_mem = 0; }
            if (g.ps_dxbc) { g_proc->free_remote(g.ps_dxbc); g.ps_dxbc = 0; }
            g = Hook{};
            g_why = nullptr;
        }

        if (!settings::chams_enabled && !g.hooked)
            return;
        if (!cluster_ready())
        {
            if (!g.warned)
            {
                g.warned = true;
                log_line("nativechams: FastCluster offsets are not filled in yet "
                    "(pending block at the top of native_chams.h) - not hooking");
            }
            return;
        }

        const bool rev = want_reverse();
        if (!g.hooked)
        {
            g.depth_rev = rev;
            if (!install())
                return;
            log_line("nativechams depth=%s", rev ? "reverse" : "standard");
        }
        if (g.stage >= 11 && g.depth_rev != rev)
        {
            g.depth_rev = rev;
            g.stage = 2;
            g_proc->write<std::uint32_t>(g.state + offsetof(RemoteState, ready), 0);
            log_line("nativechams depth=%s", rev ? "reverse" : "standard");
        }
        pump_creates();
        fill_geoms();

        static auto last_stat = std::chrono::steady_clock::time_point{};
        static int last_stage = -1;
        if (g.stage == 11 && last_stage != 11)
        {
            last_stage = 11;
            log_line("nativechams ready st=%d cre=%u", g.stage, Creates());
        }
        const auto now = std::chrono::steady_clock::now();
        if (last_stat.time_since_epoch().count() == 0 ||
            now - last_stat >= std::chrono::seconds(5))
        {
            last_stat = now;
            log_line("nativechams st=%d rdy=%u frames=%u draws=%u geom=%u node=%d ent=%d bind=%d drop=%d pick=%d unk=%d fc=%d",
                g.stage, Ready(), Entered(), Draws(), Items(), g.nnode, g.nent, g.nbind, g.ndrop,
                g_npick, g_nunk, g_nfc);
        }
    }

    inline void Stop()
    {
        teardown();
    }

    inline void Dump()
    {
        if (!g.hooked && !g.state)
            return;
        log_line("nativechams hooked=%d st=%d rdy=%u draws=%u geom=%u node=%d ent=%d bind=%d drop=%d",
            g.hooked ? 1 : 0, g.stage, Ready(), Draws(), Items(), g.nnode, g.nent, g.nbind, g.ndrop);
        if (!g.state)
            return;
        RemoteState st{};
        rd(g.state, &st, sizeof(st));
        log_line("nativechams ps=0x%llX dss=0x%llX cb=0x%llX blend=0x%llX entered=%u last_rtv=0x%llX",
            (unsigned long long)st.ps, (unsigned long long)st.dss,
            (unsigned long long)st.cb, (unsigned long long)st.blend,
            st.entered, (unsigned long long)st.last_rtv);
    }

    inline bool Hooked()
    {
        return g.hooked;
    }

    inline const char* Why()
    {
        if (g.hooked)
            return nullptr;
        return g_why ? g_why : "off";
    }

    inline std::uintptr_t Cave()
    {
        return g.cave;
    }

    inline std::size_t CaveSize()
    {
        return g.cave_n;
    }

    inline bool CaveBusy(std::uintptr_t at, std::size_t n)
    {
        if (!at || !n)
            return false;
        return g.cave && at < g.cave + g.cave_n && g.cave < at + n;
    }

    inline std::uint32_t Draws()
    {
        if (!g.state)
            return 0;
        return g_proc->read<std::uint32_t>(g.state + offsetof(RemoteState, draws));
    }

    inline std::uint32_t Items()
    {
        if (!g.state)
            return 0;
        return g_proc->read<std::uint32_t>(g.state + offsetof(RemoteState, ngeom));
    }

    inline std::uint32_t Entered()
    {
        if (!g.state)
            return 0;
        return g_proc->read<std::uint32_t>(g.state + offsetof(RemoteState, entered));
    }

    inline std::uint32_t Creates()
    {
        if (!g.state)
            return 0;
        return g_proc->read<std::uint32_t>(g.state + offsetof(RemoteState, creates));
    }

    inline std::uint32_t Stage()
    {
        return (std::uint32_t)g.stage;
    }

    inline std::uint32_t Ready()
    {
        if (!g.state)
            return 0;
        return g_proc->read<std::uint32_t>(g.state + offsetof(RemoteState, ready));
    }

    inline const char* const* ShaderNames()
    {
        static const char* k[] = {
            "Flat", "Lit", "Clay", "Glass", "Crystal", "Ice", "Prism", "Heat", "Core"
        };
        return k;
    }

    inline int ShaderNameCount()
    {
        return 9;
    }
}
