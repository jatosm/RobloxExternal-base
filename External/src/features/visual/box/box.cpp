#include "box.h"
#include <cmath>
namespace thenwefuckin::visual::box
{
    void draw(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline, ImU32 outlineCol)
    {
        if (!dl) return;
        if (outline) {
            dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), outlineCol, 0.0f, 0, 1.0f);
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
            dl->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), outlineCol, 0.0f, 0, 1.0f);
        } else {
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
        }
    }
    void draw_corner(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline, ImU32 outlineCol)
    {
        if (!dl) return;
        x0 = std::floor(x0);
        y0 = std::floor(y0);
        x1 = std::floor(x1);
        y1 = std::floor(y1);
        float w = x1 - x0;
        float h = y1 - y0;
        if (w < 4.0f || h < 4.0f) return;
        float lineW = std::floor(w * 0.25f);
        float lineH = std::floor(h * 0.25f);
        if (lineW < 3.0f) lineW = 3.0f;
        if (lineH < 3.0f) lineH = 3.0f;
        if (lineW > w * 0.45f) lineW = std::floor(w * 0.45f);
        if (lineH > h * 0.45f) lineH = std::floor(h * 0.45f);
        if (outline) {
            dl->AddRectFilled(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x0 + lineW + 1.0f, y0 + 2.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x0 + 2.0f, y0 + lineH + 1.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x1 - lineW - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y0 + 2.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x1 - 2.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y0 + lineH + 1.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x0 - 1.0f, y1 - 2.0f), ImVec2(x0 + lineW + 1.0f, y1 + 1.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x0 - 1.0f, y1 - lineH - 1.0f), ImVec2(x0 + 2.0f, y1 + 1.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x1 - lineW - 1.0f, y1 - 2.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), outlineCol);
            dl->AddRectFilled(ImVec2(x1 - 2.0f, y1 - lineH - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), outlineCol);
        }
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + lineW, y0 + 1.0f), col);
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + 1.0f, y0 + lineH), col);
        dl->AddRectFilled(ImVec2(x1 - lineW, y0), ImVec2(x1, y0 + 1.0f), col);
        dl->AddRectFilled(ImVec2(x1 - 1.0f, y0), ImVec2(x1, y0 + lineH), col);
        dl->AddRectFilled(ImVec2(x0, y1 - 1.0f), ImVec2(x0 + lineW, y1), col);
        dl->AddRectFilled(ImVec2(x0, y1 - lineH), ImVec2(x0 + 1.0f, y1), col);
        dl->AddRectFilled(ImVec2(x1 - lineW, y1 - 1.0f), ImVec2(x1, y1), col);
        dl->AddRectFilled(ImVec2(x1 - 1.0f, y1 - lineH), ImVec2(x1, y1), col);
    }
    void draw_fill(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 fill)
    {
        if (!dl) return;
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), fill);
    }
    void draw_fill_gradient(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 top, ImU32 bottom)
    {
        if (!dl) return;
        dl->AddRectFilledMultiColor(ImVec2(x0, y0), ImVec2(x1, y1), top, top, bottom, bottom);
    }
    void draw_world(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline)
    {
        if (!dl) return;
        if (outline) {
            dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
            dl->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
        } else {
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
        }
    }
}
