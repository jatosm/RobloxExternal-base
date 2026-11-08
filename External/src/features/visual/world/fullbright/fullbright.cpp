#include "fullbright.h"
#include "../env/env.h"
#include <algorithm>
namespace thenwefuckin::world::fullbright
{
    void tick(bool on)
    {
        if (on)
        {
            const env::vec3 white{ 1.0f, 1.0f, 1.0f };
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::Ambient, white);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::OutdoorAmbient, white);
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::Brightness, std::clamp(settings::fullbright_value, 1.0f, 5.0f));
            env::s_fullbrightActive = true;
        }
        else if (env::s_fullbrightActive)
        {
            env::restore_ambient(env::s_backup);
            env::s_fullbrightActive = false;
        }
    }
}
