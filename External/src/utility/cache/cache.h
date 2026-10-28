#pragma once
#pragma region sdk
#include <Windows.h>
#include <TlHelp32.h>
#include <atomic>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include "math.h"
#include "../struct/offsets.h"
#include "../struct/structs.h"
#include "../struct/world2screen.h"
#include "../memory/syscall.h"
namespace thenwefuckin
{
    class process
    {
    public:
        process() = default;
        ~process() { detach(); }
        process(const process&) = delete;
        process& operator=(const process&) = delete;
        bool attach(const std::string& exe_name)
        {
            uint32_t pid = find_pid(exe_name);
            if (!pid)
                return false;
            HANDLE h = OpenProcess(
                PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
                FALSE, pid);
            if (!h || h == INVALID_HANDLE_VALUE)
                return false;
            detach();
            handle = h;
            pid_ = pid;
            base_ = find_module(exe_name);
            return base_ != 0;
        }
        void detach()
        {
            if (handle && handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
            handle = nullptr;
            pid_ = 0;
            base_ = 0;
        }
        bool connected() const { return handle && handle != INVALID_HANDLE_VALUE; }
        bool alive() const
        {
            if (!connected())
                return false;
            DWORD code = 0;
            if (!GetExitCodeProcess(handle, &code))
                return false;
            return code == STILL_ACTIVE;
        }
        uint64_t base() const { return base_; }
        uint32_t pid() const { return pid_; }
        HANDLE raw() const { return handle; }
        template <typename T>
        T read(uint64_t addr) const
        {
            T v{};
            if (!connected())
                return v;
#ifdef _M_X64
            if (syscalls::ready())
            {
                unsigned long long done = 0;
                long st = Rax_ReadVirtualMemory(handle, reinterpret_cast<LPCVOID>(addr), &v, sizeof(T), &done);
                if (st < 0 || done != sizeof(T))
                    v = T{};
                return v;
            }
#endif
            ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(addr), &v, sizeof(T), nullptr);
            return v;
        }
        template <typename T>
        void write(uint64_t addr, const T& v) const
        {
            if (!connected())
                return;
#ifdef _M_X64
            if (syscalls::ready())
            {
                unsigned long long done = 0;
                Rax_WriteVirtualMemory(handle, reinterpret_cast<LPVOID>(addr), &v, sizeof(T), &done);
                return;
            }
#endif
            WriteProcessMemory(handle, reinterpret_cast<LPVOID>(addr), &v, sizeof(T), nullptr);
        }
        bool read_buf(uint64_t addr, void* dst, size_t size) const
        {
            if (!connected() || !dst || !size)
                return false;
#ifdef _M_X64
            if (syscalls::ready())
            {
                unsigned long long done = 0;
                long st = Rax_ReadVirtualMemory(handle, reinterpret_cast<LPCVOID>(addr), dst, size, &done);
                return st >= 0 && done == size;
            }
#endif
            SIZE_T done = 0;
            return ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(addr), dst, size, &done) && done == size;
        }
        bool write_buf(uint64_t addr, const void* src, size_t size) const
        {
            if (!connected() || !src || !size)
                return false;
#ifdef _M_X64
            if (syscalls::ready())
            {
                unsigned long long done = 0;
                long st = Rax_WriteVirtualMemory(handle, reinterpret_cast<LPVOID>(addr), src, size, &done);
                return st >= 0 && done == size;
            }
#endif
            SIZE_T done = 0;
            return WriteProcessMemory(handle, reinterpret_cast<LPVOID>(addr), src, size, &done) && done == size;
        }
        uint64_t alloc_remote(size_t size, DWORD prot) const
        {
            if (!connected() || !size)
                return 0;
            void* mem = VirtualAllocEx(handle, nullptr, size, MEM_COMMIT | MEM_RESERVE, prot);
            return reinterpret_cast<uint64_t>(mem);
        }
        bool protect_remote(uint64_t addr, size_t size, DWORD prot, DWORD* old) const
        {
            if (!connected() || !addr || !size)
                return false;
            DWORD tmp = 0;
            if (!VirtualProtectEx(handle, reinterpret_cast<LPVOID>(addr), size, prot, &tmp))
                return false;
            if (old)
                *old = tmp;
            return true;
        }
        bool free_remote(uint64_t addr) const
        {
            if (!connected() || !addr)
                return false;
            return VirtualFreeEx(handle, reinterpret_cast<LPVOID>(addr), 0, MEM_RELEASE) != FALSE;
        }
        std::string read_game_string(uint64_t addr) const
        {
            if (!addr || !connected())
                return {};
            int32_t len = read<int32_t>(addr + 0x10);
            if (len <= 0 || len > 256)
                return {};
            uint64_t ptr = (len >= 16) ? read<uint64_t>(addr) : addr;
            if (!ptr)
                return {};
            std::vector<char> buf(static_cast<size_t>(len) + 1, 0);
            if (!read_buf(ptr, buf.data(), static_cast<size_t>(len)))
                return {};
            return std::string(buf.data(), static_cast<size_t>(len));
        }
        void write_game_string(uint64_t addr, const std::string& value) const
        {
            if (!addr || !connected())
                return;
            uint64_t cap = read<uint64_t>(addr + 0x18);
            uint64_t len = static_cast<uint64_t>(value.size());
            if (len > cap)
            {
                void* mem = VirtualAllocEx(handle, nullptr, len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                if (!mem)
                    return;
                uint64_t dst = reinterpret_cast<uint64_t>(mem);
                write_buf(dst, value.data(), value.size());
                write<char>(dst + len, '\0');
                write<uint64_t>(addr, dst);
                write<uint64_t>(addr + 0x10, len);
                write<uint64_t>(addr + 0x18, len);
                return;
            }
            uint64_t dst = cap >= 16 ? read<uint64_t>(addr) : addr;
            if (!dst)
                return;
            if (!value.empty())
                write_buf(dst, value.data(), value.size());
            write<char>(dst + len, '\0');
            write<uint64_t>(addr + 0x10, len);
        }
    private:
        uint32_t find_pid(const std::string& exe_name)
        {
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snap == INVALID_HANDLE_VALUE)
                return 0;
            PROCESSENTRY32 entry{};
            entry.dwSize = sizeof(entry);
            uint32_t found = 0;
            if (Process32First(snap, &entry))
            {
                do
                {
                    char exeA[MAX_PATH]{};
                    WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, exeA, MAX_PATH, nullptr, nullptr);
                    if (_stricmp(exeA, exe_name.c_str()) == 0)
                    {
                        found = entry.th32ProcessID;
                        break;
                    }
                } while (Process32Next(snap, &entry));
            }
            CloseHandle(snap);
            return found;
        }
        uint64_t find_module(const std::string& exe_name)
        {
            if (!connected())
                return 0;
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid_);
            if (snap == INVALID_HANDLE_VALUE)
                return 0;
            MODULEENTRY32 entry{};
            entry.dwSize = sizeof(entry);
            uint64_t found = 0;
            if (Module32First(snap, &entry))
            {
                do
                {
                    char modA[MAX_PATH]{};
                    WideCharToMultiByte(CP_UTF8, 0, entry.szModule, -1, modA, MAX_PATH, nullptr, nullptr);
                    if (_stricmp(modA, exe_name.c_str()) == 0)
                    {
                        found = reinterpret_cast<uint64_t>(entry.modBaseAddr);
                        break;
                    }
                } while (Module32Next(snap, &entry));
            }
            CloseHandle(snap);
            return found;
        }
        HANDLE handle = nullptr;
        uint32_t pid_ = 0;
        uint64_t base_ = 0;
    };
    inline std::shared_ptr<process> g_proc = std::make_shared<process>();
    class node
    {
    public:
        uintptr_t addr = 0;
        node() = default;
        explicit node(uintptr_t a) : addr(a) {}
        explicit operator bool() const { return addr != 0; }
        std::string name() const
        {
            if (!addr)
                return {};
            uintptr_t container = g_proc->read<uintptr_t>(addr + Offsets::Instance::NameContainer);
            if (!container)
                return {};
            return g_proc->read_game_string(container + Offsets::Instance::Name);
        }
        std::string class_name() const
        {
            if (!addr)
                return {};
            uintptr_t desc = g_proc->read<uintptr_t>(addr + Offsets::Instance::ClassDescriptor);
            if (!desc)
                return {};
            uintptr_t ptr = g_proc->read<uintptr_t>(desc + Offsets::Instance::ClassName);
            return ptr ? g_proc->read_game_string(ptr) : std::string{};
        }
        node parent() const
        {
            return addr ? node(g_proc->read<uintptr_t>(addr + Offsets::Instance::Parent)) : node{};
        }
        std::vector<node> children() const
        {
            std::vector<node> out;
            if (!addr)
                return out;
            uintptr_t list = g_proc->read<uintptr_t>(addr + Offsets::Instance::ChildrenStart);
            if (!list)
                return out;
            uintptr_t end = g_proc->read<uintptr_t>(list + Offsets::Instance::ChildrenEnd);
            uintptr_t cur = g_proc->read<uintptr_t>(list);
            if (!cur || !end || end < cur)
                return out;
            size_t count = (size_t)(end - cur) / 0x10;
            if (count > 2048)
                count = 2048;
            if (!count)
                return out;
            size_t bytes = count * 0x10;
            if (bytes > 0x10000)
                return out;
            std::vector<uint8_t> buf(bytes);
            if (!g_proc->read_buf(cur, buf.data(), bytes))
                return out;
            out.reserve(count < 32 ? count : 32);
            const uintptr_t* w = (const uintptr_t*)buf.data();
            for (size_t n = 0; n < count * 2; n += 2)
            {
                if (w[n])
                    out.emplace_back(w[n]);
            }
            return out;
        }
        node find_child(const std::string& wanted) const
        {
            for (auto& c : children())
            {
                if (c.name() == wanted)
                    return c;
            }
            return {};
        }
        node find_child_of_class(const std::string& cls) const
        {
            for (auto& c : children())
            {
                if (c.class_name() == cls)
                    return c;
            }
            return {};
        }
        uintptr_t primitive() const
        {
            return addr ? g_proc->read<uintptr_t>(addr + Offsets::BasePart::Primitive) : 0;
        }
        math::vec3 position() const
        {
            uintptr_t prim = primitive();
            return prim ? g_proc->read<math::vec3>(prim + Offsets::Primitive::Position) : math::vec3{};
        }
        math::vec3 velocity() const
        {
            uintptr_t prim = primitive();
            return prim ? g_proc->read<math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity) : math::vec3{};
        }
        math::frame cframe() const
        {
            uintptr_t prim = primitive();
            return prim ? g_proc->read<math::frame>(prim + Offsets::Primitive::Rotation) : math::frame{};
        }
        float distance_to(const math::vec3& p) const
        {
            return position().dist_to(p);
        }
    };
    inline node find_live_humanoid(const node& character, float* outHp = nullptr, float* outMaxHp = nullptr)
    {
        node fallback{};
        float fallbackHp = 0.0f, fallbackMax = 0.0f;
        node best{};
        float bestHp = 0.0f, bestMax = 0.0f;
        float bestFrac = 2.0f;
        if (character && g_proc)
        {
            for (auto& c : character.children())
            {
                if (c.class_name() != "Humanoid")
                    continue;
                float hp = g_proc->read<float>(c.addr + Offsets::Humanoid::Health);
                float mx = g_proc->read<float>(c.addr + Offsets::Humanoid::MaxHealth);
                if (!fallback)
                {
                    fallback = c;
                    fallbackHp = hp;
                    fallbackMax = mx;
                }
                if (mx > 0.0f && hp > 0.0f)
                {
                    float frac = hp / mx;
                    if (frac < bestFrac)
                    {
                        bestFrac = frac;
                        best = c;
                        bestHp = hp;
                        bestMax = mx;
                    }
                }
            }
        }
        node pick = best ? best : fallback;
        if (outHp) *outHp = best ? bestHp : fallbackHp;
        if (outMaxHp) *outMaxHp = best ? bestMax : fallbackMax;
        return pick;
    }
    struct context
    {
        node datamodel;
        node workspace;
        node players;
        node local_player;
        node camera;
        node local_root;
        node local_humanoid;
        math::view_matrix view{};
        math::vec3 local_pos{};
        math::vec3 local_velocity{};
        bool ready = false;
        math::vec3 m_lastPos{};
        std::chrono::steady_clock::time_point m_lastSample{};
        bool m_haveSample = false;
        node m_cachedChar{};
        node m_cachedHum{};
        std::chrono::steady_clock::time_point m_lastCharCheck{};
        node get_character(const node& player_node) const
        {
            if (!player_node)
                return node{};
            uintptr_t charAddr = g_proc->read<uintptr_t>(player_node.addr + Offsets::Player::ModelInstance);
            if (charAddr)
            {
                node c{ charAddr };
                if (c.addr && (c.find_child_of_class("Humanoid") || c.find_child("Head") || c.find_child("HumanoidRootPart") || c.primitive()))
                    return c;
            }
            std::string name = player_node.name();
            if (name.empty() || !workspace)
                return node{};
            node direct = workspace.find_child(name);
            if (direct && (direct.find_child_of_class("Humanoid") || direct.find_child("Head") || direct.find_child("HumanoidRootPart")))
                return direct;
            static const char* const kFolderNames[] = {
                "PLAYERS", "Players", "Characters", "characters",
                "Entities", "entities", "Actors", "actors", "Units"
            };
            for (const char* fn : kFolderNames)
            {
                node folder = workspace.find_child(fn);
                if (folder)
                {
                    node inside = folder.find_child(name);
                    if (inside && (inside.find_child_of_class("Humanoid") || inside.find_child("Head") || inside.find_child("HumanoidRootPart")))
                        return inside;
                }
            }
            return node{};
        }
        bool resolve_services(uint64_t dmAddr)
        {
            datamodel = node(dmAddr);
            if (!datamodel)
                return false;
            workspace = node(g_proc->read<uintptr_t>(datamodel.addr + Offsets::DataModel::Workspace));
            if (!workspace)
                workspace = datamodel.find_child_of_class("Workspace");
            if (!workspace)
                workspace = datamodel.find_child("Workspace");
            players = datamodel.find_child_of_class("Players");
            if (!players)
                players = datamodel.find_child("Players");
            if (!players)
                players = datamodel.find_child("PlayerService");
            if (!players)
                return false;
            local_player = node(g_proc->read<uintptr_t>(players.addr + Offsets::Players::LocalPlayer));
            if (!local_player)
            {
                for (auto& p : players.children())
                {
                    if (p.find_child_of_class("PlayerScripts") || p.find_child_of_class("PlayerGui"))
                    {
                        local_player = p;
                        break;
                    }
                }
            }
            uintptr_t curCam = workspace ? g_proc->read<uintptr_t>(workspace.addr + Offsets::Workspace::CurrentCamera) : 0;
            camera = curCam ? node{ curCam } : (workspace ? workspace.find_child_of_class("Camera") : node{});
            local_root = node{};
            local_humanoid = node{};
            m_cachedChar = node{};
            m_cachedHum = node{};
            if (local_player)
            {
                node character = get_character(local_player);
                if (character)
                {
                    m_cachedChar = character;
                    local_root = character.find_child("HumanoidRootPart");
                    if (!local_root) local_root = character.find_child("Torso");
                    if (!local_root) local_root = character.find_child("UpperTorso");
                    if (!local_root) local_root = character.find_child("Head");
                    local_humanoid = find_live_humanoid(character);
                    m_cachedHum = local_humanoid;
                }
            }
            m_cachedDm = dmAddr;
            return true;
        }
        bool refresh()
        {
            local_velocity = math::vec3{};
            m_haveSample = false;
            if (!g_proc->connected())
            {
                ready = false;
                m_cachedDm = 0;
                m_cachedChar = node{};
                m_cachedHum = node{};
                return false;
            }
            uint64_t fake = g_proc->read<uint64_t>(g_proc->base() + Offsets::FakeDataModel::Pointer);
            if (!fake)
            {
                ready = false;
                m_cachedDm = 0;
                m_cachedChar = node{};
                m_cachedHum = node{};
                return false;
            }
            uint64_t dmAddr = g_proc->read<uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
            auto now = std::chrono::steady_clock::now();
            bool cacheValid = (dmAddr != 0 && dmAddr == m_cachedDm && datamodel && players);
            bool cacheStale = cacheValid &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastResolve).count() >= 250;
            if (!cacheValid || cacheStale)
            {
                m_lastResolve = now;
                if (!resolve_services(dmAddr))
                {
                    ready = false;
                    m_cachedDm = 0;
                    m_cachedChar = node{};
                    m_cachedHum = node{};
                    return false;
                }
            }
            uint64_t visual = g_proc->read<uint64_t>(g_proc->base() + Offsets::VisualEngine::Pointer);
            if (visual)
                view = g_proc->read<math::view_matrix>(visual + Offsets::VisualEngine::ViewMatrix);
            if (workspace)
            {
                uintptr_t curCam = g_proc->read<uintptr_t>(workspace.addr + Offsets::Workspace::CurrentCamera);
                if (curCam)
                    camera = node{ curCam };
                else if (!camera)
                    camera = workspace.find_child_of_class("Camera");
            }
            if (local_player)
            {
                uintptr_t mi = g_proc->read<uintptr_t>(local_player.addr + Offsets::Player::ModelInstance);
                bool needSlow = false;
                if (!mi || mi != m_cachedChar.addr || !m_cachedChar)
                    needSlow = true;
                else if (!m_cachedHum.addr)
                    needSlow = true;
                else if (!local_root.addr || !local_root.primitive())
                    needSlow = true;
                else
                {
                    float hh = g_proc->read<float>(m_cachedHum.addr + Offsets::Humanoid::Health);
                    if (!(hh > 0.0f))
                        needSlow = true;
                }
                if (needSlow && now - m_lastCharCheck >= std::chrono::milliseconds(250))
                {
                    m_lastCharCheck = now;
                    node character = get_character(local_player);
                    m_cachedChar = character;
                    if (character)
                    {
                        local_root = character.find_child("HumanoidRootPart");
                        if (!local_root) local_root = character.find_child("Torso");
                        if (!local_root) local_root = character.find_child("UpperTorso");
                        if (!local_root) local_root = character.find_child("Head");
                        local_humanoid = find_live_humanoid(character);
                    }
                    else
                    {
                        local_root = node{};
                        local_humanoid = node{};
                    }
                    m_cachedHum = local_humanoid;
                }
            }
            if (local_root)
            {
                local_pos = local_root.position();
                local_velocity = sample_velocity(local_root);
            }
            ready = (workspace && players && local_player);
            return ready;
        }
        node resolve_local_humanoid() const
        {
            if (!g_proc || !g_proc->connected() || !local_player)
                return {};
            node character = get_character(local_player);
            if (!character)
                return {};
            return find_live_humanoid(character);
        }
        node resolve_camera() const
        {
            if (!g_proc || !g_proc->connected())
                return {};
            if (workspace)
            {
                uintptr_t curCam = g_proc->read<uintptr_t>(workspace.addr + Offsets::Workspace::CurrentCamera);
                if (curCam)
                    return node{ curCam };
            }
            return camera;
        }
        uint64_t m_cachedDm = 0;
        std::chrono::steady_clock::time_point m_lastResolve{};
        void reset()
        {
            datamodel = node{};
            workspace = node{};
            players = node{};
            local_player = node{};
            camera = node{};
            local_root = node{};
            local_humanoid = node{};
            view = math::view_matrix{};
            local_pos = math::vec3{};
            local_velocity = math::vec3{};
            ready = false;
            m_lastPos = math::vec3{};
            m_lastSample = std::chrono::steady_clock::time_point{};
            m_haveSample = false;
            m_cachedChar = node{};
            m_cachedHum = node{};
            m_lastCharCheck = std::chrono::steady_clock::time_point{};
            m_cachedDm = 0;
            m_lastResolve = std::chrono::steady_clock::time_point{};
        }
        math::vec3 sample_velocity(const node& root)
        {
            math::vec3 v = root.velocity();
            bool sane = std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z)
                        && v.length() < 5000.0f;
            if (sane)
                return v;
            auto now = std::chrono::steady_clock::now();
            if (m_haveSample && now > m_lastSample)
            {
                float dt = std::chrono::duration<float>(now - m_lastSample).count();
                if (dt > 0.0005f && dt < 0.5f)
                    v = (local_pos - m_lastPos) / dt;
            }
            m_lastPos = local_pos;
            m_lastSample = now;
            m_haveSample = true;
            return v;
        }
        std::vector<node> player_list() const
        {
            if (!players)
                return {};
            return players.children();
        }
    };
    inline context g_ctx;
}
#pragma endregion
#pragma region pf_cache
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>
namespace PfCache
{
    inline constexpr std::uint64_t kPlaceId = 292439477ull;
    inline constexpr std::uintptr_t kTextLabelText = 0xBA0;
    inline constexpr std::uintptr_t kTextLabelTextColor3 = 0xEB8;
    inline constexpr std::uintptr_t kGuiObjectSize = 0x520;
    struct Color3
    {
        float r = 0.0f, g = 0.0f, b = 0.0f;
    };
    struct Udim
    {
        float scale = 0.0f;
        int offset = 0;
    };
    struct Udim2
    {
        Udim x{};
        Udim y{};
    };
    struct PfPlayer
    {
        std::uintptr_t modelAddr = 0;
        std::uintptr_t teamFolderAddr = 0;
        std::uintptr_t headAddr = 0;
        std::uintptr_t torsoAddr = 0;
        std::uintptr_t labelAddr = 0;
        std::uintptr_t healthBarAddr = 0;
        std::uintptr_t headPrim = 0;
        std::uintptr_t torsoPrim = 0;
        std::vector<std::uintptr_t> limbAddrs;
        std::string name;
        float health = 1.0f;
        bool isEnemy = true;
        bool colorValid = false;
        bool isValid = false;
    };
    inline std::vector<PfPlayer> players;
    inline std::uintptr_t localCharacterAddr = 0;
    inline std::uintptr_t localTeamFolderAddr = 0;
    inline std::uintptr_t workspacePlayersAddr = 0;
    inline thenwefuckin::math::vec3 localPos{};
    struct PfDebug
    {
        std::uint64_t placeIdRead = 0;
        int wsChildren = -1;
        int teamFolders = 0;
        int modelsScanned = 0;
        int modelsKept = 0;
        int modelsNoHeadTorso = 0;
        int modelsEmptyName = 0;
        int hpFrames = 0;
        int hpZero = 0;
        int hpAllZeroFix = 0;
        int enemies = 0;
        float hpSample = -1.0f;
        const char* failStage = "not run";
    };
    inline PfDebug dbg;
    inline std::chrono::steady_clock::time_point lastFull{};
    inline std::uint64_t verdictPlace = 0;
    inline std::uintptr_t verdictDm = 0;
    inline std::chrono::steady_clock::time_point verdictTime{};
    inline bool verdictPf = false;
    inline std::uintptr_t cachedWsPlayers = 0;
    inline std::uintptr_t cachedWsAddr = 0;
    inline std::vector<std::uintptr_t> wsScanKids;
    inline std::size_t wsScanPos = 0;
    inline std::uintptr_t wsScanWs = 0;
    inline bool wsScanDone = false;
    inline std::uint64_t ReadPlaceId()
    {
        if (!thenwefuckin::g_ctx.datamodel.addr)
            return 0;
        return thenwefuckin::g_proc->read<std::uint64_t>(
            thenwefuckin::g_ctx.datamodel.addr + Offsets::DataModel::PlaceId);
    }
    inline bool IsActivePlace()
    {
        return ReadPlaceId() == kPlaceId;
    }
    inline bool LooksLikePf(std::uintptr_t wsPlayers)
    {
        if (!wsPlayers || !thenwefuckin::g_proc)
            return false;
        thenwefuckin::node ws{ wsPlayers };
        for (auto& folder : ws.children())
        {
            if (folder.class_name() != "Folder")
                continue;
            for (auto& model : folder.children())
            {
                if (model.class_name() != "Model")
                    continue;
                for (auto& part : model.children())
                {
                    const std::string pc = part.class_name();
                    if (pc != "Part" && pc != "MeshPart")
                        continue;
                    for (auto& c : part.children())
                    {
                        if (c.class_name() == "BillboardGui")
                            return true;
                    }
                }
            }
        }
        return false;
    }
    inline std::uintptr_t FindWorkspacePlayersFolder(bool& done)
    {
        done = true;
        if (!thenwefuckin::g_ctx.workspace.addr)
            return 0;
        if (cachedWsPlayers && cachedWsAddr == thenwefuckin::g_ctx.workspace.addr)
        {
            std::uintptr_t par = thenwefuckin::g_proc->read<std::uintptr_t>(
                cachedWsPlayers + Offsets::Instance::Parent);
            if (par == thenwefuckin::g_ctx.workspace.addr)
                return cachedWsPlayers;
            cachedWsPlayers = 0;
            cachedWsAddr = 0;
        }
        if (wsScanWs != thenwefuckin::g_ctx.workspace.addr)
        {
            wsScanKids.clear();
            for (auto& c : thenwefuckin::g_ctx.workspace.children())
                wsScanKids.push_back(c.addr);
            wsScanPos = 0;
            wsScanWs = thenwefuckin::g_ctx.workspace.addr;
            wsScanDone = false;
        }
        if (wsScanDone)
            return 0;
        std::size_t n = 0;
        while (wsScanPos < wsScanKids.size() && n < 400)
        {
            thenwefuckin::node c{ wsScanKids[wsScanPos++] };
            ++n;
            if (c.class_name() != "Folder")
                continue;
            if (c.name() != "Players")
                continue;
            cachedWsPlayers = c.addr;
            cachedWsAddr = thenwefuckin::g_ctx.workspace.addr;
            wsScanDone = true;
            return c.addr;
        }
        if (wsScanPos >= wsScanKids.size())
        {
            wsScanDone = true;
            return 0;
        }
        done = false;
        return 0;
    }
    inline std::string ReadLabelText(std::uintptr_t labelAddr)
    {
        if (!labelAddr || !thenwefuckin::g_proc)
            return {};
        return thenwefuckin::g_proc->read_game_string(labelAddr + kTextLabelText);
    }
    inline bool ReadLabelColor(std::uintptr_t labelAddr, Color3& out)
    {
        if (!labelAddr || !thenwefuckin::g_proc)
            return false;
        out = thenwefuckin::g_proc->read<Color3>(labelAddr + kTextLabelTextColor3);
        if (!std::isfinite(out.r) || !std::isfinite(out.g) || !std::isfinite(out.b))
            return false;
        if (out.r < 0.0f || out.r > 1.0f || out.g < 0.0f || out.g > 1.0f || out.b < 0.0f || out.b > 1.0f)
            return false;
        return true;
    }
    inline bool IsEnemyLabelColor(const Color3& c)
    {
        const int r = (int)(c.r * 255.0f + 0.5f);
        const int g = (int)(c.g * 255.0f + 0.5f);
        const int b = (int)(c.b * 255.0f + 0.5f);
        return std::abs(r - 255) <= 8 && std::abs(g - 10) <= 12 && std::abs(b - 20) <= 12;
    }
    inline bool IsBotModelName(const std::string& n)
    {
        if (n == "MilitaryHitbox" || n == "ZombieHitbox" || n == "BTR")
            return true;
        if (n.find("Heli") != std::string::npos &&
            n.find("Rocket") == std::string::npos &&
            n.find("Crate") == std::string::npos)
            return true;
        return false;
    }
    inline std::uintptr_t FindBillboardLabel(std::uintptr_t modelAddr)
    {
        thenwefuckin::node model{ modelAddr };
        for (auto& part : model.children())
        {
            const std::string pc = part.class_name();
            if (pc != "Part" && pc != "MeshPart")
                continue;
            for (auto& c : part.children())
            {
                if (c.class_name() != "BillboardGui")
                    continue;
                for (auto& bc : c.children())
                {
                    if (bc.class_name() == "TextLabel")
                        return bc.addr;
                }
            }
        }
        return 0;
    }
    inline std::string ReadModelName(std::uintptr_t modelAddr)
    {
        const auto label = FindBillboardLabel(modelAddr);
        if (label)
            return ReadLabelText(label);
        return {};
    }
    inline std::uintptr_t FindHealthPercent(std::uintptr_t modelAddr)
    {
        thenwefuckin::node model{ modelAddr };
        for (auto& part : model.children())
        {
            const std::string pc = part.class_name();
            if (pc != "Part" && pc != "MeshPart")
                continue;
            for (auto& c : part.children())
            {
                if (c.class_name() != "BillboardGui")
                    continue;
                for (auto& bc : c.children())
                {
                    if (bc.class_name() != "TextLabel")
                        continue;
                    for (auto& f : bc.children())
                    {
                        if (f.class_name() != "Frame" || f.name() != "Health")
                            continue;
                        for (auto& pf : f.children())
                        {
                            if (pf.class_name() == "Frame" && pf.name() == "Percent")
                                return pf.addr;
                        }
                    }
                }
            }
        }
        return 0;
    }
    inline float ReadHealthFrac(std::uintptr_t percentAddr)
    {
        if (!percentAddr || !thenwefuckin::g_proc)
            return 1.0f;
        const auto sz = thenwefuckin::g_proc->read<Udim2>(percentAddr + kGuiObjectSize);
        float f = sz.x.scale;
        if (!std::isfinite(f) || f < 0.0f || f > 1.0f)
            return 1.0f;
        return f;
    }
    inline std::vector<std::uintptr_t> CollectLimbs(std::uintptr_t modelAddr, std::uintptr_t head, std::uintptr_t torso)
    {
        std::vector<std::uintptr_t> out;
        thenwefuckin::node model{ modelAddr };
        for (auto& part : model.children())
        {
            const std::string pc = part.class_name();
            if (pc != "Part" && pc != "MeshPart")
                continue;
            if (part.addr == head || part.addr == torso)
                continue;
            out.push_back(part.addr);
        }
        return out;
    }
    inline void ResolveCharacter(std::uintptr_t modelAddr, std::uintptr_t& outHead, std::uintptr_t& outTorso)
    {
        outHead = 0;
        outTorso = 0;
        thenwefuckin::node model{ modelAddr };
        for (auto& part : model.children())
        {
            const std::string pc = part.class_name();
            if (pc != "Part" && pc != "MeshPart")
                continue;
            bool isHead = false;
            bool isTorso = false;
            for (auto& c : part.children())
            {
                const std::string cc = c.class_name();
                if (cc == "BillboardGui")
                {
                    isHead = true;
                    break;
                }
                if (cc == "SpotLight")
                {
                    isTorso = true;
                    break;
                }
            }
            if (isHead && !outHead)
                outHead = part.addr;
            else if (isTorso && !outTorso)
                outTorso = part.addr;
        }
    }
    inline std::uintptr_t ResolveLocalCharacter(std::uintptr_t wsPlayers)
    {
        using namespace thenwefuckin;
        if (g_ctx.local_player.addr && g_proc)
        {
            const auto mi = g_proc->read<std::uintptr_t>(
                g_ctx.local_player.addr + Offsets::Player::ModelInstance);
            if (mi)
            {
                node parent{ g_proc->read<std::uintptr_t>(mi + Offsets::Instance::Parent) };
                if (parent.addr && parent.class_name() == "Folder")
                {
                    node grand{ g_proc->read<std::uintptr_t>(parent.addr + Offsets::Instance::Parent) };
                    if (grand.addr == wsPlayers)
                        return mi;
                }
                return mi;
            }
        }
        return 0;
    }
    inline int ScoreFolderEnemy(std::uintptr_t folderAddr)
    {
        int enemy = 0;
        int friendly = 0;
        thenwefuckin::node folder{ folderAddr };
        for (auto& model : folder.children())
        {
            if (model.class_name() != "Model")
                continue;
            const auto label = FindBillboardLabel(model.addr);
            Color3 col{};
            if (!ReadLabelColor(label, col))
                continue;
            if (IsEnemyLabelColor(col))
                ++enemy;
            else
                ++friendly;
        }
        return enemy - friendly;
    }
    inline bool IsTeammate(const PfPlayer& p)
    {
        if (p.colorValid)
            return !p.isEnemy;
        if (localTeamFolderAddr && p.teamFolderAddr)
            return p.teamFolderAddr == localTeamFolderAddr;
        return false;
    }
        inline void on_game_joined()
    {
        players.clear();
        localCharacterAddr = 0;
        localTeamFolderAddr = 0;
        workspacePlayersAddr = 0;
        localPos = thenwefuckin::math::vec3{};
        dbg = PfDebug{};
        lastFull = std::chrono::steady_clock::time_point{};
        verdictPlace = 0;
        verdictDm = 0;
        verdictTime = std::chrono::steady_clock::time_point{};
        verdictPf = false;
        cachedWsPlayers = 0;
        cachedWsAddr = 0;
        wsScanKids.clear();
        wsScanPos = 0;
        wsScanWs = 0;
        wsScanDone = false;
    }
    inline void RefreshFast()
    {
        using namespace thenwefuckin;        if (g_ctx.camera.addr && g_proc)
            localPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        int zeros = 0;
        for (auto& p : players)
        {
            if (!p.isValid)
                continue;
            if (p.healthBarAddr)
                p.health = ReadHealthFrac(p.healthBarAddr);
            if (p.health <= 0.0f)
                ++zeros;
        }
        if (!players.empty() && zeros == (int)players.size())
        {
            for (auto& p : players)
                p.health = 1.0f;
            dbg.hpAllZeroFix = 1;
        }
    }
    inline void update()
    {
        using namespace thenwefuckin;
        const auto now = std::chrono::steady_clock::now();
        if (lastFull != std::chrono::steady_clock::time_point{} &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFull).count() < 100)
        {
            RefreshFast();
            return;
        }
        const std::uint64_t quickPlace = ReadPlaceId();
        const std::uintptr_t quickDm = thenwefuckin::g_ctx.datamodel.addr;
        if (quickPlace != kPlaceId && quickPlace == verdictPlace && quickDm == verdictDm && !verdictPf &&
            verdictTime != std::chrono::steady_clock::time_point{} &&
            std::chrono::duration_cast<std::chrono::seconds>(now - verdictTime).count() < 30)
        {
            lastFull = now;
            RefreshFast();
            return;
        }
        lastFull = now;
        players.clear();
        localCharacterAddr = 0;
        localTeamFolderAddr = 0;
        workspacePlayersAddr = 0;
        dbg = PfDebug{};
        dbg.placeIdRead = quickPlace ? quickPlace : ReadPlaceId();
        if (!g_ctx.workspace.addr)
        {
            dbg.failStage = "no workspace";
            verdictPlace = dbg.placeIdRead;
            verdictDm = thenwefuckin::g_ctx.datamodel.addr;
            verdictTime = now;
            verdictPf = false;
            return;
        }
        bool folderDone = false;
        const auto wsPlayers = FindWorkspacePlayersFolder(folderDone);
        if (!wsPlayers)
        {
            if (!folderDone)
            {
                dbg.failStage = "scanning Workspace";
                return;
            }
            dbg.failStage = "no Workspace.Players folder";
            verdictPlace = dbg.placeIdRead;
            verdictDm = thenwefuckin::g_ctx.datamodel.addr;
            verdictTime = now;
            verdictPf = false;
            return;
        }
        workspacePlayersAddr = wsPlayers;
        if (dbg.placeIdRead != kPlaceId && !LooksLikePf(wsPlayers))
        {
            dbg.failStage = "placeid mismatch + no pf structure";
            verdictPlace = dbg.placeIdRead;
            verdictDm = thenwefuckin::g_ctx.datamodel.addr;
            verdictTime = now;
            verdictPf = false;
            return;
        }
        std::string localName;
        if (g_ctx.local_player.addr)
            localName = g_ctx.local_player.name();
        localCharacterAddr = ResolveLocalCharacter(wsPlayers);
        dbg.wsChildren = (int)node{ wsPlayers }.children().size();
        std::uintptr_t folders[2]{};
        int nf = 0;
        node ws{ wsPlayers };
        for (auto& folder : ws.children())
        {
            if (folder.class_name() != "Folder")
                continue;
            if (nf < 2)
                folders[nf++] = folder.addr;
            dbg.teamFolders++;
        }
        if (nf == 2)
        {
            const int s0 = ScoreFolderEnemy(folders[0]);
            const int s1 = ScoreFolderEnemy(folders[1]);
            if (s0 > 0 && s1 <= 0)
                localTeamFolderAddr = folders[1];
            else if (s1 > 0 && s0 <= 0)
                localTeamFolderAddr = folders[0];
            else if (s0 < 0 && s1 >= 0)
                localTeamFolderAddr = folders[0];
            else if (s1 < 0 && s0 >= 0)
                localTeamFolderAddr = folders[1];
        }
        if (!localTeamFolderAddr && localCharacterAddr && g_proc)
        {
            node parent{ g_proc->read<std::uintptr_t>(localCharacterAddr + Offsets::Instance::Parent) };
            node grand{ parent.addr ? g_proc->read<std::uintptr_t>(parent.addr + Offsets::Instance::Parent) : 0 };
            if (grand.addr == wsPlayers)
                localTeamFolderAddr = parent.addr;
        }
        for (auto& folder : ws.children())
        {
            if (folder.class_name() != "Folder")
                continue;
            for (auto& model : folder.children())
            {
                if (model.class_name() != "Model")
                    continue;
                dbg.modelsScanned++;
                if (model.addr == localCharacterAddr)
                    continue;

                if (IsBotModelName(model.name()))
                    continue;
                std::uintptr_t head = 0, torso = 0;
                ResolveCharacter(model.addr, head, torso);
                if (!head && !torso)
                {
                    dbg.modelsNoHeadTorso++;
                    continue;
                }
                const auto label = FindBillboardLabel(model.addr);
                Color3 col{};
                const bool colOk = ReadLabelColor(label, col);
                const auto percent = FindHealthPercent(model.addr);
                const float hp = ReadHealthFrac(percent);
                if (percent)
                    dbg.hpFrames++;
                if (hp <= 0.0f)
                    dbg.hpZero++;
                if (dbg.hpSample < 0.0f && percent)
                    dbg.hpSample = hp;
                if (percent && hp <= 0.0f)
                    continue;
                PfPlayer p{};
                p.modelAddr = model.addr;
                p.teamFolderAddr = folder.addr;
                p.headAddr = head;
                p.torsoAddr = torso;
                if (head)
                    p.headPrim = thenwefuckin::node{ head }.primitive();
                if (torso)
                    p.torsoPrim = thenwefuckin::node{ torso }.primitive();
                p.labelAddr = label;
                p.healthBarAddr = percent;
                p.limbAddrs = CollectLimbs(model.addr, head, torso);
                p.health = hp;
                p.colorValid = colOk;
                p.isEnemy = colOk ? IsEnemyLabelColor(col) : true;
                if (p.isEnemy)
                    dbg.enemies++;
                p.name = ReadModelName(model.addr);
                if (p.name.empty())
                    p.name = model.name();
                if (p.name.empty())
                {
                    dbg.modelsEmptyName++;
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "pf_%llx", (unsigned long long)model.addr);
                    p.name = buf;
                }
                if (!localName.empty() && p.name == localName)
                    continue;
                p.isValid = true;
                players.push_back(std::move(p));
                dbg.modelsKept++;
            }
        }
        int zeros = 0;
        for (auto& p : players)
        {
            if (p.health <= 0.0f)
                ++zeros;
        }
        if (!players.empty() && zeros == (int)players.size())
        {
            for (auto& p : players)
                p.health = 1.0f;
            dbg.hpAllZeroFix = 1;
        }
        dbg.failStage = players.empty() ? "ok but empty" : "ok";
        verdictPlace = dbg.placeIdRead;
        verdictDm = thenwefuckin::g_ctx.datamodel.addr;
        verdictTime = std::chrono::steady_clock::now();
        verdictPf = true;
        if (g_ctx.camera.addr && g_proc)
            localPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
    }
}
#pragma endregion
#pragma region visual_cache
#include <cstdint>
#include <string>
#include <vector>
#include "math.h"
#include "../struct/structs.h"
namespace thenwefuckin::visual
{
    struct LimbCache
    {
        uintptr_t characterAddr = 0;
        uintptr_t humanoidAddr = 0;
        bool isR6 = false;
        uintptr_t rootPrim = 0;
        uintptr_t headPrim = 0;
        uintptr_t torsoPrim = 0;
        uintptr_t leftArmPrim = 0;
        uintptr_t rightArmPrim = 0;
        uintptr_t leftLegPrim = 0;
        uintptr_t rightLegPrim = 0;
        uintptr_t upperPrim = 0;
        uintptr_t lowerPrim = 0;
        uintptr_t leftUpperArmPrim = 0;
        uintptr_t leftLowerArmPrim = 0;
        uintptr_t leftHandPrim = 0;
        uintptr_t rightUpperArmPrim = 0;
        uintptr_t rightLowerArmPrim = 0;
        uintptr_t rightHandPrim = 0;
        uintptr_t leftUpperLegPrim = 0;
        uintptr_t leftLowerLegPrim = 0;
        uintptr_t leftFootPrim = 0;
        uintptr_t rightUpperLegPrim = 0;
        uintptr_t rightLowerLegPrim = 0;
        uintptr_t rightFootPrim = 0;
    };
    struct WorldCache
    {
        bool valid = false;
        uintptr_t playerAddr = 0;
        math::vec3 head{};
        math::vec3 root{};
        math::vec3 torso{};
        math::vec3 upper{};
        math::vec3 lower{};
        math::vec3 leftArm{};
        math::vec3 rightArm{};
        math::vec3 leftLeg{};
        math::vec3 rightLeg{};
        math::vec3 leftUpperArm{};
        math::vec3 leftLowerArm{};
        math::vec3 leftHand{};
        math::vec3 rightUpperArm{};
        math::vec3 rightLowerArm{};
        math::vec3 rightHand{};
        math::vec3 leftUpperLeg{};
        math::vec3 leftLowerLeg{};
        math::vec3 leftFoot{};
        math::vec3 rightUpperLeg{};
        math::vec3 rightLowerLeg{};
        math::vec3 rightFoot{};
        bool isR6 = false;
        bool hasLower = false;
        std::string name{};
        std::string tool{};
        std::string team{};
        std::string state{};
        float hp = 0.f;
        float maxHp = 100.f;
        data::rect rc3d{};
        bool hasRc3d = false;
        std::vector<math::vec3> pts3d;
        bool hasPts3d = false;
        math::frame torsoFrame{};
        bool hasTorsoFrame = false;
        LimbCache lc{};
    };
}
#pragma endregion
#pragma region preview_cache
#include <cstdint>
#include <string>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
namespace thenwefuckin::visual::preview
{
    struct V3 { float x, y, z; };
    struct V2 { float x, y; };
    struct GpuVertex { V3 p; V3 n; V2 t; };
    struct FrameConsts
    {
        DirectX::XMMATRIX w;
        DirectX::XMMATRIX v;
        DirectX::XMMATRIX pr;
        DirectX::XMFLOAT4 tint;
        float textured;
        float pad[3];
    };
    struct Surface
    {
        std::string tag;
        V3 base{ 1, 1, 1 };
        std::string tex;
    };
    struct BakedMesh
    {
        std::string tag;
        std::vector<GpuVertex> verts;
        std::vector<uint32_t> idx;
    };
    struct LiveMesh
    {
        ID3D11Buffer* vb = nullptr;
        ID3D11Buffer* ib = nullptr;
        uint32_t count = 0;
        std::string tag;
        DirectX::XMFLOAT4 tint{ 1, 1, 1, 1 };
        ID3D11ShaderResourceView* srv = nullptr;
    };
    struct DecodedTex
    {
        int w = 0, h = 0;
        std::vector<unsigned char> px;
    };
}
#pragma endregion
#pragma region player_cache
#include "../../menu/overlay/settings.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <algorithm>
namespace thenwefuckin::pcache
{
    struct Entry
    {
        std::uintptr_t player = 0;
        std::uintptr_t model = 0;
        std::uintptr_t humanoid = 0;
        std::uintptr_t team = 0;
        visual::LimbCache lc{};
        float hp = 0.0f;
        float maxHp = 100.0f;
        std::string name;
        std::string teamStr;
        std::string tool;
        bool valid = false;
        bool isR6 = true;
    };
    inline std::vector<Entry> entries;
    inline std::unordered_map<std::uintptr_t, visual::LimbCache> limbMap;
    inline std::mutex mtx;
    inline std::chrono::steady_clock::time_point lastFast{};
    inline std::chrono::steady_clock::time_point lastTopo{};
    inline std::chrono::steady_clock::time_point lastMeta{};
    inline int topoTick = 0;
    inline std::uintptr_t aim_prim(const visual::LimbCache& lc, int part)
    {
        switch (part)
        {
        case 0:
            return lc.headPrim;
        case 1:
            if (lc.upperPrim)
                return lc.upperPrim;
            if (lc.torsoPrim)
                return lc.torsoPrim;
            return lc.rootPrim;
        case 2:
            return lc.rootPrim ? lc.rootPrim : lc.headPrim;
        case 3:
            if (lc.isR6)
                return lc.leftArmPrim;
            if (lc.leftUpperArmPrim)
                return lc.leftUpperArmPrim;
            if (lc.leftLowerArmPrim)
                return lc.leftLowerArmPrim;
            return lc.leftHandPrim;
        case 4:
            if (lc.isR6)
                return lc.rightArmPrim;
            if (lc.rightUpperArmPrim)
                return lc.rightUpperArmPrim;
            if (lc.rightLowerArmPrim)
                return lc.rightLowerArmPrim;
            return lc.rightHandPrim;
        case 5:
            if (lc.isR6)
                return lc.leftLegPrim;
            if (lc.leftUpperLegPrim)
                return lc.leftUpperLegPrim;
            if (lc.leftLowerLegPrim)
                return lc.leftLowerLegPrim;
            return lc.leftFootPrim;
        case 6:
            if (lc.isR6)
                return lc.rightLegPrim;
            if (lc.rightUpperLegPrim)
                return lc.rightUpperLegPrim;
            if (lc.rightLowerLegPrim)
                return lc.rightLowerLegPrim;
            return lc.rightFootPrim;
        default:
            return lc.headPrim;
        }
    }
    inline std::uintptr_t prim_of(node c, const std::string& n)
    {
        node limb = c.find_child(n);
        if (!limb)
            return 0;
        return limb.primitive();
    }
    inline visual::LimbCache build_limbs(node character)
    {
        visual::LimbCache lc{};
        if (!character)
            return lc;
        lc.characterAddr = character.addr;
        node humanoid = find_live_humanoid(character);
        lc.humanoidAddr = humanoid.addr;
        node root = character.find_child("HumanoidRootPart");
        if (!root)
            root = character.find_child("Torso");
        if (!root)
            root = character.find_child("UpperTorso");
        lc.rootPrim = root ? root.primitive() : 0;
        lc.headPrim = prim_of(character, "Head");
        if (lc.rootPrim == 0 && lc.headPrim != 0)
            lc.rootPrim = lc.headPrim;
        int32_t rig = humanoid ? g_proc->read<int32_t>(humanoid.addr + Offsets::Humanoid::RigType) : -1;
        node torso = character.find_child("Torso");
        bool isR6 = false;
        if (rig == 0)
            isR6 = true;
        else if (rig == 1)
            isR6 = false;
        else
            isR6 = (bool)torso;
        lc.isR6 = isR6;
        if (isR6)
        {
            lc.torsoPrim = torso ? torso.primitive() : 0;
            lc.leftArmPrim = prim_of(character, "Left Arm");
            lc.rightArmPrim = prim_of(character, "Right Arm");
            lc.leftLegPrim = prim_of(character, "Left Leg");
            lc.rightLegPrim = prim_of(character, "Right Leg");
        }
        else
        {
            lc.upperPrim = prim_of(character, "UpperTorso");
            lc.lowerPrim = prim_of(character, "LowerTorso");
            lc.leftUpperArmPrim = prim_of(character, "LeftUpperArm");
            lc.leftLowerArmPrim = prim_of(character, "LeftLowerArm");
            lc.leftHandPrim = prim_of(character, "LeftHand");
            lc.rightUpperArmPrim = prim_of(character, "RightUpperArm");
            lc.rightLowerArmPrim = prim_of(character, "RightLowerArm");
            lc.rightHandPrim = prim_of(character, "RightHand");
            lc.leftUpperLegPrim = prim_of(character, "LeftUpperLeg");
            lc.leftLowerLegPrim = prim_of(character, "LeftLowerLeg");
            lc.leftFootPrim = prim_of(character, "LeftFoot");
            lc.rightUpperLegPrim = prim_of(character, "RightUpperLeg");
            lc.rightLowerLegPrim = prim_of(character, "RightLowerLeg");
            lc.rightFootPrim = prim_of(character, "RightFoot");
            if (lc.upperPrim == 0 && lc.lowerPrim == 0)
            {
                node t2 = character.find_child("Torso");
                if (t2)
                {
                    lc.isR6 = true;
                    lc.torsoPrim = t2.primitive();
                }
            }
        }
        return lc;
    }
    inline void clear()
    {
        std::lock_guard<std::mutex> lk(mtx);
        entries.clear();
        limbMap.clear();
        lastFast = std::chrono::steady_clock::time_point{};
        lastTopo = std::chrono::steady_clock::time_point{};
        lastMeta = std::chrono::steady_clock::time_point{};
        topoTick = 0;
    }
    inline void update()
    {
        using namespace thenwefuckin;
        if (!g_proc || !g_proc->connected() || !g_ctx.players || !g_ctx.local_player)
        {
            std::lock_guard<std::mutex> lk(mtx);
            if (!entries.empty())
                entries.clear();
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        if (lastFast != std::chrono::steady_clock::time_point{} &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFast).count() < 16)
            return;
        lastFast = now;
        {
            std::lock_guard<std::mutex> lk(mtx);
            for (auto& e : entries)
            {
                if (!e.valid)
                    continue;
                std::uintptr_t par = g_proc->read<std::uintptr_t>(e.model + Offsets::Instance::Parent);
                if (!par)
                {
                    e.valid = false;
                    continue;
                }
                if (e.humanoid)
                {
                    float hp = g_proc->read<float>(e.humanoid + Offsets::Humanoid::Health);
                    float mx = g_proc->read<float>(e.humanoid + Offsets::Humanoid::MaxHealth);
                    e.hp = hp;
                    if (mx > 0.0f)
                        e.maxHp = mx;
                    if (hp <= 0.0f)
                    {
                        e.valid = false;
                        continue;
                    }
                }
            }
            entries.erase(std::remove_if(entries.begin(), entries.end(),
                [](const Entry& e) { return !e.valid; }), entries.end());
        }
        if (lastTopo != std::chrono::steady_clock::time_point{} &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTopo).count() < 500)
            return;
        lastTopo = now;
        ++topoTick;
        bool rescanMeta = (topoTick % 4) == 0;
        if (lastMeta == std::chrono::steady_clock::time_point{})
            rescanMeta = true;
        std::uintptr_t localAddr = g_ctx.local_player.addr;
        auto plist = g_ctx.player_list();
        {
            std::lock_guard<std::mutex> lk(mtx);
            for (auto& e : entries)
                e.valid = false;
        }
        int freshBudget = 10;
        for (auto& pl : plist)
        {
            if (!pl.addr || pl.addr == localAddr)
                continue;
            Entry* slot = nullptr;
            {
                std::lock_guard<std::mutex> lk(mtx);
                for (auto& e : entries)
                {
                    if (e.player == pl.addr)
                    {
                        slot = &e;
                        break;
                    }
                }
            }
            std::uintptr_t modelAddr = g_proc->read<std::uintptr_t>(pl.addr + Offsets::Player::ModelInstance);
            if (!modelAddr)
            {
                if (!g_ctx.workspace)
                    continue;
                std::string pname = pl.name();
                if (pname.empty())
                    continue;
                node direct = g_ctx.workspace.find_child(pname);
                if (direct.addr)
                    modelAddr = direct.addr;
            }
            if (!modelAddr)
                continue;
            std::uintptr_t mpar = g_proc->read<std::uintptr_t>(modelAddr + Offsets::Instance::Parent);
            if (!mpar)
                continue;
            if (slot && slot->model == modelAddr && slot->lc.headPrim)
            {
                std::uintptr_t rp = g_proc->read<std::uintptr_t>(slot->lc.headPrim + Offsets::Instance::Parent);
                if (rp)
                {
                    const auto teamNow = g_proc->read<std::uintptr_t>(pl.addr + Offsets::Player::Team);
                    if (rescanMeta && teamNow != slot->team)
                        slot->teamStr = teamNow ? node{ teamNow }.name() : std::string{};
                    slot->team = teamNow;
                    if (slot->humanoid)
                    {
                        float hp = g_proc->read<float>(slot->humanoid + Offsets::Humanoid::Health);
                        float mx = g_proc->read<float>(slot->humanoid + Offsets::Humanoid::MaxHealth);
                        slot->hp = hp;
                        if (mx > 0.0f)
                            slot->maxHp = mx;
                        if (hp <= 0.0f)
                            continue;
                    }
                    slot->valid = true;
                    continue;
                }
            }
            if (freshBudget <= 0)
                continue;
            --freshBudget;
            node character{ modelAddr };
            visual::LimbCache lc = build_limbs(character);
            if (!lc.headPrim)
                continue;
            float hp = 100.0f, mx = 100.0f;
            if (lc.humanoidAddr)
            {
                hp = g_proc->read<float>(lc.humanoidAddr + Offsets::Humanoid::Health);
                mx = g_proc->read<float>(lc.humanoidAddr + Offsets::Humanoid::MaxHealth);
                if (mx <= 0.0f)
                    mx = 100.0f;
                if (hp <= 0.0f)
                    continue;
            }
            std::uintptr_t team = g_proc->read<std::uintptr_t>(pl.addr + Offsets::Player::Team);
            Entry c{};
            c.player = pl.addr;
            c.model = modelAddr;
            c.humanoid = lc.humanoidAddr;
            c.team = team;
            c.lc = lc;
            c.hp = hp;
            c.maxHp = mx;
            c.name = pl.name();
            c.teamStr = team ? node{ team }.name() : std::string{};
            c.isR6 = lc.isR6;
            if (settings::tool_enabled)
            {
                for (auto& child : character.children())
                {
                    if (child.class_name() == "Tool")
                    {
                        c.tool = child.name();
                        break;
                    }
                }
            }
            c.valid = true;
            {
                std::lock_guard<std::mutex> lk(mtx);
                limbMap[modelAddr] = lc;
                if (slot)
                    *slot = std::move(c);
                else
                    entries.push_back(std::move(c));
            }
        }
        if (rescanMeta)
        {
            lastMeta = now;
            const bool wantTool = settings::tool_enabled;
            std::lock_guard<std::mutex> lk(mtx);
            for (auto& e : entries)
            {
                if (!e.valid)
                    continue;
                std::uintptr_t t = g_proc->read<std::uintptr_t>(e.player + Offsets::Player::Team);
                if (t != e.team)
                {
                    e.team = t;
                    e.teamStr = t ? node{ t }.name() : std::string{};
                }
                if (wantTool)
                {
                    node ch{ e.model };
                    e.tool.clear();
                    for (auto& child : ch.children())
                    {
                        if (child.class_name() == "Tool")
                        {
                            e.tool = child.name();
                            break;
                        }
                    }
                }
            }
        }
        {
            std::lock_guard<std::mutex> lk(mtx);
            entries.erase(std::remove_if(entries.begin(), entries.end(),
                [](const Entry& e) { return !e.valid; }), entries.end());
            if (limbMap.size() > 256)
            {
                std::unordered_set<std::uintptr_t> aliveModels;
                for (auto& e : entries)
                    aliveModels.insert(e.model);
                for (auto it = limbMap.begin(); it != limbMap.end();)
                {
                    if (aliveModels.find(it->first) == aliveModels.end())
                        it = limbMap.erase(it);
                    else
                        ++it;
                }
            }
        }
    }
}
#pragma endregion
#pragma region world_cache

