#pragma once
#include "../../../menu/imgui/imgui.h"
#include "../../../utility/struct/structs.h"
namespace thenwefuckin::visual::health
{
    void draw_bar(ImDrawList* dl, float bx0, float bx1, float by0, float by1, float frac, bool outline, ImU32 outlineCol, bool gradient, bool showText, ImU32 barCol = IM_COL32(0, 255, 0, 255));
    void draw(ImDrawList* dl, const data::rect& rc, float hp, float maxHp, bool outline, ImU32 outlineCol, bool gradient, bool showText, ImU32 barCol = IM_COL32(0, 255, 0, 255), bool boxOutline = false, float gap = 1.0f);
}
