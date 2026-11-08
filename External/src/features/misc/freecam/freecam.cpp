#include "freecam.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/global/globals.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include "../fov/fov.h"
#include "../zoom/zoom.h"
#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <thread>
namespace thenwefuckin::misc::freecam
{
    namespace
    {
        bool s_active = false;
        math::vec3 s_pos{};
        float s_yaw = 0.0f;
        float s_pitch = 0.0f;
        float s_savedWalkSpeed = 16.0f;
        bool s_speedLocked = false;
        std::chrono::steady_clock::time_point s_lastTick{};
        uint64_t s_tickCount = 0;
        math::vec3 s_lastWritten{};
        float s_lastWrittenYaw = 0.0f;
        float s_lastWrittenPitch = 0.0f;
        bool s_haveWritten = false;
        bool s_keyPrev = false;
        bool s_keyToggled = false;
        float s_savedWalkSpeedCheck = 16.0f;
        uintptr_t s_savedSubject = 0;
        int s_savedCamType = 0;
        bool s_haveSavedCam = false;
        uintptr_t s_hrpPrim = 0;
        bool s_hrpWasAnchored = false;
        uintptr_t s_camAddr = 0;
        uintptr_t s_camPrim = 0;
        bool s_camIsPart = false;
        bool s_savedIsPart = false;
        std::chrono::steady_clock::time_point s_lastCamResolve{};
        static constexpr int kCamTypeScriptable = 6;
        void set_hrp_anchored(uintptr_t prim, bool anchored)
        {
            if (!prim || !g_proc || !g_proc->connected() || !Offsets::Primitive::PrimitiveFlags)
                return;
            uint8_t flags = g_proc->read<uint8_t>(prim + Offsets::Primitive::PrimitiveFlags);
            bool has = (flags & Offsets::PrimitiveFlags::Anchored) != 0;
            if (has == anchored)
                return;
            uint8_t next = anchored
                ? static_cast<uint8_t>(flags | Offsets::PrimitiveFlags::Anchored)
                : static_cast<uint8_t>(flags & ~Offsets::PrimitiveFlags::Anchored);
            g_proc->write<uint8_t>(prim + Offsets::Primitive::PrimitiveFlags, next);
        }
        void zero_hrp_velocity(uintptr_t prim)
        {
            if (!prim || !g_proc || !g_proc->connected())
                return;
            math::vec3 zero{};
            g_proc->write<math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, zero);
        }
        uintptr_t resolve_hrp_prim()
        {
            node root = g_ctx.local_root;
            if (!root)
                return 0;
            return root.primitive();
        }
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
        void read_yaw_pitch(float& yaw, float& pitch)
        {
            math::frame cf{};
            if (s_camIsPart && s_camPrim)
                cf = g_proc->read<math::frame>(s_camPrim + Offsets::Primitive::Rotation);
            else if (g_ctx.camera)
                cf = g_proc->read<math::frame>(g_ctx.camera.addr + Offsets::Camera::Rotation);
            else
                return;
            math::vec3 fwd{ -cf.rot[2], -cf.rot[5], -cf.rot[8] };
            float len = fwd.length();
            if (len < 0.000001f)
                return;
            fwd = fwd / len;
            pitch = asinf(std::clamp(fwd.y, -1.0f, 1.0f));
            yaw = atan2f(fwd.x, -fwd.z);
        }
        math::vec3 read_cam_pos()
        {
            if (s_camIsPart && s_camPrim)
                return g_proc->read<math::vec3>(s_camPrim + Offsets::Primitive::Position);
            if (g_ctx.camera)
                return g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
            return {};
        }
        void write_cam_rot(const float blob12[12])
        {
            if (s_camIsPart && s_camPrim)
                g_proc->write_buf(s_camPrim + Offsets::Primitive::Rotation, blob12, sizeof(float) * 12);
            else if (g_ctx.camera)
                g_proc->write_buf(g_ctx.camera.addr + Offsets::Camera::Rotation, blob12, sizeof(float) * 12);
        }
        bool resolve_cam_target(bool force)
        {
            auto now = std::chrono::steady_clock::now();
            if (!force && s_camAddr != 0 &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastCamResolve).count() < 2000)
                return true;
            s_lastCamResolve = now;
            s_camAddr = 0;
            s_camPrim = 0;
            s_camIsPart = false;
            if (!g_ctx.workspace)
                return false;
            uintptr_t camParent = 0;
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
                    uintptr_t prim = ch.primitive();
                    if (prim)
                    {
                        s_camAddr = ch.addr;
                        s_camPrim = prim;
                        s_camIsPart = true;
                        return true;
                    }
                    break;
                }
            }
            s_camAddr = camParent;
            s_camIsPart = false;
            return true;
        }
    }
    void tick(bool want);
    void run()
    {
        bool want = settings::freecam_enabled;
        if (want && settings::freecam_key != 0)
        {
            want = Menu::IsMenuKeyActive("##freecam_key", settings::freecam_key, settings::freecam_key_mode);
        }
        tick(want);
    }
    bool worker_key_down(int key)
    {
        if (key <= 0) return false;
        int vk = Menu::ImGuiKeyToVirtualKey(key);
        if (vk != 0) return (GetAsyncKeyState(vk) & 0x8000) != 0;
        return false;
    }
    bool worker_want_active()
    {
        if (!settings::freecam_enabled)
        {
            s_keyPrev = false;
            s_keyToggled = false;
            return false;
        }
        int key = settings::freecam_key;
        int mode = settings::freecam_key_mode;
        if (mode == 0) return true;
        if (key <= 0) return false;
        bool down = worker_key_down(key);
        if (mode == 2) return down;
        if (down && !s_keyPrev) s_keyToggled = !s_keyToggled;
        s_keyPrev = down;
        return s_keyToggled;
    }
    void thread_main(std::atomic<bool>& stop)
    {
        HANDLE self = GetCurrentThread();
        SetThreadPriority(self, THREAD_PRIORITY_ABOVE_NORMAL);
        while (!stop.load(std::memory_order_relaxed))
        {
            bool want = worker_want_active();
            bool fovWant = fov::worker_want_active();
            bool zoomWant = zoom::worker_want_active();
            if (!want && !s_active && !fovWant && !fov::is_active() && !zoomWant && !zoom::is_active())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
            if (want || s_active)
            {
                tick(want);
            }
            fov::tick_fast(fovWant);
            zoom::tick_fast(zoomWant);
        }
        fov::tick_fast(false);
        zoom::tick_fast(false);
        SetThreadPriority(self, THREAD_PRIORITY_NORMAL);
    }
    void tick(bool want)
    {
        if (!want && !s_active)
            return;
        if (!g_proc || !g_proc->connected())
        {
            s_active = false;
            s_speedLocked = false;
            return;
        }
        if (!want)
        {
            if (s_speedLocked)
            {
                node humanoid = g_ctx.resolve_local_humanoid();
                if (humanoid)
                {
                    g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeed, s_savedWalkSpeed);
                    g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeedCheck, s_savedWalkSpeedCheck);
                }
                s_speedLocked = false;
            }
            if (s_hrpPrim)
            {
                set_hrp_anchored(s_hrpPrim, s_hrpWasAnchored);
                zero_hrp_velocity(s_hrpPrim);
                s_hrpPrim = 0;
            }
            if (s_haveSavedCam && !s_savedIsPart && g_ctx.camera)
            {
                if (Offsets::Camera::CameraType)
                    g_proc->write<int>(g_ctx.camera.addr + Offsets::Camera::CameraType, s_savedCamType);
                g_proc->write<uintptr_t>(g_ctx.camera.addr + Offsets::Camera::CameraSubject, s_savedSubject);
                s_haveSavedCam = false;
            }
            s_active = false;
            s_haveWritten = false;
            s_camAddr = 0;
            s_camPrim = 0;
            s_camIsPart = false;
            return;
        }
        if (!g_ctx.ready || !g_ctx.camera)
            return;
        auto now = std::chrono::steady_clock::now();
        if (!s_active)
        {
            if (!resolve_cam_target(true))
                return;
            s_pos = read_cam_pos();
            if (s_pos.x == 0.0f && s_pos.y == 0.0f && s_pos.z == 0.0f)
                return;
            read_yaw_pitch(s_yaw, s_pitch);
            node humanoid = g_ctx.local_humanoid;
            if (humanoid)
            {
                float ws = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::WalkSpeed);
                s_savedWalkSpeed = (ws > 0.0f && ws <= 500.0f) ? ws : 16.0f;
                float wsc = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::WalkSpeedCheck);
                s_savedWalkSpeedCheck = (wsc > 0.0f && wsc <= 500.0f) ? wsc : s_savedWalkSpeed;
            }
            s_haveSavedCam = false;
            s_savedIsPart = s_camIsPart;
            s_hrpPrim = 0;
            if (!s_camIsPart)
            {
                s_savedSubject = g_proc->read<uintptr_t>(g_ctx.camera.addr + Offsets::Camera::CameraSubject);
                if (Offsets::Camera::CameraType)
                    s_savedCamType = g_proc->read<int>(g_ctx.camera.addr + Offsets::Camera::CameraType);
                s_haveSavedCam = true;
            }
            {
                s_hrpPrim = resolve_hrp_prim();
                if (s_hrpPrim && Offsets::Primitive::PrimitiveFlags)
                {
                    uint8_t flags = g_proc->read<uint8_t>(s_hrpPrim + Offsets::Primitive::PrimitiveFlags);
                    s_hrpWasAnchored = (flags & Offsets::PrimitiveFlags::Anchored) != 0;
                    set_hrp_anchored(s_hrpPrim, true);
                    zero_hrp_velocity(s_hrpPrim);
                }
            }
            s_lastTick = now;
            s_active = true;
        }
        float dt = std::chrono::duration<float>(now - s_lastTick).count();
        s_lastTick = now;
        if (dt > 0.05f) dt = 0.05f;
        if (dt <= 0.0f) dt = 0.000001f;
        ++s_tickCount;
        resolve_cam_target(false);
        if (!s_camIsPart)
        {
            if (Offsets::Camera::CameraType)
                g_proc->write<int>(g_ctx.camera.addr + Offsets::Camera::CameraType, kCamTypeScriptable);
            g_proc->write<uintptr_t>(g_ctx.camera.addr + Offsets::Camera::CameraSubject, 0);
        }
        if ((s_tickCount & 15) == 0 || !s_speedLocked)
        {
            node humanoid = g_ctx.local_humanoid;
            if (humanoid)
            {
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeed, 0.0f);
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeedCheck, 0.0f);
                s_speedLocked = true;
            }
            if (!s_hrpPrim)
            {
                s_hrpPrim = resolve_hrp_prim();
                if (s_hrpPrim && Offsets::Primitive::PrimitiveFlags)
                {
                    uint8_t flags = g_proc->read<uint8_t>(s_hrpPrim + Offsets::Primitive::PrimitiveFlags);
                    s_hrpWasAnchored = (flags & Offsets::PrimitiveFlags::Anchored) != 0;
                }
            }
            if (s_hrpPrim)
            {
                set_hrp_anchored(s_hrpPrim, true);
                zero_hrp_velocity(s_hrpPrim);
            }
        }
        bool canInput = roblox_foreground() && !g_menuOpen;
        if (canInput)
        {
            long rdx = 0, rdy = 0;
            Overlay::ConsumeMouseDelta(rdx, rdy);
            if (rdx != 0 || rdy != 0)
            {
                float sens = std::clamp(settings::freecam_sensitivity, 0.0005f, 0.01f) * 0.25f;
                s_yaw += static_cast<float>(rdx) * sens;
                s_pitch -= static_cast<float>(rdy) * sens;
                s_pitch = std::clamp(s_pitch, -1.5f, 1.5f);
            }
            float fwdIn = 0.0f, sideIn = 0.0f, upIn = 0.0f;
            if (GetAsyncKeyState('W') & 0x8000) fwdIn += 1.0f;
            if (GetAsyncKeyState('S') & 0x8000) fwdIn -= 1.0f;
            if (GetAsyncKeyState('D') & 0x8000) sideIn += 1.0f;
            if (GetAsyncKeyState('A') & 0x8000) sideIn -= 1.0f;
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) upIn += 1.0f;
            if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) upIn -= 1.0f;
            if (fwdIn != 0.0f || sideIn != 0.0f || upIn != 0.0f)
            {
                float cp = cosf(s_pitch);
                math::vec3 forward{ sinf(s_yaw) * cp, sinf(s_pitch), -cosf(s_yaw) * cp };
                math::vec3 right{ cosf(s_yaw), 0.0f, sinf(s_yaw) };
                math::vec3 up{ 0.0f, 1.0f, 0.0f };
                math::vec3 move = forward * fwdIn + right * sideIn + up * upIn;
                float len = move.length();
                if (len > 0.000001f)
                {
                    move = move / len;
                    float spd = std::clamp(settings::freecam_speed, 5.0f, 500.0f);
                    s_pos = s_pos + move * (spd * dt);
                }
            }
        }
        {
            float cp = cosf(s_pitch);
            math::vec3 look{
                sinf(s_yaw) * cp,
                sinf(s_pitch),
                -cosf(s_yaw) * cp
            };
            look = look.normalized();
            math::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
            math::vec3 right{
                look.y * worldUp.z - look.z * worldUp.y,
                look.z * worldUp.x - look.x * worldUp.z,
                look.x * worldUp.y - look.y * worldUp.x
            };
            float rightLen = right.length();
            if (rightLen > 0.000001f)
            {
                right = right / rightLen;
                math::vec3 up{
                    right.y * look.z - right.z * look.y,
                    right.z * look.x - right.x * look.z,
                    right.x * look.y - right.y * look.x
                };
                float upLen = up.length();
                if (upLen > 0.000001f)
                {
                    up = up / upLen;
                    math::vec3 back = look * -1.0f;
                    float blob[12] = {
                        right.x, up.x, back.x,
                        right.y, up.y, back.y,
                        right.z, up.z, back.z,
                        s_pos.x, s_pos.y, s_pos.z
                    };
                    write_cam_rot(blob);
                }
            }
        }
    }
}
