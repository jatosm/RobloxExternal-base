#pragma once
#include <atomic>
#include <Windows.h>
namespace thenwefuckin::app
{
    constexpr WORD C_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    constexpr WORD C_DIM = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr WORD C_GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD C_RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
    constexpr WORD C_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD C_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    void badge(const char* label, WORD labelColor, const char* msg);
    extern std::atomic<bool> g_overlayReady;
    extern std::atomic<bool> g_firstFrame;
    int run();
}
