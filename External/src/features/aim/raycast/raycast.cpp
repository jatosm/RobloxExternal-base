#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "raycast.h"
#include "../core/aim.h"
#include "../predict/predict.h"
#include "../../visual/world/geo/geo.h"
#include "../pftarget/pftarget.h"
#include "../tracking/tracking.h"
#include "../health/health.h"
#include "../distance/distance.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/logger/logger.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include <TlHelp32.h>
#include <Windows.h>
#include <vector>
#include <cstring>
#include <chrono>
#include <cstddef>
#include <mutex>
#ifndef CFG_CALL_TARGET_VALID
#define CFG_CALL_TARGET_VALID 0x00000001
#endif
namespace thenwefuckin::aim::raycast
{
    namespace
    {
        constexpr std::uintptr_t desc_rva_z = Offsets::WorldRoot::RaycastBoundDesc;
        constexpr std::uintptr_t bound_fn_offset = Offsets::WorldRoot::RaycastBoundFn;
        constexpr std::size_t k_stub_bytes = 0x200;
#pragma pack(push, 4)
        struct RaycastState
        {
            std::uint32_t active = 0;
            std::uint32_t reserved = 0;
            float target_x = 0.f;
            float target_y = 0.f;
            float target_z = 0.f;
            float scale = 1.15f;
            std::uint64_t calls = 0;
        };
#pragma pack(pop)
        static_assert(offsetof(RaycastState, active) == 0x00, "active");
        static_assert(offsetof(RaycastState, target_x) == 0x08, "target");
        static_assert(offsetof(RaycastState, calls) == 0x18, "calls");
        struct Hook
        {
            std::uintptr_t thunk = 0;
            std::uintptr_t state = 0;
            std::uintptr_t originalFunction = 0;
            std::uintptr_t module_base = 0;
            bool thunk_owned = false;
            bool installed = false;
            bool active = false;
        };
        Hook g_hook{};
        bool g_wallbang = false;
        auto g_lastFail = std::chrono::steady_clock::time_point{};
        char g_status[80] = "idle: hook not requested";
        void set_status(const char* s)
        {
            if (!s || !s[0]) return;
            size_t n = strlen(s);
            if (n >= sizeof(g_status)) n = sizeof(g_status) - 1;
            memcpy(g_status, s, n);
            g_status[n] = '\0';
        }
        bool ok_addr(std::uintptr_t a)
        {
            return a >= 0x10000ull && a < 0x00007FFFFFFFFFFFull;
        }
        bool connected()
        {
            return g_proc && g_proc->connected();
        }
        std::uintptr_t ray_alloc(std::size_t n, DWORD prot)
        {
            if (!connected())
                return 0;
            return (std::uintptr_t)VirtualAllocEx(g_proc->raw(), nullptr, n, MEM_COMMIT | MEM_RESERVE, prot);
        }
        bool ray_free(std::uintptr_t a)
        {
            if (!a || !connected())
                return false;
            return VirtualFreeEx(g_proc->raw(), (LPVOID)a, 0, MEM_RELEASE) != 0;
        }
        std::uintptr_t ray_module(const wchar_t* name)
        {
            if (!connected() || !name)
                return 0;
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, g_proc->pid());
            if (snap == INVALID_HANDLE_VALUE)
                return 0;
            MODULEENTRY32W me{};
            me.dwSize = sizeof(me);
            std::uintptr_t out = 0;
            if (Module32FirstW(snap, &me))
            {
                do
                {
                    if (_wcsicmp(me.szModule, name) == 0)
                    {
                        out = (std::uintptr_t)me.modBaseAddr;
                        break;
                    }
                } while (Module32NextW(snap, &me));
            }
            CloseHandle(snap);
            return out;
        }
        bool w_mem(std::uintptr_t a, const void* d, std::size_t s)
        {
            if (!ok_addr(a) || !d || !s || !connected())
                return false;
            return g_proc->write_buf(a, d, s);
        }
        std::size_t pg()
        {
            static const std::size_t p = []
            {
                SYSTEM_INFO i{};
                GetSystemInfo(&i);
                return i.dwPageSize ? static_cast<std::size_t>(i.dwPageSize) : 0x1000u;
            }();
            return p;
        }
        DWORD query_protect(std::uintptr_t a)
        {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQueryEx(g_proc->raw(), reinterpret_cast<void*>(a), &mbi, sizeof(mbi)))
                return 0;
            return mbi.Protect;
        }
        bool is_executable_protect(DWORD p)
        {
            const DWORD x = p & 0xFF;
            return x == PAGE_EXECUTE || x == PAGE_EXECUTE_READ || x == PAGE_EXECUTE_READWRITE || x == PAGE_EXECUTE_WRITECOPY;
        }
        bool protect_remote(std::uintptr_t address, std::size_t size, DWORD protection, DWORD* old_protect = nullptr)
        {
            if (!ok_addr(address) || !size || !connected())
                return false;
            const std::uintptr_t page_mask = ~(static_cast<std::uintptr_t>(pg()) - 1);
            const std::uintptr_t base = address & page_mask;
            const std::uintptr_t end = (address + size + pg() - 1) & page_mask;
            const std::size_t span = static_cast<std::size_t>(end - base);
            using NtProtectFn = LONG(WINAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
            static NtProtectFn nt_protect = []() -> NtProtectFn
            {
                HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
                if (!ntdll)
                    return nullptr;
                return reinterpret_cast<NtProtectFn>(GetProcAddress(ntdll, "NtProtectVirtualMemory"));
            }();
            auto try_one = [&](DWORD req) -> bool
            {
                DWORD old = 0;
                if (VirtualProtectEx(g_proc->raw(), reinterpret_cast<void*>(base), span, req, &old))
                {
                    if (old_protect)
                        *old_protect = old;
                    return true;
                }
                if (!nt_protect)
                    return false;
                PVOID nt_base = reinterpret_cast<void*>(base);
                SIZE_T nt_size = span;
                ULONG nt_old = 0;
                const LONG st = nt_protect(g_proc->raw(), &nt_base, &nt_size, req, &nt_old);
                if (st >= 0)
                {
                    if (old_protect)
                        *old_protect = static_cast<DWORD>(nt_old);
                    return true;
                }
                return false;
            };
            if (try_one(protection))
                return true;
            if (protection == PAGE_EXECUTE_READWRITE && try_one(PAGE_EXECUTE_WRITECOPY))
                return true;
            if (protection == PAGE_READWRITE && try_one(PAGE_WRITECOPY))
                return true;
            return false;
        }
        bool write_protected(std::uintptr_t address, const void* data, std::size_t size)
        {
            if (!ok_addr(address) || !data || !size)
                return false;
            DWORD old = 0;
            const bool changed = protect_remote(address, size, PAGE_EXECUTE_READWRITE, &old);
            const bool wrote = w_mem(address, data, size);
            if (changed)
                protect_remote(address, size, old, nullptr);
            return wrote;
        }
        bool mark_cfg(std::uintptr_t t)
        {
            auto resolve = []() -> FARPROC
            {
                const char* mods[] = { "kernelbase.dll", "kernel32.dll", "api-ms-win-core-memory-l1-1-3.dll" };
                for (auto* m : mods)
                {
                    HMODULE h = GetModuleHandleA(m);
                    if (!h)
                        h = LoadLibraryA(m);
                    if (!h)
                        continue;
                    if (auto p = GetProcAddress(h, "SetProcessValidCallTargets"))
                        return p;
                }
                return nullptr;
            };
            auto proc = resolve();
            if (!proc)
                return false;
            struct Info
            {
                ULONG_PTR Offset;
                ULONG Flags;
            } info{};
            info.Offset = t & (pg() - 1);
            info.Flags = CFG_CALL_TARGET_VALID;
            using Fn = BOOL(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, void*);
            SetLastError(0);
            const BOOL ok = reinterpret_cast<Fn>(proc)(g_proc->raw(),
                reinterpret_cast<void*>(t & ~(static_cast<std::uintptr_t>(pg()) - 1)), pg(), 1, &info);
            return ok != 0;
        }
        void append_u64(std::vector<std::uint8_t>& c, std::uint64_t v)
        {
            const auto* b = reinterpret_cast<const std::uint8_t*>(&v);
            c.insert(c.end(), b, b + 8);
        }
        void patch_rel32(std::vector<std::uint8_t>& c, std::size_t o, std::size_t t)
        {
            const std::int32_t v =
                static_cast<std::int32_t>(static_cast<std::ptrdiff_t>(t) - static_cast<std::ptrdiff_t>(o + 4));
            std::memcpy(c.data() + o, &v, 4);
        }
        std::vector<std::uint8_t> make_hook_thunk(std::uintptr_t state, std::uintptr_t orig)
        {
            std::vector<std::uint8_t> c;
            c.reserve(384);
            std::vector<std::size_t> inactive;
            auto je_inactive = [&]
            {
                c.insert(c.end(), { 0x0F, 0x84 });
                inactive.push_back(c.size());
                c.insert(c.end(), { 0, 0, 0, 0 });
            };
            auto jbe_inactive = [&]
            {
                c.insert(c.end(), { 0x0F, 0x86 });
                inactive.push_back(c.size());
                c.insert(c.end(), { 0, 0, 0, 0 });
            };
            c.insert(c.end(), { 0x48, 0x83, 0xEC, 0x68 });
            c.insert(c.end(), { 0x49, 0xBA });
            append_u64(c, state);
            c.insert(c.end(), { 0x41, 0x83, 0x3A, 0x00 });
            je_inactive();
            c.insert(c.end(), { 0x4D, 0x85, 0xC0 });
            je_inactive();
            c.insert(c.end(), { 0x4D, 0x85, 0xC9 });
            je_inactive();
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x42, 0x08 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x00 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x44, 0x24, 0x40 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x4A, 0x0C });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x48, 0x04 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x4C, 0x24, 0x44 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x52, 0x10 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x50, 0x08 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x54, 0x24, 0x48 });
            c.insert(c.end(), { 0x0F, 0x28, 0xD8 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xDB });
            c.insert(c.end(), { 0x0F, 0x28, 0xE1 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xDC });
            c.insert(c.end(), { 0x0F, 0x28, 0xE2 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xDC });
            c.insert(c.end(), { 0xF3, 0x0F, 0x51, 0xDB });
            c.insert(c.end(), { 0x0F, 0x57, 0xED });
            c.insert(c.end(), { 0x0F, 0x2E, 0xDD });
            jbe_inactive();
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x21 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x69, 0x04 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xED });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE5 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x69, 0x08 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xED });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE5 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x51, 0xE4 });
            c.insert(c.end(), { 0x0F, 0x57, 0xED });
            c.insert(c.end(), { 0x0F, 0x2E, 0xE5 });
            jbe_inactive();
            c.insert(c.end(), { 0x41, 0x8B, 0x42, 0x04 });
            c.insert(c.end(), { 0xA8, 0x01 });
            c.insert(c.end(), { 0x0F, 0x85 });
            const std::size_t wallbang_jmp = c.size();
            c.insert(c.end(), { 0, 0, 0, 0 });
            c.insert(c.end(), { 0x0F, 0x28, 0xEC });
            c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xEB });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xC5 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xCD });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xD5 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x11, 0x01 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x11, 0x49, 0x04 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x11, 0x51, 0x08 });
            c.insert(c.end(), { 0x49, 0xFF, 0x42, 0x18 });
            c.push_back(0xE9);
            const std::size_t to_call = c.size();
            c.insert(c.end(), { 0, 0, 0, 0 });
            const std::size_t wallbang_off = c.size();
            patch_rel32(c, wallbang_jmp, wallbang_off);
            c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xC3 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xCB });
            c.insert(c.end(), { 0xF3, 0x0F, 0x5E, 0xD3 });
            c.insert(c.end(), { 0x0F, 0x28, 0xE0 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x59, 0x62, 0x14 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x6A, 0x08 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x5C, 0xEC });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x6C, 0x24, 0x50 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x64, 0x24, 0x40 });
            c.insert(c.end(), { 0x0F, 0x28, 0xE1 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x59, 0x62, 0x14 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x6A, 0x0C });
            c.insert(c.end(), { 0xF3, 0x0F, 0x5C, 0xEC });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x6C, 0x24, 0x54 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x64, 0x24, 0x44 });
            c.insert(c.end(), { 0x0F, 0x28, 0xE2 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x59, 0x62, 0x14 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x6A, 0x10 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x5C, 0xEC });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x6C, 0x24, 0x58 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x11, 0x64, 0x24, 0x48 });
            c.insert(c.end(), { 0x4C, 0x8D, 0x44, 0x24, 0x50 });
            c.insert(c.end(), { 0x4C, 0x8D, 0x4C, 0x24, 0x40 });
            c.insert(c.end(), { 0x49, 0xFF, 0x42, 0x18 });
            const std::size_t call_off = c.size();
            patch_rel32(c, to_call, call_off);
            const std::size_t inactive_off = c.size();
            for (auto o : inactive)
                patch_rel32(c, o, inactive_off);
            c.insert(c.end(), { 0x48, 0x8B, 0x84, 0x24, 0x90, 0x00, 0x00, 0x00 });
            c.insert(c.end(), { 0x48, 0x89, 0x44, 0x24, 0x20 });
            c.insert(c.end(), { 0x48, 0xB8 });
            append_u64(c, orig);
            c.insert(c.end(), { 0xFF, 0xD0 });
            c.insert(c.end(), { 0x48, 0x83, 0xC4, 0x68 });
            c.push_back(0xC3);
            return c;
        }
        bool region_is_padding(std::uintptr_t a, std::size_t n)
        {
            std::vector<std::uint8_t> buf(n);
            if (!g_proc->read_buf(a, buf.data(), n))
                return false;
            for (auto b : buf)
            {
                if (b != 0xCC && b != 0x00 && b != 0x90)
                    return false;
            }
            return true;
        }
        bool read_val(std::uintptr_t a, void* d, std::size_t s)
        {
            if (!connected())
                return false;
            return g_proc->read_buf(a, d, s);
        }
        std::uintptr_t find_cave_in_module(std::uintptr_t module_base, std::size_t need, std::uintptr_t min_offset,
            std::uintptr_t ignore)
        {
            if (!ok_addr(module_base) || !need)
                return 0;
            IMAGE_DOS_HEADER dos{};
            if (!read_val(module_base, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE)
                return 0;
            IMAGE_NT_HEADERS64 nt{};
            const std::uintptr_t nt_addr = module_base + static_cast<std::uintptr_t>(dos.e_lfanew);
            if (!read_val(nt_addr, &nt, sizeof(nt)) || nt.Signature != IMAGE_NT_SIGNATURE)
                return 0;
            const std::uintptr_t section_base =
                nt_addr + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) + nt.FileHeader.SizeOfOptionalHeader;
            for (WORD i = 0; i < nt.FileHeader.NumberOfSections; ++i)
            {
                IMAGE_SECTION_HEADER section{};
                if (!read_val(section_base + static_cast<std::uintptr_t>(i) * sizeof(section), &section, sizeof(section)))
                    break;
                if ((section.Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0)
                    continue;
                const std::uintptr_t section_off = section.VirtualAddress;
                std::size_t section_size = section.Misc.VirtualSize ? section.Misc.VirtualSize : section.SizeOfRawData;
                if (!section_off || section_size < need)
                    continue;
                std::uintptr_t scan_off = section_off;
                if (scan_off < min_offset)
                    scan_off = min_offset;
                if (scan_off >= section_off + section_size)
                    continue;
                const std::uintptr_t scan_start = module_base + scan_off;
                const std::size_t scan_size = static_cast<std::size_t>(section_off + section_size - scan_off);
                std::vector<std::uint8_t> buf(scan_size);
                if (!read_val(scan_start, buf.data(), buf.size()))
                    continue;
                std::size_t run_start = 0;
                std::size_t run_len = 0;
                for (std::size_t j = 0; j < buf.size(); ++j)
                {
                    const std::uint8_t b = buf[j];
                    if (b != 0x00 && b != 0xCC && b != 0x90)
                    {
                        run_len = 0;
                        run_start = j + 1;
                        continue;
                    }
                    ++run_len;
                    if (run_len < need)
                        continue;
                    std::uintptr_t cand = scan_start + run_start;
                    const std::uintptr_t aligned = (cand + 0x0F) & ~static_cast<std::uintptr_t>(0x0F);
                    const std::size_t loss = static_cast<std::size_t>(aligned - cand);
                    if (run_len < need + loss)
                        continue;
                    if (ignore && aligned == ignore)
                        continue;
                    if (g_hook.thunk && aligned == g_hook.thunk)
                        continue;
                    return aligned;
                }
            }
            return 0;
        }
        std::uintptr_t find_exec_cave(std::size_t need, std::uintptr_t, std::uintptr_t ignore = 0)
        {
            static const wchar_t* k_pref[] = {
                L"winsta.dll",
                L"win32u.dll",
                L"uxtheme.dll",
                L"dwmapi.dll",
                L"msctf.dll",
                L"TextInputFramework.dll",
                L"CoreMessaging.dll",
                L"user32.dll",
            };
            for (std::size_t mi = 0; mi < sizeof(k_pref) / sizeof(k_pref[0]); ++mi)
            {
                const wchar_t* name = k_pref[mi];
                const std::uintptr_t mod = ray_module(name);
                if (!mod)
                    continue;
                const std::uintptr_t min_off = (mi == 0) ? 0x2000u : 0x1000u;
                const std::uintptr_t cave = find_cave_in_module(mod, need, min_off, ignore);
                if (!cave)
                    continue;
                return cave;
            }
            MEMORY_BASIC_INFORMATION mbi{};
            std::uintptr_t addr = 0;
            std::uintptr_t fallback_rwx = 0;
            while (VirtualQueryEx(g_proc->raw(), reinterpret_cast<void*>(addr), &mbi, sizeof(mbi)))
            {
                const auto base = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress);
                const auto size = static_cast<std::size_t>(mbi.RegionSize);
                addr = base + size;
                if (addr < base)
                    break;
                if (mbi.State != MEM_COMMIT)
                    continue;
                if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
                    continue;
                if (!is_executable_protect(mbi.Protect))
                    continue;
                if (size < need)
                    continue;
                if (g_hook.thunk && base <= g_hook.thunk && g_hook.thunk < addr)
                    continue;
                for (std::size_t off = 0; off + need <= size; off += 0x10)
                {
                    const std::uintptr_t cand = base + off;
                    if (ignore && cand == ignore)
                        continue;
                    if (region_is_padding(cand, need))
                        return cand;
                }
                if (!fallback_rwx && mbi.Type == MEM_PRIVATE && (mbi.Protect & 0xFF) == PAGE_EXECUTE_READWRITE &&
                    size >= need + 0x40)
                {
                    const std::uintptr_t cand = base + size - need;
                    if (!ignore || cand != ignore)
                        fallback_rwx = cand;
                }
            }
            if (fallback_rwx)
                return fallback_rwx;
            return 0;
        }
        std::uintptr_t alloc_exec_page()
        {
            const std::uintptr_t p = ray_alloc(pg(), PAGE_EXECUTE_READWRITE);
            if (!p)
                return 0;
            const DWORD prot = query_protect(p);
            if (!is_executable_protect(prot))
            {
                ray_free(p);
                return 0;
            }
            return p;
        }
        bool install_hook()
        {
            if (g_hook.installed)
                return true;
            const std::uintptr_t base = ray_module(L"RobloxPlayerBeta.exe");
            if (!base)
            {
                set_status("install: no game module");
                return false;
            }
            const auto now = std::chrono::steady_clock::now();
            if (g_lastFail.time_since_epoch().count() != 0 && now - g_lastFail < std::chrono::milliseconds(1500))
            {
                return false;
            }
            const std::uintptr_t desc = base + desc_rva_z;
            const std::uintptr_t slot = desc + bound_fn_offset;
            {
                DWORD pdesc = query_protect(desc), pslot = query_protect(slot);
                (void)pdesc; (void)pslot;
                std::uintptr_t probe = 0;
                bool canReadDesc = read_val(desc, &probe, sizeof(probe));
                if (!canReadDesc)
                {
                    set_status("install: unreadable desc (offsets stale?)");
                    g_lastFail = now;
                    return false;
                }
            }
            const std::uintptr_t fn = g_proc->read<std::uintptr_t>(slot);
            if (!ok_addr(fn))
            {
                set_status("install: bad slot fn (offsets stale?)");
                g_lastFail = now;
                return false;
            }
            {
                DWORD pfn = query_protect(fn);
                (void)pfn;
                std::uint8_t head[16] = {};
                bool rh = read_val(fn, head, sizeof(head));
                if (!rh)
                {
                    set_status("install: fn unreadable");
                    g_lastFail = now;
                    return false;
                }
            }
            if (!g_hook.state)
            {
                g_hook.state = ray_alloc(pg(), PAGE_READWRITE);
            }
            else
            if (!g_hook.state)
            {
                set_status("install: no state mem");
                g_lastFail = now;
                return false;
            }
            auto thunk = make_hook_thunk(g_hook.state, fn);
            if (thunk.size() > k_stub_bytes)
            {
                set_status("install: thunk too big");
                g_lastFail = now;
                return false;
            }
            bool owned = false;
            std::uintptr_t stub = 0;
            std::uintptr_t ignore_cave = 0;
            for (int attempt = 0; attempt < 8 && !stub; ++attempt)
            {
                std::uintptr_t cand = find_exec_cave(k_stub_bytes, base, ignore_cave);
                if (!cand)
                    break;
                DWORD old_prot = 0;
                const bool prot_ok = protect_remote(cand, thunk.size(), PAGE_EXECUTE_READWRITE, &old_prot);
                if (!write_protected(cand, thunk.data(), thunk.size()))
                {
                    if (prot_ok)
                        protect_remote(cand, thunk.size(), old_prot, nullptr);
                    ignore_cave = cand;
                    continue;
                }

                std::vector<std::uint8_t> back(thunk.size());
                bool rb = read_val(cand, back.data(), back.size());
                bool match = rb && memcmp(back.data(), thunk.data(), thunk.size()) == 0;
                if (!match)
                {
                    ignore_cave = cand;
                    continue;
                }
                stub = cand;
                owned = false;
            }
            if (!stub)
            {
                stub = alloc_exec_page();
                owned = stub != 0;
                if (stub)
                {
                    if (!write_protected(stub, thunk.data(), thunk.size()))
                    {
                        ray_free(stub);
                        stub = 0;
                        owned = false;
                    }
                    else
                    {
                        std::vector<std::uint8_t> back(thunk.size());
                        bool rb = read_val(stub, back.data(), back.size());
                        bool match = rb && memcmp(back.data(), thunk.data(), thunk.size()) == 0;
                        if (!match) { ray_free(stub); stub = 0; owned = false; }
                    }
                }
            }
            if (!stub)
            {
                set_status("install: no exec cave");
                g_lastFail = now;
                return false;
            }
            RaycastState empty{};
            if (!w_mem(g_hook.state, &empty, sizeof(empty)))
            {
                set_status("install: state write fail");
                g_lastFail = now;
                if (owned)
                    ray_free(stub);
                return false;
            }
            FlushInstructionCache(g_proc->raw(), reinterpret_cast<void*>(stub), thunk.size());
            bool cfg = mark_cfg(stub);
            const DWORD prot = query_protect(stub);
            if (!is_executable_protect(prot))
            {
                set_status("install: cave not executable");
                g_lastFail = now;
                if (owned)
                    ray_free(stub);
                return false;
            }
            protect_remote(slot, 8, PAGE_READWRITE, nullptr);
            std::uintptr_t before = g_proc->read<std::uintptr_t>(slot);
            (void)before;
            bool wok = write_protected(slot, &stub, sizeof(stub));
            std::uintptr_t after = g_proc->read<std::uintptr_t>(slot);
            if (!wok || after != stub)
            {
                set_status("install: slot patch fail");
                g_lastFail = now;
                if (owned)
                    ray_free(stub);
                return false;
            }
            g_hook.module_base = base;
            g_hook.originalFunction = fn;
            g_hook.thunk = stub;
            g_hook.thunk_owned = owned;
            g_hook.installed = true;
            g_hook.active = false;
            set_status("installed: hook live, waiting for target");
            return true;
        }
        void remove_hook()
        {
            if (g_hook.installed && g_hook.state)
            {
                std::uint32_t v = 0;
                w_mem(g_hook.state, &v, sizeof(v));
                g_hook.active = false;
                Sleep(150);
            }
            if (g_hook.installed)
                set_status("idle: hook removed");
            if (g_hook.installed && ok_addr(g_hook.originalFunction) && g_hook.module_base)
            {
                const std::uintptr_t slot = g_hook.module_base + desc_rva_z + bound_fn_offset;
                std::uintptr_t cur = g_proc ? g_proc->read<std::uintptr_t>(slot) : 0;
                if (cur == g_hook.thunk)
                {
                    write_protected(slot, &g_hook.originalFunction, sizeof(g_hook.originalFunction));
                    FlushInstructionCache(g_proc->raw(), reinterpret_cast<void*>(slot), 8);
                }
            }
            if (g_hook.thunk && g_hook.thunk_owned)
                ray_free(g_hook.thunk);
            if (g_hook.state)
                ray_free(g_hook.state);
            g_hook = {};
            g_wallbang = false;
        }
        void ensure_hook(bool want)
        {
            using Clock = std::chrono::steady_clock;
            static std::uintptr_t s_cachedBase = 0;
            static auto s_lastQuery = Clock::now() - std::chrono::seconds(10);
            if (!want)
            {
                if (g_hook.installed)
                    remove_hook();
                return;
            }
            const auto now = Clock::now();
            if (!s_cachedBase || std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastQuery).count() >= 1500)
            {
                s_lastQuery = now;
                const std::uintptr_t base = ray_module(L"RobloxPlayerBeta.exe");
                if (base)
                {
                    if (s_cachedBase && base != s_cachedBase && g_hook.installed)
                        remove_hook();
                    s_cachedBase = base;
                }
                else if (!g_hook.installed)
                {
                    return;
                }
            }
            if (!g_hook.installed)
                install_hook();
        }
        void set_hook_active(bool on, const math::vec3& world_target, bool wallbang)
        {
            if (!on)
            {
                if (g_hook.active && g_hook.state)
                {
                    std::uint32_t v = 0;
                    w_mem(g_hook.state, &v, sizeof(v));
                    g_hook.active = false;
                }
                g_wallbang = false;
                if (g_hook.installed)
                    set_status("installed: hook live, waiting for target");
                return;
            }
            if (!g_hook.installed)
            {
                set_status("setactive: not installed");
                return;
            }
            float pos[3]{ world_target.x, world_target.y, world_target.z };
            const std::uint32_t flags = wallbang ? 1u : 0u;
            const float scale = 1.15f;
            const std::uint32_t one = 1;
            bool ok1 = w_mem(g_hook.state + offsetof(RaycastState, reserved), &flags, sizeof(flags));
            bool ok2 = w_mem(g_hook.state + offsetof(RaycastState, target_x), pos, sizeof(pos));
            bool ok3 = w_mem(g_hook.state + offsetof(RaycastState, scale), &scale, sizeof(scale));
            bool ok4 = w_mem(g_hook.state + offsetof(RaycastState, active), &one, sizeof(one));
            if (!ok1 || !ok2 || !ok3 || !ok4)
            {
                remove_hook();
                return;
            }
            g_hook.active = true;
            g_wallbang = wallbang;
            set_status(wallbang ? "aiming: magic bullet (wallbang)" : "aiming: redirect active");
        }
        struct CachedTarget
        {
            uintptr_t playerAddr = 0;
            uintptr_t characterAddr = 0;
            uintptr_t humanoidAddr = 0;
            uintptr_t targetPrim = 0;
            uintptr_t teamAddr = 0;
            int partIdx = -1;
            bool isBot = false;
            float botHp = 100.0f;
        };
        std::vector<CachedTarget> s_targets;
        std::chrono::steady_clock::time_point s_lastResolve{};
        std::chrono::steady_clock::time_point s_lastScan{};
        void resolve_targets()
        {
            s_targets.clear();
            int wantPart = settings::aim_part;
            std::vector<pcache::Entry> snap;
            {
                std::lock_guard<std::mutex> lk(pcache::mtx);
                snap = pcache::entries;
            }
            s_targets.reserve(snap.size());
            std::uintptr_t localAddr = g_ctx.local_player.addr;
            for (auto& e : snap)
            {
                if (!e.valid || e.player == localAddr)
                    continue;
                if (!e.humanoid)
                    continue;
                std::uintptr_t prim = pcache::aim_prim(e.lc, wantPart);
                if (!prim)
                    continue;
                CachedTarget t{};
                t.playerAddr = e.player;
                t.characterAddr = e.model;
                t.humanoidAddr = e.humanoid;
                t.targetPrim = prim;
                t.teamAddr = e.team;
                t.partIdx = wantPart;
                s_targets.push_back(t);
            }
            if (settings::aim_bots)
            {
                std::vector<BotAimTarget> bots;
                collect_bot_targets(bots);
                for (auto& b : bots)
                {
                    std::uintptr_t prim = (wantPart == 0) ? b.headPrim : b.rootPrim;
                    if (!prim)
                        prim = b.headPrim ? b.headPrim : b.rootPrim;
                    if (!prim)
                        continue;
                    CachedTarget t{};
                    t.playerAddr = b.inst;
                    t.characterAddr = b.inst;
                    t.humanoidAddr = b.humanoid;
                    t.targetPrim = prim;
                    t.teamAddr = 0;
                    t.partIdx = wantPart;
                    t.isBot = true;
                    t.botHp = b.hp;
                    s_targets.push_back(t);
                }
            }
        }
    }
    bool ready()
    {
        return g_hook.installed;
    }
    bool is_active()
    {
        return g_hook.active;
    }
    std::uint64_t calls()
    {
        if (!g_hook.installed || !g_hook.state || !connected())
            return 0;
        return g_proc->read<std::uint64_t>(g_hook.state + offsetof(RaycastState, calls));
    }
    const char* status()
    {
        return g_status;
    }
    void idle()
    {
        set_hook_active(false, math::vec3{}, false);
        s_targets.clear();
    }
    void on_game_joined()
    {
        g_hook = {};
        g_wallbang = false;
        g_lastFail = std::chrono::steady_clock::time_point{};
        s_targets.clear();
        s_lastResolve = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
        set_status("idle: hook not requested");
    }
    void shutdown()
    {
        ensure_hook(false);
        s_targets.clear();
    }
    void run(int w, int h)
    {
        if (!connected() || !g_ctx.ready || !g_ctx.camera || w <= 0 || h <= 0)
        {
            set_hook_active(false, math::vec3{}, false);
            thenwefuckin::aim::clear_debug_target();
            return;
        }
        auto now = std::chrono::steady_clock::now();
        if (now - s_lastScan < std::chrono::milliseconds(8))
            return;
        s_lastScan = now;
        if (pftarget::active())
        {
            pftarget::PfBest b{};
            if (!pftarget::select_best(w, h, b))
            {
                set_hook_active(false, math::vec3{}, false);
                thenwefuckin::aim::clear_debug_target();
                return;
            }
            thenwefuckin::aim::set_debug_target(b.world, b.model);
            if (!settings::aim_magicbullet)
            {
                set_hook_active(false, math::vec3{}, false);
                return;
            }
            ensure_hook(true);
            set_hook_active(true, b.world, true);
            return;
        }
        int wantPart = settings::aim_part;
        if (s_targets.empty()
            || now - s_lastResolve >= std::chrono::milliseconds(200)
            || s_targets[0].partIdx != wantPart)
        {
            resolve_targets();
            s_lastResolve = now;
            if (s_targets.empty())
            {
                set_hook_active(false, math::vec3{}, false);
                thenwefuckin::aim::clear_debug_target();
                return;
            }
        }
        float cx = static_cast<float>(w) * 0.5f;
        float cy = static_cast<float>(h) * 0.5f;
        const math::view_matrix& view = g_ctx.view;
        math::vec3 camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        if (camPos.x == 0.0f && camPos.y == 0.0f && camPos.z == 0.0f)
            return;
        math::vec3 best{};
        bool found = false;
        const bool wide = settings::aim_360;
        const int prio = settings::aim_priority;
        float bestDist = (wide || prio != 0) ? 1e9f : settings::aim_fov;
        math::vec3 lpPos = g_ctx.local_pos;
        if (lpPos.x == 0.0f && lpPos.y == 0.0f && lpPos.z == 0.0f)
            lpPos = camPos;
        uintptr_t bestPlayer = 0;
        for (auto& t : s_targets)
        {
            if (thenwefuckin::aim::is_teammate(t.playerAddr, t.characterAddr, t.teamAddr))
                continue;
            if (t.isBot)
            {
                std::uintptr_t par = g_proc->read<std::uintptr_t>(t.characterAddr + Offsets::Instance::Parent);
                if (!par)
                    continue;
                float bhp = t.humanoidAddr ? g_proc->read<float>(t.humanoidAddr + Offsets::Humanoid::Health) : t.botHp;
                if (!health::check(bhp))
                    continue;
            }
            else
            {
                uintptr_t charAddr = g_proc->read<uintptr_t>(t.playerAddr + Offsets::Player::ModelInstance);
                if (charAddr != t.characterAddr)
                {
                    tracking::clear(t.playerAddr);
                    continue;
                }
                float hp = g_proc->read<float>(t.humanoidAddr + Offsets::Humanoid::Health);
                if (!health::check(hp))
                    continue;
            }
            math::vec3 pos = g_proc->read<math::vec3>(t.targetPrim + Offsets::Primitive::Position);
            if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
                continue;
            tracking::push(t.playerAddr, pos.x, pos.y, pos.z);
            if (!distance::check(pos, camPos))
                continue;
            if (settings::aim_visible_check && !worldgeo::visible(camPos, pos))
                continue;
            float d = 0.0f;
            if (wide || prio == 2)
            {
                d = (pos - camPos).length();
            }
            else if (prio == 1)
            {
                d = (pos - lpPos).length();
            }
            else
            {
                math::vec2 s{};
                if (!projection::to_screen(pos, view, static_cast<float>(w), static_cast<float>(h), s))
                    continue;
                float dx = s.x - cx;
                float dy = s.y - cy;
                d = sqrtf(dx * dx + dy * dy);
            }
            if (d < bestDist)
            {
                bestDist = d;
                best = pos;
                bestPlayer = t.playerAddr;
                found = true;
            }
        }
        if (!found)
        {
            set_hook_active(false, math::vec3{}, false);
            thenwefuckin::aim::clear_debug_target();
            return;
        }
        {
            tracking::Velocity tv{};
            bool haveTv = bestPlayer && tracking::sample(bestPlayer, tv);
            if (settings::aim_prediction && haveTv)
            {
                if (settings::aim_pred_mode == 1)
                {
                    predict::fallen_update_weapon_auto();
                    float bv = predict::fallen_get_bullet_velocity(predict::fallen_get_local_weapon());
                    if (bv > 0.0f)
                    {
                        math::vec3 tp = best;
                        math::vec3 tvel{ tv.vx * tv.damp, tv.vy, tv.vz * tv.damp };
                        predict::fallen_predict(tp, camPos, tvel, bv, g_ctx.local_velocity);
                        best = tp;
                    }
                }
                else
                {
                    float scanDt = 0.004f;
                    float smoothLag = 0.0f;
                    if (settings::aim_smoothing_enabled)
                    {
                        float sx = settings::aim_smoothing_x;
                        if (sx < 1.0f) sx = 1.0f;
                        smoothLag = (sx - 1.0f) * scanDt;
                    }
                    float leadT = scanDt + smoothLag;
                    float damp = tv.damp;
                    float airDampY = tv.airborne ? 0.35f : 1.0f;
                    math::vec3 lv = g_ctx.local_velocity;
                    best.x += (tv.vx - lv.x) * leadT * damp;
                    best.y += (tv.vy - lv.y) * leadT * damp * airDampY;
                    best.z += (tv.vz - lv.z) * leadT * damp * airDampY;
                    best.y += settings::aim_pred_height;
                    float fvx = tv.vx * tv.damp;
                    float fvz = tv.vz * tv.damp;
                    float fl = sqrtf(fvx * fvx + fvz * fvz);
                    if (fl > 0.01f && settings::aim_pred_forward != 0.0f)
                    {
                        best.x += fvx / fl * settings::aim_pred_forward;
                        best.z += fvz / fl * settings::aim_pred_forward;
                    }
                }
            }
        }
        thenwefuckin::aim::set_debug_target(best, bestPlayer);
        if (!settings::aim_magicbullet)
        {
            set_hook_active(false, math::vec3{}, false);
            return;
        }
        ensure_hook(true);
        set_hook_active(true, best, true);
    }
}
