#pragma once
#include "../../../menu/imgui/imgui.h"
namespace thenwefuckin::visual
{
    void render(ImDrawList* dl, int w, int h, bool uiOpen = false);
    void on_game_joined();
}
