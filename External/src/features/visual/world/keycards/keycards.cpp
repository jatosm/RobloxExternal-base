#include "keycards.h"
#include "../core/scan.h"
#include "../core/draw.h"
#include "../../../../menu/core/gui/menu.h"
#include "../../../../menu/resources/font.h"
#include "../../../../utility/struct/world2screen.h"
#include <cstdio>
namespace thenwefuckin::keycards
{
    void clear()
    {
        world::clear_entries();
    }
    void tick(ImDrawList* dl, int w, int h)
    {
        bool want = settings::world_keycards_enabled;
        if (want && settings::world_keycards_key != 0)
            want = Menu::IsMenuKeyActive("##keycards_key", settings::world_keycards_key, settings::world_keycards_key_mode);
        if (!dl || !want)
            return;
        if (!g_proc || !g_proc->connected() || !g_ctx.workspace)
            return;
        world::refresh_entries();
        if (world::g_entries.empty())
            return;

        math::view_matrix view = thenwefuckin::visual::wcache::viewValid
            ? thenwefuckin::visual::wcache::view
            : math::view_matrix{};
        if (!thenwefuckin::visual::wcache::viewValid && !world::fetch_view(view))
            return;
        ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
        float fw = static_cast<float>(w);
        float fh = static_cast<float>(h);
        char buf[32];
        for (auto& e : world::g_entries)
        {
            if (!e.keycard)
                continue;
            if (e.idx < 0 || e.idx > 4 || !settings::world_keycards_sel[e.idx])
                continue;
            math::vec2 s{};
            if (!projection::to_screen(e.pos, view, fw, fh, s))
                continue;
            if (s.x < -12.0f || s.x > fw + 12.0f || s.y < -12.0f || s.y > fh + 12.0f)
                continue;
            float* col = settings::world_keycards_color[e.idx];
            ImU32 dc = ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 1.0f));
            bool n = settings::world_keycards_name[e.idx];
            bool d = settings::world_keycards_dist[e.idx];
            if (!n && !d)
                continue;
            float meters = e.dist * 0.28f;
            if (meters < 10.0f)
                snprintf(buf, sizeof(buf), "%.1fm", (double)meters);
            else
                snprintf(buf, sizeof(buf), "%.0fm", (double)meters);
            if (n && d)
            {
                ImVec2 tsD = font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, buf);
                ImVec2 tsN = font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, e.name.c_str());
                world::draw_text(dl, font, s.x - tsN.x * 0.5f, s.y - tsN.y - tsD.y - 3.0f, IM_COL32(255, 255, 255, 255), e.name.c_str());
                world::draw_text(dl, font, s.x - tsD.x * 0.5f, s.y - tsD.y - 1.0f, dc, buf);
            }
            else if (d)
            {
                ImVec2 tsD = font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, buf);
                world::draw_text(dl, font, s.x - tsD.x * 0.5f, s.y - tsD.y - 1.0f, dc, buf);
            }
            else
            {
                ImVec2 tsN = font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, e.name.c_str());
                world::draw_text(dl, font, s.x - tsN.x * 0.5f, s.y - tsN.y - 1.0f, IM_COL32(255, 255, 255, 255), e.name.c_str());
            }
        }
    }
}
