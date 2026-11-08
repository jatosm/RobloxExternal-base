#include "performance.h"

#include "../../menu/core/gui/menu.h"

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <chrono>
#include <cmath>
#include <cstdio>

#pragma comment(lib, "psapi.lib")

namespace thenwefuckin::performance
{
    namespace
    {
        using clock = std::chrono::steady_clock;

        constexpr int kHistory = 32;
        constexpr float kPanelW = 380.0f;
        constexpr float kHeaderH = 20.0f;
        constexpr float kRowH = 16.0f;
        constexpr float kPad = 6.0f;
        constexpr float kFontSize = 12.0f;

        struct stage_t
        {
            const char* name;
            const double* ms;
            float history[kHistory] = {};
        };

        stage_t g_stages[] = {
            { "refresh", &Menu::g_dbgMsRefresh },
            { "player cache", &Menu::g_dbgMsPf },
            { "menu", &Menu::g_dbgMsMenu },
            { "aim", &Menu::g_dbgMsAim },
            { "visuals", &Menu::g_dbgMsVisual },
            { "misc", &Menu::g_dbgMsMisc },
        };
        constexpr int kStageCount = (int)(sizeof(g_stages) / sizeof(g_stages[0]));

        bool g_open = false;
        bool g_dragging = false;
        ImVec2 g_pos(-1.0f, -1.0f);
        ImVec2 g_size(0.0f, 0.0f);
        ImVec2 g_drag_off(0.0f, 0.0f);

        clock::time_point g_last_sample{};
        float g_stats_acc = 0.0f;

        float g_fps = 0.0f;
        float g_frame_ms = 0.0f;
        float g_cpu_pct = 0.0f;
        float g_ram_mb = 0.0f;
        float g_priv_mb = 0.0f;
        int g_threads = 0;

        float g_frame_hist[kHistory] = {};
        int g_head = 0;

        ULONGLONG g_prev_proc_kernel = 0;
        ULONGLONG g_prev_proc_user = 0;
        ULONGLONG g_prev_sys = 0;

        ULONGLONG to_u64(const FILETIME& ft)
        {
            return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
        }

        int count_threads()
        {
            int n = 0;
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (snap == INVALID_HANDLE_VALUE)
                return n;

            THREADENTRY32 te{};
            te.dwSize = sizeof(te);
            const DWORD pid = GetCurrentProcessId();
            if (Thread32First(snap, &te))
            {
                do
                {
                    if (te.th32OwnerProcessID == pid)
                        ++n;
                } while (Thread32Next(snap, &te));
            }
            CloseHandle(snap);
            return n;
        }

        void sample_stats()
        {
            PROCESS_MEMORY_COUNTERS_EX pmc{};
            if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
            {
                g_ram_mb = (float)pmc.WorkingSetSize / (1024.0f * 1024.0f);
                g_priv_mb = (float)pmc.PrivateUsage / (1024.0f * 1024.0f);
            }

            FILETIME creation, exit_time, kernel, user;
            FILETIME idle, sys_kernel, sys_user;
            if (GetProcessTimes(GetCurrentProcess(), &creation, &exit_time, &kernel, &user) &&
                GetSystemTimes(&idle, &sys_kernel, &sys_user))
            {
                const ULONGLONG proc_k = to_u64(kernel);
                const ULONGLONG proc_u = to_u64(user);
                const ULONGLONG sys = to_u64(sys_kernel) + to_u64(sys_user);

                if (g_prev_sys != 0 && sys > g_prev_sys)
                {
                    const ULONGLONG proc_diff = (proc_k - g_prev_proc_kernel) + (proc_u - g_prev_proc_user);
                    const ULONGLONG sys_diff = sys - g_prev_sys;
                    SYSTEM_INFO si{};
                    GetSystemInfo(&si);
                    g_cpu_pct = (float)proc_diff / (float)sys_diff * 100.0f * (float)si.dwNumberOfProcessors;
                    if (g_cpu_pct < 0.0f)
                        g_cpu_pct = 0.0f;
                }

                g_prev_proc_kernel = proc_k;
                g_prev_proc_user = proc_u;
                g_prev_sys = sys;
            }

            g_threads = count_threads();
        }
    }

