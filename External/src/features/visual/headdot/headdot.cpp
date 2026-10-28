#include "headdot.h"
#include "../../../utility/struct/world2screen.h"
#include "../../../menu/overlay/settings.h"
namespace thenwefuckin::visual::headdot
{
    void draw(ImDrawList* dl, const math::vec3& head, const math::view_matrix& view, float screen_w, float screen_h, float distScale)
    {
        if (!dl) return;
        if (head.x == 0 && head.y == 0 && head.z == 0) return;
        if (!settings::headdot_enabled) return;
        math::vec2 hs;
        if (!projection::to_screen(head, view, screen_w, screen_h, hs)) return;
        const float r = settings::headdot_thickness * distScale;
        if (settings::headdot_fill) {
            ImU32 hcol = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::headdot_color[0], settings::headdot_color[1], settings::headdot_color[2], 1.0f));
            ImU32 hoc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::headdot_outline_color[0], settings::headdot_outline_color[1], settings::headdot_outline_color[2], 1.0f));
            if (settings::headdot_outline) dl->AddCircleFilled(ImVec2(hs.x, hs.y), r + 1.0f*distScale, hoc);
            dl->AddCircleFilled(ImVec2(hs.x, hs.y), r, hcol);
        } else if (settings::headdot_outline) {
            ImU32 hoc = ImGui::ColorConvertFloat4ToU32(ImVec4(settings::headdot_outline_color[0], settings::headdot_outline_color[1], settings::headdot_outline_color[2], 1.0f));
            dl->AddCircle(ImVec2(hs.x, hs.y), r, hoc, 0, 1.5f*distScale);
        }
    }
}
