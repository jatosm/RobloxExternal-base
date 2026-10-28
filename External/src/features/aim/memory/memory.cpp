#include "memory.h"
#include "../core/aim.h"
#include "../predict/predict.h"
#include "../pftarget/pftarget.h"
#include "../tracking/tracking.h"
#include "../health/health.h"
#include "../distance/distance.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/struct/offsets.h"
#include "../../../utility/global/globals.h"
#include "../../../menu/overlay/settings.h"
#include "../../visual/world/geo/geo.h"
#include <Windows.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <vector>
namespace thenwefuckin::aim::memory
{
    namespace
    {
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
        bool roblox_foreground()
        {
            if (!g_proc || !g_proc->connected())
                return false;
            HWND fg = GetForegroundWindow();
            if (!fg)
                return false;
            DWORD pid = 0;
            GetWindowThreadProcessId(fg, &pid);
            return pid != 0 && pid == g_proc->pid();
        }
        void write_camera_at(const math::vec3& world)
        {
            if (g_menuOpen || !roblox_foreground())
                return;
            if (!g_ctx.camera)
                return;
            math::vec3 camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
            if (camPos.x == 0.0f && camPos.y == 0.0f && camPos.z == 0.0f)
                return;
            math::vec3 d{ world.x - camPos.x, world.y - camPos.y, world.z - camPos.z };
            float len = d.length();
            if (len < 0.1f)
                return;
            math::vec3 fwd = d / len;
            float yaw = atan2f(fwd.x, -fwd.z);
            float pitch = asinf((std::max)(-1.0f, (std::min)(1.0f, fwd.y)));
            float cp = cosf(pitch);
            math::vec3 look{ sinf(yaw) * cp, sinf(pitch), -cosf(yaw) * cp };
            look = look.normalized();
            math::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
            math::vec3 right{
                look.y * worldUp.z - look.z * worldUp.y,
                look.z * worldUp.x - look.x * worldUp.z,
                look.x * worldUp.y - look.y * worldUp.x
            };
            float rightLen = right.length();
            if (rightLen < 0.000001f)
                return;
            right = right / rightLen;
            math::vec3 up{
                right.y * look.z - right.z * look.y,
                right.z * look.x - right.x * look.z,
                right.x * look.y - right.y * look.x
            };
            float upLen = up.length();
            if (upLen < 0.000001f)
                return;
            up = up / upLen;
            math::vec3 back = look * -1.0f;
            float blob[12] = {
                right.x, up.x, back.x,
                right.y, up.y, back.y,
                right.z, up.z, back.z,
                camPos.x, camPos.y, camPos.z
            };
            g_proc->write_buf(g_ctx.camera.addr + Offsets::Camera::Rotation, blob, sizeof(blob));
        }
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
    void clear()
    {
        s_targets.clear();
        s_lastResolve = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
    }
    void on_game_joined()
    {
        s_targets.clear();
        s_lastResolve = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
    }
    void run(int w, int h)
    {
        if (!g_ctx.ready || !g_ctx.camera || w <= 0 || h <= 0)
        {
            clear();
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
                clear();
                thenwefuckin::aim::clear_debug_target();
                return;
            }
            thenwefuckin::aim::set_debug_target(b.world, b.model);
            write_camera_at(b.world);
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
                clear();
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
        float bestDist = wide ? 1e9f : settings::aim_fov;
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
            float d = wide ? (pos - camPos).length() : sqrtf(dx * dx + dy * dy);
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
            clear();
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
        write_camera_at(best);
    }
}
