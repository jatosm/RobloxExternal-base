#pragma once
#include <cstdint>
namespace thenwefuckin::visual::players::friends
{
    void set_friend(uintptr_t playerAddr, uintptr_t modelAddr, bool friended);
    bool is_friend(uintptr_t addr);
    void clear();
}
