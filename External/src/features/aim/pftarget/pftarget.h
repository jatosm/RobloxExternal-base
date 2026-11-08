#pragma once
#include "../../../utility/cache/math.h"
#include <cstdint>
namespace thenwefuckin::aim::pftarget
{
    struct PfBest
    {
        math::vec3 world{};
        math::vec2 screen{};
        float dist2d = 0.0f;
        uintptr_t model = 0;
    };
    bool active();
    bool select_best(int w, int h, PfBest& out);
}
