#pragma once
#include "types.h"
#include <string>
namespace thenwefuckin::world
{
    bool match_node(const std::string& n, bool& plant, bool& bots, bool& animal, bool& tool, bool& crate, bool& keycard, int& idx);
    math::vec3 keycard_pos(node inst);
    math::vec3 instance_pos(node inst);
    math::vec3 local_anchor();
    math::vec3 part_pos(std::uintptr_t part);
    void fast_refresh();
    void full_scan();
    void refresh_entries();
    void clear_entries();
}
