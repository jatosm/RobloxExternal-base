#pragma once
#include "../../../../menu/imgui/imgui.h"
#include "../../../../utility/cache/math.h"
#include <cstdint>
#include <vector>
namespace thenwefuckin::worldgeo
{
    struct Box
    {
        math::vec3 c{};
        math::vec3 ax[3]{};
        float h[3]{};
        bool solid = true;
        float radius = 0.0f;
        std::uintptr_t prim = 0;
    };
    void tick(ImDrawList* dl, int w, int h);
    void clear();
    bool visible(const math::vec3& from, const math::vec3& to);
    const std::vector<Box>& boxes();
    bool terrain_found();
}
