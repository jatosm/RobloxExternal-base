#include "mascot.h"
#include "menu.h"
#include <cmath>
namespace Menu
{
    bool IsChristmasActive()
    {
        return (g_themeMask & (1 << kChristmasThemeBit)) != 0;
    }
    namespace
    {
        const ImU32 kOutline  = IM_COL32(8, 10, 14, 255);
        const ImU32 kRed      = IM_COL32(202, 38, 48, 255);
        const ImU32 kRedDark  = IM_COL32(146, 24, 32, 255);
        const ImU32 kFur      = IM_COL32(250, 253, 255, 255);
        const ImU32 kFurShade = IM_COL32(214, 226, 238, 255);
        const ImU32 kSkin     = IM_COL32(242, 194, 162, 255);
        const ImU32 kSkinDark = IM_COL32(208, 152, 122, 255);
        const ImU32 kBoot     = IM_COL32(38, 32, 36, 255);
        const ImU32 kBelt     = IM_COL32(34, 28, 32, 255);
        const ImU32 kGold     = IM_COL32(236, 196, 92, 255);
        const ImU32 kSack     = IM_COL32(150, 108, 66, 255);
        const ImU32 kSackDark = IM_COL32(112, 78, 46, 255);
        const ImU32 kCheek    = IM_COL32(226, 138, 138, 190);

