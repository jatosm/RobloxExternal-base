#include "notifications.h"

#include "../../menu/core/gui/menu.h"

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

namespace thenwefuckin::notifications
{
    namespace
    {
        using clock = std::chrono::steady_clock;

        struct item_t
        {
            std::string text;
            clock::time_point born;
            float duration = 4.5f;
        };

        std::vector<item_t> g_items;
        std::mutex g_mutex;

        constexpr size_t kMaxVisible = 6;
        constexpr float kFadeIn = 0.18f;
        constexpr float kFadeOut = 0.40f;
        constexpr float kMargin = 16.0f;
        constexpr float kSpacing = 6.0f;
        constexpr float kPadX = 10.0f;
        constexpr float kPadY = 7.0f;
        constexpr float kBarW = 2.0f;

        float elapsed(const item_t& it, clock::time_point now)
        {
            return std::chrono::duration<float>(now - it.born).count();
        }

        ImU32 coloured(const ImVec4& c, float alpha)
        {
            return ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, std::clamp(alpha, 0.0f, 1.0f)));
        }
    }

    void add(const std::string& text, float duration)
    {
        if (text.empty())
            return;

        std::lock_guard<std::mutex> lock(g_mutex);
        g_items.push_back({ text, clock::now(), duration > 0.1f ? duration : 4.5f });
        if (g_items.size() > kMaxVisible)
            g_items.erase(g_items.begin());
    }

    void add_welcome()
    {
        static std::once_flag once;
        std::call_once(once, [] {
            add("Thanks for using thenwefuckin", 5.5f);
            add(std::string("join the discord - ") + kDiscordInvite, 7.0f);
        });
    }

    bool active()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        const auto now = clock::now();
        for (const auto& it : g_items)
        {
            if (elapsed(it, now) < it.duration)
                return true;
        }
        return false;
    }

    void render(ImDrawList* dl)
    {
        if (!dl)
            return;

        std::lock_guard<std::mutex> lock(g_mutex);
        const auto now = clock::now();
        g_items.erase(std::remove_if(g_items.begin(), g_items.end(),
            [&](const item_t& it) { return elapsed(it, now) >= it.duration; }),
            g_items.end());
        if (g_items.empty())
            return;

        const ImVec2 screen = ImGui::GetIO().DisplaySize;
        ImFont* font = ImGui::GetFont();
        const float font_size = ImGui::GetFontSize();

        float y = kMargin;
        for (size_t i = g_items.size(); i-- > 0;)
        {
            const item_t& it = g_items[i];
            const float t = elapsed(it, now);

            float alpha = 1.0f;
            if (t < kFadeIn)
                alpha = t / kFadeIn;
            else if (t > it.duration - kFadeOut)
                alpha = (it.duration - t) / kFadeOut;

            const float in = std::clamp(t / kFadeIn, 0.0f, 1.0f);
            const float ease = 1.0f - (1.0f - in) * (1.0f - in) * (1.0f - in);
            const ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, it.text.c_str());
            const float box_w = kBarW + kPadX * 2.0f + text_size.x;
            const float box_h = text_size.y + kPadY * 2.0f;
            const float slide = (1.0f - ease) * 28.0f;

            const ImVec2 box_min(std::floor(screen.x - box_w - kMargin + slide), std::floor(y));
            const ImVec2 box_max(std::floor(box_min.x + box_w), std::floor(box_min.y + box_h));

            dl->AddRectFilled(box_min, box_max, coloured(Menu::g_colFrame, alpha));
            dl->AddRect(ImVec2(box_min.x - 1.0f, box_min.y - 1.0f), ImVec2(box_max.x + 1.0f, box_max.y + 1.0f),
                coloured(Menu::g_colFrame, alpha));
            dl->AddRect(box_min, box_max, coloured(Menu::g_colBorder, alpha));
            dl->AddRectFilled(ImVec2(box_min.x + 1.0f, box_min.y + 1.0f),
                ImVec2(box_min.x + 1.0f + kBarW, box_max.y - 1.0f), coloured(Menu::g_accent1, alpha));
            dl->AddText(font, font_size, ImVec2(box_min.x + kBarW + kPadX, std::floor(box_min.y + kPadY)),
                coloured(Menu::g_colText, alpha), it.text.c_str());

            y += box_h + kSpacing;
        }
    }
}
