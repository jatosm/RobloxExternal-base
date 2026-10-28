#include "clock.h"
#include "../env/env.h"
namespace thenwefuckin::world::clock
{
    void tick(bool on)
    {
        if (on)
        {
            env::apply_clock_time(env::s_lightingAddress, settings::time_value);
            env::s_clockActive = true;
        }
        else if (env::s_clockActive)
        {
            env::restore_clock_params(env::s_backup);
            env::s_clockActive = false;
        }
    }
}
