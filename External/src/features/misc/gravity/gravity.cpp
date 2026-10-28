#include "gravity.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
namespace thenwefuckin::misc::gravity
{
    void run()
    {
        static bool wasEnabled = false;
        static float originalGravity = 196.2f;
        static uintptr_t lastWorld = 0;
        bool active = settings::gravity_enabled;
        if (active && settings::gravity_key != 0)
        {
            active = Menu::IsMenuKeyActive("##gravity_key", settings::gravity_key, settings::gravity_key_mode);
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
            if (g_ctx.workspace && lastWorld)
            {
                const uintptr_t world = g_proc->read<uintptr_t>(g_ctx.workspace.addr + Offsets::Workspace::World);
                if (world && world == lastWorld)
                    g_proc->write<float>(world + Offsets::World::Gravity, originalGravity);
            }
            wasEnabled = false;
            lastWorld = 0;
            return;
        }
        if (!g_ctx.ready || !g_ctx.workspace)
            return;
        const uintptr_t world = g_proc->read<uintptr_t>(g_ctx.workspace.addr + Offsets::Workspace::World);
        if (!world)
            return;
        if (world != lastWorld)
        {
            wasEnabled = false;
            lastWorld = world;
        }
        float cur = g_proc->read<float>(world + Offsets::World::Gravity);
        if (!wasEnabled)
        {
            if (cur > 0.0f && cur <= 2000.0f)
                originalGravity = cur;
            else
                originalGravity = 196.2f;
        }
        if (cur != settings::gravity_value)
        {
            g_proc->write<float>(world + Offsets::World::Gravity, settings::gravity_value);
        }
        wasEnabled = true;
    }
}
