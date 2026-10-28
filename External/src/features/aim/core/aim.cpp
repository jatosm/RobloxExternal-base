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
    void on_game_joined()
    {
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
        bool keyOn = settings::aim_enabled && Menu::IsMenuKeyActive("##aim_key", settings::aim_key, settings::aim_key_mode);
        if (keyOn && settings::aim_method == 0)
            memory::run(w, h);
        else if (keyOn && settings::aim_method == 1)
            mouse::run(w, h);
        else if (keyOn && settings::aim_method == 2)
            viewport::run(w, h);
        else if (keyOn && settings::aim_method == 3)
            raycast::run(w, h);
        else if (keyOn && settings::aim_method == 4)
            wraith::run(w, h);
        else
        {
            viewport::clear();
            clear_debug_target();
        }
        if (settings::aim_method != 2)
            viewport::clear();
        if (settings::aim_method != 3 || !keyOn)
            raycast::idle();
        if (settings::aim_method != 4 || !keyOn)
            wraith::idle();
        if (settings::aim_method != 0)
            memory::clear();
        if (settings::aim_method != 3 || !keyOn)
            wraith::idle();
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
