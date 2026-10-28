#include "shadows.h"
#include "../env/env.h"
#include <algorithm>
namespace thenwefuckin::world::shadows
{
    void tick(bool on)
    {
        if (on)
        {
            g_proc->write<uint8_t>(env::s_lightingAddress + Offsets::Lighting::GlobalShadows, 0);
            if (Offsets::Lighting::ShadowSoftness)
                g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::ShadowSoftness,
                    std::clamp(settings::shadow_softness, 0.0f, 1.0f));
            env::s_shadowActive = true;
        }
        else if (env::s_shadowActive)
        {
            g_proc->write<uint8_t>(env::s_lightingAddress + Offsets::Lighting::GlobalShadows, env::s_backup.shadows);
            if (Offsets::Lighting::ShadowSoftness)
                g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::ShadowSoftness, env::s_backup.shadow_soft);
            env::s_shadowActive = false;
        }
    }
}
