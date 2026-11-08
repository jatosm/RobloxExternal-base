#pragma once
#include <string>

struct ImDrawList;

namespace thenwefuckin::notifications
{
    inline constexpr const char* kDiscordInvite = "https://discord.gg/2h4rBWaRg3";

    void add(const std::string& text, float duration = 4.5f);
    void add_welcome();
    void render(ImDrawList* dl);
    bool active();
}
