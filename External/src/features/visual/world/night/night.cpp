#include "night.h"
#include "../env/env.h"
#include <algorithm>
namespace thenwefuckin::world::night
{
    void tick(bool on)
    {
        if (on)
        {
            env::apply_clock_time(env::s_lightingAddress, 0.0f);
            const float l = std::clamp(settings::night_level, 0.0f, 1.0f);
            const env::vec3 dark{ 0.07f * l, 0.07f * l, 0.11f * l };
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::Ambient, dark);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::OutdoorAmbient, dark);
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::Brightness, 1.0f);
            env::s_nightActive = true;
        }
        else if (env::s_nightActive)
        {
            env::restore_clock_params(env::s_backup);
            env::restore_ambient(env::s_backup);
            env::s_nightActive = false;
        }
    }
}
