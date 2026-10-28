#include "noclip.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
namespace thenwefuckin::misc::noclip
{
    static void set_collide(node character, bool collide)
    {
        if (!character || !Offsets::Primitive::PrimitiveFlags)
            return;
        for (auto& part : character.children())
        {
            uintptr_t prim = part.primitive();
            if (!prim)
                continue;
            uint8_t flags = g_proc->read<uint8_t>(prim + Offsets::Primitive::PrimitiveFlags);
            bool has = (flags & Offsets::PrimitiveFlags::CanCollide) != 0;
            if (has == collide)
                continue;
            uint8_t next = collide
                ? static_cast<uint8_t>(flags | Offsets::PrimitiveFlags::CanCollide)
                : static_cast<uint8_t>(flags & ~Offsets::PrimitiveFlags::CanCollide);
            g_proc->write<uint8_t>(prim + Offsets::Primitive::PrimitiveFlags, next);
        }
    }
    void run()
    {
        static bool wasEnabled = false;
        bool active = settings::noclip_enabled;
        if (active && settings::noclip_key != 0)
        {
            active = Menu::IsMenuKeyActive("##noclip_key", settings::noclip_key, settings::noclip_key_mode);
        }
        if (!active && !wasEnabled)
            return;
        if (!g_proc || !g_proc->connected())
        {
            wasEnabled = false;
            return;
        }
        if (!g_ctx.ready)
            return;
        node character = g_ctx.get_character(g_ctx.local_player);
        if (!character)
            return;
        if (!active)
        {
            set_collide(character, true);
            wasEnabled = false;
            return;
        }
        set_collide(character, false);
        wasEnabled = true;
    }
}
