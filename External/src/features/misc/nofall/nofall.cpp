#include "nofall.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <algorithm>
namespace thenwefuckin::misc::nofall
{
    void run()
    {
        bool active = settings::nofall_enabled;
        if (active && settings::nofall_key != 0)
        {
            active = Menu::IsMenuKeyActive("##nofall_key", settings::nofall_key, settings::nofall_key_mode);
        }
        if (!active)
            return;
        if (!g_proc || !g_proc->connected() || !g_ctx.ready || !g_ctx.local_root)
            return;
        const uintptr_t prim = g_ctx.local_root.primitive();
        if (!prim)
            return;
        const float cap = std::clamp(settings::nofall_value, 0.0f, 200.0f);
        math::vec3 vel = g_proc->read<math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
        if (vel.y < -cap)
        {
            vel.y = -cap;
            g_proc->write<math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, vel);
        }
    }
}
