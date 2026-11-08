#pragma once
#include <cstdint>
#include "../cache/math.h"
namespace thenwefuckin::data
{
    using thenwefuckin::math::vec2;
    using thenwefuckin::math::vec3;
    enum class rig_kind : uint8_t
    {
        unknown = 0,
        r6 = 1,
        r15 = 2
    };
    struct limbs
    {
        uintptr_t head = 0;
        uintptr_t root = 0;
        uintptr_t torso_a = 0;
        uintptr_t torso_b = 0;
        uintptr_t arm_l = 0;
        uintptr_t arm_r = 0;
        uintptr_t leg_l = 0;
        uintptr_t leg_r = 0;
        uintptr_t humanoid = 0;
        rig_kind kind = rig_kind::unknown;
        bool has_spine() const { return root != 0 && humanoid != 0; }
        void clear()
        {
            head = 0;
            root = 0;
            torso_a = 0;
            torso_b = 0;
            arm_l = 0;
            arm_r = 0;
            leg_l = 0;
            leg_r = 0;
            humanoid = 0;
            kind = rig_kind::unknown;
        }
    };
    struct entity
    {
        uintptr_t player = 0;
        uintptr_t model = 0;
        limbs bones{};
        vec3 feet{};
        vec3 skull{};
        vec3 focus{};
        float hp = 0.f;
        float hp_max = 100.f;
        float range = 0.f;
        uint64_t team = 0;
        bool alive = false;
        char label[48]{};
        float hp_frac() const
        {
            if (hp_max <= 0.5f)
                return 0.f;
            float f = hp / hp_max;
            if (f < 0.f)
                return 0.f;
            if (f > 1.f)
                return 1.f;
            return f;
        }
        bool same_team(uint64_t mine) const
        {
            return mine != 0 && team != 0 && team == mine;
        }
    };
    struct frame_state
    {
        vec3 viewer{};
        uint64_t viewer_team = 0;
        float w = 0.f;
        float h = 0.f;
        uint32_t tick = 0;
        vec2 center() const { return { w * 0.5f, h * 0.5f }; }
    };
    struct rect
    {
        float x0 = 0.f;
        float y0 = 0.f;
        float x1 = 0.f;
        float y1 = 0.f;
        float width() const { return x1 - x0; }
        float height() const { return y1 - y0; }
        bool empty() const { return width() <= 0.f || height() <= 0.f; }
        static rect from_points(const vec2& top, const vec2& bottom, float wide)
        {
            rect r{};
            r.x0 = top.x - wide * 0.5f;
            r.x1 = top.x + wide * 0.5f;
            r.y0 = top.y;
            r.y1 = bottom.y;
            return r;
        }
    };
}
