#include "bots.h"
#include "../core/scan.h"
#include "../core/draw.h"
#include "../../health/health.h"
#include "../../../../menu/core/gui/menu.h"
#include "../../../../menu/resources/font.h"
#include "../../../../utility/struct/world2screen.h"
#include <algorithm>
#include <cstdio>
namespace thenwefuckin::bots
{
    void clear()
    {
        world::clear_entries();
    }
    void tick(ImDrawList* dl, int w, int h)
    {
        bool want = settings::world_bots_enabled;
        if (want && settings::world_bots_key != 0)
            want = Menu::IsMenuKeyActive("##bots_key", settings::world_bots_key, settings::world_bots_key_mode);
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
            if (!e.bots)
                continue;
            if (e.idx < 0 || e.idx > 3 || !settings::world_bots_sel[e.idx])
                continue;
            math::vec3 hp = (e.headPos.x != 0.0f || e.headPos.y != 0.0f || e.headPos.z != 0.0f) ? e.headPos : e.pos;
            math::vec3 rp = (e.rootPos.x != 0.0f || e.rootPos.y != 0.0f || e.rootPos.z != 0.0f) ? e.rootPos : e.pos;
            math::vec3 top3{ hp.x, hp.y + 0.5f, hp.z };
            math::vec3 bot3{ rp.x, rp.y - 2.5f, rp.z };
            math::vec2 top{};
            math::vec2 bot{};
            bool a = projection::to_screen(top3, view, fw, fh, top);
            bool b = projection::to_screen(bot3, view, fw, fh, bot);

            if (!a || !b)
                continue;
            float hh = fabsf(bot.y - top.y) + 10.0f;
            if (hh < 4.0f)
                hh = 4.0f;
            float ww = hh * 0.72f;
            if (ww < 3.0f)
                ww = 3.0f;
            float midX = (top.x + bot.x) * 0.5f;
            float minY = (std::min)(top.y, bot.y) - 5.0f;
            float x0 = std::floor(midX - ww * 0.5f);
            float x1 = std::floor(midX + ww * 0.5f);
            float y0 = std::floor(minY);
            float y1 = std::floor(minY + hh);

            if (midX < -12.0f || midX > fw + 12.0f || (y0 + y1) * 0.5f < -12.0f || (y0 + y1) * 0.5f > fh + 12.0f)
                continue;

            bool oversize = ((x1 - x0) > fw * 0.85f || (y1 - y0) > fh * 0.85f);
            float* bc = settings::world_bots_color[e.idx];
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(bc[0], bc[1], bc[2], 1.0f));
            bool showBox = settings::world_bots_box[e.idx] && !oversize;
            bool showName = settings::world_bots_name[e.idx];
            bool showDist = settings::world_bots_dist[e.idx];
            if (showBox)
            {
                dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), IM_COL32(0, 0, 0, 255));
                dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col);
                dl->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), IM_COL32(0, 0, 0, 255));
            }
            float meters = e.dist * 0.28f;
            if (meters < 10.0f)
                snprintf(buf, sizeof(buf), "%.1fm", (double)meters);
            else
                snprintf(buf, sizeof(buf), "%.0fm", (double)meters);
            if (showName)
            {
                ImVec2 tsN = font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, e.name.c_str());
                world::draw_text(dl, font, midX - tsN.x * 0.5f, y0 - tsN.y - 1.0f, IM_COL32(255, 255, 255, 255), e.name.c_str());
            }
            if (showDist)
            {
                ImVec2 tsD = font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, buf);
                world::draw_text(dl, font, midX - tsD.x * 0.5f, y1 + 1.0f, col, buf);
            }
        }
    }
}
