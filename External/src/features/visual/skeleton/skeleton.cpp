#include "skeleton.h"
#include "../../../utility/struct/world2screen.h"
namespace thenwefuckin::visual::skeleton
{
    bool project_pair(
        const math::vec3& wa,
        const math::vec3& wb,
        const math::view_matrix& view,
        float screen_w,
        float screen_h,
        math::vec2& sa,
        math::vec2& sb)
    {
        auto zero = [](const math::vec3& v) {
            return v.x == 0.f && v.y == 0.f && v.z == 0.f;
        };
        if (zero(wa) || zero(wb))
            return false;
        if (!projection::to_screen(wa, view, screen_w, screen_h, sa))
            return false;
        if (!projection::to_screen(wb, view, screen_w, screen_h, sb))
            return false;
        return true;
    }
    static void try_push(
        output& out,
        const math::vec3& wa,
        const math::vec3& wb,
        const math::view_matrix& view,
        float screen_w,
        float screen_h)
    {
        math::vec2 sa{};
        math::vec2 sb{};
        if (project_pair(wa, wb, view, screen_w, screen_h, sa, sb))
            out.push(sa, sb);
    }
    output build_r6(
        const pose_r6& p,
        const math::view_matrix& view,
        float screen_w,
        float screen_h)
    {
        output out{};
        auto zero = [](const math::vec3& v){ return v.x==0&&v.y==0&&v.z==0; };
        if (p.has_torsoFrame && !zero(p.head) && !zero(p.torso)) {
            math::vec3 right = p.torsoFrame.right();
            math::vec3 up = p.torsoFrame.up();
            float rl = right.length(); if(rl>0.01f) right = right * (1.0f/rl);
            float ul = up.length(); if(ul>0.01f) up = up * (1.0f/ul);
            math::vec3 neck = {p.torso.x + up.x*0.82f, p.torso.y + up.y*0.82f, p.torso.z + up.z*0.82f};
            math::vec3 hip = {p.torso.x - up.x*1.05f, p.torso.y - up.y*1.05f, p.torso.z - up.z*1.05f};
            math::vec3 lShoulder = {neck.x + right.x*(-1.38f), neck.y + right.y*(-1.38f), neck.z + right.z*(-1.38f)};
            math::vec3 rShoulder = {neck.x + right.x*(1.38f), neck.y + right.y*(1.38f), neck.z + right.z*(1.38f)};
            math::vec3 lHip = {hip.x + right.x*(-0.60f), hip.y + right.y*(-0.60f), hip.z + right.z*(-0.60f)};
            math::vec3 rHip = {hip.x + right.x*(0.60f), hip.y + right.y*(0.60f), hip.z + right.z*(0.60f)};
            math::vec3 armLExt = p.arm_l, armRExt = p.arm_r, legLExt = p.leg_l, legRExt = p.leg_r;
            if (p.has_arm_l) armLExt = {p.arm_l.x + up.x*(-0.55f), p.arm_l.y + up.y*(-0.55f), p.arm_l.z + up.z*(-0.55f)};
            if (p.has_arm_r) armRExt = {p.arm_r.x + up.x*(-0.55f), p.arm_r.y + up.y*(-0.55f), p.arm_r.z + up.z*(-0.55f)};
            if (p.has_leg_l) legLExt = {p.leg_l.x + up.x*(-0.85f), p.leg_l.y + up.y*(-0.85f), p.leg_l.z + up.z*(-0.85f)};
            if (p.has_leg_r) legRExt = {p.leg_r.x + up.x*(-0.85f), p.leg_r.y + up.y*(-0.85f), p.leg_r.z + up.z*(-0.85f)};
            try_push(out, p.head, neck, view, screen_w, screen_h);
            if (p.has_arm_l) {
                try_push(out, neck, lShoulder, view, screen_w, screen_h);
                try_push(out, lShoulder, armLExt, view, screen_w, screen_h);
            } else try_push(out, neck, lShoulder, view, screen_w, screen_h);
            if (p.has_arm_r) {
                try_push(out, neck, rShoulder, view, screen_w, screen_h);
                try_push(out, rShoulder, armRExt, view, screen_w, screen_h);
            } else try_push(out, neck, rShoulder, view, screen_w, screen_h);
            try_push(out, neck, hip, view, screen_w, screen_h);
            if (p.has_leg_l) {
                try_push(out, hip, lHip, view, screen_w, screen_h);
                try_push(out, lHip, legLExt, view, screen_w, screen_h);
            } else try_push(out, hip, lHip, view, screen_w, screen_h);
            if (p.has_leg_r) {
                try_push(out, hip, rHip, view, screen_w, screen_h);
                try_push(out, rHip, legRExt, view, screen_w, screen_h);
            } else try_push(out, hip, rHip, view, screen_w, screen_h);
            return out;
        }
        auto lerp = [](const math::vec3& a, const math::vec3& b, float t){ return math::vec3(a.x + (b.x-a.x)*t, a.y + (b.y-a.y)*t, a.z + (b.z-a.z)*t); };
        math::vec3 neck = p.torso;
        math::vec3 hip = p.torso;
        bool hasNeck = false, hasHip = false;
        if (!zero(p.head) && !zero(p.torso)) {
            neck = lerp(p.torso, p.head, 0.48f);
            hasNeck = true;
            math::vec3 legAvg{0,0,0}; int c=0;
            if(p.has_leg_l){ legAvg+=p.leg_l; ++c; }
            if(p.has_leg_r){ legAvg+=p.leg_r; ++c; }
            if(c>0){ legAvg = legAvg * (1.0f/c); hip = lerp(p.torso, legAvg, 0.32f); hasHip = true; }
        }
        if (hasNeck) try_push(out, p.head, neck, view, screen_w, screen_h);
        else try_push(out, p.head, p.torso, view, screen_w, screen_h);
        if (hasNeck && hasHip) try_push(out, neck, hip, view, screen_w, screen_h);
        else if (hasNeck) try_push(out, neck, p.torso, view, screen_w, screen_h);
        math::vec3 armSrc = hasNeck ? neck : p.torso;
        math::vec3 legSrc = hasHip ? hip : p.torso;
        if (p.has_arm_l) try_push(out, armSrc, p.arm_l, view, screen_w, screen_h);
        if (p.has_arm_r) try_push(out, armSrc, p.arm_r, view, screen_w, screen_h);
        if (p.has_leg_l) try_push(out, legSrc, p.leg_l, view, screen_w, screen_h);
        if (p.has_leg_r) try_push(out, legSrc, p.leg_r, view, screen_w, screen_h);
        return out;
    }
    output build_r15(
        const pose_r15& p,
        const math::view_matrix& view,
        float screen_w,
        float screen_h)
    {
        output out{};
        auto lerp = [](const math::vec3& a, const math::vec3& b, float t){ return math::vec3(a.x + (b.x-a.x)*t, a.y + (b.y-a.y)*t, a.z + (b.z-a.z)*t); };
        auto zero = [](const math::vec3& v){ return v.x==0 && v.y==0 && v.z==0; };
        math::vec3 neck = p.upper;
        bool hasNeck = false;
        if (!zero(p.head) && !zero(p.upper)) {
            neck = lerp(p.upper, p.head, 0.45f);
            hasNeck = true;
        }
        if (hasNeck) {
            try_push(out, p.head, neck, view, screen_w, screen_h);
            try_push(out, neck, p.upper, view, screen_w, screen_h);
        } else {
            try_push(out, p.head, p.upper, view, screen_w, screen_h);
        }
        if (p.has_lower && !zero(p.lower)) try_push(out, p.upper, p.lower, view, screen_w, screen_h);
        math::vec3 shoulderSrc = hasNeck ? neck : p.upper;
        if (!zero(p.l_upArm)) {
            try_push(out, shoulderSrc, p.l_upArm, view, screen_w, screen_h);
            if (!zero(p.l_lowArm)) {
                try_push(out, p.l_upArm, p.l_lowArm, view, screen_w, screen_h);
                try_push(out, p.l_lowArm, p.hand_l, view, screen_w, screen_h);
            } else {
                try_push(out, p.l_upArm, p.hand_l, view, screen_w, screen_h);
            }
        } else if (!zero(p.elbow_l)) {
            try_push(out, shoulderSrc, p.elbow_l, view, screen_w, screen_h);
            try_push(out, p.elbow_l, p.hand_l, view, screen_w, screen_h);
        } else if (!zero(p.hand_l)) {
            try_push(out, shoulderSrc, p.hand_l, view, screen_w, screen_h);
        }
        if (!zero(p.r_upArm)) {
            try_push(out, shoulderSrc, p.r_upArm, view, screen_w, screen_h);
            if (!zero(p.r_lowArm)) {
                try_push(out, p.r_upArm, p.r_lowArm, view, screen_w, screen_h);
                try_push(out, p.r_lowArm, p.hand_r, view, screen_w, screen_h);
            } else {
                try_push(out, p.r_upArm, p.hand_r, view, screen_w, screen_h);
            }
        } else if (!zero(p.elbow_r)) {
            try_push(out, shoulderSrc, p.elbow_r, view, screen_w, screen_h);
            try_push(out, p.elbow_r, p.hand_r, view, screen_w, screen_h);
        } else if (!zero(p.hand_r)) {
            try_push(out, shoulderSrc, p.hand_r, view, screen_w, screen_h);
        }
        math::vec3 hipSrc = (p.has_lower && !zero(p.lower)) ? p.lower : p.upper;
        if (!zero(p.l_upLeg)) {
            try_push(out, hipSrc, p.l_upLeg, view, screen_w, screen_h);
            if (!zero(p.l_lowLeg)) {
                try_push(out, p.l_upLeg, p.l_lowLeg, view, screen_w, screen_h);
                try_push(out, p.l_lowLeg, p.foot_l, view, screen_w, screen_h);
            } else {
                try_push(out, p.l_upLeg, p.foot_l, view, screen_w, screen_h);
            }
        } else if (!zero(p.knee_l)) {
            try_push(out, hipSrc, p.knee_l, view, screen_w, screen_h);
            try_push(out, p.knee_l, p.foot_l, view, screen_w, screen_h);
        } else if (!zero(p.foot_l)) {
            try_push(out, hipSrc, p.foot_l, view, screen_w, screen_h);
        }
        if (!zero(p.r_upLeg)) {
            try_push(out, hipSrc, p.r_upLeg, view, screen_w, screen_h);
            if (!zero(p.r_lowLeg)) {
                try_push(out, p.r_upLeg, p.r_lowLeg, view, screen_w, screen_h);
                try_push(out, p.r_lowLeg, p.foot_r, view, screen_w, screen_h);
            } else {
                try_push(out, p.r_upLeg, p.foot_r, view, screen_w, screen_h);
            }
        } else if (!zero(p.knee_r)) {
            try_push(out, hipSrc, p.knee_r, view, screen_w, screen_h);
            try_push(out, p.knee_r, p.foot_r, view, screen_w, screen_h);
        } else if (!zero(p.foot_r)) {
            try_push(out, hipSrc, p.foot_r, view, screen_w, screen_h);
        }
        return out;
    }
}
