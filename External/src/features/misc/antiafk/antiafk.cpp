#include "antiafk.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include <Windows.h>
#include <chrono>
#include <algorithm>
namespace thenwefuckin::misc::antiafk
{
    void run()
    {
        static std::chrono::steady_clock::time_point s_lastPress{};
        static bool s_wasEnabled = false;
        if (!settings::antiafk_enabled)
        {
            s_lastPress = std::chrono::steady_clock::now();
            s_wasEnabled = false;
            return;
        }
        if (!g_proc || !g_proc->connected())
        {
            s_lastPress = std::chrono::steady_clock::now();
            s_wasEnabled = false;
            return;
        }
        if (!s_wasEnabled)
        {
            s_lastPress = std::chrono::steady_clock::now();
            s_wasEnabled = true;
            return;
        }
        const float minutes = std::clamp(settings::antiafk_interval, 1.0f, 15.0f);
        const auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastPress).count() < (long long)(minutes * 60000.0f))
            return;
        DWORD fgPid = 0;
        HWND fg = GetForegroundWindow();
        if (fg)
            GetWindowThreadProcessId(fg, &fgPid);
        if (!fgPid || fgPid != g_proc->pid())
        {
            s_lastPress = now;
            return;
        }
        INPUT inputs[2]{};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_SPACE;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_SPACE;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(2, inputs, sizeof(INPUT));
        s_lastPress = now;
    }
}