    void update(float fps)
    {
        g_fps = fps;
        g_frame_ms = (fps > 0.0f) ? 1000.0f / fps : 0.0f;

        const auto now = clock::now();
        if (g_last_sample.time_since_epoch().count() == 0)
        {
            g_last_sample = now;
            return;
        }

        const float dt = std::chrono::duration<float>(now - g_last_sample).count();
        if (dt < 0.1f)
            return;

        g_last_sample = now;

        for (int i = 0; i < kStageCount; ++i)
            g_stages[i].history[g_head] = (float)*g_stages[i].ms;
        g_frame_hist[g_head] = g_frame_ms;
        g_head = (g_head + 1) % kHistory;

        g_stats_acc += dt;
        if (g_stats_acc >= 0.25f)
        {
            g_stats_acc = 0.0f;
            sample_stats();
        }
    }

    bool open()
    {
        return g_open;
    }

    void toggle()
    {
        g_open = !g_open;
        if (g_open && g_pos.x < 0.0f)
            g_pos = ImVec2(20.0f, 20.0f);
        if (!g_open)
            g_dragging = false;
    }

    void close()
    {
        g_open = false;
        g_dragging = false;
    }

    bool hit_test(int x, int y)
    {
        if (!g_open || g_pos.x < 0.0f)
            return false;
        return x >= (int)g_pos.x && x <= (int)(g_pos.x + g_size.x)
            && y >= (int)(g_pos.y - 16.0f) && y <= (int)(g_pos.y + g_size.y + 16.0f);
    }

    void render(ImDrawList* dl)
    {
        if (!dl || !g_open)
            return;

        const float graph_h = 42.0f;
        const float head_row_h = 14.0f;
        const float body_h = kPad * 2.0f + kRowH * 4.0f + 4.0f + graph_h + 8.0f
            + head_row_h + kRowH * (float)kStageCount;

        g_size = ImVec2(kPanelW, kHeaderH + 2.0f + body_h);

        if (g_pos.x < 0.0f)
            g_pos = ImVec2(20.0f, 20.0f);

        ImGuiIO& io = ImGui::GetIO();
        const ImVec2 m = io.MousePos;
        const ImU32 outline = IM_COL32(0, 0, 0, 255);
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();

        const ImVec2 h_min = g_pos;
        const ImVec2 h_max(g_pos.x + kPanelW, g_pos.y + kHeaderH);

        const bool h_hov = g_menuOpen && !Menu::IsModalActive()
            && m.x >= h_min.x && m.x <= h_max.x && m.y >= h_min.y && m.y <= h_max.y;
        if (!g_dragging && h_hov && ImGui::IsMouseClicked(0))
        {
            g_dragging = true;
            g_drag_off = ImVec2(m.x - g_pos.x, m.y - g_pos.y);
        }
        if (g_dragging)
        {
            if (ImGui::IsMouseDown(0))
                g_pos = ImVec2(m.x - g_drag_off.x, m.y - g_drag_off.y);
            else
                g_dragging = false;
        }

        Menu::DrawPixelBox(dl, h_min, h_max, Menu::CtrlBgU32(), Menu::BorderU32(), outline);
        {
            const char* title = "performance";
            const ImVec2 ts = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, title);
            Menu::DrawOutlinedText(dl, f, 13.0f,
                ImVec2(floorf(h_min.x + (kPanelW - ts.x) * 0.5f), floorf(h_min.y + (kHeaderH - ts.y) * 0.5f)),
                Menu::TextU32(), outline, title);
        }

        const ImVec2 b_min(g_pos.x, h_max.y + 2.0f);
        const ImVec2 b_max(b_min.x + kPanelW, b_min.y + body_h);
        Menu::DrawPixelBox(dl, b_min, b_max, Menu::CtrlBgU32(), Menu::BorderU32(), outline);

        auto right_text = [&](float rx, float ry, ImU32 col, const char* txt) {
            const ImVec2 sz = f->CalcTextSizeA(kFontSize, FLT_MAX, 0.0f, txt);
            Menu::DrawOutlinedText(dl, f, kFontSize, ImVec2(floorf(rx - sz.x), ry), col, outline, txt);
        };

        char vals[4][96];
        snprintf(vals[0], sizeof(vals[0]), "%.1f", (double)g_fps);
        snprintf(vals[1], sizeof(vals[1]), "%.2f ms", (double)g_frame_ms);
        snprintf(vals[2], sizeof(vals[2]), "%.1f%%  |  %d threads", (double)g_cpu_pct, g_threads);
        snprintf(vals[3], sizeof(vals[3]), "%.1f MB  |  priv %.1f MB", (double)g_ram_mb, (double)g_priv_mb);
        const char* labels[4] = { "fps", "frametime", "cpu", "memory" };

