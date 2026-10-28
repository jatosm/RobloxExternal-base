#pragma once
#include <cstdint>
namespace thenwefuckin::misc::spectate
{

    void toggle(uintptr_t playerAddr);
    void stop();
    bool active();
    bool is_spectating(uintptr_t playerAddr);
    uintptr_t target();

    bool teleport_to(uintptr_t playerAddr);
    void run();
}
