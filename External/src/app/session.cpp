#include "session.h"
#include "app.h"
#include "../utility/logger/logger.h"
#include "../utility/cache/cache.h"
#include "../utility/struct/offsets.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
namespace thenwefuckin::session
{
    void stage_watcher(std::atomic<bool>& stop) {
        int shown = 0;
        LOG("info", "stage watcher started - waiting for game process...");
        while (!stop.load()) {
            if (shown < 1 && thenwefuckin::g_proc && thenwefuckin::g_proc->connected()) {
                shown = 1;
                char buf[192];
                snprintf(buf, sizeof(buf), "pid=%u base=0x%llX client=%s",
                    (unsigned)thenwefuckin::g_proc->pid(),
                    (unsigned long long)thenwefuckin::g_proc->base(),
                    Offsets::ClientVersion.c_str());
                app::badge("attached", app::C_GREEN, buf);
                LOG("ok", buf);
            }
            if (shown < 2 && app::g_overlayReady.load()) {
                shown = 2;
                app::badge("overlay", app::C_GREEN, "DirectX ready");
                LOG("ok", "overlay DirectX ready");
            }
            if (shown < 3 && app::g_firstFrame.load()) {
                shown = 3;
                app::badge("ready", app::C_CYAN, "press INSERT for menu - have fun");
                LOG("ok", "ready - press INSERT for menu");
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        LOG("info", "stage watcher stopped");
    }
    void target_watcher(std::atomic<bool>& stop) {
        int ticks = 0;
        while (!stop.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (stop.load())
                return;
            if (++ticks < 5)
                continue;
            ticks = 0;
            if (thenwefuckin::g_proc && thenwefuckin::g_proc->connected() && !thenwefuckin::g_proc->alive()) {
                unsigned pid = (unsigned)thenwefuckin::g_proc->pid();
                thenwefuckin::g_proc->detach();
                char buf[160];
                snprintf(buf, sizeof(buf), "game closed (pid=%u) - handle parked, waiting for restart", pid);
                app::badge("left", app::C_YELLOW, buf);
                LOG("warn", buf);
                Logger::note_session("no game (closed)");
            }
        }
    }
}
