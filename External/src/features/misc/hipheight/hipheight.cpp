#include "hipheight.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <algorithm>
namespace thenwefuckin::misc::hipheight
{
    void run()
    {
        static bool wasEnabled = false;
        static float originalHipHeight = 0.0f;
        static uintptr_t lastHumanoid = 0;
        bool active = settings::hipheight_enabled;
        if (active && settings::hipheight_key != 0)
        {
            active = Menu::IsMenuKeyActive("##hipheight_key", settings::hipheight_key, settings::hipheight_key_mode);
        }
        if (!active && !wasEnabled)
            return;
        if (!g_proc || !g_proc->connected())
        {
            wasEnabled = false;
            return;
        }
        if (!active)
        {
            node humanoid = g_ctx.resolve_local_humanoid();
            if (humanoid)
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::HipHeight, originalHipHeight);
            wasEnabled = false;
            lastHumanoid = 0;
            return;
        }
        if (!g_ctx.ready)
            return;
        node humanoid = g_ctx.local_humanoid;
        if (!humanoid)
            return;
        if (humanoid.addr != lastHumanoid)
        {
            wasEnabled = false;
            lastHumanoid = humanoid.addr;
        }
        float cur = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::HipHeight);
        if (!wasEnabled)
        {
            if (cur >= 0.0f && cur <= 50.0f)
                originalHipHeight = cur;
            else
                originalHipHeight = 0.0f;
        }
        float target = std::clamp(settings::hipheight_value, 0.0f, 20.0f);
        if (cur != target)
            g_proc->write<float>(humanoid.addr + Offsets::Humanoid::HipHeight, target);
        wasEnabled = true;
    }
}
