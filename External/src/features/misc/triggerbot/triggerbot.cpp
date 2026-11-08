#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "triggerbot.h"
#include "../../aim/core/aim.h"
#include "../../visual/world/geo/geo.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <vector>
namespace thenwefuckin::misc::triggerbot
{
    namespace
    {
        struct Mat3
        {
            float m[3][3]{};
        };
        struct Ray
        {
            math::vec3 origin{};
            math::vec3 dir{};
        };
        static std::chrono::steady_clock::time_point s_lastPoll{};
        static std::chrono::steady_clock::time_point s_firstSeen{};
        static std::chrono::steady_clock::time_point s_lastShot{};
        static bool s_hasTarget = false;

        math::vec3 axis(const Mat3& r, int i)
        {
            return { r.m[i][0], r.m[i][1], r.m[i][2] };
        }
        bool ray_obb(const Ray& ray, const math::vec3& center, const math::vec3& size,
            const Mat3& rot, float max_dist, float& out_t)
        {
            const math::vec3 half = size * 0.5f;
            const float h[3] = { half.x, half.y, half.z };
            const math::vec3 p = center - ray.origin;
            float tmin = 0.0f;
            float tmax = max_dist;
            for (int i = 0; i < 3; ++i)
            {
                const math::vec3 ax = axis(rot, i);
                const float e = ax.dot(p);
                const float f = ax.dot(ray.dir);
                const float eps = 1e-6f;
                if (std::fabs(f) > eps)
                {
                    float t1 = (e - h[i]) / f;
                    float t2 = (e + h[i]) / f;
                    if (t1 > t2)
                        std::swap(t1, t2);
                    tmin = (std::max)(tmin, t1);
                    tmax = (std::min)(tmax, t2);
                    if (tmax < tmin)
                        return false;
                }
                else if ((-e - h[i] > 0.0f) || (-e + h[i] < 0.0f))
                {
                    return false;
                }
            }
            if (tmax < 0.0f)
                return false;
            out_t = (tmin >= 0.0f) ? tmin : tmax;
            return true;
        }
        void push_prim(std::vector<std::uintptr_t>& out, std::uintptr_t prim)
        {
            if (prim)
                out.push_back(prim);
        }
        void collect_prims(const visual::LimbCache& lc, int part, std::vector<std::uintptr_t>& out)
        {
            out.clear();
            switch (part)
            {
            case 0:
                push_prim(out, lc.headPrim);
                break;
            case 1:
                push_prim(out, lc.upperPrim);
                push_prim(out, lc.torsoPrim);
                push_prim(out, lc.lowerPrim);
                break;
            case 2:
                push_prim(out, lc.rootPrim);
                break;
            case 3:
                push_prim(out, lc.leftUpperArmPrim);
                push_prim(out, lc.leftArmPrim);
                push_prim(out, lc.leftLowerArmPrim);
                push_prim(out, lc.leftHandPrim);
                break;
            case 4:
                push_prim(out, lc.rightUpperArmPrim);
                push_prim(out, lc.rightArmPrim);
                push_prim(out, lc.rightLowerArmPrim);
                push_prim(out, lc.rightHandPrim);
                break;
            case 5:
                push_prim(out, lc.leftUpperLegPrim);
                push_prim(out, lc.leftLegPrim);
                push_prim(out, lc.leftLowerLegPrim);
                push_prim(out, lc.leftFootPrim);
                break;
            case 6:
                push_prim(out, lc.rightUpperLegPrim);
                push_prim(out, lc.rightLegPrim);
                push_prim(out, lc.rightLowerLegPrim);
                push_prim(out, lc.rightFootPrim);
                break;
            default:
                push_prim(out, lc.headPrim);
                push_prim(out, lc.upperPrim);
                push_prim(out, lc.torsoPrim);
                push_prim(out, lc.rootPrim);
                break;
            }
            if (out.empty())
            {
                push_prim(out, lc.headPrim);
                push_prim(out, lc.torsoPrim);
                push_prim(out, lc.rootPrim);
            }
        }
        bool usable_pos(const math::vec3& p)
        {
            return !(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f);
        }
        bool skip_entry(const pcache::Entry& e, float& hp)
        {
            hp = e.hp;
            if (!e.valid || e.player == g_ctx.local_player.addr || !e.model)
                return true;
            if (hp <= 0.0f)
                return true;
            if (settings::triggerbot_team_check && thenwefuckin::aim::is_teammate(e.player, e.model, e.team))
                return true;
            return false;
        }
        void snapshot(std::vector<pcache::Entry>& out)
        {
            std::lock_guard<std::mutex> lk(pcache::mtx);
            out = pcache::entries;
        }
        bool screen_on_target(float sw, float sh, math::vec3& out_hit)
        {
            const float cx = sw * 0.5f;
            const float cy = sh * 0.5f;
            const float thresh = (std::max)(1.0f, settings::triggerbot_threshold);
            const float threshSq = thresh * thresh;
            math::vec3 camPos{};
            if (settings::triggerbot_wallcheck && g_ctx.camera)
                camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
            std::vector<pcache::Entry> snap;
            snapshot(snap);
            std::vector<std::uintptr_t> prims;
            for (auto& e : snap)
            {
                float hp = 0.0f;
                if (skip_entry(e, hp))
                    continue;
                collect_prims(e.lc, settings::triggerbot_part, prims);
                for (auto prim : prims)
                {
                    const math::vec3 pos = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
                    if (!usable_pos(pos))
                        continue;
                    math::vec2 s{};
                    if (!projection::to_screen(pos, g_ctx.view, sw, sh, s))
                        continue;
                    const float dx = s.x - cx;
                    const float dy = s.y - cy;
                    if (dx * dx + dy * dy > threshSq)
                        continue;
                    if (settings::triggerbot_wallcheck && usable_pos(camPos) && !worldgeo::visible(camPos, pos))
                        continue;
                    out_hit = pos;
                    return true;
                }
            }
            return false;
        }
        bool ray_on_target(math::vec3& out_hit)
        {
            const std::uintptr_t cam = g_ctx.camera.addr;
            const Mat3 rot = g_proc->read<Mat3>(cam + Offsets::Camera::Rotation);
            const math::vec3 origin = g_proc->read<math::vec3>(cam + Offsets::Camera::Position);
            if (!usable_pos(origin))
                return false;
            Ray ray{ origin, (axis(rot, 2) * -1.0f).normalized() };
            if (!usable_pos(ray.dir))
                return false;
            const float padding = (std::max)(0.0f, (settings::triggerbot_threshold - 1.0f) * 0.05f);
            float bestT = 1000.0f;
            bool hit = false;
            math::vec3 hitPoint{};
            std::vector<pcache::Entry> snap;
            snapshot(snap);
            std::vector<std::uintptr_t> prims;
            for (auto& e : snap)
            {
                float hp = 0.0f;
                if (skip_entry(e, hp))
                    continue;
                collect_prims(e.lc, settings::triggerbot_part, prims);
                for (auto prim : prims)
                {
                    const math::vec3 c = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
                    if (!usable_pos(c))
                        continue;
                    math::vec3 sz = g_proc->read<math::vec3>(prim + Offsets::Primitive::Size);
                    if (sz.x <= 0.01f || sz.y <= 0.01f || sz.z <= 0.01f)
                        continue;
                    const Mat3 prot = g_proc->read<Mat3>(prim + Offsets::Primitive::Rotation);
                    sz.x += padding;
                    sz.y += padding;
                    sz.z += padding;
                    float t = 0.0f;
                    if (!ray_obb(ray, c, sz, prot, bestT, t))
                        continue;
                    bestT = t;
                    hit = true;
                    hitPoint = ray.origin + ray.dir * t;
                }
            }
            if (!hit)
                return false;
            if (settings::triggerbot_wallcheck && !worldgeo::visible(origin, hitPoint))
                return false;
            out_hit = hitPoint;
            return true;
        }
        void fire()
        {
            INPUT inputs[2] = {};
            inputs[0].type = INPUT_MOUSE;
            inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            inputs[1].type = INPUT_MOUSE;
            inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(2, inputs, sizeof(INPUT));
        }
        void reset_state()
        {
            s_hasTarget = false;
            s_firstSeen = std::chrono::steady_clock::time_point{};
        }
    }
    void run()
    {
        if (!settings::triggerbot_enabled)
        {
            reset_state();
            return;
        }
        if (g_menuOpen)
        {
            reset_state();
            return;
        }
        if (!g_ctx.ready || !g_proc || !g_proc->connected() || !g_ctx.camera)
        {
            reset_state();
            return;
        }
        if (settings::triggerbot_key_mode != 0 &&
            !Menu::IsMenuKeyActive("##triggerbot_key", settings::triggerbot_key, settings::triggerbot_key_mode))
        {
            reset_state();
            return;
        }
        using clock = std::chrono::steady_clock;
        const auto now = clock::now();
        if (s_lastPoll.time_since_epoch().count() != 0 &&
            now - s_lastPoll < std::chrono::milliseconds(10))
            return;
        s_lastPoll = now;
        const math::vec2 dims = projection::screen_size();
        if (dims.x < 100.0f || dims.y < 100.0f)
            return;
        math::vec3 hit{};
        const bool onTarget = (settings::triggerbot_method == 1)
            ? ray_on_target(hit)
            : screen_on_target(dims.x, dims.y, hit);
        if (!onTarget)
        {
            reset_state();
            return;
        }
        if (!s_hasTarget)
        {
            s_hasTarget = true;
            s_firstSeen = now;
        }
        const long long seenFor = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_firstSeen).count();
        const long long sinceShot = s_lastShot.time_since_epoch().count() == 0
            ? 1LL << 40
            : std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastShot).count();
        float delay = settings::triggerbot_delay;
        if (delay < 0.0f)
            delay = 0.0f;
        const long long cooldown = static_cast<long long>((delay > 60.0f) ? delay : 60.0f);
        if (seenFor >= static_cast<long long>(delay) && sinceShot >= cooldown)
        {
            fire();
            s_lastShot = now;
        }
    }
}
