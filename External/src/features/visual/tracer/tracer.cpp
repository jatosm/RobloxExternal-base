#include "tracer.h"
namespace thenwefuckin::visual::tracer
{
    void draw(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline, ImU32 outlineCol, float thickness, int screenW, int screenH, int origin)
    {
        if (!dl) return;
        float targetX = (x0 + x1) * 0.5f;
        float targetY = y1;
        float startX = screenW * 0.5f;
        float startY = (float)screenH;
        if (origin == 1) startY = 0;
        else if (origin == 2) startY = screenH * 0.5f;
        if (outline) {
            dl->AddLine(ImVec2(startX, startY), ImVec2(targetX, targetY), outlineCol, thickness + 1.5f);
        }
        dl->AddLine(ImVec2(startX, startY), ImVec2(targetX, targetY), col, thickness);
    }
}
