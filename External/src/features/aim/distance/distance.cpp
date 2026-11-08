#include "distance.h"
#include "../../../menu/overlay/settings.h"
namespace thenwefuckin::aim::distance
{
    bool check(float dist)
    {
        if (settings::aim_distance_check && dist > settings::aim_distance_value)
            return false;
        return true;
    }
    bool check(const math::vec3& targetPos, const math::vec3& camPos)
    {
        if (!settings::aim_distance_check)
            return true;
        float d = (targetPos - camPos).length();
        return d <= settings::aim_distance_value;
    }
}
