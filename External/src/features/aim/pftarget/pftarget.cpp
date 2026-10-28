#include "pftarget.h"
#include "../core/aim.h"
#include "../predict/predict.h"
#include "../../visual/world/geo/geo.h"
#include "../tracking/tracking.h"
#include "../health/health.h"
#include "../distance/distance.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include <chrono>
#include <cmath>
#include <vector>
namespace thenwefuckin::aim::pftarget
{
    bool active()
    {
        using Clock = std::chrono::steady_clock;
        static bool cached = false;
        static Clock::time_point lastCheck{};
        auto now = Clock::now();
        if (now - lastCheck > std::chrono::milliseconds(2000))
        {
            lastCheck = now;
            cached = PfCache::IsActivePlace();
        }
        return cached;
    }
    bool select_best(int w, int h, PfBest& out)
    {
        if (!g_ctx.ready || !g_ctx.camera || w <= 0 || h <= 0)
            return false;
        if (PfCache::players.empty())
            return false;
        float cx = static_cast<float>(w) * 0.5f;
        float cy = static_cast<float>(h) * 0.5f;
        const math::view_matrix& view = g_ctx.view;
        math::vec3 camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        if (camPos.x == 0.0f && camPos.y == 0.0f && camPos.z == 0.0f)
            return false;
        bool wantHead = (settings::aim_part == 0);
        math::vec3 best{};
        math::vec2 bestS{};
        bool found = false;
        const bool wide = settings::aim_360;
        const int prio = settings::aim_priority;
        float bestDist = (wide || prio != 0) ? 1e9f : settings::aim_fov;
        uintptr_t bestModel = 0;
        math::vec3 lpPos = g_ctx.local_pos;
        if (lpPos.x == 0.0f && lpPos.y == 0.0f && lpPos.z == 0.0f)
            lpPos = camPos;
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
            uintptr_t prim = wantHead ? p.headPrim : p.torsoPrim;
            if (!prim)
                prim = p.headPrim ? p.headPrim : p.torsoPrim;
            if (!prim)
                continue;
            math::vec3 pos = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
            if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
                continue;
            tracking::push(p.modelAddr, pos.x, pos.y, pos.z);
            if (!distance::check(pos, camPos))
                continue;
            if (settings::aim_visible_check && !worldgeo::visible(camPos, pos))
                continue;
            math::vec2 s{};
            if (!projection::to_screen(pos, view, static_cast<float>(w), static_cast<float>(h), s))
                continue;
            float dx = s.x - cx;
            float dy = s.y - cy;
            float d = 0.0f;
            if (wide || prio == 2)
                d = (pos - camPos).length();
            else if (prio == 1)
                d = (pos - lpPos).length();
            else
                d = sqrtf(dx * dx + dy * dy);
            if (d < bestDist)
            {
                bestDist = d;
                best = pos;
                bestS = s;
                bestModel = p.modelAddr;
                found = true;
            }
        }
        if (settings::aim_bots)
        {
            std::vector<BotAimTarget> bots;
            collect_bot_targets(bots);
            for (auto& b : bots)
            {
                std::uintptr_t par = g_proc->read<std::uintptr_t>(b.inst + Offsets::Instance::Parent);
                if (!par)
                    continue;
                float bhp = b.humanoid ? g_proc->read<float>(b.humanoid + Offsets::Humanoid::Health) : b.hp;
                if (!health::check(bhp))
                    continue;
                uintptr_t prim = wantHead ? b.headPrim : b.rootPrim;
                if (!prim)
                    prim = b.headPrim ? b.headPrim : b.rootPrim;
                if (!prim)
                    continue;
                math::vec3 pos = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
                if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
                    continue;
                tracking::push(b.inst, pos.x, pos.y, pos.z);
                if (!distance::check(pos, camPos))
                    continue;
                if (settings::aim_visible_check && !worldgeo::visible(camPos, pos))
                    continue;
                math::vec2 s{};
                if (!projection::to_screen(pos, view, static_cast<float>(w), static_cast<float>(h), s))
                    continue;
                float dx = s.x - cx;
                float dy = s.y - cy;
                float d = 0.0f;
                if (wide || prio == 2)
                    d = (pos - camPos).length();
                else if (prio == 1)
                    d = (pos - lpPos).length();
                else
                    d = sqrtf(dx * dx + dy * dy);
                if (d < bestDist)
                {
                    bestDist = d;
                    best = pos;
                    bestS = s;
                    bestModel = b.inst;
                    found = true;
                }
            }
        }
        if (!found)
            return false;
        tracking::Velocity tv{};
        bool haveTv = tracking::sample(bestModel, tv);
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
        math::vec2 sLead{};
        if (projection::to_screen(best, view, static_cast<float>(w), static_cast<float>(h), sLead))
            bestS = sLead;
        out.world = best;
        out.screen = bestS;
        out.dist2d = sqrtf((bestS.x - cx) * (bestS.x - cx) + (bestS.y - cy) * (bestS.y - cy));
        out.model = bestModel;
        return true;
    }
}
