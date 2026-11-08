#include "name.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/resources/font.h"
#include <cmath>
namespace thenwefuckin::visual::name
{
    void draw(ImDrawList* dl, const data::rect& rc, const std::string& str)
    {
        if (!dl || str.empty()) return;
        ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
        float fs = 13.0f;
        ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, str.c_str());
        float tx = std::floor(rc.x0 + (rc.x1 - rc.x0) * 0.5f - ts.x * 0.5f);
        float ty = std::floor(rc.y0 - ts.y - 3.0f);
        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::name_color[0], thenwefuckin::settings::name_color[1], thenwefuckin::settings::name_color[2], 1.0f));
        if (thenwefuckin::settings::name_outline) {
            ImU32 oc = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::name_outline_color[0], thenwefuckin::settings::name_outline_color[1], thenwefuckin::settings::name_outline_color[2], 1.0f));
            static const ImVec2 offsets[8] = {
                ImVec2(-1.0f, -1.0f), ImVec2(0.0f, -1.0f), ImVec2(1.0f, -1.0f),
                ImVec2(-1.0f,  0.0f),                      ImVec2(1.0f,  0.0f),
                ImVec2(-1.0f,  1.0f), ImVec2(0.0f,  1.0f), ImVec2(1.0f,  1.0f)
            };
            for (int i = 0; i < 8; i++) {
                dl->AddText(font, fs, ImVec2(tx + offsets[i].x, ty + offsets[i].y), oc, str.c_str());
            }
        }
        dl->AddText(font, fs, ImVec2(tx, ty), col, str.c_str());
    }
}