        float y = b_min.y + kPad;
        for (int i = 0; i < 4; ++i)
        {
            const float ty = floorf(y + (kRowH - kFontSize) * 0.5f);
            Menu::DrawOutlinedText(dl, f, kFontSize, ImVec2(floorf(b_min.x + 8.0f), ty),
                Menu::TextDimU32(), outline, labels[i]);
            right_text(b_max.x - 8.0f, ty, i == 0 ? Menu::AccentU32() : Menu::TextU32(), vals[i]);
            y += kRowH;
        }

        const ImVec2 g_min(b_min.x + 8.0f, y + 4.0f);
        const ImVec2 g_max(b_max.x - 8.0f, g_min.y + graph_h);
        dl->AddRectFilled(g_min, g_max, Menu::GroupU32());
        dl->AddRect(g_min, g_max, Menu::BorderU32());

        float frame_peak = (g_frame_ms > 0.0f) ? g_frame_ms : 0.001f;
        for (int i = 0; i < kHistory; ++i)
        {
            if (g_frame_hist[i] > frame_peak)
                frame_peak = g_frame_hist[i];
        }
        {
            const float step = (g_max.x - g_min.x) / (float)(kHistory - 1);
            ImVec2 prev{};
            for (int i = 0; i < kHistory; ++i)
            {
                const int idx = (g_head + i) % kHistory;
                const float norm = g_frame_hist[idx] / frame_peak;
                const ImVec2 pt(g_min.x + i * step, g_max.y - 3.0f - norm * (graph_h - 6.0f));
                if (i > 0)
                    dl->AddLine(prev, pt, Menu::AccentU32(), 1.4f);
                prev = pt;
            }
        }
        {
            char cap[64];
            snprintf(cap, sizeof(cap), "frame time   peak %.2f ms", (double)frame_peak);
            Menu::DrawOutlinedText(dl, f, 11.0f, ImVec2(floorf(g_min.x + 4.0f), floorf(g_min.y + 2.0f)),
                Menu::TextDimU32(), outline, cap);
        }

        y = g_max.y + 8.0f;
        const float x_stage = b_min.x + 8.0f;
        const float x_ms = b_min.x + kPanelW * 0.46f;
        const float x_pct = b_min.x + kPanelW * 0.60f;
        const float x_graph = b_min.x + kPanelW * 0.66f;
        const float x_graph_end = b_max.x - 8.0f;

        {
            const float hy = floorf(y + (head_row_h - 11.0f) * 0.5f);
            Menu::DrawOutlinedText(dl, f, 11.0f, ImVec2(floorf(x_stage), hy), Menu::TextDimU32(), outline, "stage");
            right_text(x_ms, hy, Menu::TextDimU32(), "ms");
            right_text(x_pct, hy, Menu::TextDimU32(), "%");
            Menu::DrawOutlinedText(dl, f, 11.0f, ImVec2(floorf(x_graph), hy), Menu::TextDimU32(), outline, "graph");
            y += head_row_h;
        }

        for (int i = 0; i < kStageCount; ++i)
        {
            stage_t& st = g_stages[i];
            const float ms = (float)*st.ms;
            const float share = (g_frame_ms > 0.001f) ? (ms / g_frame_ms) * 100.0f : 0.0f;

            ImU32 col = Menu::TextU32();
            if (ms >= 10.0f)
                col = IM_COL32(240, 70, 70, 255);
            else if (ms >= 4.0f)
                col = IM_COL32(230, 190, 70, 255);

            const float ry = floorf(y + (kRowH - kFontSize) * 0.5f);
            Menu::DrawOutlinedText(dl, f, kFontSize, ImVec2(floorf(x_stage), ry), col, outline, st.name);

            char b[32];
            snprintf(b, sizeof(b), "%.2f", (double)ms);
            right_text(x_ms, ry, col, b);
            snprintf(b, sizeof(b), "%.0f%%", (double)share);
            right_text(x_pct, ry, Menu::TextDimU32(), b);

            float peak = 0.001f;
            for (int k = 0; k < kHistory; ++k)
            {
                if (st.history[k] > peak)
                    peak = st.history[k];
            }
            const float step = (x_graph_end - x_graph) / (float)(kHistory - 1);
            ImVec2 prev{};
            for (int k = 0; k < kHistory; ++k)
            {
                const int idx = (g_head + k) % kHistory;
                const float norm = st.history[idx] / peak;
                const ImVec2 pt(x_graph + k * step, y + kRowH - 3.0f - norm * (kRowH - 6.0f));
                if (k > 0)
                    dl->AddLine(prev, pt, Menu::AccentU32(), 1.0f);
                prev = pt;
            }

            y += kRowH;
        }
    }
}
