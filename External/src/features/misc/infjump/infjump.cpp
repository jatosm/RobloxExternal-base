#include "infjump.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <Windows.h>
#include <algorithm>
namespace thenwefuckin::misc::infjump
{
    void run()
    {
        bool active = settings::infjump_enabled;
        if (active && settings::infjump_key != 0)
        {
            active = Menu::IsMenuKeyActive("##infjump_key", settings::infjump_key, settings::infjump_key_mode);
        }
        if (!active)
            return;
        if (!g_proc || !g_proc->connected() || !g_ctx.ready)
            return;
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) == 0)
            return;
        node root = g_ctx.local_root;
        if (!root)
            return;
        uintptr_t prim = root.primitive();
        if (!prim)
            return;
        float jumpPower = 50.0f;
        node humanoid = g_ctx.local_humanoid;
        if (humanoid)
        {
            float jp = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::JumpPower);
            if (jp > 0.0f && jp < 1000.0f)
                jumpPower = jp;
        }
        math::vec3 vel = g_proc->read<math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
        vel.y = jumpPower;
        g_proc->write<math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, vel);
    }
}
