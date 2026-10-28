#include "time.h"
#include "../env/env.h"
#include "../clock/clock.h"
#include "../night/night.h"
#include "../fog/fog.h"
#include "../exposure/exposure.h"
#include "../ambient/ambient.h"
#include "../fullbright/fullbright.h"
#include "../shadows/shadows.h"
#include "../grass/grass.h"
namespace thenwefuckin::world::time
{
    void reset()
    {
        env::reset_all();
    }
    void run()
    {
        const bool nightOn = env::keyed(settings::night_enabled, settings::night_key, settings::night_key_mode, "##night_key");
        const bool timeOn = env::keyed(settings::time_enabled, settings::time_key, settings::time_key_mode, "##time_key") && !nightOn;
        const bool fogOn = env::keyed(settings::world_fog_enabled, settings::world_fog_key, settings::world_fog_key_mode, "##fog_key");
        const bool expOn = env::keyed(settings::world_exposure_enabled, settings::world_exposure_key, settings::world_exposure_key_mode, "##exposure_key");
        const bool fbOn = env::keyed(settings::fullbright_enabled, settings::fullbright_key, settings::fullbright_key_mode, "##fullbright_key") && !nightOn;
        const bool shadowOn = env::keyed(settings::noshadow_enabled, settings::noshadow_key, settings::noshadow_key_mode, "##noshadow_key");
        const bool ambOn = env::keyed(settings::ambient_enabled, settings::ambient_key, settings::ambient_key_mode, "##ambient_key") && !nightOn && !fbOn;
        const bool grassOn = env::keyed(settings::nograss_enabled, settings::nograss_key, settings::nograss_key_mode, "##nograss_key");
        const bool terrain_active = grassOn;
        const bool terrain_was_active = env::s_grassActive;
        const bool any_active = nightOn || timeOn || fogOn || expOn || fbOn || shadowOn || ambOn
            || terrain_active;
        const bool any_was_active = env::s_nightActive || env::s_clockActive || env::s_fogActive || env::s_exposureActive
            || env::s_fullbrightActive || env::s_shadowActive || env::s_ambientActive || terrain_was_active;
        if (!any_active && !any_was_active)
            return;
        if (!env::resolve_lighting(any_active))
            return;
        night::tick(nightOn);
        clock::tick(timeOn);
        fog::tick(fogOn);
        exposure::tick(expOn);
        fullbright::tick(fbOn);
        ambient::tick(ambOn);
        shadows::tick(shadowOn);
        if (terrain_active)
            env::resolve_terrain();
        grass::tick(grassOn);
        if (!any_active && env::s_backup.captured && env::s_backup.address == env::s_lightingAddress)
            env::s_backup = {};
        if (any_active || any_was_active)
            env::force_lighting_dirty();
    }
}
