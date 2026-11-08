#include "oof.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../menu/overlay/settings.h"
#include <cmath>
#include <algorithm>
namespace thenwefuckin::visual::oof
{
    void draw(ImDrawList* dl, const math::vec3& worldPos, const math::view_matrix& view, int screenW, int screenH, ImU32 col, bool outline, ImU32 outlineCol, float size, float radius)
    {
        if (!dl) return;
        math::vec2 sp;
        bool onScreen = projection::to_screen(worldPos, view, (float)screenW, (float)screenH, sp);
        float cx = screenW * 0.5f;
        float cy = screenH * 0.5f;
        bool off = !onScreen || sp.x < 0 || sp.x > screenW || sp.y < 0 || sp.y > screenH;
        if (!off) return;
        float dx, dy;
        math::vec4 clip = view.transform(worldPos);
        if (clip.w < 0.1f) {
            clip.x = -clip.x; clip.y = -clip.y;
        }
        float inv = 1.0f / (fabsf(clip.w) < 0.001f ? 0.001f : clip.w);
        float nx = clip.x * inv;
        float ny = clip.y * inv;
        float sx = (screenW * 0.5f * nx) + cx;
        float sy = -(screenH * 0.5f * ny) + cy;
        dx = sx - cx;
        dy = sy - cy;
        if (clip.w < 0.1f) { dx = -dx; dy = -dy; }
        float len = sqrtf(dx*dx + dy*dy);
        if (len < 1.0f) return;
        dx /= len; dy /= len;
        float ex = cx + dx * radius;
        float ey = cy + dy * radius;
        ex = std::clamp(ex, 30.0f, (float)screenW - 30.0f);
        ey = std::clamp(ey, 30.0f, (float)screenH - 30.0f);
        float ang = atan2f(dy, dx);
        float s = size;
        float ca = cosf(ang), sa = sinf(ang);
        float px = -sa, py = ca;
        if (settings::oof_arrow_style == 1)
        {
            float H = s * 2.0f;
            float W = H * 0.76f;
            ImVec2 pts[4] = {
                ImVec2(ex + ca * (H * 0.50f), ey + sa * (H * 0.50f)),
                ImVec2(ex - ca * (H * 0.50f) + px * (W * 0.50f), ey - sa * (H * 0.50f) + py * (W * 0.50f)),
                ImVec2(ex - ca * (H * 0.20f), ey - sa * (H * 0.20f)),
                ImVec2(ex - ca * (H * 0.50f) - px * (W * 0.50f), ey - sa * (H * 0.50f) - py * (W * 0.50f))
            };
            dl->AddTriangleFilled(pts[0], pts[1], pts[2], col);
            dl->AddTriangleFilled(pts[0], pts[2], pts[3], col);
            if (outline)
                dl->AddPolyline(pts, 4, outlineCol, ImDrawFlags_Closed, 1.0f);
        }
        else if (settings::oof_arrow_style == 2)
        {
            float H = s * 2.0f;
            float W = H * 0.76f;
            ImVec2 t = ImVec2(ex + ca * (H * 0.50f), ey + sa * (H * 0.50f));
            ImVec2 l = ImVec2(ex - ca * (H * 0.50f) + px * (W * 0.50f), ey - sa * (H * 0.50f) + py * (W * 0.50f));
            ImVec2 r = ImVec2(ex - ca * (H * 0.50f) - px * (W * 0.50f), ey - sa * (H * 0.50f) - py * (W * 0.50f));
            dl->AddTriangleFilled(t, l, r, col);
            if (outline) {
                ImVec2 e[3] = { t, l, r };
                dl->AddPolyline(e, 3, outlineCol, ImDrawFlags_Closed, 1.0f);
            }
        }
        else
        {
            float H = s * 2.0f;
            float W = H * 0.76f;
            ImVec2 pts[7] = {
                ImVec2(ex + ca * (H * 0.50f), ey + sa * (H * 0.50f)),
                ImVec2(ex - ca * (H * 0.15f) + px * (W * 0.50f), ey - sa * (H * 0.15f) + py * (W * 0.50f)),
                ImVec2(ex - ca * (H * 0.32f) + px * (W * 0.50f), ey - sa * (H * 0.32f) + py * (W * 0.50f)),
                ImVec2(ex - ca * (H * 0.50f) + px * (W * 0.28f), ey - sa * (H * 0.50f) + py * (W * 0.28f)),
                ImVec2(ex - ca * (H * 0.50f) - px * (W * 0.28f), ey - sa * (H * 0.50f) - py * (W * 0.28f)),
                ImVec2(ex - ca * (H * 0.32f) - px * (W * 0.50f), ey - sa * (H * 0.32f) - py * (W * 0.50f)),
                ImVec2(ex - ca * (H * 0.15f) - px * (W * 0.50f), ey - sa * (H * 0.15f) - py * (W * 0.50f))
            };
            dl->AddConvexPolyFilled(pts, 7, col);
            if (outline)
                dl->AddPolyline(pts, 7, outlineCol, ImDrawFlags_Closed, 1.0f);
        }
    }
}
