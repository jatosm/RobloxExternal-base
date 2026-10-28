#pragma once
#include "../../../utility/cache/math.h"
#include <cstdint>
namespace thenwefuckin::aim::raycast
{
    void run(int w, int h);
    void idle();
    void shutdown();
    void on_game_joined();
    bool is_active();
    bool ready();
    std::uint64_t calls();
    const char* status();
}
