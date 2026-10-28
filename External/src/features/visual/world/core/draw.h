#pragma once
#include "types.h"
#include "../../../../menu/imgui/imgui.h"
namespace thenwefuckin::world
{
    bool fetch_view(math::view_matrix& out);
    void draw_text(ImDrawList* dl, ImFont* font, float x, float y, ImU32 col, const char* text);
}
