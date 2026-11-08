#include "smooth.h"
#include "../../../menu/overlay/settings.h"
#include <algorithm>
#include <cmath>
namespace thenwefuckin::aim::smooth
{
    void calculate(float curYaw, float curPitch, float wantYaw, float wantPitch, float& outYaw, float& outPitch)
    {
        float deltaYaw = wantYaw - curYaw;
        while (deltaYaw > 3.14159265f) deltaYaw -= 6.2831853f;
        while (deltaYaw < -3.14159265f) deltaYaw += 6.2831853f;
        float deltaPitch = wantPitch - curPitch;
        float kx = 1.0f;
        float ky = 1.0f;
        if (settings::aim_smoothing_enabled)
        {
            float sx = settings::aim_smoothing_x;
            float sy = settings::aim_smoothing_y;
            if (sx < 1.0f) sx = 1.0f;
            if (sy < 1.0f) sy = 1.0f;
            kx = 1.0f / sx;
            ky = 1.0f / sy;
            if (kx > 1.0f) kx = 1.0f;
            if (ky > 1.0f) ky = 1.0f;
        }
        outYaw = curYaw + deltaYaw * kx;
        outPitch = curPitch + deltaPitch * ky;
        if (outPitch > 1.55f) outPitch = 1.55f;
        if (outPitch < -1.55f) outPitch = -1.55f;
    }
}
