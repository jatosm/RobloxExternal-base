#pragma once
#include <atomic>
namespace thenwefuckin::misc::freecam
{
    void run();
    void thread_main(std::atomic<bool>& stop);
}
