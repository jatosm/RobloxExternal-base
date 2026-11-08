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
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include <TlHelp32.h>
#include <Windows.h>
#include <vector>
#include <cstring>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <mutex>
#ifndef CFG_CALL_TARGET_VALID
#define CFG_CALL_TARGET_VALID 0x00000001
#endif
namespace thenwefuckin::aim::raycast
{
    namespace
    {
        constexpr std::uintptr_t desc_rva = Offsets::WorldRoot::RaycastBoundDesc;
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
            float cam_x = 0.f;
            float cam_y = 0.f;
            float cam_z = 0.f;
        };
#pragma pack(pop)
        static_assert(offsetof(RaycastState, active) == 0x00, "active");
        static_assert(offsetof(RaycastState, target_x) == 0x08, "target");
        static_assert(offsetof(RaycastState, calls) == 0x18, "calls");
        static_assert(offsetof(RaycastState, cam_x) == 0x20, "cam");
        struct Hook
        {
            std::uintptr_t thunk = 0;
            std::uintptr_t state = 0;
            std::uintptr_t original = 0;
            std::uintptr_t module_base = 0;
            bool installed = false;
            bool active = false;
            bool wallbang = false;
        };
        Hook g_hook{};
        auto g_last_fail = std::chrono::steady_clock::time_point{};
        std::uintptr_t g_last_base = 0;
        auto g_last_slot_check = std::chrono::steady_clock::time_point{};
        char g_status[96] = "idle: hook not requested";
        void set_status(const char* s)
        {
            if (!s || !s[0])
                return;
            std::size_t n = strlen(s);
            if (n >= sizeof(g_status))
                n = sizeof(g_status) - 1;
            memcpy(g_status, s, n);
            g_status[n] = '\0';
        }
        bool connected()
        {
            return g_proc && g_proc->connected();
        }
        bool proc_alive()
        {
            return g_proc && g_proc->connected() && g_proc->alive();
        }
        bool addr_ok(std::uintptr_t a)
        {
            return a >= 0x10000ull && a < 0x00007FFFFFFFFFFFull;
        }
        bool w_mem(std::uintptr_t a, const void* d, std::size_t s)
        {
            if (!addr_ok(a) || !d || !s || !g_proc || !g_proc->connected())
                return false;
            return g_proc->write_buf(a, d, s);
        }
        bool read_val(std::uintptr_t a, void* d, std::size_t s)
        {
            if (!g_proc || !g_proc->connected())
                return false;
            return g_proc->read_buf(a, d, s);
        }
        std::size_t page_sz()
        {
            static std::size_t p = 0;
            if (!p)
            {
                SYSTEM_INFO i{};
                GetSystemInfo(&i);
                p = i.dwPageSize ? (std::size_t)i.dwPageSize : 0x1000u;
            }
            return p;
        }
        bool is_exec_prot(DWORD p)
        {
            const DWORD x = p & 0xFF;
            return x == PAGE_EXECUTE || x == PAGE_EXECUTE_READ ||
                x == PAGE_EXECUTE_READWRITE || x == PAGE_EXECUTE_WRITECOPY;
        }
        bool region_exec(std::uintptr_t a)
        {
            if (!g_proc || !g_proc->connected())
                return false;
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQueryEx(g_proc->raw(), reinterpret_cast<void*>(a), &mbi, sizeof(mbi)))
                return false;
            return mbi.State == MEM_COMMIT && is_exec_prot(mbi.Protect);
        }
        bool protect_remote(std::uintptr_t address, std::size_t size, DWORD protection, DWORD* old_protect = nullptr)
        {
            if (!addr_ok(address) || !size || !g_proc || !g_proc->connected())
                return false;
            const std::uintptr_t mask = ~(static_cast<std::uintptr_t>(page_sz()) - 1);
            const std::uintptr_t base = address & mask;
            const std::uintptr_t end = (address + size + page_sz() - 1) & mask;
            DWORD old = 0;
            if (!VirtualProtectEx(g_proc->raw(), reinterpret_cast<void*>(base),
                    static_cast<SIZE_T>(end - base), protection, &old))
                return false;
            if (old_protect)
                *old_protect = old;
            return true;
        }
        bool write_protected(std::uintptr_t address, const void* data, std::size_t size)
        {
            if (!addr_ok(address) || !data || !size)
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
            if (!proc || !g_proc || !g_proc->connected())
                return false;
            struct Info
            {
                ULONG_PTR Offset;
                ULONG Flags;
            } info{};
            info.Offset = t & (page_sz() - 1);
            info.Flags = CFG_CALL_TARGET_VALID;
            using Fn = BOOL(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, void*);
            return reinterpret_cast<Fn>(proc)(
                g_proc->raw(),
                reinterpret_cast<void*>((t & ~(static_cast<std::uintptr_t>(page_sz()) - 1))),
                page_sz(), 1, &info) != 0;
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
            c.insert(c.end(), { 0x4D, 0x85, 0xC0 }); je_inactive();
            c.insert(c.end(), { 0x4D, 0x85, 0xC9 }); je_inactive();

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

            const std::size_t dir_only_off = c.size();
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

            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x20 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x62, 0x20 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xE4 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x68, 0x04 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x6A, 0x24 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xED });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE5 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x10, 0x68, 0x08 });
            c.insert(c.end(), { 0xF3, 0x41, 0x0F, 0x5C, 0x6A, 0x28 });
            c.insert(c.end(), { 0xF3, 0x0F, 0x59, 0xED });
            c.insert(c.end(), { 0xF3, 0x0F, 0x58, 0xE5 });
            c.insert(c.end(), { 0xB8, 0x00, 0x00, 0x10, 0x41 });
            c.insert(c.end(), { 0x66, 0x0F, 0x6E, 0xE8 });
            c.insert(c.end(), { 0x0F, 0x2F, 0xE5 });
            c.insert(c.end(), { 0x0F, 0x82 });
            const std::size_t cam_jb = c.size();
            c.insert(c.end(), { 0, 0, 0, 0 });
            patch_rel32(c, cam_jb, dir_only_off);

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
        std::uintptr_t module_base_of(const wchar_t* name)
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
                        out = reinterpret_cast<std::uintptr_t>(me.modBaseAddr);
                        break;
                    }
                } while (Module32NextW(snap, &me));
            }
            CloseHandle(snap);
            return out;
        }
        std::uintptr_t module_base()
        {
            return module_base_of(L"RobloxPlayerBeta.exe");
        }
        std::uintptr_t alloc_page(std::size_t size, DWORD prot)
        {
            if (!connected())
                return 0;
            void* p = VirtualAllocEx(g_proc->raw(), nullptr, size, MEM_COMMIT | MEM_RESERVE, prot);
            return reinterpret_cast<std::uintptr_t>(p);
        }
        void free_remote(std::uintptr_t p)
        {
            if (!p || !connected())
                return;
            VirtualFreeEx(g_proc->raw(), reinterpret_cast<void*>(p), 0, MEM_RELEASE);
        }
        std::uintptr_t find_cave_in_module(std::uintptr_t mod, std::size_t need,
            std::uintptr_t min_off, std::uintptr_t ignore)
        {
            if (!addr_ok(mod) || !need)
                return 0;
            IMAGE_DOS_HEADER dos{};
            if (!read_val(mod, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE)
                return 0;
            IMAGE_NT_HEADERS64 nt{};
            const std::uintptr_t nt_addr = mod + static_cast<std::uintptr_t>(dos.e_lfanew);
            if (!read_val(nt_addr, &nt, sizeof(nt)) || nt.Signature != IMAGE_NT_SIGNATURE)
                return 0;
            const std::uintptr_t sec_base = nt_addr + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) +
                nt.FileHeader.SizeOfOptionalHeader;
            for (WORD i = 0; i < nt.FileHeader.NumberOfSections; ++i)
            {
                IMAGE_SECTION_HEADER sec{};
                if (!read_val(sec_base + static_cast<std::uintptr_t>(i) * sizeof(sec), &sec, sizeof(sec)))
                    break;
                if ((sec.Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0)
                    continue;
                const std::uintptr_t off = sec.VirtualAddress;
                const std::size_t size = sec.Misc.VirtualSize ? sec.Misc.VirtualSize : sec.SizeOfRawData;
                if (!off || size < need)
                    continue;
                std::uintptr_t scan_off = off < min_off ? min_off : off;
                if (scan_off >= off + size)
                    continue;
                const std::uintptr_t start = mod + scan_off;
                const std::size_t scan = static_cast<std::size_t>(off + size - scan_off);
                std::vector<std::uint8_t> buf(scan);
                if (!read_val(start, buf.data(), buf.size()))
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
                    const std::uintptr_t cand = start + run_start;
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
        std::uintptr_t find_dll_cave(std::size_t need, std::uintptr_t ignore)
        {
            static const wchar_t* pref[] = {
                L"winsta.dll", L"win32u.dll", L"user32.dll",
            };
            for (std::size_t i = 0; i < sizeof(pref) / sizeof(pref[0]); ++i)
            {
                const std::uintptr_t mod = module_base_of(pref[i]);
                if (!mod)
                    continue;
                const std::uintptr_t cave =
                    find_cave_in_module(mod, need, i == 0 ? 0x2000u : 0x1000u, ignore);
                if (cave)
                    return cave;
            }
            return 0;
        }
        std::uintptr_t slot_addr(std::uintptr_t base)
        {
            return base + desc_rva + bound_fn_offset;
        }
        void deactivate_state()
        {
            if (!g_hook.state || !connected())
                return;
            std::uint32_t z = 0;
            w_mem(g_hook.state, &z, sizeof(z));
        }
        void free_state()
        {
            if (g_hook.state)
            {
                free_remote(g_hook.state);
                g_hook.state = 0;
            }
        }
        bool restore_slot()
        {
            if (!g_hook.module_base || !addr_ok(g_hook.original) || !g_hook.thunk)
                return false;
            if (!connected())
                return false;
            const std::uintptr_t live = module_base();
            if (!live || live != g_hook.module_base)
                return false;
            const std::uintptr_t slot = slot_addr(g_hook.module_base);
            if (g_proc->read<std::uintptr_t>(slot) != g_hook.thunk)
                return false;
            return write_protected(slot, &g_hook.original, sizeof(g_hook.original));
        }
        bool do_install()
        {
            if (g_hook.installed)
                return true;
            const std::uintptr_t base = module_base();
            if (!base || !proc_alive())
                return false;
            const auto now = std::chrono::steady_clock::now();
            if (g_last_fail.time_since_epoch().count() != 0 &&
                now - g_last_fail < std::chrono::milliseconds(1500))
                return false;
            const std::uintptr_t slot = slot_addr(base);
            const std::uintptr_t fn = g_proc->read<std::uintptr_t>(slot);
            if (!addr_ok(fn) || !region_exec(fn))
            {
                g_last_fail = now;
                set_status("install: bad handler (offsets stale?)");
                return false;
            }
            if (g_hook.thunk && g_hook.state && g_hook.original == fn)
            {
                mark_cfg(g_hook.thunk);
                if (write_protected(slot, &g_hook.thunk, sizeof(g_hook.thunk)) &&
                    g_proc->read<std::uintptr_t>(slot) == g_hook.thunk)
                {
                    g_hook.module_base = base;
                    g_hook.installed = true;
                    deactivate_state();
                    set_status("installed: hook live, waiting for target");
                    return true;
                }
            }
            if (!g_hook.state)
                g_hook.state = alloc_page(page_sz(), PAGE_READWRITE);
            if (!g_hook.state)
            {
                g_last_fail = now;
                set_status("install: no state mem");
                return false;
            }
            auto thunk = make_hook_thunk(g_hook.state, fn);
            if (thunk.size() > k_stub_bytes)
            {
                g_last_fail = now;
                set_status("install: thunk too big");
                return false;
            }
            std::uintptr_t stub = 0;
            std::uintptr_t ignore = 0;
            for (int attempt = 0; attempt < 3 && !stub; ++attempt)
            {
                const std::uintptr_t cand = find_dll_cave(k_stub_bytes, ignore);
                if (!cand)
                    break;
                if (!write_protected(cand, thunk.data(), thunk.size()))
                {
                    ignore = cand;
                    continue;
                }
                std::vector<std::uint8_t> back(thunk.size());
                if (!read_val(cand, back.data(), back.size()) ||
                    memcmp(back.data(), thunk.data(), thunk.size()) != 0)
                {
                    ignore = cand;
                    continue;
                }
                stub = cand;
            }
            if (!stub)
            {
                g_last_fail = now;
                set_status("install: no exec cave");
                return false;
            }
            RaycastState empty{};
            empty.scale = 1.15f;
            if (!w_mem(g_hook.state, &empty, sizeof(empty)))
            {
                g_last_fail = now;
                set_status("install: state write fail");
                return false;
            }
            FlushInstructionCache(g_proc->raw(), reinterpret_cast<void*>(stub), thunk.size());
            mark_cfg(stub);
            if (!region_exec(stub))
            {
                g_last_fail = now;
                set_status("install: cave not executable");
                return false;
            }
            if (!write_protected(slot, &stub, sizeof(stub)) ||
                g_proc->read<std::uintptr_t>(slot) != stub)
            {
                g_last_fail = now;
                set_status("install: slot patch fail");
                return false;
            }
            g_hook.module_base = base;
            g_hook.original = fn;
            g_hook.thunk = stub;
            g_hook.installed = true;
            g_hook.active = false;
            set_status("installed: hook live, waiting for target");
            return true;
        }
        void remove_hook()
        {
            deactivate_state();
            restore_slot();
            g_hook.installed = false;
            g_hook.active = false;
            g_hook.wallbang = false;
            if (!proc_alive())
            {
                free_state();
                g_hook = {};
            }
            set_status("idle: hook removed");
        }
        void ensure_hook(bool want)
        {
            const std::uintptr_t base = module_base();
            const bool alive = proc_alive();
            if (g_hook.installed && (!alive || !base))
            {
                remove_hook();
                g_last_base = 0;
                return;
            }
            if (g_hook.installed && base && g_last_base && base != g_last_base)
                remove_hook();
            if (base)
                g_last_base = base;
            if (!want)
            {
                if (g_hook.installed)
                    remove_hook();
                return;
            }
            if (!base || !alive)
                return;
            const auto now = std::chrono::steady_clock::now();
            if (g_hook.installed && g_hook.thunk)
            {
                if (g_last_slot_check.time_since_epoch().count() == 0 ||
                    now - g_last_slot_check >= std::chrono::milliseconds(250))
                {
                    g_last_slot_check = now;
                    if (g_proc->read<std::uintptr_t>(slot_addr(base)) != g_hook.thunk)
                    {
                        g_hook.installed = false;
                        g_hook.module_base = 0;
                    }
                }
            }
            if (!g_hook.installed)
                do_install();
        }
        void set_hook_active(bool on, const math::vec3& world_target, bool wallbang)
        {
            if (!on)
            {
                deactivate_state();
                g_hook.active = false;
                g_hook.wallbang = false;
                if (g_hook.installed)
                    set_status("installed: hook live, waiting for target");
                return;
            }
            if (!g_hook.installed || !g_hook.state)
                return;
#pragma pack(push, 4)
            struct Payload
            {
                std::uint32_t reserved;
                float target_x;
                float target_y;
                float target_z;
                float scale;
                std::uint64_t calls;
                float cam_x;
                float cam_y;
                float cam_z;
            };
#pragma pack(pop)
            Payload p{};
            p.reserved = wallbang ? 1u : 0u;
            p.target_x = world_target.x;
            p.target_y = world_target.y;
            p.target_z = world_target.z;
            p.scale = 1.15f;
            if (g_ctx.camera.addr && connected())
            {
                const math::vec3 cp = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
                if (std::isfinite(cp.x) && std::isfinite(cp.y) && std::isfinite(cp.z))
                {
                    p.cam_x = cp.x;
                    p.cam_y = cp.y;
                    p.cam_z = cp.z;
                }
            }
            w_mem(g_hook.state + offsetof(RaycastState, reserved), &p, sizeof(p));
            std::uint32_t one = 1;
            w_mem(g_hook.state + offsetof(RaycastState, active), &one, sizeof(one));
            g_hook.active = true;
            g_hook.wallbang = wallbang;
            set_status(wallbang ? "aiming: magic bullet (wallbang)" : "aiming: redirect active");
        }
        struct AimTarget
        {
            uintptr_t playerAddr = 0;
            uintptr_t characterAddr = 0;
            uintptr_t humanoidAddr = 0;
            uintptr_t targetPrim = 0;
            uintptr_t teamAddr = 0;
            bool isBot = false;
            float botHp = 100.0f;
        };
        std::chrono::steady_clock::time_point s_lastScan{};
        node resolve_part(const node& character, int partIdx)
        {
            node n;
            switch (partIdx)
            {
            case 0:
                n = character.find_child("Head");
                break;
            case 1:
                n = character.find_child("UpperTorso");
                if (!n) n = character.find_child("Torso");
                if (!n) n = character.find_child("HumanoidRootPart");
                break;
            case 2:
                n = character.find_child("HumanoidRootPart");
                if (!n) n = character.find_child("Torso");
                if (!n) n = character.find_child("UpperTorso");
                break;
            case 3:
                n = character.find_child("LeftUpperArm");
                if (!n) n = character.find_child("Left Arm");
                if (!n) n = character.find_child("LeftLowerArm");
                break;
            case 4:
                n = character.find_child("RightUpperArm");
                if (!n) n = character.find_child("Right Arm");
                if (!n) n = character.find_child("RightLowerArm");
                break;
            case 5:
                n = character.find_child("LeftUpperLeg");
                if (!n) n = character.find_child("Left Leg");
                if (!n) n = character.find_child("LeftLowerLeg");
                break;
            case 6:
                n = character.find_child("RightUpperLeg");
                if (!n) n = character.find_child("Right Leg");
                if (!n) n = character.find_child("RightLowerLeg");
                break;
            default:
                break;
            }
            if (!n)
                n = character.find_child("Head");
            if (!n)
                n = character.find_child("HumanoidRootPart");
            return n;
        }
        void resolve_targets(std::vector<AimTarget>& out)
        {
            out.clear();
            const int wantPart = settings::aim_part;
            const std::uintptr_t localAddr = g_ctx.local_player.addr;
            auto list = g_ctx.player_list();
            out.reserve(list.size());
            for (auto& pl : list)
            {
                if (!pl.addr || pl.addr == localAddr)
                    continue;
                const std::uintptr_t modelAddr =
                    g_proc->read<std::uintptr_t>(pl.addr + Offsets::Player::ModelInstance);
                if (!modelAddr)
                    continue;
                node character{ modelAddr };
                node humanoid = find_live_humanoid(character);
                if (!humanoid)
                    continue;
                node part = resolve_part(character, wantPart);
                if (!part)
                    continue;
                const std::uintptr_t prim = part.primitive();
                if (!prim)
                    continue;
                AimTarget t{};
                t.playerAddr = pl.addr;
                t.characterAddr = modelAddr;
                t.humanoidAddr = humanoid.addr;
                t.targetPrim = prim;
                t.teamAddr = g_proc->read<std::uintptr_t>(pl.addr + Offsets::Player::Team);
                out.push_back(t);
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
                    AimTarget t{};
                    t.playerAddr = b.inst;
                    t.characterAddr = b.inst;
                    t.humanoidAddr = b.humanoid;
                    t.targetPrim = prim;
                    t.teamAddr = 0;
                    t.isBot = true;
                    t.botHp = b.hp;
                    out.push_back(t);
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
    }
    void on_game_joined()
    {
        g_hook = {};
        g_last_base = 0;
        g_last_fail = std::chrono::steady_clock::time_point{};
        g_last_slot_check = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
        set_status("idle: hook not requested");
    }
    void shutdown()
    {
        ensure_hook(false);
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
            ensure_hook(true);
            set_hook_active(true, b.world, true);
            return;
        }
        std::vector<AimTarget> targets;
        resolve_targets(targets);
        if (targets.empty())
        {
            set_hook_active(false, math::vec3{}, false);
            thenwefuckin::aim::clear_debug_target();
            return;
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
        float bestDist = (wide || prio != 0 || thenwefuckin::aim::lock_active()) ? 1e9f : settings::aim_fov;
        math::vec3 lpPos = g_ctx.local_pos;
        if (lpPos.x == 0.0f && lpPos.y == 0.0f && lpPos.z == 0.0f)
            lpPos = camPos;
        uintptr_t bestPlayer = 0;
        for (auto& t : targets)
        {
            if (!thenwefuckin::aim::lock_matches(t.playerAddr, t.characterAddr))
                continue;
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
                    best.z += (tv.vz - lv.z) * leadT * damp;
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
        ensure_hook(true);
        set_hook_active(true, best, true);
    }
}
