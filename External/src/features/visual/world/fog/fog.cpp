#include "fog.h"
#include "../env/env.h"
namespace thenwefuckin::world::fog
{
    void tick(bool on)
    {
        if (on)
        {
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::FogStart, settings::world_fog_start);
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::FogEnd, settings::world_fog_end);
            env::vec3 fc{ settings::world_fog_color[0], settings::world_fog_color[1], settings::world_fog_color[2] };
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::FogColor, fc);
            env::s_fogActive = true;
        }
        else if (env::s_fogActive)
        {
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::FogStart, env::s_backup.fog_start);
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::FogEnd, env::s_backup.fog_end);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::FogColor, env::s_backup.fog_color);
            env::s_fogActive = false;
        }
    }
}
