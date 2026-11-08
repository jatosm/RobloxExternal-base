#pragma once
#include "../../../menu/imgui/imgui.h"
namespace thenwefuckin::visual::box
{
    void draw(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline = true, ImU32 outlineCol = IM_COL32(0,0,0,255));
    void draw_corner(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline = true, ImU32 outlineCol = IM_COL32(0,0,0,255));
    void draw_fill(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 fill);
    void draw_fill_gradient(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 top, ImU32 bottom);
    void draw_world(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col, bool outline = true);
}