namespace thenwefuckin::visual::wcache
{
    struct DynRectState { data::rect rc; std::chrono::steady_clock::time_point t{}; };
    inline std::vector<WorldCache> entries;
    inline math::view_matrix view{};
    inline bool viewValid = false;
    inline math::vec3 camPos{};
    inline std::unordered_map<uintptr_t, DynRectState> dynRect;
    inline std::unordered_map<uintptr_t, std::string> teamNames;
    inline std::mutex mtx;
    inline bool pfMode = false;
    inline std::chrono::steady_clock::time_point pfCheck{};
    inline math::vec3 prevCamFwd{};
    inline bool havePrevCamFwd = false;
    inline uintptr_t getPrim(node c, const std::string& n)
    {
        node limb = c.find_child(n);
        if (!limb) return 0;
        return limb.primitive();
    }
    inline math::vec3 getPos(uintptr_t prim)
    {
        if (!prim) return {};
        return g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
    }
    inline math::frame getFrame(uintptr_t prim)
    {
        if (!prim) return {};
        return g_proc->read<math::frame>(prim + Offsets::Primitive::Rotation);
    }
    inline math::view_matrix fetchView()
    {
        uint64_t vis = g_proc->read<uint64_t>(g_proc->base() + Offsets::VisualEngine::Pointer);
        if (!vis) return g_ctx.view;
        return g_proc->read<math::view_matrix>(vis + Offsets::VisualEngine::ViewMatrix);
    }
    inline void addJoint(std::vector<math::vec3>& out, const math::vec3& p, const math::vec3& root, float rx, float ry, float rz)
    {
        if (p.x == 0.0f && p.y == 0.0f && p.z == 0.0f) return;
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) return;
        if ((p - root).length() > 25.0f) return;
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2)
                    out.push_back({ p.x + x * rx, p.y + y * ry, p.z + z * rz });
    }
    inline void collectBoxPoints(const WorldCache& wc, std::vector<math::vec3>& pts)
    {
        if (wc.root.x == 0 && wc.root.y == 0 && wc.root.z == 0) return;
        pts.reserve(128);
        addJoint(pts, wc.root, wc.root, 1.0f, 1.0f, 0.6f);
        if (wc.isR6)
        {
            addJoint(pts, wc.head, wc.root, 1.1f, 0.7f, 1.1f);
            const math::vec3 torso = (wc.torso.x != 0.0f || wc.torso.y != 0.0f || wc.torso.z != 0.0f) ? wc.torso : wc.root;
            addJoint(pts, torso, wc.root, 1.1f, 1.1f, 0.7f);
            addJoint(pts, wc.leftArm, wc.root, 0.7f, 1.1f, 0.7f);
            addJoint(pts, wc.rightArm, wc.root, 0.7f, 1.1f, 0.7f);
            addJoint(pts, wc.leftLeg, wc.root, 0.7f, 1.1f, 0.7f);
            addJoint(pts, wc.rightLeg, wc.root, 0.7f, 1.1f, 0.7f);
        }
        else
        {
            addJoint(pts, wc.head, wc.root, 0.8f, 0.7f, 0.8f);
            const math::vec3 upper = (wc.upper.x != 0.0f || wc.upper.y != 0.0f || wc.upper.z != 0.0f) ? wc.upper : wc.torso;
            addJoint(pts, upper, wc.root, 0.9f, 0.7f, 0.5f);
            addJoint(pts, wc.lower, wc.root, 0.9f, 0.7f, 0.5f);
            addJoint(pts, wc.leftUpperArm, wc.root, 0.4f, 0.5f, 0.4f);
            addJoint(pts, wc.leftLowerArm, wc.root, 0.35f, 0.45f, 0.35f);
            addJoint(pts, wc.leftHand, wc.root, 0.35f, 0.35f, 0.35f);
            addJoint(pts, wc.rightUpperArm, wc.root, 0.4f, 0.5f, 0.4f);
            addJoint(pts, wc.rightLowerArm, wc.root, 0.35f, 0.45f, 0.35f);
            addJoint(pts, wc.rightHand, wc.root, 0.35f, 0.35f, 0.35f);
            addJoint(pts, wc.leftUpperLeg, wc.root, 0.4f, 0.6f, 0.4f);
            addJoint(pts, wc.leftLowerLeg, wc.root, 0.35f, 0.55f, 0.35f);
            addJoint(pts, wc.leftFoot, wc.root, 0.35f, 0.3f, 0.5f);
            addJoint(pts, wc.rightUpperLeg, wc.root, 0.4f, 0.6f, 0.4f);
            addJoint(pts, wc.rightLowerLeg, wc.root, 0.35f, 0.55f, 0.35f);
            addJoint(pts, wc.rightFoot, wc.root, 0.35f, 0.3f, 0.5f);
        }
    }
    inline void build_pf_world(std::vector<WorldCache>& out, float fw, float fh)
    {
        (void)fw; (void)fh;
        out.clear();
        if (PfCache::players.empty())
            return;
        bool needSkel = settings::skeleton_enabled;
        bool need3d = (settings::box_mode == 1);
        bool needLimbs = needSkel || need3d;
        for (auto& pp : PfCache::players)
        {
            if (!pp.isValid)
                continue;
            if (pp.modelAddr == PfCache::localCharacterAddr)
                continue;
            if (pp.health <= 0.0f)
                continue;
            if (settings::vis_team_check && PfCache::IsTeammate(pp))
                continue;
            WorldCache wc{};
            wc.playerAddr = pp.modelAddr;
            node headNode{ pp.headAddr };
            node torsoNode{ pp.torsoAddr };
            uintptr_t headPrim = headNode ? headNode.primitive() : 0;
            uintptr_t torsoPrim = torsoNode ? torsoNode.primitive() : 0;
            if (!headPrim)
                continue;
            wc.head = getPos(headPrim);
            if (wc.head.x == 0 && wc.head.y == 0 && wc.head.z == 0)
                continue;
            wc.root = torsoPrim ? getPos(torsoPrim) : math::vec3{};
            if ((wc.root.x == 0 && wc.root.y == 0 && wc.root.z == 0) || (wc.head - wc.root).length() > 25.0f)
                wc.root = math::vec3{ wc.head.x, wc.head.y - 2.5f, wc.head.z };
            wc.hp = pp.health * 100.0f;
            wc.maxHp = 100.0f;
            wc.name = pp.name;
            if (settings::flags_enabled && settings::flags_team && pp.teamFolderAddr)
            {
                std::lock_guard<std::mutex> lk(mtx);
                auto tit = teamNames.find(pp.teamFolderAddr);
                if (tit != teamNames.end())
                    wc.team = tit->second;
                else
                {
                    wc.team = node{ pp.teamFolderAddr }.name();
                    teamNames[pp.teamFolderAddr] = wc.team;
                }
            }
            wc.valid = true;
            wc.isR6 = true;
            LimbCache lc{};
            lc.characterAddr = pp.modelAddr;
            lc.isR6 = true;
            lc.headPrim = headPrim;
            lc.rootPrim = torsoPrim ? torsoPrim : headPrim;
            lc.torsoPrim = torsoPrim;
            auto safePos = [&](uintptr_t prim, float maxDist) -> math::vec3 {
                if (!prim) return {};
                math::vec3 p = getPos(prim);
                if (p.x == 0 && p.y == 0 && p.z == 0) return {};
                if ((p - wc.root).length() > maxDist) return {};
                return p;
            };
            if (needLimbs)
            {
                std::vector<uintptr_t> lprims;
                lprims.reserve(pp.limbAddrs.size());
                for (auto la : pp.limbAddrs)
                {
                    node ln{ la };
                    uintptr_t prim = ln ? ln.primitive() : 0;
                    if (prim) lprims.push_back(prim);
                }
                uintptr_t laArm = lprims.size() > 0 ? lprims[0] : 0;
                uintptr_t raArm = lprims.size() > 1 ? lprims[1] : 0;
                uintptr_t laLeg = lprims.size() > 2 ? lprims[2] : 0;
                uintptr_t raLeg = lprims.size() > 3 ? lprims[3] : 0;
                lc.leftArmPrim = laArm;
                lc.rightArmPrim = raArm;
                lc.leftLegPrim = laLeg;
                lc.rightLegPrim = raLeg;
                wc.torso = safePos(torsoPrim, 15.0f);
                wc.leftArm = safePos(laArm, 20.0f);
                wc.rightArm = safePos(raArm, 20.0f);
                wc.leftLeg = safePos(laLeg, 20.0f);
                wc.rightLeg = safePos(raLeg, 20.0f);
            }
            else
            {
                wc.torso = safePos(torsoPrim, 15.0f);
            }
            wc.lc = lc;
            if (need3d && lc.headPrim)
            {
                std::vector<math::vec3> pts;
                pts.reserve(200);
                collectBoxPoints(wc, pts);
                if (!pts.empty())
                {
                    wc.pts3d = std::move(pts);
                    wc.hasPts3d = true;
                }
            }
            out.push_back(std::move(wc));
        }
    }

    inline void update(float fw, float fh)
    {
        if (!g_proc || !g_proc->connected() || !g_ctx.ready)
            return;
        const auto now = std::chrono::steady_clock::now();
        if (now - pfCheck > std::chrono::milliseconds(2000))
        {
            pfCheck = now;
            pfMode = PfCache::IsActivePlace();
        }
        view = fetchView();
        viewValid = true;
        if (g_ctx.camera)
            camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        {
            math::vec3 fwd{};
            if (g_ctx.camera)
            {
                math::frame cf = g_proc->read<math::frame>(g_ctx.camera.addr + Offsets::Camera::Rotation);
                fwd = math::vec3{ -cf.rot[2], -cf.rot[5], -cf.rot[8] };
                float fl = fwd.length();
                if (fl > 0.000001f)
                    fwd = fwd / fl;
                else
                    fwd = math::vec3{};
            }
            if (fwd.x != 0.0f || fwd.y != 0.0f || fwd.z != 0.0f)
            {
                if (havePrevCamFwd)
                {
                    float d = prevCamFwd.dot(fwd);
                    if (d < 0.996f)
                        dynRect.clear();
                }
                prevCamFwd = fwd;
                havePrevCamFwd = true;
            }
            else
            {
                havePrevCamFwd = false;
            }
        }
        bool needSkel = settings::skeleton_enabled;
        bool need3dBox = (settings::box_mode == 1);
        bool needFullLimbs = needSkel || need3dBox;
        (void)needFullLimbs;
        std::vector<WorldCache> next;
        if (pfMode)
        {
            build_pf_world(next, fw, fh);
        }
        else
        {
            pcache::update();
            std::vector<pcache::Entry> snapshot;
            {
                std::lock_guard<std::mutex> lk(pcache::mtx);
                snapshot = pcache::entries;
            }
            next.reserve(snapshot.size());
            uintptr_t localTeamAddr = 0;
            if (settings::vis_team_check && g_ctx.local_player.addr)
                localTeamAddr = g_proc->read<uintptr_t>(g_ctx.local_player.addr + Offsets::Player::Team);
            for (auto& pe : snapshot)
            {
                WorldCache wc{};
                wc.playerAddr = pe.player;
                if (pe.player == g_ctx.local_player.addr) { next.push_back(std::move(wc)); continue; }
                if (settings::vis_team_check && localTeamAddr && pe.team && pe.team == localTeamAddr) { next.push_back(std::move(wc)); continue; }
                if (pe.hp <= 0.f) { next.push_back(std::move(wc)); continue; }
                wc.hp = pe.hp;
                wc.maxHp = pe.maxHp > 0 ? pe.maxHp : 100.f;
                wc.name = pe.name;
                if (settings::tool_enabled)
                    wc.tool = pe.tool;
                if (settings::flags_enabled && settings::flags_team)
                    wc.team = pe.teamStr;
                LimbCache lc = pe.lc;
                if (lc.headPrim == 0) { next.push_back(std::move(wc)); continue; }
                wc.head = getPos(lc.headPrim);
                if (wc.head.x == 0 && wc.head.y == 0 && wc.head.z == 0) { next.push_back(wc); continue; }
                wc.root = lc.rootPrim ? getPos(lc.rootPrim) : math::vec3{};
                float drop = lc.isR6 ? 3.0f : 2.5f;
                if ((wc.root.x == 0 && wc.root.y == 0 && wc.root.z == 0) || (wc.head - wc.root).length() > 25.0f)
                {
                    wc.root = math::vec3{ wc.head.x, wc.head.y - drop, wc.head.z };
                }

                {
                    float dh = (wc.head - g_ctx.local_pos).length();
                    float dr = (wc.root - g_ctx.local_pos).length();
                    float nearest = dh < dr ? dh : dr;
                    if (nearest > settings::visual_maxdist) { next.push_back(std::move(wc)); continue; }
                    math::vec2 hs{};
                    if (!projection::to_screen(wc.head, view, fw, fh, hs)) { next.push_back(std::move(wc)); continue; }
                    const float m = 80.0f;
                    if (hs.x < -m || hs.x > fw + m || hs.y < -m || hs.y > fh + m) { next.push_back(std::move(wc)); continue; }
                }
                if (settings::flags_enabled && settings::flags_state && pe.humanoid)
                {
                    uintptr_t seat = g_proc->read<uintptr_t>(pe.humanoid + Offsets::Humanoid::SeatPart);
                    uint8_t plat = g_proc->read<uint8_t>(pe.humanoid + Offsets::Humanoid::PlatformStand);
                    if (seat != 0)
                        wc.state = "Seated";
                    else if (plat != 0)
                        wc.state = "Platform";
                    else
                    {
                        math::vec3 vel{};
                        if (lc.rootPrim)
                            vel = g_proc->read<math::vec3>(lc.rootPrim + Offsets::Primitive::AssemblyLinearVelocity);
                        float hSpeed = sqrtf(vel.x * vel.x + vel.z * vel.z);
                        if (fabsf(vel.y) > 8.0f)
                            wc.state = vel.y > 0.0f ? "Jumping" : "Falling";
                        else if (hSpeed > 4.0f)
                            wc.state = "Moving";
                        else
                            wc.state = "Standing";
                    }
                }
                wc.valid = true;
                wc.isR6 = lc.isR6;
                auto safePos = [&](uintptr_t prim, float maxDist) -> math::vec3 {
                    if (!prim) return {};
                    math::vec3 p = getPos(prim);
                    if (p.x == 0 && p.y == 0 && p.z == 0) return {};
                    if ((p - wc.root).length() > maxDist) return {};
                    return p;
                };
                if (lc.isR6)
                {
                    wc.torso = safePos(lc.torsoPrim, 15.0f);
                    wc.leftArm = safePos(lc.leftArmPrim, 20.0f);
                    wc.rightArm = safePos(lc.rightArmPrim, 20.0f);
                    wc.leftLeg = safePos(lc.leftLegPrim, 20.0f);
                    wc.rightLeg = safePos(lc.rightLegPrim, 20.0f);
                }
                else
                {
                    wc.upper = safePos(lc.upperPrim, 15.0f);
                    wc.lower = safePos(lc.lowerPrim, 15.0f);
                    wc.hasLower = lc.lowerPrim != 0 && (wc.lower.x != 0 || wc.lower.y != 0 || wc.lower.z != 0);
                    wc.leftUpperArm = safePos(lc.leftUpperArmPrim, 20.0f);
                    wc.leftLowerArm = safePos(lc.leftLowerArmPrim, 20.0f);
                    wc.leftHand = safePos(lc.leftHandPrim, 25.0f);
                    wc.rightUpperArm = safePos(lc.rightUpperArmPrim, 20.0f);
                    wc.rightLowerArm = safePos(lc.rightLowerArmPrim, 20.0f);
                    wc.rightHand = safePos(lc.rightHandPrim, 25.0f);
                    wc.leftUpperLeg = safePos(lc.leftUpperLegPrim, 20.0f);
                    wc.leftLowerLeg = safePos(lc.leftLowerLegPrim, 20.0f);
                    wc.leftFoot = safePos(lc.leftFootPrim, 25.0f);
                    wc.rightUpperLeg = safePos(lc.rightUpperLegPrim, 20.0f);
                    wc.rightLowerLeg = safePos(lc.rightLowerLegPrim, 20.0f);
                    wc.rightFoot = safePos(lc.rightFootPrim, 25.0f);
                    wc.torso = wc.upper;
                }
                wc.lc = lc;
                if (needSkel && lc.isR6 && lc.torsoPrim)
                {
                    auto tf = getFrame(lc.torsoPrim);
                    if (!(tf.pos.x == 0 && tf.pos.y == 0 && tf.pos.z == 0) && (tf.pos - wc.root).length() <= 6.0f)
                    {
                        wc.torsoFrame = tf;
                        wc.hasTorsoFrame = true;
                    }
                }
                if (need3dBox && lc.headPrim)
                {
                    std::vector<math::vec3> pts;
                    pts.reserve(200);
                    collectBoxPoints(wc, pts);
                    if (!pts.empty()) { wc.pts3d = std::move(pts); wc.hasPts3d = true; }
                }
                next.push_back(std::move(wc));
            }
        }
        if (dynRect.size() > 64)
        {
            for (auto it = dynRect.begin(); it != dynRect.end();)
            {
                if (now - it->second.t > std::chrono::seconds(5))
                    it = dynRect.erase(it);
                else
                    ++it;
            }
        }
        entries.swap(next);
    }
    inline void clear()
    {
        entries.clear();
        dynRect.clear();
        teamNames.clear();
        view = math::view_matrix{};
        viewValid = false;
        camPos = math::vec3{};
        pfCheck = std::chrono::steady_clock::time_point{};
        havePrevCamFwd = false;
    }
}
#pragma endregion
