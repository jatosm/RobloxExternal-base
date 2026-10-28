#pragma once
#include "../../../menu/imgui/imgui.h"
#include "../../../utility/cache/math.h"
namespace thenwefuckin::visual::oof
{
    void draw(ImDrawList* dl, const math::vec3& worldPos, const math::view_matrix& view, int screenW, int screenH, ImU32 col, bool outline, ImU32 outlineCol, float size, float radius);
}
