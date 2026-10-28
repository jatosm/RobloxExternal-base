#include "walkspeed.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
namespace thenwefuckin::misc::walkspeed
{
    void run()
    {
        static bool wasEnabled = false;
        static float originalWalkSpeed = 16.0f;
        static uintptr_t lastHumanoid = 0;
        bool active = settings::walkspeed_enabled;
        if (active && settings::walkspeed_key != 0)
        {
            active = Menu::IsMenuKeyActive("##walkspeed_key", settings::walkspeed_key, settings::walkspeed_key_mode);
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
            {
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeed, originalWalkSpeed);
                if (Offsets::Humanoid::WalkSpeedCheck)
                    g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeedCheck, originalWalkSpeed);
            }
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
        float cur = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::WalkSpeed);
        if (!wasEnabled)
        {
            if (cur > 0.0f && cur <= 100.0f)
                originalWalkSpeed = cur;
            else
                originalWalkSpeed = 16.0f;
        }
        if (cur != settings::walkspeed_value)
        {
            g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeed, settings::walkspeed_value);
            if (Offsets::Humanoid::WalkSpeedCheck)
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::WalkSpeedCheck, settings::walkspeed_value);
        }
        wasEnabled = true;
    }
}
