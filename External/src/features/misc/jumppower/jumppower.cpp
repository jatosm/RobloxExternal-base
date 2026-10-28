#include "jumppower.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
namespace thenwefuckin::misc::jumppower
{
    void run()
    {
        static bool wasEnabled = false;
        static float originalJumpPower = 50.0f;
        static float originalJumpHeight = 7.2f;
        static uint8_t originalUseJumpPower = 0;
        static uintptr_t lastHumanoid = 0;
        bool active = settings::jumppower_enabled;
        if (active && settings::jumppower_key != 0)
        {
            active = Menu::IsMenuKeyActive("##jumppower_key", settings::jumppower_key, settings::jumppower_key_mode);
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
                g_proc->write<uint8_t>(humanoid.addr + Offsets::Humanoid::UseJumpPower, originalUseJumpPower);
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::JumpPower, originalJumpPower);
                g_proc->write<float>(humanoid.addr + Offsets::Humanoid::JumpHeight, originalJumpHeight);
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
        if (!wasEnabled)
        {
            float jp = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::JumpPower);
            float jh = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::JumpHeight);
            uint8_t ujp = g_proc->read<uint8_t>(humanoid.addr + Offsets::Humanoid::UseJumpPower);
            if (jp > 0.0f && jp < 1000.0f) originalJumpPower = jp;
            if (jh > 0.0f && jh < 500.0f) originalJumpHeight = jh;
            originalUseJumpPower = ujp;
        }
        float cur = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::JumpPower);
        if (cur != settings::jumppower_value)
        {
            g_proc->write<uint8_t>(humanoid.addr + Offsets::Humanoid::UseJumpPower, 1);
            g_proc->write<float>(humanoid.addr + Offsets::Humanoid::JumpPower, settings::jumppower_value);
            g_proc->write<float>(humanoid.addr + Offsets::Humanoid::JumpHeight, settings::jumppower_value * 0.144f);
        }
        wasEnabled = true;
    }
}
