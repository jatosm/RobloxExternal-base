#pragma once
#include "../../../utility/cache/math.h"
#include "../../../utility/cache/cache.h"
#include <cstdint>
#include <vector>
namespace thenwefuckin::aim
{
    void run(int w, int h);
    node resolve_part(const node& character, int partIdx);
    void set_debug_target(const math::vec3& world, uintptr_t playerAddr);
    void clear_debug_target();
    bool debug_target(math::vec3& out);
    bool debug_target_player(uintptr_t& out);
    bool debug_recent_player(uintptr_t addr);
    bool debug_in_use();
    uintptr_t local_team_addr();
    bool should_skip_teammate(uintptr_t localTeam, uintptr_t targetTeam);
    bool is_teammate(uintptr_t playerAddr, uintptr_t characterAddr, uintptr_t teamAddr);
    void on_game_joined();

    struct BotAimTarget
    {
        uintptr_t inst = 0;
        uintptr_t humanoid = 0;
        uintptr_t headPrim = 0;
        uintptr_t rootPrim = 0;
        float hp = 100.0f;
    };
    void collect_bot_targets(std::vector<BotAimTarget>& out);
}
