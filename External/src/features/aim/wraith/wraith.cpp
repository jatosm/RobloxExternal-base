#include "wraith.h"
#include "../core/aim.h"
#include "../predict/predict.h"
#include "../../visual/world/geo/geo.h"
#include "../pftarget/pftarget.h"
#include "../smooth/smooth.h"
#include "../tracking/tracking.h"
#include "../health/health.h"
#include "../distance/distance.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include <Windows.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>
namespace thenwefuckin::aim::wraith
{
    namespace
    {
        std::atomic<bool> s_run{ false };
        std::atomic<bool> s_active{ false };
        std::atomic<std::uintptr_t> s_waddr{ 0 };
        std::atomic<bool> s_isPart{ false };
        float s_blob[12]{};
        std::mutex s_blobMtx;
        std::thread s_thread;
        bool s_started = false;
        void apply_blob(std::uintptr_t addr, bool isPart, const float blob[12])
        {
            if (!addr || !g_proc)
                return;
            if (isPart)
                g_proc->write_buf(addr + Offsets::Primitive::Rotation, blob, sizeof(float) * 12);
            else
                g_proc->write_buf(addr + Offsets::Camera::Rotation, blob, sizeof(float) * 12);
        }
        void writer_loop()
        {
            while (s_run.load(std::memory_order_acquire))
            {
                if (s_active.load(std::memory_order_acquire))
                {
                    float blob[12];
                    std::uintptr_t addr = 0;
                    bool isPart = false;
                    {
                        std::lock_guard<std::mutex> lk(s_blobMtx);
                        memcpy(blob, s_blob, sizeof(blob));
                        addr = s_waddr.load(std::memory_order_relaxed);
                        isPart = s_isPart.load(std::memory_order_relaxed);
                    }
                    apply_blob(addr, isPart, blob);
                    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
                        apply_blob(addr, isPart, blob);
                }
                bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(
                    (s_active.load(std::memory_order_acquire) && held) ? 1 : 10));
            }
        }
        void ensure_writer()
        {
            if (s_started)
                return;
            s_started = true;
            s_run.store(true, std::memory_order_release);
            try
            {
                s_thread = std::thread(writer_loop);
            }
            catch (...)
            {
                s_started = false;
                s_run.store(false, std::memory_order_release);
            }
        }
        bool resolve_target(std::uintptr_t& outAddr, bool& outIsPart)
        {
            outAddr = 0;
            outIsPart = false;
            if (!g_ctx.workspace)
                return false;
            std::uintptr_t camParent = 0;
            for (auto& ch : g_ctx.workspace.children())
            {
                if (ch.class_name() == "Camera")
                {
                    camParent = ch.addr;
                    break;
                }
            }
            if (!camParent && g_ctx.camera)
                camParent = g_ctx.camera.addr;
            if (!camParent)
                return false;
            for (auto& ch : node{ camParent }.children())
            {
                if (ch.name() == "Part")
                {
                    std::uintptr_t prim = ch.primitive();
                    if (prim)
                    {
                        outAddr = prim;
                        outIsPart = true;
                        return true;
                    }
                    break;
                }
            }
            outAddr = camParent;
            outIsPart = false;
            return true;
        }
        math::vec3 target_pos(std::uintptr_t addr, bool isPart, bool& ok)
        {
            ok = false;
            if (!addr || !g_proc)
                return {};
            math::vec3 p = isPart
                ? g_proc->read<math::vec3>(addr + Offsets::Primitive::Position)
                : g_proc->read<math::vec3>(addr + Offsets::Camera::Position);
            if (p.x == 0.0f && p.y == 0.0f && p.z == 0.0f)
                return {};
            ok = true;
            return p;
        }
        bool current_forward(std::uintptr_t addr, bool isPart, math::vec3& fwd)
        {
            if (!addr || !g_proc)
                return false;
            math::frame cf = isPart
                ? g_proc->read<math::frame>(addr + Offsets::Primitive::Rotation)
                : g_proc->read<math::frame>(addr + Offsets::Camera::Rotation);
            fwd = math::vec3{ -cf.rot[2], -cf.rot[5], -cf.rot[8] };
            if (fwd.length() < 0.000001f)
                return false;
            fwd = fwd / fwd.length();
            return true;
        }
        void set_active(bool on, const math::vec3& target)
        {
            if (!on)
            {
                s_active.store(false, std::memory_order_release);
                return;
            }
            if (target.x == 0.0f && target.y == 0.0f && target.z == 0.0f)
                return;
            std::uintptr_t addr = 0;
            bool isPart = false;
            if (!resolve_target(addr, isPart))
            {
                s_active.store(false, std::memory_order_release);
                return;
            }
            bool posOk = false;
            math::vec3 pos = target_pos(addr, isPart, posOk);
            if (!posOk)
                return;
            math::vec3 wantFwd = target - pos;
            if (wantFwd.length() < 0.000001f)
                return;
            wantFwd = wantFwd / wantFwd.length();
            float curYaw = 0.0f, curPitch = 0.0f;
            math::vec3 curFwd{};
            if (current_forward(addr, isPart, curFwd))
            {
                curPitch = asinf(std::clamp(curFwd.y, -1.0f, 1.0f));
                curYaw = atan2f(curFwd.x, -curFwd.z);
            }
            float wantPitch = asinf(std::clamp(wantFwd.y, -1.0f, 1.0f));
            float wantYaw = atan2f(wantFwd.x, -wantFwd.z);
            float newYaw = wantYaw, newPitch = wantPitch;
            smooth::calculate(curYaw, curPitch, wantYaw, wantPitch, newYaw, newPitch);
            float cp = cosf(newPitch);
            math::vec3 look{ sinf(newYaw) * cp, sinf(newPitch), -cosf(newYaw) * cp };
            look = look.normalized();
            if (look.length() < 0.000001f)
                return;
            math::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
            math::vec3 right{
                look.y * worldUp.z - look.z * worldUp.y,
                look.z * worldUp.x - look.x * worldUp.z,
                look.x * worldUp.y - look.y * worldUp.x
            };
            if (right.length() < 0.000001f)
                return;
            right = right / right.length();
            math::vec3 up{
                right.y * look.z - right.z * look.y,
                right.z * look.x - right.x * look.z,
                right.x * look.y - right.y * look.x
            };
            if (up.length() < 0.000001f)
                return;
            up = up / up.length();
            math::vec3 back = look * -1.0f;
            float blob[12] = {
                right.x, up.x, back.x,
                right.y, up.y, back.y,
                right.z, up.z, back.z,
                pos.x, pos.y, pos.z
            };
            apply_blob(addr, isPart, blob);
            {
                std::lock_guard<std::mutex> lk(s_blobMtx);
                memcpy(s_blob, blob, sizeof(blob));
                s_waddr.store(addr, std::memory_order_relaxed);
                s_isPart.store(isPart, std::memory_order_relaxed);
            }
            ensure_writer();
            s_active.store(true, std::memory_order_release);
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
            auto list = g_ctx.player_list();
            s_targets.reserve(list.size());
            for (auto& entry : list)
            {
                if (entry.addr == g_ctx.local_player.addr)
                    continue;
                node character = g_ctx.get_character(entry);
                if (!character)
                    continue;
                uintptr_t charAddr = character.addr;
                node humanoid = find_live_humanoid(character);
                if (!humanoid)
                    continue;
                node targetPart = resolve_part(character, wantPart);
                if (!targetPart)
                    targetPart = character.find_child("Head");
                if (!targetPart)
                    continue;
                uintptr_t prim = targetPart.primitive();
                if (!prim)
                    continue;
                CachedTarget t{};
                t.playerAddr = entry.addr;
                t.characterAddr = charAddr;
                t.humanoidAddr = humanoid.addr;
                t.targetPrim = prim;
                t.teamAddr = g_proc ? g_proc->read<uintptr_t>(entry.addr + Offsets::Player::Team) : 0;
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
    bool is_active()
    {
        return s_active.load(std::memory_order_acquire);
    }
    void idle()
    {
        s_active.store(false, std::memory_order_release);
        s_targets.clear();
    }
    void on_game_joined()
    {
        s_active.store(false, std::memory_order_release);
        s_waddr.store(0, std::memory_order_relaxed);
        s_isPart.store(false, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lk(s_blobMtx);
            memset(s_blob, 0, sizeof(s_blob));
        }
        s_targets.clear();
        s_lastResolve = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
    }
    void shutdown()
    {
        s_active.store(false, std::memory_order_release);
        s_run.store(false, std::memory_order_release);
        s_targets.clear();
        if (s_thread.joinable())
        {
            try { s_thread.join(); }
            catch (...) {}
        }
        s_started = false;
    }
    void run(int w, int h)
    {
        if (!g_ctx.ready || !g_ctx.camera || w <= 0 || h <= 0)
        {
            set_active(false, math::vec3{});
            thenwefuckin::aim::clear_debug_target();
            return;
        }
        auto now = std::chrono::steady_clock::now();
        if (now - s_lastScan < std::chrono::milliseconds(4))
            return;
        s_lastScan = now;
        if (pftarget::active())
        {
            pftarget::PfBest b{};
            if (!pftarget::select_best(w, h, b))
            {
                set_active(false, math::vec3{});
                thenwefuckin::aim::clear_debug_target();
                return;
            }
            thenwefuckin::aim::set_debug_target(b.world, b.model);
            set_active(true, b.world);
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
                set_active(false, math::vec3{});
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
                bestPlayer = t.playerAddr;
                found = true;
            }
        }
        if (!found)
        {
            set_active(false, math::vec3{});
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
        set_active(true, best);
    }
}
