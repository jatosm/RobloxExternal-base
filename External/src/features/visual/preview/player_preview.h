#pragma once
#include "../../../menu/imgui/imgui.h"
#include <cstdint>
namespace thenwefuckin::visual::preview
{
    void render(ImDrawList* dl, ImVec2 min, ImVec2 max);
    void render_for(int64_t uid, ImDrawList* dl, ImVec2 min, ImVec2 max);
    void refresh();
}
