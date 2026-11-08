#include "aim.h"
#include "../mouse/mouse.h"
#include "../memory/memory.h"
#include "../viewport/viewport.h"
#include "../raycast/raycast.h"
#include "../wraith/wraith.h"
#include "../autoshoot/autoshoot.h"
#include "../tracking/tracking.h"
#include "../pftarget/pftarget.h"
#include "../../visual/world/core/scan.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <Windows.h>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>
namespace thenwefuckin::aim
{
    namespace
    {
        math::vec3 s_debugTarget{};
        uintptr_t s_debugPlayer = 0;
        bool s_hasDebugTarget = false;
        std::chrono::steady_clock::time_point s_debugT{};
        uintptr_t s_prevPlayer = 0;
        std::chrono::steady_clock::time_point s_prevT{};
        uintptr_t s_lockPlayer = 0;
        uintptr_t s_lockModel = 0;
        uintptr_t s_lockHumanoid = 0;
        bool s_lockIsPlayer = false;
        std::string s_lockName{};
        std::chrono::steady_clock::time_point s_lockLostT{};
        std::chrono::steady_clock::time_point s_lockExistT{};
        constexpr int kLockRespawnGraceMs = 12000;
    }
    static bool debug_fresh()
    {
        if (!s_hasDebugTarget)
            return false;
        if (std::chrono::steady_clock::now() - s_debugT > std::chrono::milliseconds(300))
        {
            s_hasDebugTarget = false;
            s_debugPlayer = 0;
            return false;
        }
        return true;
    }
    void set_debug_target(const math::vec3& world, uintptr_t playerAddr)
    {
        auto now = std::chrono::steady_clock::now();
        if (s_hasDebugTarget && playerAddr != s_debugPlayer)
        {
            s_prevPlayer = s_debugPlayer;
            s_prevT = now;
        }
        s_debugTarget = world;
        s_debugPlayer = playerAddr;
        s_hasDebugTarget = true;
        s_debugT = now;
    }
    void clear_debug_target()
    {
        s_hasDebugTarget = false;
        s_debugPlayer = 0;
    }
    bool debug_target(math::vec3& out)
    {
        if (!debug_fresh())
            return false;
        out = s_debugTarget;
        return true;
    }
    bool debug_target_player(uintptr_t& out)
    {
        if (!debug_fresh())
            return false;
        out = s_debugPlayer;
        return true;
    }
    bool debug_recent_player(uintptr_t addr)
    {
        if (addr == 0)
            return false;
        auto now = std::chrono::steady_clock::now();
        if (s_hasDebugTarget && now - s_debugT <= std::chrono::milliseconds(300) && addr == s_debugPlayer)
            return true;
        if (s_prevPlayer != 0 && addr == s_prevPlayer && now - s_prevT <= std::chrono::milliseconds(750))
            return true;
        return false;
    }
    bool debug_in_use()
    {
        auto now = std::chrono::steady_clock::now();
        if (s_hasDebugTarget && now - s_debugT <= std::chrono::milliseconds(300))
            return true;
        if (s_prevPlayer != 0 && now - s_prevT <= std::chrono::milliseconds(750))
            return true;
        return false;
    }
    uintptr_t local_team_addr()
    {
        if (!settings::aim_team_check)
            return 0;
        if (!g_proc || !g_proc->connected())
            return 0;
        if (!g_ctx.local_player.addr)
            return 0;
        return g_proc->read<uintptr_t>(g_ctx.local_player.addr + Offsets::Player::Team);
    }
    void collect_bot_targets(std::vector<BotAimTarget>& out)
    {
        out.clear();
        if (!settings::aim_bots)
            return;
        if (!g_proc || !g_proc->connected() || !g_ctx.workspace)
            return;

        world::refresh_entries();
        if (world::g_entries.empty())
            return;

        uintptr_t localChar = 0;
        if (g_ctx.local_player)
        {
            node lc = g_ctx.get_character(g_ctx.local_player);
            if (lc)
                localChar = lc.addr;
        }
        out.reserve(world::g_entries.size());
        for (auto& e : world::g_entries)
        {
            if (!e.bots || !e.inst)
                continue;
            if (localChar && (e.inst == localChar))
                continue;
            uintptr_t par = g_proc->read<uintptr_t>(e.inst + Offsets::Instance::Parent);
            if (!par)
                continue;
            if (localChar && par == localChar)
                continue;
            uintptr_t headPrim = e.headPrim;
            uintptr_t rootPrim = e.rootPrim;
            if (!headPrim && !rootPrim)
                continue;
            BotAimTarget b{};
            b.inst = e.inst;
            b.humanoid = e.humanoid;
            b.headPrim = headPrim;
            b.rootPrim = rootPrim;
            if (e.humanoid)
            {
                float bhp = g_proc->read<float>(e.humanoid + Offsets::Humanoid::Health);
                b.hp = (std::isfinite(bhp) && bhp > 0.0f) ? bhp : 0.0f;
            }
            else if (e.hpValid && std::isfinite(e.hp) && e.hp > 0.0f)
            {
                b.hp = e.hp;
            }
            else
            {
                b.hp = 100.0f;
            }
            out.push_back(b);
        }
    }
    bool should_skip_teammate(uintptr_t localTeam, uintptr_t targetTeam)
    {
        if (!settings::aim_team_check)
            return false;
        if (!localTeam || !targetTeam)
            return false;
        return localTeam == targetTeam;
    }
    bool is_teammate(uintptr_t playerAddr, uintptr_t characterAddr, uintptr_t teamAddr)
    {
        if (!settings::aim_team_check)
            return false;
        uintptr_t localTeam = local_team_addr();
        if (localTeam && teamAddr && localTeam == teamAddr)
            return true;
        if (pftarget::active() && characterAddr)
        {
            for (auto& p : PfCache::players)
            {
                if (!p.isValid || p.modelAddr != characterAddr)
                    continue;
                if (p.modelAddr == PfCache::localCharacterAddr)
                    return true;
                return PfCache::IsTeammate(p);
            }
        }
        (void)playerAddr;
        return false;
    }
    bool lock_active()
    {
        return settings::lock_enabled && (s_lockPlayer != 0 || s_lockModel != 0);
    }
    bool lock_matches(uintptr_t a, uintptr_t b)
    {
        if (!settings::lock_enabled)
            return true;
        if (!s_lockPlayer && !s_lockModel)
            return true;
        if (a && (a == s_lockPlayer || a == s_lockModel))
            return true;
        if (b && (b == s_lockPlayer || b == s_lockModel))
            return true;
        return false;
    }
    void clear_lock()
    {
        s_lockPlayer = 0;
        s_lockModel = 0;
        s_lockHumanoid = 0;
        s_lockIsPlayer = false;
        s_lockName.clear();
        s_lockLostT = std::chrono::steady_clock::time_point{};
        s_lockExistT = std::chrono::steady_clock::time_point{};
    }
    static bool acquire_lock(int w, int h)
    {
        if (!g_ctx.ready || !g_ctx.camera || w <= 0 || h <= 0 || !g_proc || !g_proc->connected())
            return false;
        const math::view_matrix& view = g_ctx.view;
        const float fw = static_cast<float>(w);
        const float fh = static_cast<float>(h);
        const float cx = fw * 0.5f;
        const float cy = fh * 0.5f;
        const float fov = settings::aim_fov > 0.0f ? settings::aim_fov : 1e9f;
        float bestD = 1e9f;
        uintptr_t bestPlayer = 0;
        uintptr_t bestModel = 0;
        uintptr_t bestHumanoid = 0;
        auto consider = [&](uintptr_t player, uintptr_t model, uintptr_t humanoid, uintptr_t prim)
        {
            if (!prim)
                return;
            math::vec3 pos = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
            if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
                return;
            math::vec2 s{};
            if (!projection::to_screen(pos, view, fw, fh, s))
                return;
            float dx = s.x - cx;
            float dy = s.y - cy;
            float d = sqrtf(dx * dx + dy * dy);
            if (d > fov || d >= bestD)
                return;
            bestD = d;
            bestPlayer = player;
            bestModel = model;
            bestHumanoid = humanoid;
        };
        if (pftarget::active())
        {
            for (auto& p : PfCache::players)
            {
                if (!p.isValid || p.modelAddr == PfCache::localCharacterAddr || !(p.health > 0.0f))
                    continue;
                if (settings::aim_team_check && PfCache::IsTeammate(p))
                    continue;
                uintptr_t prim = settings::aim_part == 0 ? p.headPrim : p.torsoPrim;
                if (!prim)
                    prim = p.headPrim ? p.headPrim : p.torsoPrim;
                consider(0, p.modelAddr, 0, prim);
            }
        }
        else
        {
            std::vector<pcache::Entry> snap;
            {
                std::lock_guard<std::mutex> lk(pcache::mtx);
                snap = pcache::entries;
            }
            for (auto& e : snap)
            {
                if (!e.valid || !e.humanoid || e.player == g_ctx.local_player.addr)
                    continue;
                if (is_teammate(e.player, e.model, e.team))
                    continue;
                float hp = g_proc->read<float>(e.humanoid + Offsets::Humanoid::Health);
                if (!(hp > 0.0f))
                    continue;
                consider(e.player, e.model, e.humanoid, pcache::aim_prim(e.lc, settings::aim_part));
            }
        }
        if (settings::aim_bots)
        {
            std::vector<BotAimTarget> bots;
            collect_bot_targets(bots);
            for (auto& b : bots)
            {
                uintptr_t prim = settings::aim_part == 0 ? b.headPrim : b.rootPrim;
                if (!prim)
                    prim = b.headPrim ? b.headPrim : b.rootPrim;
                consider(b.inst, b.inst, b.humanoid, prim);
            }
        }
        if (!bestPlayer && !bestModel)
            return false;
        s_lockPlayer = bestPlayer;
        s_lockModel = bestModel;
        s_lockHumanoid = bestHumanoid;
        s_lockIsPlayer = false;
        s_lockName.clear();
        s_lockLostT = std::chrono::steady_clock::time_point{};
        s_lockExistT = std::chrono::steady_clock::time_point{};
        if (pftarget::active())
        {
            for (auto& p : PfCache::players)
            {
                if (p.modelAddr != bestModel)
                    continue;
                s_lockName = p.name;
                break;
            }
        }
        if (bestPlayer)
        {
            std::lock_guard<std::mutex> lk(pcache::mtx);
            for (auto& e : pcache::entries)
            {
                if (e.player != bestPlayer)
                    continue;
                s_lockName = e.name;
                s_lockIsPlayer = true;
                break;
            }
        }
        if (s_lockName.empty() && bestModel)
            s_lockName = node{ bestModel }.name();
        return true;
    }
    static bool resolve_living_target(uintptr_t& model, uintptr_t& humanoid)
    {
        model = 0;
        humanoid = 0;
        if (!g_proc || !g_proc->connected())
            return false;
        if (s_lockIsPlayer && s_lockPlayer)
        {
            std::uintptr_t m = g_proc->read<std::uintptr_t>(s_lockPlayer + Offsets::Player::ModelInstance);
            if (m && g_proc->read<std::uintptr_t>(m + Offsets::Instance::Parent))
            {
                std::uintptr_t hum = 0;
                {
                    std::lock_guard<std::mutex> lk(pcache::mtx);
                    for (auto& e : pcache::entries)
                    {
                        if (!e.valid || e.player != s_lockPlayer)
                            continue;
                        hum = e.humanoid;
                        break;
                    }
                }
                if (!hum)
                    hum = node{ m }.find_child_of_class("Humanoid").addr;
                if (hum && g_proc->read<float>(hum + Offsets::Humanoid::Health) > 0.0f)
                {
                    model = m;
                    humanoid = hum;
                    return true;
                }
            }
        }
        if (s_lockModel && g_proc->read<std::uintptr_t>(s_lockModel + Offsets::Instance::Parent) && s_lockHumanoid)
        {
            if (g_proc->read<float>(s_lockHumanoid + Offsets::Humanoid::Health) > 0.0f)
            {
                model = s_lockModel;
                humanoid = s_lockHumanoid;
                return true;
            }
        }
        if (!s_lockName.empty())
        {
            if (pftarget::active())
            {
                for (auto& p : PfCache::players)
                {
                    if (!p.isValid || p.health <= 0.0f || !p.modelAddr || p.name != s_lockName)
                        continue;
                    model = p.modelAddr;
                    return true;
                }
            }
            std::lock_guard<std::mutex> lk(pcache::mtx);
            for (auto& e : pcache::entries)
            {
                if (!e.valid || e.hp <= 0.0f || !e.model || e.name != s_lockName)
                    continue;
                model = e.model;
                humanoid = e.humanoid;
                return true;
            }
        }
        return false;
    }
    static bool player_still_connected(uintptr_t playerAddr)
    {
        if (!g_ctx.players)
            return true;
        for (auto& pl : g_ctx.player_list())
        {
            if (pl.addr == playerAddr)
                return true;
        }
        return false;
    }
    static bool lock_target_alive()
    {
        std::uintptr_t model = 0;
        std::uintptr_t humanoid = 0;
        if (resolve_living_target(model, humanoid))
        {
            s_lockModel = model;
            if (humanoid)
                s_lockHumanoid = humanoid;
            s_lockLostT = std::chrono::steady_clock::time_point{};
            s_lockExistT = std::chrono::steady_clock::time_point{};
            return true;
        }

        const auto now = std::chrono::steady_clock::now();
        if (s_lockIsPlayer && s_lockPlayer &&
            (s_lockExistT == std::chrono::steady_clock::time_point{} ||
                std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lockExistT).count() >= 500))
        {
            s_lockExistT = now;
            if (!player_still_connected(s_lockPlayer))
                return false;
        }
        if (s_lockLostT == std::chrono::steady_clock::time_point{})
        {
            s_lockLostT = now;
            return true;
        }
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lockLostT).count() < kLockRespawnGraceMs;
    }
    void update_lock(int w, int h)
    {
        static bool s_prevKey = false;
        if (!settings::lock_enabled)
        {
            clear_lock();
            s_prevKey = false;
            return;
        }
        const int mode = settings::lock_key_mode;
        const bool down = settings::lock_key > 0 && Menu::IsKeybindDown(settings::lock_key);

        if (mode == 0)
        {
            if (!lock_active())
                acquire_lock(w, h);
        }
        else if (mode == 2)
        {
            if (down && !lock_active())
                acquire_lock(w, h);
            else if (!down && lock_active())
                clear_lock();
        }
        else
        {
            const bool pressed = down && !s_prevKey;
            if (pressed)
            {
                if (lock_active())
                    clear_lock();
                else
                    acquire_lock(w, h);
            }
        }
        s_prevKey = down;

        if (lock_active() && !lock_target_alive())
            clear_lock();
    }
    void on_game_joined()
    {
        clear_lock();
        s_hasDebugTarget = false;
        s_debugPlayer = 0;
        s_prevPlayer = 0;
        tracking::clear_all();
        mouse::on_game_joined();
        memory::on_game_joined();
        viewport::on_game_joined();
        raycast::on_game_joined();
        wraith::on_game_joined();
        autoshoot::on_game_joined();
    }
    node resolve_part(const node& character, int partIdx)
    {
        switch (partIdx)
        {
        case 0:
            return character.find_child("Head");
        case 1:
        {
            node t = character.find_child("UpperTorso");
            if (!t) t = character.find_child("Torso");
            if (!t) t = character.find_child("HumanoidRootPart");
            return t;
        }
        case 2:
        {
            node hrp = character.find_child("HumanoidRootPart");
            if (!hrp) hrp = character.find_child("Torso");
            if (!hrp) hrp = character.find_child("UpperTorso");
            return hrp;
        }
        case 3:
        {
            node a = character.find_child("LeftUpperArm");
            if (!a) a = character.find_child("Left Arm");
            if (!a) a = character.find_child("LeftLowerArm");
            return a;
        }
        case 4:
        {
            node a = character.find_child("RightUpperArm");
            if (!a) a = character.find_child("Right Arm");
            if (!a) a = character.find_child("RightLowerArm");
            return a;
        }
        case 5:
        {
            node l = character.find_child("LeftUpperLeg");
            if (!l) l = character.find_child("Left Leg");
            if (!l) l = character.find_child("LeftLowerLeg");
            return l;
        }
        case 6:
        {
            node l = character.find_child("RightUpperLeg");
            if (!l) l = character.find_child("Right Leg");
            if (!l) l = character.find_child("RightLowerLeg");
            return l;
        }
        default:
            return character.find_child("Head");
        }
    }
    void run(int w, int h)
    {
        update_lock(w, h);
        bool aimOn = settings::aim_enabled && Menu::IsMenuKeyActive("##aim_key", settings::aim_key, settings::aim_key_mode);
        bool silentOn = settings::silent_enabled;
        int method = -1;
        if (aimOn)
            method = settings::aim_method;
        if (silentOn)
            method = settings::silent_method == 1 ? 3 : 2;
        if (method == 0)
            memory::run(w, h);
        else if (method == 1)
            mouse::run(w, h);
        else if (method == 2)
            viewport::run(w, h);
        else if (method == 3)
            raycast::run(w, h);
        else if (method == 4)
            wraith::run(w, h);
        else
        {
            viewport::clear();
            clear_debug_target();
        }
        if (method != 2)
            viewport::clear();
        if (method != 3)
            raycast::idle();
        if (method != 4)
            wraith::idle();
        if (method != 0)
            memory::clear();
        if (settings::aim_autoshoot)
        {
            bool keyActive = (settings::aim_autoshoot_key == 0) || Menu::IsMenuKeyActive("##autoshoot_key", settings::aim_autoshoot_key, settings::aim_autoshoot_key_mode);
            if (keyActive)
            {
                autoshoot::run(w, h);
            }
        }
    }
}
