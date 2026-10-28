#include "health.h"
#include "../../../menu/overlay/settings.h"
namespace thenwefuckin::aim::health
{
    bool check(float hp)
    {
        if (hp <= 0.0f)
            return false;
        if (settings::aim_health_check && hp > settings::aim_health_value)
            return false;
        return true;
    }
}
