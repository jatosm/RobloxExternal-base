#pragma once
#include "../cache/math.h"
namespace thenwefuckin::projection
{
    inline bool to_screen(
        const math::vec3& world,
        const math::view_matrix& view,
        float screen_w,
        float screen_h,
        math::vec2& out)
    {
        math::vec4 clip = view.transform(world);
        if (clip.w < 0.1f)
            return false;
        float inv = 1.0f / clip.w;
        float nx = clip.x * inv;
        float ny = clip.y * inv;
        out.x = (screen_w * 0.5f * nx) + (screen_w * 0.5f);
        out.y = -(screen_h * 0.5f * ny) + (screen_h * 0.5f);
        return true;
    }
    inline math::vec2 screen_size()
    {
        return {
            static_cast<float>(GetSystemMetrics(SM_CXSCREEN)),
            static_cast<float>(GetSystemMetrics(SM_CYSCREEN))
        };
    }
}
