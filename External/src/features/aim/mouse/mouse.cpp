#include "mouse.h"
#include "../core/aim.h"
#include "../predict/predict.h"
#include "../../visual/world/geo/geo.h"
#include "../health/health.h"
#include "../distance/distance.h"
#include "../tracking/tracking.h"
#include "../pftarget/pftarget.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/global/globals.h"
#include "../../../menu/overlay/settings.h"
#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <vector>
namespace thenwefuckin::aim::mouse
{
    static inline node get_target_node(const node& character, int partIdx);
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
    static std::vector<CachedTarget> s_targets;
    static std::chrono::steady_clock::time_point s_lastResolve{};
    static std::chrono::steady_clock::time_point s_lastScan{};
    static float s_remX = 0.0f;
    static float s_remY = 0.0f;
    static bool s_inDeadzone = false;
    static uintptr_t s_dzPlayer = 0;
    static std::chrono::steady_clock::time_point s_lastSend{};
    void on_game_joined()
    {
        s_targets.clear();
        s_lastResolve = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
        s_remX = 0.0f;
        s_remY = 0.0f;
        s_inDeadzone = false;
        s_dzPlayer = 0;
        s_lastSend = std::chrono::steady_clock::time_point{};
    }
    static void resolve_targets()
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
    static void send_mouse_move(float dx, float dy)
    {
        auto now = std::chrono::steady_clock::now();
        if (s_lastSend != std::chrono::steady_clock::time_point{} &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastSend).count() < 8)
        {
            s_remX += dx;
            s_remY += dy;
            if (fabsf(s_remX) > 32.0f) s_remX = 0.0f;
            if (fabsf(s_remY) > 32.0f) s_remY = 0.0f;
            return;
        }
        s_lastSend = now;
        dx += s_remX;
        dy += s_remY;
        int ix = static_cast<int>(std::round(dx));
        int iy = static_cast<int>(std::round(dy));
        s_remX = dx - static_cast<float>(ix);
        s_remY = dy - static_cast<float>(iy);
        if (fabsf(s_remX) > 8.0f) s_remX = 0.0f;
        if (fabsf(s_remY) > 8.0f) s_remY = 0.0f;
        if (ix == 0 && iy == 0)
            return;
        INPUT in{};
        in.type = INPUT_MOUSE;
        in.mi.dwFlags = MOUSEEVENTF_MOVE;
        in.mi.dx = ix;
        in.mi.dy = iy;
        SendInput(1, &in, sizeof(in));
    }
    static void pf_run(int w, int h)
    {
        pftarget::PfBest b{};
        if (!pftarget::select_best(w, h, b))
        {
            s_inDeadzone = false;
            s_dzPlayer = 0;
            s_remX = 0.0f;
            s_remY = 0.0f;
            thenwefuckin::aim::clear_debug_target();
            return;
        }
        if (b.model != s_dzPlayer)
        {
            s_inDeadzone = false;
            s_dzPlayer = b.model;
        }
        if (settings::aim_deadzone_enabled)
        {
            int dzMin = settings::aim_deadzone_min;
            int dzMax = settings::aim_deadzone_max;
            if (dzMin < 0) dzMin = 0;
            if (dzMax < dzMin) dzMax = dzMin;
            if (!s_inDeadzone && b.dist2d <= (float)dzMin)
                s_inDeadzone = true;
            else if (s_inDeadzone && b.dist2d > (float)dzMax)
                s_inDeadzone = false;
            if (s_inDeadzone)
                return;
        }
        else
        {
            s_inDeadzone = false;
        }
        thenwefuckin::aim::set_debug_target(b.world, b.model);
        float dx = b.screen.x - (float)w * 0.5f;
        float dy = b.screen.y - (float)h * 0.5f;
        if (!std::isfinite(dx) || !std::isfinite(dy))
            return;
        if (fabsf(dx) > (float)w || fabsf(dy) > (float)h)
            return;
        if (settings::aim_smoothing_enabled)
        {
            float sx = settings::aim_smoothing_x;
            float sy = settings::aim_smoothing_y;
            if (sx < 1.0f) sx = 1.0f;
            if (sy < 1.0f) sy = 1.0f;
            dx /= sx;
            dy /= sy;
        }
        send_mouse_move(dx, dy);
    }
    void run(int w, int h)
    {
        if (!g_ctx.ready)
            return;
        if (!g_ctx.camera)
            return;
        if (w <= 0 || h <= 0)
            return;
        if (g_menuOpen)
        {
            s_remX = 0.0f;
            s_remY = 0.0f;
            thenwefuckin::aim::clear_debug_target();
            return;
        }
        auto now = std::chrono::steady_clock::now();
        if (now - s_lastScan < std::chrono::milliseconds(8))
            return;
        s_lastScan = now;
        if (pftarget::active())
        {
            pf_run(w, h);
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
        float bestDist = (wide || prio != 0) ? 1e9f : settings::aim_fov;
        math::vec3 lpPos = g_ctx.local_pos;
        if (lpPos.x == 0.0f && lpPos.y == 0.0f && lpPos.z == 0.0f)
            lpPos = camPos;
        float bestPx = 1e9f;
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
            math::vec2 s{};
            if (!projection::to_screen(pos, view, static_cast<float>(w), static_cast<float>(h), s))
                continue;
            float dx = s.x - cx;
            float dy = s.y - cy;
            float px = sqrtf(dx * dx + dy * dy);
            float d = px;
            if (wide || prio == 2)
                d = (pos - camPos).length();
            else if (prio == 1)
                d = (pos - lpPos).length();
            if (d < bestDist)
            {
                bestDist = d;
                bestPx = px;
                best = pos;
                bestPlayer = t.playerAddr;
                found = true;
            }
        }
        if (!found)
        {
            s_inDeadzone = false;
            s_dzPlayer = 0;
            s_remX = 0.0f;
            s_remY = 0.0f;
            thenwefuckin::aim::clear_debug_target();
            return;
        }
        if (bestPlayer != s_dzPlayer)
        {
            s_inDeadzone = false;
            s_dzPlayer = bestPlayer;
        }
        if (settings::aim_deadzone_enabled)
        {
            int dzMin = settings::aim_deadzone_min;
            int dzMax = settings::aim_deadzone_max;
            if (dzMin < 0) dzMin = 0;
            if (dzMax < dzMin) dzMax = dzMin;
            if (!s_inDeadzone && bestPx <= (float)dzMin)
                s_inDeadzone = true;
            else if (s_inDeadzone && bestPx > (float)dzMax)
                s_inDeadzone = false;
            if (s_inDeadzone)
                return;
        }
        else
        {
            s_inDeadzone = false;
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
        math::vec2 targetScreen{};
        if (!projection::to_screen(best, view, static_cast<float>(w), static_cast<float>(h), targetScreen))
            return;
        float dx = targetScreen.x - cx;
        float dy = targetScreen.y - cy;
        if (!std::isfinite(dx) || !std::isfinite(dy))
            return;
        if (fabsf(dx) > (float)w || fabsf(dy) > (float)h)
            return;
        if (settings::aim_smoothing_enabled)
        {
            float sx = settings::aim_smoothing_x;
            float sy = settings::aim_smoothing_y;
            if (sx < 1.0f) sx = 1.0f;
            if (sy < 1.0f) sy = 1.0f;
            dx /= sx;
            dy /= sy;
        }
        send_mouse_move(dx, dy);
    }
}
