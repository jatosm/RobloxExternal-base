#include "grass.h"
#include "../env/env.h"
#include <algorithm>
namespace thenwefuckin::world::grass
{
    void tick(bool on)
    {
        if (on && env::s_terrainAddress)
        {
            g_proc->write<float>(env::s_terrainAddress + Offsets::Terrain::GrassLength,
                std::clamp(settings::grass_value, 0.0f, 3.0f));
            env::s_grassActive = true;
        }
        else if (env::s_grassActive)
        {
            if (env::s_terrainBackup.captured && env::s_terrainBackup.address == env::s_terrainAddress)
                g_proc->write<float>(env::s_terrainAddress + Offsets::Terrain::GrassLength, env::s_terrainBackup.grass);
            env::s_grassActive = false;
        }
    }
}
