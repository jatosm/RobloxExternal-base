#include "zoom.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <Windows.h>
#include <cmath>
#include <algorithm>
namespace thenwefuckin::misc::zoom
{
    namespace
    {
        bool s_keyPrev = false;
        bool s_keyToggled = false;
        bool s_wasEnabled = false;
        float s_originalStored = 0.0f;
        bool s_haveOriginalBase = false;
        uintptr_t s_lastCamera = 0;
    }
    static inline float deg_to_stored(float deg)
    {
        return std::tan((deg * 3.14159265358979323846f / 180.f) * 0.5f);
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
        if (!settings::zoom_enabled)
        {
            s_keyPrev = false;
            s_keyToggled = false;
            return false;
        }
        int key = settings::zoom_key;
        int mode = settings::zoom_key_mode;
        if (mode == 0) return true;
        if (key <= 0) return false;
        bool down = worker_key_down(key);
        if (mode == 2) return down;
        if (down && !s_keyPrev) s_keyToggled = !s_keyToggled;
        s_keyPrev = down;
        return s_keyToggled;
    }
    bool is_active()
    {
        return s_wasEnabled;
    }
    void tick_fast(bool want)
    {
        if (!want && !s_wasEnabled)
            return;
        if (!g_proc || !g_proc->connected())
        {
            s_wasEnabled = false;
            return;
        }
        if (!want)
        {
            node cam = g_ctx.resolve_camera();
            if (cam)
                g_proc->write<float>(cam.addr + Offsets::Camera::FieldOfView, s_haveOriginalBase ? s_originalStored : deg_to_stored(70.0f));
            s_wasEnabled = false;
            s_lastCamera = 0;
            return;
        }
        if (!g_ctx.ready || !g_ctx.camera)
            return;
        if (g_ctx.camera.addr != s_lastCamera)
        {
            s_wasEnabled = false;
            s_lastCamera = g_ctx.camera.addr;
        }
        float targetDeg = std::clamp(settings::zoom_value, 5.0f, 60.0f);
        float targetStored = deg_to_stored(targetDeg);
        if (!s_wasEnabled)
        {
            float curStored = g_proc->read<float>(g_ctx.camera.addr + Offsets::Camera::FieldOfView);
            if (curStored > 0.05f && curStored < 5.0f)
            {
                s_originalStored = curStored;
                s_haveOriginalBase = true;
            }
        }
        g_proc->write<float>(g_ctx.camera.addr + Offsets::Camera::FieldOfView, targetStored);
        s_wasEnabled = true;
    }
}
