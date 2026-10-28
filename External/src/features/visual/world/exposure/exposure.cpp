#include "exposure.h"
#include "../env/env.h"
namespace thenwefuckin::world::exposure
{
    void tick(bool on)
    {
        if (on)
        {
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::ExposureCompensation, settings::world_exposure_value);
            env::s_exposureActive = true;
        }
        else if (env::s_exposureActive)
        {
            g_proc->write<float>(env::s_lightingAddress + Offsets::Lighting::ExposureCompensation, env::s_backup.exposure);
            env::s_exposureActive = false;
        }
    }
}
