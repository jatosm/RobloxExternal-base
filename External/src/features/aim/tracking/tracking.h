#pragma once
#include <cstdint>
#include <deque>
#include <chrono>
#include <unordered_map>
namespace thenwefuckin::aim::tracking
{
    struct Sample
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        std::chrono::steady_clock::time_point t{};
    };
    struct Velocity
    {
        float vx = 0.0f;
        float vy = 0.0f;
        float vz = 0.0f;
        float damp = 1.0f;
        bool airborne = false;
    };
    void push(uintptr_t key, float x, float y, float z);
    bool sample(uintptr_t key, Velocity& out);
    void clear(uintptr_t key);
    void clear_all();
}
