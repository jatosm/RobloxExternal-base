#include "autoshoot.h"
#include "../core/aim.h"
#include "../../visual/world/geo/geo.h"
#include "../health/health.h"
#include "../distance/distance.h"
#include "../pftarget/pftarget.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../menu/overlay/settings.h"
#include <Windows.h>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <vector>
namespace thenwefuckin::aim::autoshoot
{
    static inline node get_target_node(const node& character, int partIdx);
    struct CachedCrosshairTarget
    {
        uintptr_t playerAddr = 0;
        uintptr_t characterAddr = 0;
        uintptr_t humanoidAddr = 0;
        uintptr_t headPrim = 0;
        uintptr_t rootPrim = 0;
        uintptr_t targetPrim = 0;
        uintptr_t teamAddr = 0;
        int partIdx = -1;
        bool isBot = false;
        float botHp = 100.0f;
    };
    static std::vector<CachedCrosshairTarget> s_chTargets;
    static std::chrono::steady_clock::time_point s_chLastResolve{};
    static std::chrono::steady_clock::time_point s_shotLastPoll{};
    static std::chrono::steady_clock::time_point s_shotTargetFirstSeen{};
    static std::chrono::steady_clock::time_point s_shotLastShotTime{};
    static bool s_shotHasTarget = false;
    static void resolve_crosshair_targets()
    {
        s_chTargets.clear();
        int wantPart = settings::aim_part;
        std::vector<pcache::Entry> snap;
        {
            std::lock_guard<std::mutex> lk(pcache::mtx);
            snap = pcache::entries;
        }
        s_chTargets.reserve(snap.size());
        std::uintptr_t localAddr = g_ctx.local_player.addr;
        for (auto& e : snap)
        {
            if (!e.valid || e.player == localAddr)
                continue;
            if (!e.humanoid)
                continue;
            if (!e.lc.headPrim || !e.lc.rootPrim)
                continue;
            std::uintptr_t targetPrim = pcache::aim_prim(e.lc, wantPart);
            CachedCrosshairTarget t{};
            t.playerAddr = e.player;
            t.characterAddr = e.model;
            t.humanoidAddr = e.humanoid;
            t.headPrim = e.lc.headPrim;
            t.rootPrim = e.lc.rootPrim;
            t.targetPrim = targetPrim;
            t.teamAddr = e.team;
            t.partIdx = wantPart;
            s_chTargets.push_back(t);
        }
        if (settings::aim_bots)
        {
            std::vector<BotAimTarget> bots;
            collect_bot_targets(bots);
            for (auto& b : bots)
            {
                if (!b.headPrim || !b.rootPrim)
                    continue;
                std::uintptr_t prim = (wantPart == 0) ? b.headPrim : b.rootPrim;
                if (!prim)
                    prim = b.headPrim ? b.headPrim : b.rootPrim;
                CachedCrosshairTarget t{};
                t.playerAddr = b.inst;
                t.characterAddr = b.inst;
                t.humanoidAddr = b.humanoid;
                t.headPrim = b.headPrim;
                t.rootPrim = b.rootPrim;
                t.targetPrim = prim;
                t.teamAddr = 0;
                t.partIdx = wantPart;
                t.isBot = true;
                t.botHp = b.hp;
                s_chTargets.push_back(t);
            }
        }
    }
    static inline node get_target_node(const node& character, int partIdx)
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
    bool is_target_in_crosshair(int w, int h)
    {
        if (!g_ctx.ready || !g_ctx.camera || w <= 0 || h <= 0)
            return false;
        if (pftarget::active() && !PfCache::players.empty())
        {
            float cx = static_cast<float>(w) * 0.5f;
            float cy = static_cast<float>(h) * 0.5f;
            const math::view_matrix& view = g_ctx.view;
            math::vec3 camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
            for (auto& p : PfCache::players)
            {
                if (!p.isValid)
                    continue;
                if (p.modelAddr == PfCache::localCharacterAddr)
                    continue;
                if (p.health <= 0.0f)
                    continue;
                if (!health::check(p.health * 100.0f))
                    continue;
                if (settings::aim_team_check && PfCache::IsTeammate(p))
                    continue;
                uintptr_t prim = (settings::aim_part == 0) ? p.headPrim : p.torsoPrim;
                if (!prim)
                    prim = p.headPrim ? p.headPrim : p.torsoPrim;
                if (!prim)
                    continue;
                math::vec3 pos = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
                if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
                    continue;
                if (!distance::check(pos, camPos))
                    continue;
                if (settings::aim_visible_check && !worldgeo::visible(camPos, pos))
                    continue;
                math::vec2 s{};
                if (!projection::to_screen(pos, view, static_cast<float>(w), static_cast<float>(h), s))
                    continue;
                float dx = s.x - cx;
                float dy = s.y - cy;
                float dist = sqrtf(dx * dx + dy * dy);
                float hitRadius = fmaxf(24.0f, fminf((float)h * 0.05f, 40.0f));
                if (dist <= hitRadius)
                    return true;
            }
            if (settings::aim_bots)
            {
                std::vector<BotAimTarget> bots;
                collect_bot_targets(bots);
                for (auto& b : bots)
                {
                    std::uintptr_t bpar = g_proc->read<std::uintptr_t>(b.inst + Offsets::Instance::Parent);
                    if (!bpar)
                        continue;
                    float bhp = b.humanoid ? g_proc->read<float>(b.humanoid + Offsets::Humanoid::Health) : b.hp;
                    if (!health::check(bhp))
                        continue;
                    uintptr_t prim = (settings::aim_part == 0) ? b.headPrim : b.rootPrim;
                    if (!prim)
                        prim = b.headPrim ? b.headPrim : b.rootPrim;
                    if (!prim)
                        continue;
                    math::vec3 pos = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
                    if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
                        continue;
                    if (!distance::check(pos, camPos))
                        continue;
                    if (settings::aim_visible_check && !worldgeo::visible(camPos, pos))
                        continue;
                    math::vec2 s{};
                    if (!projection::to_screen(pos, view, static_cast<float>(w), static_cast<float>(h), s))
                        continue;
                    float dx = s.x - cx;
                    float dy = s.y - cy;
                    float dist = sqrtf(dx * dx + dy * dy);
                    float hitRadius = fmaxf(24.0f, fminf((float)h * 0.05f, 40.0f));
                    if (dist <= hitRadius)
                        return true;
                }
            }
            return false;
        }
        auto now = std::chrono::steady_clock::now();
        int wantPart = settings::aim_part;
        if (s_chTargets.empty()
            || now - s_chLastResolve >= std::chrono::milliseconds(200)
            || s_chTargets[0].partIdx != wantPart)
        {
            resolve_crosshair_targets();
            s_chLastResolve = now;
            if (s_chTargets.empty())
                return false;
        }
        float cx = static_cast<float>(w) * 0.5f;
        float cy = static_cast<float>(h) * 0.5f;
        const math::view_matrix& view = g_ctx.view;
        math::vec3 camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        for (auto& t : s_chTargets)
        {
            if (thenwefuckin::aim::is_teammate(t.playerAddr, t.characterAddr, t.teamAddr))
                continue;
            if (t.isBot)
            {
                std::uintptr_t bpar = g_proc->read<std::uintptr_t>(t.characterAddr + Offsets::Instance::Parent);
                if (!bpar)
                    continue;
                float bhp = t.humanoidAddr ? g_proc->read<float>(t.humanoidAddr + Offsets::Humanoid::Health) : t.botHp;
                if (!health::check(bhp))
                    continue;
            }
            else
            {
                uintptr_t charAddr = g_proc->read<uintptr_t>(t.playerAddr + Offsets::Player::ModelInstance);
                if (charAddr != t.characterAddr)
                    continue;
                float hp = g_proc->read<float>(t.humanoidAddr + Offsets::Humanoid::Health);
                if (!health::check(hp))
                    continue;
            }
            math::vec3 headPos = g_proc->read<math::vec3>(t.headPrim + Offsets::Primitive::Position);
            math::vec3 rootPos = g_proc->read<math::vec3>(t.rootPrim + Offsets::Primitive::Position);
            if (headPos.x == 0.0f && headPos.y == 0.0f && headPos.z == 0.0f) continue;
            if (rootPos.x == 0.0f && rootPos.y == 0.0f && rootPos.z == 0.0f) continue;
            if ((headPos - rootPos).length() > 10.0f) continue;
            if (!distance::check(headPos, camPos))
                continue;
            if (settings::aim_visible_check && !worldgeo::visible(camPos, headPos))
                continue;
            math::vec3 top3{ headPos.x, headPos.y + 0.6f, headPos.z };
            math::vec3 bot3{ rootPos.x, rootPos.y - 2.8f, rootPos.z };
            math::vec2 topScreen{}, botScreen{};
            if (!projection::to_screen(top3, view, static_cast<float>(w), static_cast<float>(h), topScreen))
                continue;
            if (!projection::to_screen(bot3, view, static_cast<float>(w), static_cast<float>(h), botScreen))
                continue;
            float hh = botScreen.y - topScreen.y;
            if (hh <= 2.0f || hh > static_cast<float>(h) * 1.5f) continue;
            float ww = hh * 0.55f;
            float x0 = topScreen.x - ww * 0.5f;
            float x1 = topScreen.x + ww * 0.5f;
            float y0 = topScreen.y;
            float y1 = botScreen.y;
            if (cx >= x0 && cx <= x1 && cy >= y0 && cy <= y1)
                return true;
            if (t.targetPrim)
            {
                math::vec3 tPos = g_proc->read<math::vec3>(t.targetPrim + Offsets::Primitive::Position);
                math::vec2 tScreen{};
                if (projection::to_screen(tPos, view, static_cast<float>(w), static_cast<float>(h), tScreen))
                {
                    float dx = tScreen.x - cx;
                    float dy = tScreen.y - cy;
                    float dist = sqrtf(dx * dx + dy * dy);
                    float hitRadius = fmaxf(hh * 0.18f, 12.0f);
                    if (dist <= hitRadius)
                        return true;
                }
            }
        }
        return false;
    }
    void run(int w, int h)
    {
        if (!settings::aim_autoshoot)
            return;
        using clock = std::chrono::steady_clock;
        auto pollNow = clock::now();
        if (pollNow - s_shotLastPoll < std::chrono::milliseconds(8))
            return;
        s_shotLastPoll = pollNow;
        bool inCrosshair = is_target_in_crosshair(w, h);
        auto now = clock::now();
        if (inCrosshair)
        {
            if (!s_shotHasTarget)
            {
                s_shotHasTarget = true;
                s_shotTargetFirstSeen = now;
            }
            auto elapsedSinceSeen = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_shotTargetFirstSeen).count();
            auto elapsedSinceLastShot = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_shotLastShotTime).count();
            float delayMs = settings::aim_autoshoot_delay;
            if (delayMs < 0.0f) delayMs = 0.0f;
            long long minCooldown = static_cast<long long>((delayMs > 60.0f) ? delayMs : 60.0f);
            if (elapsedSinceSeen >= static_cast<long long>(delayMs) && elapsedSinceLastShot >= minCooldown)
            {
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                s_shotLastShotTime = now;
            }
        }
        else
        {
            s_shotHasTarget = false;
        }
    }
    void on_game_joined()
    {
        s_chTargets.clear();
        s_chLastResolve = std::chrono::steady_clock::time_point{};
        s_shotLastPoll = std::chrono::steady_clock::time_point{};
        s_shotTargetFirstSeen = std::chrono::steady_clock::time_point{};
        s_shotLastShotTime = std::chrono::steady_clock::time_point{};
        s_shotHasTarget = false;
    }
}
