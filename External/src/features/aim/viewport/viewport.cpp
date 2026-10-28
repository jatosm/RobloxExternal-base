#include "viewport.h"
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
#include <Windows.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <vector>
namespace thenwefuckin::aim::viewport
{
    namespace
    {
        struct Vec2i16
        {
            std::int16_t x = 0;
            std::int16_t y = 0;
        };
        static_assert(sizeof(Vec2i16) == 4, "viewport pair must be 4 bytes");
        std::atomic<bool> g_active{ false };
        std::atomic<bool> g_stop{ false };
        std::atomic<bool> g_spoofed{ false };
        std::atomic<float> g_tx{ 0.f }, g_ty{ 0.f }, g_tz{ 0.f };
        HANDLE g_thread = nullptr;
        uintptr_t g_cam = 0;
        float g_dims_x = 0.f, g_dims_y = 0.f;
        Vec2i16 g_last{};
        int g_fails = 0;
        bool get_scene(float view[16], float& dw, float& dh)
        {
            uint64_t base = g_proc ? g_proc->base() : 0;
            if (!base) return false;
            uintptr_t ve = g_proc->read<uintptr_t>(base + Offsets::VisualEngine::Pointer);
            if (!ve) return false;
            if (!g_proc->read_buf(ve + Offsets::VisualEngine::ViewMatrix, view, sizeof(float) * 16))
                return false;
            if (Offsets::VisualEngine::Dimensions)
            {
                dw = g_proc->read<float>(ve + Offsets::VisualEngine::Dimensions);
                dh = g_proc->read<float>(ve + Offsets::VisualEngine::Dimensions + sizeof(float));
            }
            else
            {
                dw = (float)GetSystemMetrics(SM_CXSCREEN);
                dh = (float)GetSystemMetrics(SM_CYSCREEN);
            }
            return (dw >= 1.f && dh >= 1.f);
        }
        bool world_to_screen(const math::vec3& pos, float view[16], float dw, float dh, float& sx, float& sy)
        {
            float x = pos.x * view[0] + pos.y * view[1] + pos.z * view[2] + view[3];
            float y = pos.x * view[4] + pos.y * view[5] + pos.z * view[6] + view[7];
            float w = pos.x * view[12] + pos.y * view[13] + pos.z * view[14] + view[15];
            if (w < 0.001f) return false;
            float inv = 1.0f / w;
            sx = (dw * 0.5f) * (1.0f + x * inv);
            sy = (dh * 0.5f) * (1.0f - y * inv);
            return (sx > 0.f && sy > 0.f && sx < dw && sy < dh);
        }
        Vec2i16 calc_viewport(float tx, float ty, float dw, float dh, float mx, float my)
        {
            constexpr double kMaxShift = 0.65;
            double tyd = (double)ty;
            if (tyd > (double)dh - 1.0) tyd = (double)dh - 1.0;
            if (tyd < 1.0) tyd = 1.0;
            double ratio = (double)my / tyd;
            double vy = (double)dh * ratio;
            double maxDy = (double)dh * kMaxShift;
            double rawDy = vy - (double)dh;
            if (rawDy > maxDy) vy = (double)dh + maxDy;
            else if (rawDy < -maxDy) vy = (double)dh - maxDy;
            if (vy > 32767.0) vy = 32767.0;
            if (vy < 1.0) vy = 1.0;
            ratio = vy / (double)dh;
            double vx = 2.0 * (double)mx - ratio * (2.0 * (double)tx - (double)dw);
            double maxDx = (double)dw * kMaxShift;
            double rawDx = vx - (double)dw;
            if (rawDx > maxDx) vx = (double)dw + maxDx;
            else if (rawDx < -maxDx) vx = (double)dw - maxDx;
            if (vx > 32767.0) vx = 32767.0;
            if (vx < 1.0) vx = 1.0;
            return { (std::int16_t)std::lround(vx), (std::int16_t)std::lround(vy) };
        }
        HWND s_hwnd = nullptr;
        float s_cw = 0.f, s_ch = 0.f;
        std::chrono::steady_clock::time_point s_lastWnd{};
        bool get_mouse_in_viewport(float dw, float dh, float& mx, float& my)
        {
            auto now = std::chrono::steady_clock::now();
            if (!s_hwnd || !IsWindow(s_hwnd) ||
                std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastWnd).count() >= 2000)
            {
                s_lastWnd = now;
                s_hwnd = FindWindowW(nullptr, L"Roblox");
                if (!s_hwnd || !IsWindow(s_hwnd)) return false;
                RECT cr{};
                if (!GetClientRect(s_hwnd, &cr)) { s_hwnd = nullptr; return false; }
                s_cw = (float)(cr.right - cr.left);
                s_ch = (float)(cr.bottom - cr.top);
                if (s_cw < 1.f || s_ch < 1.f) { s_hwnd = nullptr; return false; }
            }
            POINT pt{};
            if (!GetCursorPos(&pt) || !ScreenToClient(s_hwnd, &pt)) return false;
            if (pt.x < 0 || pt.y < 0 || pt.x >= (LONG)s_cw || pt.y >= (LONG)s_ch) return false;
            mx = (float)pt.x * (dw / s_cw);
            my = (float)pt.y * (dh / s_ch);
            if (mx > dw - 1.f) mx = dw - 1.f;
            if (mx < 1.f) mx = 1.f;
            if (my > dh - 1.f) my = dh - 1.f;
            if (my < 1.f) my = 1.f;
            return true;
        }
        void write_viewport(uintptr_t cam, const Vec2i16& v)
        {
            if (!cam || !g_proc || !Offsets::Camera::ViewportInt16) return;
            g_proc->write<Vec2i16>(cam + Offsets::Camera::ViewportInt16, v);
            g_spoofed.store(true, std::memory_order_release);
        }
        void restore_viewport()
        {
            if (!g_cam || g_dims_x < 1.f || g_dims_y < 1.f || !g_proc || !Offsets::Camera::ViewportInt16) return;
            Vec2i16 v{
                (std::int16_t)std::lround(g_dims_x),
                (std::int16_t)std::lround(g_dims_y)
            };
            g_proc->write<Vec2i16>(g_cam + Offsets::Camera::ViewportInt16, v);
        }
        bool compute(Vec2i16& out)
        {
            if (!g_ctx.camera.addr) return false;
            math::vec3 world{
                g_tx.load(std::memory_order_relaxed),
                g_ty.load(std::memory_order_relaxed),
                g_tz.load(std::memory_order_relaxed)
            };
            float view[16]{};
            float dw = 0.f, dh = 0.f;
            if (!get_scene(view, dw, dh)) return false;
            float sx = 0.f, sy = 0.f;
            if (!world_to_screen(world, view, dw, dh, sx, sy)) return false;
            float mx = 0.f, my = 0.f;
            if (!get_mouse_in_viewport(dw, dh, mx, my)) return false;
            out = calc_viewport(sx, sy, dw, dh, mx, my);
            g_cam = g_ctx.camera.addr;
            g_dims_x = dw;
            g_dims_y = dh;
            return true;
        }
        DWORD WINAPI writer_thread(LPVOID)
        {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
            bool on = false;
            while (!g_stop.load(std::memory_order_acquire))
            {
                if (!g_active.load(std::memory_order_acquire))
                {
                    if (on)
                    {
                        restore_viewport();
                        g_spoofed.store(false, std::memory_order_release);
                        g_fails = 0;
                        on = false;
                    }
                    Sleep(16);
                    continue;
                }
                Vec2i16 v{};
                if (compute(v))
                {
                    Vec2i16 cur = (g_proc && Offsets::Camera::ViewportInt16) ? g_proc->read<Vec2i16>(g_cam + Offsets::Camera::ViewportInt16) : Vec2i16{};
                    if (cur.x != v.x || cur.y != v.y)
                        write_viewport(g_cam, v);
                    else
                        g_spoofed.store(true, std::memory_order_release);
                    g_last = v;
                    g_fails = 0;
                    on = true;
                }
                else if (on && g_fails < 40)
                {
                    ++g_fails;
                    write_viewport(g_cam, g_last);
                }
                else if (on)
                {
                    restore_viewport();
                    g_spoofed.store(false, std::memory_order_release);
                    on = false;
                    g_fails = 0;
                }
                Sleep(4);
            }
            restore_viewport();
            g_spoofed.store(false, std::memory_order_release);
            return 0;
        }
        void ensure_thread()
        {
            if (g_thread) return;
            g_stop.store(false, std::memory_order_release);
            g_thread = CreateThread(nullptr, 0, &writer_thread, nullptr, 0, nullptr);
            if (g_thread)
                SetThreadPriority(g_thread, THREAD_PRIORITY_NORMAL);
        }
        void set_target(const math::vec3& world)
        {
            ensure_thread();
            g_tx.store(world.x, std::memory_order_relaxed);
            g_ty.store(world.y, std::memory_order_relaxed);
            g_tz.store(world.z, std::memory_order_relaxed);
            g_active.store(true, std::memory_order_release);
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
        g_active.store(false, std::memory_order_release);
        if (g_spoofed.load(std::memory_order_acquire)) {
            restore_viewport();
            g_spoofed.store(false, std::memory_order_release);
        }
    }
    void on_game_joined()
    {
        g_active.store(false, std::memory_order_release);
        g_spoofed.store(false, std::memory_order_release);
        g_cam = 0;
        g_dims_x = 0.f;
        g_dims_y = 0.f;
        g_last = {};
        g_fails = 0;
        s_targets.clear();
        s_lastResolve = std::chrono::steady_clock::time_point{};
        s_lastScan = std::chrono::steady_clock::time_point{};
    }
    void shutdown()
    {
        g_active.store(false, std::memory_order_release);
        s_targets.clear();
        if (!g_thread) return;
        g_stop.store(true, std::memory_order_release);
        WaitForSingleObject(g_thread, 1000);
        CloseHandle(g_thread);
        g_thread = nullptr;
        g_cam = 0;
        g_dims_x = g_dims_y = 0.f;
    }
    bool is_active()
    {
        return g_active.load(std::memory_order_acquire) &&
               g_spoofed.load(std::memory_order_acquire);
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
            set_target(b.world);
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
        set_target(best);
    }
}
