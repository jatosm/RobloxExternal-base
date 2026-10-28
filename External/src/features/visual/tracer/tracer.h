#pragma once
#include "../../../menu/imgui/imgui.h"
namespace thenwefuckin::visual::tracer
{
    void draw(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline, ImU32 outlineCol, float thickness, int screenW, int screenH, int origin = 0);
}
