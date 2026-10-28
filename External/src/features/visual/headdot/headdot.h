#pragma once
#include "../../../menu/imgui/imgui.h"
#include "../../../utility/cache/math.h"
namespace thenwefuckin::visual::headdot
{
    void draw(ImDrawList* dl, const math::vec3& head, const math::view_matrix& view, float screen_w, float screen_h, float distScale);
}
