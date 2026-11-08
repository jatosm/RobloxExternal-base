#include "app.h"
#include "session.h"
#include "frame.h"
#include "../menu/core/overlay/overlay.h"
#include "../utility/global/globals.h"
#include "../utility/cache/cache.h"
#include "../utility/logger/logger.h"
#include "../features/misc/freecam/freecam.h"
#include "../features/aim/viewport/viewport.h"
#include "../features/aim/raycast/raycast.h"
#include "../features/aim/wraith/wraith.h"
#include "../features/lua/vm/lua_vm.h"
#include <Windows.h>
#include <atomic>
#include <iostream>
#include <thread>
namespace thenwefuckin::app
{
    std::atomic<bool> g_overlayReady{ false };
    std::atomic<bool> g_firstFrame{ false };
    void badge(const char* label, WORD labelColor, const char* msg) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO old{};
        GetConsoleScreenBufferInfo(h, &old);
        std::string ts = Logger::now_str();
        std::string up = Logger::upper_of(label ? label : "");
        SetConsoleTextAttribute(h, C_WHITE);
        std::cout << "[";
        SetConsoleTextAttribute(h, C_RED);
        std::cout << ts;
        SetConsoleTextAttribute(h, C_WHITE);
        std::cout << "]";
        SetConsoleTextAttribute(h, C_DIM);
        std::cout << " - ";
        SetConsoleTextAttribute(h, C_WHITE);
        std::cout << "[";
        SetConsoleTextAttribute(h, labelColor);
        std::cout << up;
        SetConsoleTextAttribute(h, C_WHITE);
        std::cout << "] ";
        SetConsoleTextAttribute(h, C_DIM);
        std::cout << msg << "\n";
        SetConsoleTextAttribute(h, old.wAttributes);
    }
    int run()
    {
        thenwefuckin::syscalls::init();
        if (thenwefuckin::syscalls::ready())
        {
            badge("syscall", C_GREEN, "direct syscalls active (rax.asm)");
            LOG("boot", "direct syscalls active");
        }
        else
        {
            badge("syscall", C_YELLOW, "direct syscalls unavailable, using RPM/WPM fallback");
            LOG("warn", "syscall fallback mode - using RPM/WPM");
        }
        LOG("info", "starting background watchers...");
        std::atomic<bool> stopWatcher{ false };
        std::thread watcher(session::stage_watcher, std::ref(stopWatcher));
        std::atomic<bool> stopTargetWatch{ false };
        std::thread targetWatch(session::target_watcher, std::ref(stopTargetWatch));
        LOG("info", "watcher threads started");
        if (thenwefuckin::lua::initialize())
            LOG("ok", "lua vm ready");
        else
            LOG("error", "lua vm failed to initialize - execute disabled");
        OverlayConfig cfg;
        cfg.windowTitle = L" ";
        cfg.targetWindow = L"";
        Overlay overlay(cfg);
        LOG("overlay", "initializing overlay...");
        if (!overlay.Initialize())
        {
            badge("fail", C_RED, "failed to initialize overlay - see thenwefuckin_log.txt");
            LOG("error", "overlay Initialize failed - see thenwefuckin_log.txt");
            MessageBoxW(nullptr, L"Failed to initialize overlay", L"Error", MB_OK);
            thenwefuckin::lua::shutdown();
            stopWatcher.store(true);
            if (watcher.joinable()) watcher.join();
            stopTargetWatch.store(true);
            if (targetWatch.joinable()) targetWatch.join();
            return 1;
        }
        g_overlayReady.store(true);
        LOG("ok", "overlay initialized (DirectX ready)");
        overlay.SetRenderCallback([](ImDrawList* dl, int width, int height, float fps, bool menuOpen) {
            frame::tick(dl, width, height, fps, menuOpen);
        });
        overlay.SetVSync(g_vsync);
        overlay.SetStreamProof(g_streamProof);
        Overlay::SetMenuOpen(true);
        LOG("info", "entering overlay loop - press INSERT for menu");
        std::atomic<bool> stopFreecam{ false };
        std::thread freecamThread(thenwefuckin::misc::freecam::thread_main, std::ref(stopFreecam));
        overlay.Run();
        LOG("info", "overlay loop exited - shutting down...");
        stopFreecam.store(true, std::memory_order_relaxed);
        if (freecamThread.joinable()) freecamThread.join();
        thenwefuckin::aim::viewport::shutdown();
        thenwefuckin::aim::raycast::shutdown();
        thenwefuckin::aim::wraith::shutdown();
        LOG("info", "stopping background watchers...");
        thenwefuckin::lua::shutdown();
        stopWatcher.store(true);
        if (watcher.joinable()) watcher.join();
        stopTargetWatch.store(true);
        if (targetWatch.joinable()) targetWatch.join();
        badge("done", C_GREEN, "session ended cleanly");
        LOG("ok", "session ended cleanly");
        return 0;
    }
}