        void SnowCross(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
        {
            dl->AddLine(ImVec2(c.x - s, c.y), ImVec2(c.x + s, c.y), col, 1.0f);
            dl->AddLine(ImVec2(c.x, c.y - s), ImVec2(c.x, c.y + s), col, 1.0f);
        }
    }
    void DrawChristmasMascot()
    {
        if (!IsChristmasActive())
            return;
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        if (!dl)
            return;

        const float W = 132.0f;
        const float H = 140.0f;
        const float bx = floorf(g_pos.x + g_menuSize.x - 6.0f);
        const float by = floorf(g_pos.y + 1.0f);
        const float cx = bx - W * 0.5f;
        const float t = (float)ImGui::GetTime();

        for (int i = 0; i < 9; i++)
        {
            const float sx = bx - W + 8.0f + (float)((i * 41) % (int)W);
            const float span = H - 8.0f;
            const float off = fmodf(t * 10.0f + (float)i * 19.0f, span);
            const float sy = by - span + off;
            const float sway = sinf(t * 0.9f + (float)i) * 3.5f;
            const ImVec2 c(floorf(sx + sway), floorf(sy));
            const bool big = (i % 3 == 0);
            dl->AddCircleFilled(c, big ? 2.0f : 1.4f, big ? IM_COL32(232, 243, 255, 200) : IM_COL32(214, 232, 248, 150), 8);
            if (big)
                SnowCross(dl, c, 4.0f, IM_COL32(226, 240, 255, 110));
        }

        const float bob = sinf(t * 1.6f) * 1.6f;
        const float lean = sinf(t * 0.8f) * 1.1f;
        const float base = by + bob;

        auto P = [&](float ox, float oy) { return ImVec2(cx + ox + lean, base + oy); };
        auto ball = [&](float ox, float oy, float r, ImU32 col) {
            const ImVec2 c = P(ox, oy);
            dl->AddCircleFilled(c, r, col, 0);
            dl->AddCircle(c, r, kOutline, 0, 1.0f);
        };
        auto box = [&](float x0, float y0, float x1, float y1, float rounding, ImU32 col) {
            const ImVec2 a = P(x0, y0);
            const ImVec2 b = P(x1, y1);
            dl->AddRectFilled(a, b, col, rounding);
            dl->AddRect(a, b, kOutline, rounding, 0, 1.0f);
        };

        dl->AddRectFilled(ImVec2(cx - 31.0f, by - 8.0f), ImVec2(cx + 31.0f, by - 1.0f), IM_COL32(0, 0, 0, 80), 3.0f);

        box(-42.0f, -58.0f, -14.0f, -16.0f, 10.0f, kSack);
        box(-42.0f, -62.0f, -14.0f, -52.0f, 5.0f, kFur);
        dl->AddCircleFilled(P(-36.0f, -34.0f), 9.0f, kSackDark, 0);
        dl->AddLine(P(-31.0f, -52.0f), P(-27.0f, -45.0f), kSackDark, 2.0f);

        box(-19.0f, -31.0f, -5.0f, -13.0f, 3.0f, kRedDark);
        box(5.0f, -31.0f, 19.0f, -13.0f, 3.0f, kRedDark);
        box(-22.0f, -15.0f, -3.0f, -1.0f, 4.0f, kBoot);
        box(3.0f, -15.0f, 22.0f, -1.0f, 4.0f, kBoot);
        box(-22.0f, -18.0f, -3.0f, -13.0f, 3.0f, kFur);
        box(3.0f, -18.0f, 22.0f, -13.0f, 3.0f, kFur);

        dl->AddLine(P(-21.0f, -62.0f), P(-35.0f, -41.0f), kRed, 9.0f);
        ball(-22.0f, -62.0f, 5.0f, kRed);
        ball(-36.0f, -39.0f, 5.5f, kFur);
        dl->AddLine(P(21.0f, -62.0f), P(37.0f, -73.0f), kRed, 9.0f);
        ball(22.0f, -62.0f, 5.0f, kRed);
        ball(39.0f, -76.0f, 5.5f, kFur);

        box(-24.0f, -68.0f, 24.0f, -29.0f, 9.0f, kRed);
        box(-25.0f, -37.0f, 25.0f, -29.0f, 4.0f, kFur);
        box(-4.0f, -68.0f, 4.0f, -54.0f, 2.0f, kFur);
        box(-25.0f, -53.0f, 25.0f, -44.0f, 2.0f, kBelt);
        box(-8.0f, -54.0f, 8.0f, -43.0f, 2.0f, kGold);
        box(-4.5f, -51.0f, 4.5f, -46.0f, 1.0f, kBelt);

        box(-17.0f, -78.0f, 17.0f, -65.0f, 5.0f, kFur);

        ball(0.0f, -88.0f, 12.5f, kSkin);
        box(-12.0f, -86.0f, 12.0f, -71.0f, 6.0f, kFur);
        ball(0.0f, -72.0f, 7.5f, kFur);
        ball(-4.5f, -85.0f, 4.0f, kFur);
        ball(4.5f, -85.0f, 4.0f, kFur);
        ball(0.0f, -88.5f, 3.0f, kSkinDark);
        dl->AddCircleFilled(P(-4.5f, -93.5f), 1.6f, kOutline, 8);
        dl->AddCircleFilled(P(4.5f, -93.5f), 1.6f, kOutline, 8);
        dl->AddCircleFilled(P(-4.5f, -94.0f), 0.6f, IM_COL32(255, 255, 255, 210), 6);
        dl->AddCircleFilled(P(4.5f, -94.0f), 0.6f, IM_COL32(255, 255, 255, 210), 6);
        dl->AddLine(P(-7.5f, -97.0f), P(-2.5f, -98.0f), kFur, 1.6f);
        dl->AddLine(P(2.5f, -98.0f), P(7.5f, -97.0f), kFur, 1.6f);
        dl->AddCircleFilled(P(-8.5f, -90.5f), 2.4f, kCheek, 8);
        dl->AddCircleFilled(P(8.5f, -90.5f), 2.4f, kCheek, 8);

        box(-15.0f, -102.0f, 15.0f, -95.0f, 4.0f, kFur);
        {
            const ImVec2 a = P(-13.0f, -100.0f);
            const ImVec2 b = P(13.0f, -100.0f);
            const ImVec2 c = P(18.0f, -124.0f);
            dl->AddTriangleFilled(a, b, c, kRed);
            dl->AddTriangle(a, b, c, kOutline, 1.0f);
        }
        ball(19.0f, -126.0f, 4.5f, kFur);
        dl->AddLine(P(-5.0f, -102.0f), P(6.0f, -119.0f), kRedDark, 1.0f);

        dl->AddCircleFilled(P(-9.0f, -103.0f), 1.1f, kFur, 6);
        dl->AddCircleFilled(P(6.0f, -103.5f), 0.9f, kFurShade, 6);
        dl->AddCircleFilled(P(-14.5f, -66.0f), 1.2f, kFur, 6);
        dl->AddCircleFilled(P(14.0f, -64.0f), 1.0f, kFurShade, 6);
    }
}
