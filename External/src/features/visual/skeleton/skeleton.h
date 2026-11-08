#pragma once
#include "../../../utility/cache/math.h"
namespace thenwefuckin::visual::skeleton
{
    struct link
    {
        math::vec2 a{};
        math::vec2 b{};
    };
    struct output
    {
        link items[24]{};
        int count = 0;
        void push(const math::vec2& a, const math::vec2& b)
        {
            if (count < 24)
                items[count++] = link{ a, b };
        }
        void clear() { count = 0; }
    };
    struct pose_r6
    {
        math::vec3 head{};
        math::vec3 torso{};
        math::vec3 arm_l{};
        math::vec3 arm_r{};
        math::vec3 leg_l{};
        math::vec3 leg_r{};
        math::frame torsoFrame{};
        bool has_torsoFrame = false;
        bool has_arm_l = false;
        bool has_arm_r = false;
        bool has_leg_l = false;
        bool has_leg_r = false;
    };
    struct pose_r15
    {
        math::vec3 head{};
        math::vec3 upper{};
        math::vec3 lower{};
        math::vec3 l_upArm{};
        math::vec3 r_upArm{};
        math::vec3 l_lowArm{};
        math::vec3 r_lowArm{};
        math::vec3 hand_l{};
        math::vec3 hand_r{};
        math::vec3 l_upLeg{};
        math::vec3 r_upLeg{};
        math::vec3 l_lowLeg{};
        math::vec3 r_lowLeg{};
        math::vec3 foot_l{};
        math::vec3 foot_r{};
        math::vec3 elbow_l{};
        math::vec3 elbow_r{};
        math::vec3 knee_l{};
        math::vec3 knee_r{};
        bool has_lower = false;
    };
    bool project_pair(
        const math::vec3& wa,
        const math::vec3& wb,
        const math::view_matrix& view,
        float screen_w,
        float screen_h,
        math::vec2& sa,
        math::vec2& sb);
    output build_r6(
        const pose_r6& p,
        const math::view_matrix& view,
        float screen_w,
        float screen_h);
    output build_r15(
        const pose_r15& p,
        const math::view_matrix& view,
        float screen_w,
        float screen_h);
}
