#include "visual.h"
#include "../../aim/core/aim.h"
#include "../../../utility/cache/cache.h"
#include "../world/ore/ore.h"
#include "../world/plants/plants.h"
#include "../world/bots/bots.h"
#include "../world/animals/animals.h"
#include "../world/tools/tools.h"
#include "../world/crates/crates.h"
#include "../world/keycards/keycards.h"
#include "../world/geo/geo.h"
#include "../players/friends.h"
#include "../../../menu/overlay/settings.h"
#include "../box/box.h"
#include "../skeleton/skeleton.h"
#include "../health/health.h"
#include "../name/name.h"
#include "../headdot/headdot.h"
#include "../tracer/tracer.h"
#include "../oof/oof.h"
#include "../../../menu/resources/font.h"
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstdio>
#include <cstring>
static constexpr float kStaticBoxAspect = 0.72f;
static constexpr float kStaticBoxPadY   = 5.0f;
namespace thenwefuckin::visual
{
    static inline void DrawVisualText(ImDrawList* dl, ImFont* font, float fontSize, float x, float y, ImU32 col, bool outline, ImU32 outlineCol, const char* text)
    {
        if (!dl || !text || !text[0]) return;
        if (!font) font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
        float ix = std::floor(x);
        float iy = std::floor(y);
        if (outline) {
            dl->AddText(font, fontSize, ImVec2(ix + 1.0f, iy + 1.0f), outlineCol, text);
        }
        dl->AddText(font, fontSize, ImVec2(ix, iy), col, text);
    }
    static inline data::rect projectBoxPoints(const std::vector<math::vec3>& pts, const math::view_matrix& view, float screenW, float screenH, const math::vec3& camPos)
    {
        if (pts.empty()) return {};
        float minX=1e9f, minY=1e9f, maxX=-1e9f, maxY=-1e9f;
        int valid=0;
        for (auto &p: pts) {

            if ((p - camPos).length() < 1.5f) continue;
            math::vec2 s;
            if (!projection::to_screen(p, view, screenW, screenH, s)) continue;
            if (s.x==0 && s.y==0) continue;
            if (!std::isfinite(s.x) || !std::isfinite(s.y)) continue;
            minX = (std::min)(minX, s.x);
            minY = (std::min)(minY, s.y);
            maxX = (std::max)(maxX, s.x);
            maxY = (std::max)(maxY, s.y);
            ++valid;
        }
        if (valid < 3 || minX==1e9f) return {};
        float bw = maxX - minX, bh = maxY - minY;
        if (bw <= 2.0f || bh <= 4.0f) return {};
        float pad = 1.0f + 0.03f * ((std::max)(bw, bh));
        minX -= pad; minY -= pad; maxX += pad; maxY += pad;
        constexpr float m = 50.0f;
        minX = (std::max)(minX, -m); minY = (std::max)(minY, -m);
        maxX = (std::min)(maxX, screenW + m); maxY = (std::min)(maxY, screenH + m);
        if (minX >= maxX || minY >= maxY) return {};
        data::rect rc{}; rc.x0=minX; rc.y0=minY; rc.x1=maxX; rc.y1=maxY;
        return rc;
    }
    void on_game_joined()
    {
        wcache::clear();
        pcache::clear();
        PfCache::on_game_joined();
        thenwefuckin::aim::on_game_joined();
        ore::clear();
        plants::clear();
        bots::clear();
        worldgeo::clear();
        animals::clear();
        tools::clear();
        crates::clear();
        keycards::clear();
        players::friends::clear();
    }
    void render(ImDrawList* dl, int w, int h)
    {
        if (!dl) return;
        if (settings::aim_show_fov && settings::aim_fov > 0.0f)
        {
            float cx = (float)w * 0.5f;
            float cy = (float)h * 0.5f;
            float r = settings::aim_fov;
            if (settings::aim_fov_fill) {
                ImU32 fc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::aim_fov_fill_color[0], settings::aim_fov_fill_color[1], settings::aim_fov_fill_color[2], settings::aim_fov_fill_color[3]));
                int style = settings::aim_fov_style;
                if (style == 2)
                    dl->AddRectFilled(ImVec2(cx - r, cy - r), ImVec2(cx + r, cy + r), fc);
                else if (style == 3)
                {
                    ImVec2 hex[6];
                    for (int i = 0; i < 6; ++i)
                    {
                        float a = 6.2831853f * (float)i / 6.0f - 1.5707963f;
                        hex[i] = ImVec2(cx + cosf(a) * r, cy + sinf(a) * r);
                    }
                    dl->AddConvexPolyFilled(hex, 6, fc);
                }
                else if (style == 4)
                    dl->AddQuadFilled(ImVec2(cx, cy - r), ImVec2(cx + r, cy), ImVec2(cx, cy + r), ImVec2(cx - r, cy), fc);
                else
                    dl->AddCircleFilled(ImVec2(cx, cy), r, fc, 64);
            }
            ImU32 mc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::aim_fov_color[0], settings::aim_fov_color[1], settings::aim_fov_color[2], settings::aim_fov_color[3]));
            ImU32 oc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::aim_fov_outline_color[0], settings::aim_fov_outline_color[1], settings::aim_fov_outline_color[2], settings::aim_fov_outline_color[3]));
            if (settings::aim_fov_style == 1)
            {
                int n = (int)(r * 0.9f);
                if (n < 12) n = 12;
                if (n > 64) n = 64;
                for (int i = 0; i < n; ++i)
                {
                    float a = (float)i * 6.2831853f / (float)n;
                    ImVec2 p{ cx + cosf(a) * r, cy + sinf(a) * r };
                    if (settings::aim_fov_outline)
                        dl->AddCircleFilled(p, 2.5f, oc);
                    dl->AddCircleFilled(p, 1.5f, mc);
                }
            }
            else if (settings::aim_fov_style == 2)
            {
                if (settings::aim_fov_outline) {
                    dl->AddRect(ImVec2(cx - r - 1.0f, cy - r - 1.0f), ImVec2(cx + r + 1.0f, cy + r + 1.0f), oc);
                    dl->AddRect(ImVec2(cx - r + 1.0f, cy - r + 1.0f), ImVec2(cx + r - 1.0f, cy + r - 1.0f), oc);
                }
                dl->AddRect(ImVec2(cx - r, cy - r), ImVec2(cx + r, cy + r), mc);
            }
            else if (settings::aim_fov_style == 3)
            {
                ImVec2 hex[6];
                for (int i = 0; i < 6; ++i)
                {
                    float a = 6.2831853f * (float)i / 6.0f - 1.5707963f;
                    hex[i] = ImVec2(cx + cosf(a) * r, cy + sinf(a) * r);
                }
                if (settings::aim_fov_outline)
                    dl->AddPolyline(hex, 6, oc, 2.0f, ImDrawFlags_Closed);
                dl->AddPolyline(hex, 6, mc, 1.0f, ImDrawFlags_Closed);
            }
            else if (settings::aim_fov_style == 4)
            {
                ImVec2 dia[4] = { ImVec2(cx, cy - r), ImVec2(cx + r, cy), ImVec2(cx, cy + r), ImVec2(cx - r, cy) };
                if (settings::aim_fov_outline)
                    dl->AddPolyline(dia, 4, oc, 2.0f, ImDrawFlags_Closed);
                dl->AddPolyline(dia, 4, mc, 1.0f, ImDrawFlags_Closed);
            }
            else
            {
                if (settings::aim_fov_outline) {
                    dl->AddCircle(ImVec2(cx, cy), r - 1.0f, oc, 64, 1.0f);
                    dl->AddCircle(ImVec2(cx, cy), r + 1.0f, oc, 64, 1.0f);
                }
                dl->AddCircle(ImVec2(cx, cy), r, mc, 64, 1.0f);
            }
        }
        if (!g_ctx.ready) return;
        float fw = (float)w;
        float fh = (float)h;
        auto now = std::chrono::steady_clock::now();

        wcache::update(fw, fh);
        bool any_flags = settings::flags_enabled && (settings::flags_team || settings::flags_state || settings::flags_health);
        if (!settings::box_enabled && !settings::skeleton_enabled && !settings::health_enabled && !settings::name_enabled && !settings::headdot_enabled && !settings::distance_enabled && !settings::tool_enabled && !any_flags && !settings::tracer_enabled && !settings::oof_enabled && !settings::aim_tracer && !settings::aim_enabled && !settings::aim_autoshoot) return;
        if (!wcache::viewValid) return;
        const math::view_matrix& view = wcache::view;
        const math::vec3& camPos = wcache::camPos;
        ImU32 boxCol = 0, boxOutlineCol = 0, boxFill1 = 0, boxFill2 = 0;
        if (settings::box_enabled)
        {
            boxCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::box_color[0], settings::box_color[1], settings::box_color[2], 1.0f));
            boxOutlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::box_outline_color[0], settings::box_outline_color[1], settings::box_outline_color[2], 1.0f));
            if (settings::box_filled) {
                boxFill1 = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::box_fill_color[0], settings::box_fill_color[1], settings::box_fill_color[2], settings::box_fill_color[3]));
                if (settings::box_fill_gradient)
                    boxFill2 = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::box_fill_color2[0], settings::box_fill_color2[1], settings::box_fill_color2[2], settings::box_fill_color2[3]));
            }
        }
        ImU32 skelCol = 0, skelOutlineCol = 0;
        if (settings::skeleton_enabled)
        {
            skelCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::skeleton_color[0], settings::skeleton_color[1], settings::skeleton_color[2], 1.0f));
            skelOutlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::skeleton_outline_color[0], settings::skeleton_outline_color[1], settings::skeleton_outline_color[2], 1.0f));
        }
        ImFont* verdana = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
        for (auto& wc : wcache::entries)
        {
            if (!wc.valid) continue;
            data::rect rc{};
            if (thenwefuckin::settings::box_mode == 1 && wc.hasPts3d && !wc.pts3d.empty()) {
                data::rect dyn = projectBoxPoints(wc.pts3d, view, fw, fh, camPos);
                auto dit = wcache::dynRect.find(wc.playerAddr);
                bool havePrev = (dit != wcache::dynRect.end()) &&
                    (now - dit->second.t < std::chrono::milliseconds(250)) &&
                    !dit->second.rc.empty();
                if (!dyn.empty()) {

                    rc = dyn;
                    wcache::dynRect[wc.playerAddr] = wcache::DynRectState{ rc, now };
                } else if (havePrev) {

                    rc = dit->second.rc;
                }
            }
            if (rc.empty()) {

                if ((wc.head - camPos).length() < 2.5f || (wc.root - camPos).length() < 2.5f)
                    continue;
                math::vec3 top3{ wc.head.x, wc.head.y + 0.6f, wc.head.z };
                float drop = wc.isR6 ? 3.0f : 2.5f;
                math::vec3 bot3{ wc.root.x, wc.root.y - drop, wc.root.z };
                math::vec2 top{};
                math::vec2 bot{};
                bool a = projection::to_screen(top3, view, fw, fh, top);
                bool b = projection::to_screen(bot3, view, fw, fh, bot);
                if (!a || !b) continue;
                float hh = fabsf(bot.y - top.y) + kStaticBoxPadY * 2.0f;
                if (hh < 4.0f) hh = 4.0f;
                float ww = (std::max)(hh * kStaticBoxAspect, 3.0f);
                float minY = (std::min)(top.y, bot.y) - kStaticBoxPadY;
                float midX = (top.x + bot.x) * 0.5f;
                rc.x0 = midX - ww * 0.5f;
                rc.x1 = midX + ww * 0.5f;
                rc.y0 = minY;
                rc.y1 = minY + hh;
            }
            rc.x0 = std::floor(rc.x0);
            rc.x1 = std::floor(rc.x1);
            rc.y0 = std::floor(rc.y0);
            rc.y1 = std::floor(rc.y1);

            {
                float ccx = (rc.x0 + rc.x1) * 0.5f;
                float ccy = (rc.y0 + rc.y1) * 0.5f;
                if (ccx < -12.0f || ccx > fw + 12.0f || ccy < -12.0f || ccy > fh + 12.0f)
                    continue;
            }
            float studsDist = (wc.root - g_ctx.local_pos).length();
            if (studsDist < 0.1f) studsDist = (wc.head - g_ctx.local_pos).length();
            if (studsDist > settings::visual_maxdist)
                continue;
            float metersDist = studsDist * 0.28f;
            float distScale = std::clamp(18.0f / (studsDist + 8.0f) + 0.55f, 0.55f, 1.45f);

            float rcW = rc.x1 - rc.x0;
            float rcH = rc.y1 - rc.y0;
            bool oversize = (rcW > fw * 0.85f || rcH > fh * 0.85f);
            if (settings::box_enabled && !oversize)
            {
                ImU32 drawCol = players::friends::is_friend(wc.playerAddr) ? IM_COL32(0, 255, 0, 255) : boxCol;
                if (settings::box_filled) {
                    if (settings::box_fill_gradient) {
                        box::draw_fill_gradient(dl, rc.x0, rc.y0, rc.x1, rc.y1, boxFill1, boxFill2);
                    } else {
                        box::draw_fill(dl, rc.x0, rc.y0, rc.x1, rc.y1, boxFill1);
                    }
                }
                if (settings::box_style == 1) {
                    box::draw_corner(dl, rc.x0, rc.y0, rc.x1, rc.y1, drawCol, settings::box_outline, boxOutlineCol);
                } else {
                    box::draw(dl, rc.x0, rc.y0, rc.x1, rc.y1, drawCol, settings::box_outline, boxOutlineCol);
                }
            }
            if (settings::health_enabled) {
                ImU32 hoc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::health_outline_color[0], settings::health_outline_color[1], settings::health_outline_color[2], 1.0f));
                ImU32 hcol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::health_color[0], settings::health_color[1], settings::health_color[2], 1.0f));
                health::draw(dl, rc, wc.hp, wc.maxHp, settings::health_outline, hoc, settings::health_gradient, settings::health_text, hcol, settings::box_enabled && settings::box_outline, 1.0f);
            }
            if (settings::name_enabled && !wc.name.empty())
            {
                ImU32 nameCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::name_color[0], settings::name_color[1], settings::name_color[2], 1.0f));
                ImU32 nameOutlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::name_outline_color[0], settings::name_outline_color[1], settings::name_outline_color[2], 1.0f));
                float fontSize = 13.0f;
                ImVec2 ts = verdana->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, wc.name.c_str());
                float tx = rc.x0 + (rc.x1 - rc.x0) * 0.5f - ts.x * 0.5f;
                float ty = rc.y0 - ts.y - 3.0f;
                DrawVisualText(dl, verdana, fontSize, tx, ty, nameCol, settings::name_outline, nameOutlineCol, wc.name.c_str());
            }
            if (settings::skeleton_enabled)
            {
                const float baseThick = 1.5f * distScale;
                const float outlineThick = baseThick + 1.0f * distScale;
                if (wc.isR6)
                {
                    skeleton::pose_r6 pose{};
                    pose.head = wc.head;
                    pose.torso = wc.torso.x != 0 ? wc.torso : wc.root;
                    if (wc.leftArm.x != 0) { pose.arm_l = wc.leftArm; pose.has_arm_l = true; }
                    if (wc.rightArm.x != 0) { pose.arm_r = wc.rightArm; pose.has_arm_r = true; }
                    if (wc.leftLeg.x != 0) { pose.leg_l = wc.leftLeg; pose.has_leg_l = true; }
                    if (wc.rightLeg.x != 0) { pose.leg_r = wc.rightLeg; pose.has_leg_r = true; }
                    if (wc.hasTorsoFrame) { pose.torsoFrame = wc.torsoFrame; pose.has_torsoFrame = true; }
                    auto out = skeleton::build_r6(pose, view, fw, fh);
                    if (settings::skeleton_outline) for (int i=0;i<out.count;++i) dl->AddLine(ImVec2(out.items[i].a.x,out.items[i].a.y), ImVec2(out.items[i].b.x,out.items[i].b.y), skelOutlineCol, outlineThick);
                    for (int i=0;i<out.count;++i) dl->AddLine(ImVec2(out.items[i].a.x,out.items[i].a.y), ImVec2(out.items[i].b.x,out.items[i].b.y), skelCol, baseThick);
                }
                else
                {
                    skeleton::pose_r15 pose{};
                    pose.head = wc.head;
                    pose.upper = wc.upper.x != 0 ? wc.upper : wc.root;
                    pose.lower = wc.lower;
                    pose.has_lower = wc.hasLower;
                    if (pose.lower.x == 0) pose.lower = pose.upper;
                    pose.l_upArm = wc.leftUpperArm;
                    pose.r_upArm = wc.rightUpperArm;
                    pose.l_lowArm = wc.leftLowerArm;
                    pose.r_lowArm = wc.rightLowerArm;
                    pose.hand_l = wc.leftHand;
                    pose.hand_r = wc.rightHand;
                    pose.l_upLeg = wc.leftUpperLeg;
                    pose.r_upLeg = wc.rightUpperLeg;
                    pose.l_lowLeg = wc.leftLowerLeg;
                    pose.r_lowLeg = wc.rightLowerLeg;
                    pose.foot_l = wc.leftFoot;
                    pose.foot_r = wc.rightFoot;
                    pose.elbow_l = pose.l_lowArm;
                    pose.elbow_r = pose.r_lowArm;
                    pose.knee_l = pose.l_lowLeg;
                    pose.knee_r = pose.r_lowLeg;
                    if (pose.hand_l.x==0 && pose.l_lowArm.x!=0) pose.hand_l = pose.l_lowArm;
                    else if (pose.hand_l.x==0 && pose.l_upArm.x!=0) pose.hand_l = pose.l_upArm;
                    if (pose.hand_r.x==0 && pose.r_lowArm.x!=0) pose.hand_r = pose.r_lowArm;
                    else if (pose.hand_r.x==0 && pose.r_upArm.x!=0) pose.hand_r = pose.r_upArm;
                    if (pose.foot_l.x==0 && pose.l_lowLeg.x!=0) pose.foot_l = pose.l_lowLeg;
                    else if (pose.foot_l.x==0 && pose.l_upLeg.x!=0) pose.foot_l = pose.l_upLeg;
                    if (pose.foot_r.x==0 && pose.r_lowLeg.x!=0) pose.foot_r = pose.r_lowLeg;
                    else if (pose.foot_r.x==0 && pose.r_upLeg.x!=0) pose.foot_r = pose.r_upLeg;
                    auto out = skeleton::build_r15(pose, view, fw, fh);
                    if (settings::skeleton_outline) for (int i=0;i<out.count;++i) dl->AddLine(ImVec2(out.items[i].a.x,out.items[i].a.y), ImVec2(out.items[i].b.x,out.items[i].b.y), skelOutlineCol, outlineThick);
                    for (int i=0;i<out.count;++i) dl->AddLine(ImVec2(out.items[i].a.x,out.items[i].a.y), ImVec2(out.items[i].b.x,out.items[i].b.y), skelCol, baseThick);
                }
            }
            headdot::draw(dl, wc.head, view, fw, fh, distScale);
            float distY = rc.y1 + 2.0f;
            ImVec2 distTs{};
            if (settings::distance_enabled)
            {
                char buf[32];
                if (metersDist < 10.0f) snprintf(buf, sizeof(buf), "%.1fm", metersDist);
                else snprintf(buf, sizeof(buf), "%.0fm", metersDist);
                ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                float fontSize = 13.0f;
                distTs = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, buf);
                float tx = rc.x0 + (rc.x1 - rc.x0) * 0.5f - distTs.x * 0.5f;
                float ty = distY;
                ImU32 dcol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::distance_color[0], settings::distance_color[1], settings::distance_color[2], 1.0f));
                ImU32 doc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::distance_outline_color[0], settings::distance_outline_color[1], settings::distance_outline_color[2], 1.0f));
                DrawVisualText(dl, font, fontSize, tx, ty, dcol, settings::distance_outline, doc, buf);
                distY += distTs.y + 2.0f;
            }
            if (settings::tool_enabled)
            {
                bool inRange = true;
                if (settings::tool_distance_check && studsDist > settings::tool_distance_value)
                    inRange = false;
                if (inRange)
                {
                    const char* toolTxt = wc.tool.empty() ? "None" : wc.tool.c_str();
                    ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                    float fontSize = 13.0f;
                    ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, toolTxt);
                    float tx = rc.x0 + (rc.x1 - rc.x0) * 0.5f - ts.x * 0.5f;
                    float ty = distY;
                    ImU32 tcol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::tool_color[0], settings::tool_color[1], settings::tool_color[2], 1.0f));
                    ImU32 tocol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::tool_outline_color[0], settings::tool_outline_color[1], settings::tool_outline_color[2], 1.0f));
                    DrawVisualText(dl, font, fontSize, tx, ty, tcol, settings::tool_outline, tocol, toolTxt);
                }
            }
            if (settings::flags_enabled && (settings::flags_team || settings::flags_state || settings::flags_health))
            {
                ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                float fontSize = 13.0f;
                float fx = rc.x1 + 4.0f;
                float fy = rc.y0;
                if (settings::flags_team && !wc.team.empty())
                {
                    ImU32 fcol = settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_color[0], settings::flags_color[1], settings::flags_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_team_color[0], settings::flags_team_color[1], settings::flags_team_color[2], 1.0f));
                    ImU32 focol = settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_outline_color[0], settings::flags_outline_color[1], settings::flags_outline_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_team_outline_color[0], settings::flags_team_outline_color[1], settings::flags_team_outline_color[2], 1.0f));
                    bool use_outline = settings::flags_global ? settings::flags_outline : settings::flags_team_outline;
                    DrawVisualText(dl, font, fontSize, fx, fy, fcol, use_outline, focol, wc.team.c_str());
                    fy += fontSize + 2.0f;
                }
                if (settings::flags_state && !wc.state.empty())
                {
                    ImU32 fcol = settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_color[0], settings::flags_color[1], settings::flags_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_state_color[0], settings::flags_state_color[1], settings::flags_state_color[2], 1.0f));
                    ImU32 focol = settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_outline_color[0], settings::flags_outline_color[1], settings::flags_outline_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_state_outline_color[0], settings::flags_state_outline_color[1], settings::flags_state_outline_color[2], 1.0f));
                    bool use_outline = settings::flags_global ? settings::flags_outline : settings::flags_state_outline;
                    DrawVisualText(dl, font, fontSize, fx, fy, fcol, use_outline, focol, wc.state.c_str());
                    fy += fontSize + 2.0f;
                }
                if (settings::flags_health)
                {
                    ImU32 fcol = settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_color[0], settings::flags_color[1], settings::flags_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_health_color[0], settings::flags_health_color[1], settings::flags_health_color[2], 1.0f));
                    ImU32 focol = settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_outline_color[0], settings::flags_outline_color[1], settings::flags_outline_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(settings::flags_health_outline_color[0], settings::flags_health_outline_color[1], settings::flags_health_outline_color[2], 1.0f));
                    bool use_outline = settings::flags_global ? settings::flags_outline : settings::flags_health_outline;
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.0f%%", (double)wc.hp);
                    DrawVisualText(dl, font, fontSize, fx, fy, fcol, use_outline, focol, buf);
                }
            }
            if (settings::tracer_enabled) {
                ImU32 tcol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::tracer_color[0], settings::tracer_color[1], settings::tracer_color[2], 1.0f));
                ImU32 tocol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::tracer_outline_color[0], settings::tracer_outline_color[1], settings::tracer_outline_color[2], 1.0f));
                math::vec2 hsp;
                bool hasHead = projection::to_screen(wc.head, view, fw, fh, hsp);
                if (hasHead && hsp.x >= 0 && hsp.x <= w && hsp.y >= 0 && hsp.y <= h) {
                    tracer::draw(dl, hsp.x - 1, hsp.y - 1, hsp.x + 1, hsp.y + 1, tcol, settings::tracer_outline, tocol, settings::tracer_thickness * distScale, w, h, settings::tracer_origin);
                } else {
                    tracer::draw(dl, rc.x0, rc.y0, rc.x1, rc.y1, tcol, settings::tracer_outline, tocol, settings::tracer_thickness * distScale, w, h, settings::tracer_origin);
                }
            }
        }
        if (settings::aim_tracer)
        {
            math::vec3 aw{};
            if (thenwefuckin::aim::debug_target(aw))
            {
                math::vec2 asp;
                if (projection::to_screen(aw, view, fw, fh, asp)
                    && asp.x >= -100.0f && asp.x <= (float)w + 100.0f
                    && asp.y >= -100.0f && asp.y <= (float)h + 100.0f)
                {
                    float thick = settings::aim_tracer_thickness;
                    if (thick < 0.5f) thick = 0.5f;
                    if (thick > 8.0f) thick = 8.0f;
                    ImVec2 a{ (float)w * 0.5f, (float)h * 0.5f };
                    ImVec2 b{ asp.x, asp.y };
                    if (settings::aim_tracer_outline)
                    {
                        ImU32 oc = ImGui::ColorConvertFloat4ToU32(ImVec4(
                            settings::aim_tracer_outline_color[0], settings::aim_tracer_outline_color[1],
                            settings::aim_tracer_outline_color[2], 1.0f));
                        dl->AddLine(a, b, oc, thick + 1.5f);
                    }
                    ImU32 tc = ImGui::ColorConvertFloat4ToU32(ImVec4(
                        settings::aim_tracer_color[0], settings::aim_tracer_color[1],
                        settings::aim_tracer_color[2], 1.0f));
                    dl->AddLine(a, b, tc, thick);
                    dl->AddCircleFilled(b, 3.0f, tc);
                }
            }
        }
        if (settings::oof_enabled) {
            auto drawOofExtra = [&](WorldCache& wc2){
                math::vec4 clip = view.transform(wc2.head);
                float cx = w*0.5f, cy = h*0.5f;
                if (clip.w < 0.1f) { clip.x = -clip.x; clip.y = -clip.y; }
                float inv = 1.0f / (fabsf(clip.w) < 0.001f ? 0.001f : clip.w);
                float nx = clip.x * inv; float ny = clip.y * inv;
                float sx = (w*0.5f * nx) + cx; float sy = -(h*0.5f * ny) + cy;
                float dx = sx - cx, dy = sy - cy;
                if (clip.w < 0.1f) { dx = -dx; dy = -dy; }
                float len = sqrtf(dx*dx + dy*dy); if (len < 1) return;
                dx/=len; dy/=len;
                float ex = cx + dx * settings::oof_radius;
                float ey = cy + dy * settings::oof_radius;
                ex = std::clamp(ex, 30.0f, (float)w-30.0f); ey = std::clamp(ey, 30.0f, (float)h-30.0f);
                ImFont* f = g_verdanaSmallFont ? g_verdanaSmallFont : (g_verdanaFont ? g_verdanaFont : ImGui::GetFont());
                float fs = 10.0f;
                float yOff = 0;
                float inwardX = -dx * 18.0f;
                float inwardY = -dy * 18.0f;
                float baseX = std::floor(ex + inwardX);
                float baseY = std::floor(ey + inwardY);
                if (settings::oof_avatar) {
                    dl->AddRectFilled(ImVec2(baseX-8, baseY -8), ImVec2(baseX+8, baseY +8), IM_COL32(80,80,80,255));
                    dl->AddRect(ImVec2(baseX-8, baseY -8), ImVec2(baseX+8, baseY +8), IM_COL32(0,0,0,255));
                    baseY += 18;
                }
                if (settings::oof_names && !wc2.name.empty()) {
                    ImU32 nc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::oof_name_color[0], settings::oof_name_color[1], settings::oof_name_color[2], 1.0f));
                    ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, wc2.name.c_str());
                    float tx = baseX - ts.x*0.5f, ty = baseY;
                    DrawVisualText(dl, f, fs, tx, ty, nc, true, IM_COL32(0,0,0,255), wc2.name.c_str());
                    baseY += 12;
                }
                if (settings::oof_health) {
                    ImU32 hc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::oof_health_color[0], settings::oof_health_color[1], settings::oof_health_color[2], 1.0f));
                    char buf[16]; int pct=(int)(wc2.hp/wc2.maxHp*100+0.5f); snprintf(buf,sizeof(buf),"%d%%",pct);
                    ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, buf);
                    float tx = baseX - ts.x*0.5f, ty = baseY;
                    DrawVisualText(dl, f, fs, tx, ty, hc, true, IM_COL32(0,0,0,255), buf);
                    baseY += 12;
                }
                if (settings::oof_distance) {
                    ImU32 dc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::oof_distance_color[0], settings::oof_distance_color[1], settings::oof_distance_color[2], 1.0f));
                    float d = (wc2.root - g_ctx.local_pos).length()*0.28f;
                    char buf[16]; snprintf(buf,sizeof(buf),"%.0fm",d);
                    ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, buf);
                    float tx = baseX - ts.x*0.5f, ty = baseY;
                    DrawVisualText(dl, f, fs, tx, ty, dc, true, IM_COL32(0,0,0,255), buf);
                }
            };
            for (auto& wc2 : wcache::entries) {
                math::vec2 sp; bool on = projection::to_screen(wc2.head, view, fw, fh, sp);
                bool off = !on || sp.x < 0 || sp.x > w || sp.y < 0 || sp.y > h || !wc2.valid;
                if (!off) continue;
                if (wc2.head.x==0 && wc2.head.y==0 && wc2.head.z==0) continue;
                ImU32 ocol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::oof_color[0], settings::oof_color[1], settings::oof_color[2], 1.0f));
                ImU32 oocol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::oof_outline_color[0], settings::oof_outline_color[1], settings::oof_outline_color[2], 1.0f));
                oof::draw(dl, wc2.head, view, w, h, ocol, settings::oof_outline, oocol, settings::oof_size, settings::oof_radius);
                drawOofExtra(wc2);
            }
        }
    }
}
