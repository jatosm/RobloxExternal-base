#pragma once
#include "../../../menu/imgui/imgui.h"
#include "../../../utility/struct/structs.h"
#include <string>
namespace thenwefuckin::visual::name
{
    void draw(ImDrawList* dl, const data::rect& rc, const std::string& str);
}
