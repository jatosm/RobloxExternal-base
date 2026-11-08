#pragma once
namespace thenwefuckin::aim::viewport
{
    void run(int w, int h);
    void clear();
    void shutdown();
    bool is_active();
    void on_game_joined();
}
