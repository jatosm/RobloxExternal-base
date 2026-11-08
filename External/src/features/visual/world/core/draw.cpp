#include "draw.h"
#include "../../../../utility/struct/offsets.h"
#include <cmath>
namespace thenwefuckin::world
{
    bool fetch_view(math::view_matrix& out)
    {
        if (!g_proc || !g_proc->connected())
            return false;
        uint64_t vis = g_proc->read<uint64_t>(g_proc->base() + Offsets::VisualEngine::Pointer);
        if (!vis)
            return false;
        out = g_proc->read<math::view_matrix>(vis + Offsets::VisualEngine::ViewMatrix);
        return true;
    }
    void draw_text(ImDrawList* dl, ImFont* font, float x, float y, ImU32 col, const char* text)
    {
        if (!dl || !text || !text[0])
            return;
        if (!font)
            font = ImGui::GetFont();
        float ix = std::floor(x);
        float iy = std::floor(y);
        dl->AddText(font, 13.0f, ImVec2(ix - 1.0f, iy - 1.0f), IM_COL32(0, 0, 0, 255), text);
        dl->AddText(font, 13.0f, ImVec2(ix + 1.0f, iy - 1.0f), IM_COL32(0, 0, 0, 255), text);
        dl->AddText(font, 13.0f, ImVec2(ix - 1.0f, iy + 1.0f), IM_COL32(0, 0, 0, 255), text);
        dl->AddText(font, 13.0f, ImVec2(ix + 1.0f, iy + 1.0f), IM_COL32(0, 0, 0, 255), text);
        dl->AddText(font, 13.0f, ImVec2(ix, iy), col, text);
    }
}
