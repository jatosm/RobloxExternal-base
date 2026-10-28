#include "health.h"
#include "../../../menu/resources/font.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
namespace thenwefuckin::visual::health
{
    void draw_bar(ImDrawList* dl, float bx0, float bx1, float by0, float by1, float frac, bool outline, ImU32 outlineCol, bool gradient, bool showText, ImU32 barCol)
    {
        if (!dl) return;
        if (by1 < by0) std::swap(by0, by1);
        bx0 = std::floor(bx0 + 0.5f);
        bx1 = bx0 + 2.0f;
        by0 = std::floor(by0 + 0.5f);
        by1 = std::floor(by1 + 0.5f);
        float h = by1 - by0;
        if (h < 4.0f) {
            h = 4.0f;
            by1 = by0 + h;
        }
        if (frac < 0.f) frac = 0.f;
        if (frac > 1.f) frac = 1.f;
        dl->AddRectFilled(ImVec2(bx0, by0), ImVec2(bx1, by1), IM_COL32(0, 0, 0, 200));
        float fillTop = by1 - h * frac;
        if (fillTop < by0) fillTop = by0;
        if (frac > 0.001f) {
            if (gradient) {
                ImU32 topCol = IM_COL32(255, 0, 0, 255);
                ImU32 midCol = IM_COL32(255, 255, 0, 255);
                ImU32 botCol = IM_COL32(0, 255, 0, 255);
                float midY = by0 + h * 0.5f;
                dl->AddRectFilledMultiColor(ImVec2(bx0, by0), ImVec2(bx1, midY), topCol, topCol, midCol, midCol);
                dl->AddRectFilledMultiColor(ImVec2(bx0, midY), ImVec2(bx1, by1), midCol, midCol, botCol, botCol);
                if (fillTop > by0) {
                    dl->AddRectFilled(ImVec2(bx0, by0), ImVec2(bx1, fillTop), IM_COL32(0,0,0,200));
                }
            } else {
                ImU32 c = barCol ? barCol : IM_COL32((int)(255 * (1.f - frac)), (int)(255 * frac), 0, 255);
                dl->AddRectFilled(ImVec2(bx0, fillTop), ImVec2(bx1, by1), c);
            }
        }
        if (outline) {
            const ImU32 ob = outlineCol ? outlineCol : IM_COL32(0, 0, 0, 255);
            dl->AddRectFilled(ImVec2(bx0 - 1, by0 - 1), ImVec2(bx1 + 1, by0), ob);
            dl->AddRectFilled(ImVec2(bx0 - 1, by1), ImVec2(bx1 + 1, by1 + 1), ob);
            dl->AddRectFilled(ImVec2(bx0 - 1, by0), ImVec2(bx0, by1), ob);
            dl->AddRectFilled(ImVec2(bx1, by0), ImVec2(bx1 + 1, by1), ob);
        }
        if (showText) {
            int pct = (int)(frac * 100.0f + 0.5f);
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", pct);
            ImFont* font = g_verdanaSmallFont ? g_verdanaSmallFont : (g_verdanaFont ? g_verdanaFont : ImGui::GetFont());
            float fs = 10.0f;
            ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, buf);
            float cx = std::floor(bx0 + (bx1 - bx0) * 0.5f - ts.x * 0.5f);
            float cy = std::floor(by0 - ts.y * 0.5f + 1.0f);
            static const ImVec2 offsets[8] = {
                ImVec2(-1.0f, -1.0f), ImVec2(0.0f, -1.0f), ImVec2(1.0f, -1.0f),
                ImVec2(-1.0f,  0.0f),                      ImVec2(1.0f,  0.0f),
                ImVec2(-1.0f,  1.0f), ImVec2(0.0f,  1.0f), ImVec2(1.0f,  1.0f)
            };
            for (int i = 0; i < 8; i++) {
                dl->AddText(font, fs, ImVec2(cx + offsets[i].x, cy + offsets[i].y), IM_COL32(0,0,0,255), buf);
            }
            dl->AddText(font, fs, ImVec2(cx, cy), IM_COL32(255,255,255,255), buf);
        }
    }
    void draw(ImDrawList* dl, const data::rect& rc, float hp, float maxHp, bool outline, ImU32 outlineCol, bool gradient, bool showText, ImU32 barCol, bool boxOutline, float gap)
    {
        if (!dl) return;
        float f = (maxHp > 0.01f) ? (hp / maxHp) : 1.f;
        if (!std::isfinite(f)) f = 1.f;
        f = std::clamp(f, 0.0f, 1.0f);
        float boxLeft = rc.x0 - (boxOutline ? 1.0f : 0.0f);
        float barRight = boxLeft - gap - (outline ? 1.0f : 0.0f);
        const float bx1 = std::floor(barRight);
        const float bx0 = bx1 - 2.0f;
        draw_bar(dl, bx0, bx1, rc.y0, rc.y1, f, outline, outlineCol, gradient, showText, barCol);
    }
}
