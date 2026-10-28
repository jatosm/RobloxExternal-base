#include "frame.h"
#include "app.h"
#include "../menu/core/overlay/overlay.h"
#include "../menu/core/gui/menu.h"
#include "../menu/overlay/settings.h"
#include "../utility/cache/cache.h"
#include "../utility/logger/logger.h"
#include "../utility/struct/offsets.h"
#include "../utility/global/globals.h"
#include "../features/visual/core/visual.h"
#include "../features/visual/world/ore/ore.h"
#include "../features/visual/world/plants/plants.h"
#include "../features/visual/world/bots/bots.h"
#include "../features/visual/world/animals/animals.h"
#include "../features/visual/world/tools/tools.h"
#include "../features/visual/world/crates/crates.h"
#include "../features/visual/world/keycards/keycards.h"
#include "../features/visual/world/geo/geo.h"
#include "../features/aim/core/aim.h"
#include "../features/misc/core/misc.h"
#include "../features/lua/vm/lua_vm.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <string>
static void PrintDebugLines(float fps)
{
    using namespace thenwefuckin;
    bool conn = g_proc && g_proc->connected();
    uint32_t pid = conn ? g_proc->pid() : 0;
    uint64_t base = conn ? g_proc->base() : 0;
    uint64_t dm = g_ctx.datamodel ? g_ctx.datamodel.addr : 0;
    uint64_t ws = g_ctx.workspace ? g_ctx.workspace.addr : 0;
    uint64_t pl = g_ctx.players ? g_ctx.players.addr : 0;
    std::string plName = g_ctx.players ? g_ctx.players.name() : "none";
    std::string lpName = g_ctx.local_player ? g_ctx.local_player.name() : "none";
    size_t plCount = g_ctx.player_list().size();
    math::vec3 pos = g_ctx.local_pos;
    LOGF("debug", "FPS: %5.1f | PID: %u Base: 0x%llX | DM: 0x%llX WS: 0x%llX | Players(%s): %zu (0x%llX) | Local: %s | Pos: (%.1f, %.1f, %.1f) Speed: %.1f | Ready: %d",
        fps, pid, (unsigned long long)base, (unsigned long long)dm, (unsigned long long)ws,
        plName.c_str(), plCount, (unsigned long long)pl,
        lpName.c_str(), pos.x, pos.y, pos.z, g_localSpeed, g_ctx.ready ? 1 : 0);
}
static bool NeedsGameData()
{
    using namespace thenwefuckin;
    return settings::aim_enabled
        || settings::aim_autoshoot
        || settings::aim_show_fov
         || settings::box_enabled
        || settings::skeleton_enabled
        || settings::health_enabled
        || settings::name_enabled
        || settings::headdot_enabled
        || settings::distance_enabled
        || settings::tool_enabled
        || settings::flags_team
        || settings::flags_state
        || settings::flags_health
        || settings::tracer_enabled
        || settings::oof_enabled
         || settings::walkspeed_enabled
         || settings::jumppower_enabled
         || settings::gravity_enabled
         || settings::fov_enabled
         || settings::zoom_enabled
        || settings::noclip_enabled
         || settings::freecam_enabled
         || settings::hipheight_enabled
        || settings::infjump_enabled
        || settings::hitsound_enabled
          || settings::time_enabled
         || settings::night_enabled
         || settings::ambient_enabled
         || settings::world_fog_enabled
         || settings::world_exposure_enabled
         || settings::fullbright_enabled
         || settings::noshadow_enabled
         || settings::nograss_enabled
         || settings::nofall_enabled
        || settings::world_ore_enabled
        || settings::world_plant_enabled
        || settings::world_bots_enabled
        || settings::world_animals_enabled
        || settings::world_tools_enabled
        || settings::world_crates_enabled
        || settings::world_keycards_enabled;
}
static bool AnyGameFeatureActive()
{
    using namespace thenwefuckin;
    if (NeedsGameData())
        return true;
    return Menu::g_movement
            || Menu::g_explorerOpen
            || Menu::g_playerPreviewOpen;
}
void thenwefuckin::frame::tick(ImDrawList* dl, int width, int height, float fps, bool menuOpen)
{
    (void)menuOpen;
    app::g_firstFrame.store(true);
    static double lastAttachTry = -10.0;
    static double lastRefresh = -10.0;
    static double lastPfCache = -10.0;
    static const auto steadyEpoch = std::chrono::steady_clock::now();
    double now = std::chrono::duration<double>(std::chrono::steady_clock::now() - steadyEpoch).count();
    bool needGame = NeedsGameData();
    bool overlayActive = needGame || Menu::g_movement || Menu::g_explorerOpen || Menu::g_playerPreviewOpen || Menu::g_luaOpen;
    static LARGE_INTEGER qpf{};
    if (!qpf.QuadPart) QueryPerformanceFrequency(&qpf);
    auto qnow = []() { LARGE_INTEGER c; QueryPerformanceCounter(&c); return c; };
    auto qms = [&](LARGE_INTEGER s) { LARGE_INTEGER e; QueryPerformanceCounter(&e); return (e.QuadPart - s.QuadPart) * 1000.0 / (double)qpf.QuadPart; };
    auto qema = [](double prev, double v) { return prev * 0.92 + v * 0.08; };
    LARGE_INTEGER t0 = qnow();
    if ((!thenwefuckin::g_proc || !thenwefuckin::g_proc->connected()))
    {
        thenwefuckin::g_ctx.reset();
        if (now - lastAttachTry > 2.0)
        {
            lastAttachTry = now;
            if (thenwefuckin::g_proc->attach("RobloxPlayerBeta.exe"))
            {
                char ab[128];
                snprintf(ab, sizeof(ab), "attached to RobloxPlayerBeta.exe pid=%u base=0x%llX",
                    (unsigned)thenwefuckin::g_proc->pid(),
                    (unsigned long long)thenwefuckin::g_proc->base());
                LOG("ok", ab);
            }
            else
            {
                LOG("debug", "attach retry: RobloxPlayerBeta.exe not found, waiting...");
            }
        }
    }
    else
    {
        double refreshInterval = needGame ? 0.008 : 0.25;
        if (now - lastRefresh >= refreshInterval)
        {
            lastRefresh = now;
            thenwefuckin::g_ctx.refresh();
            if (needGame)
            {
                thenwefuckin::math::vec3 lv = thenwefuckin::g_ctx.local_velocity;
                bool ok = thenwefuckin::g_ctx.ready && thenwefuckin::g_ctx.local_root
                          && std::isfinite(lv.x) && std::isfinite(lv.y) && std::isfinite(lv.z);
                g_localSpeed = ok ? sqrtf(lv.x * lv.x + lv.z * lv.z) : -1.0f;
            }
        }
        if (!needGame)
        {
            g_localSpeed = -1.0f;
        }
    }
    Menu::g_dbgMsRefresh = qema(Menu::g_dbgMsRefresh, qms(t0));
    t0 = qnow();
    if (thenwefuckin::g_proc && thenwefuckin::g_proc->connected() && now - lastPfCache >= 0.016)
    {
        lastPfCache = now;
        PfCache::update();
    }
    Menu::g_dbgMsPf = qema(Menu::g_dbgMsPf, qms(t0));
    {
        using namespace thenwefuckin;
        static double lastSessionCheck = -10.0;
        static uint64_t s_place = 0;
        static uint64_t s_dm = 0;
        static int s_zeroCount = 0;
        if (now - lastSessionCheck >= 0.5)
        {
            lastSessionCheck = now;
            bool conn = g_proc && g_proc->connected();
            uint64_t dmAddr = 0;
            uint64_t place = 0;
            if (conn)
            {
                dmAddr = g_ctx.datamodel.addr;
                if (!dmAddr)
                {
                    uint64_t fake = g_proc->read<uint64_t>(g_proc->base() + Offsets::FakeDataModel::Pointer);
                    if (fake) dmAddr = g_proc->read<uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
                }
                if (dmAddr) place = g_proc->read<uint64_t>(dmAddr + Offsets::DataModel::PlaceId);
            }
            if (!conn)
            {
                if (s_place != 0)
                {
                    settings::push_state();
                    settings::disable_all();
                    thenwefuckin::visual::on_game_joined();
                    app::badge("left", app::C_YELLOW, "disconnected - all features disabled");
                    LOG("warn", "disconnected - all features disabled");
                    Logger::note_session("disconnected");
                }
                s_place = 0; s_dm = 0; s_zeroCount = 0;
            }
            else if (place != 0 && place != s_place)
            {
                if (s_place != 0)
                {
                    settings::push_state();
                    settings::disable_all();
                    char lb[96];
                    snprintf(lb, sizeof(lb), "left %llu - all features disabled", (unsigned long long)s_place);
                    app::badge("left", app::C_YELLOW, lb);
                    LOG("warn", lb);
                }
                s_place = place; s_dm = dmAddr; s_zeroCount = 0;
                char jb[96];
                snprintf(jb, sizeof(jb), "joined place id %llu (dm=0x%llX)", (unsigned long long)place, (unsigned long long)dmAddr);
                app::badge("game", app::C_GREEN, jb);
                LOG("game", jb);
                Logger::note_session(jb);
                thenwefuckin::visual::on_game_joined();
                if (settings::restore_state())
                {
                    app::badge("game", app::C_CYAN, "features restored");
                    LOG("ok", "features restored after join");
                }
                else
                {
                    LOG("info", "fresh join - no prior feature state to restore");
                }
            }
            else if (place != 0 && dmAddr != 0 && dmAddr != s_dm)
            {
                s_dm = dmAddr; s_zeroCount = 0;
                thenwefuckin::visual::on_game_joined();
                char jb[128];
                snprintf(jb, sizeof(jb), "rejoined place id %llu (new server dm=0x%llX)", (unsigned long long)place, (unsigned long long)dmAddr);
                app::badge("game", app::C_YELLOW, jb);
                LOG("warn", jb);
                Logger::note_session(jb);
            }
            else if (place == 0 && s_place != 0)
            {
                if (++s_zeroCount >= 2)
                {
                    settings::push_state();
                    settings::disable_all();
                    thenwefuckin::visual::on_game_joined();
                    char lb[96];
                    snprintf(lb, sizeof(lb), "left %llu - all features disabled", (unsigned long long)s_place);
                    app::badge("left", app::C_YELLOW, lb);
                    LOG("warn", lb);
                    Logger::note_session("left game");
                    s_place = 0; s_dm = 0; s_zeroCount = 0;
                }
                else
                {
                    LOG("debug", "place id unreadable - retrying before treating as leave...");
                }
            }
            else s_zeroCount = 0;
        }
    }
    bool anythingToDraw = g_menuOpen || Menu::g_watermark || Menu::g_keybinds
                          || Menu::g_movement || Menu::g_internalDebug || overlayActive;
    Overlay::SetRenderEnabled(anythingToDraw);
    Overlay::SetFrameInterval(0.0f);
    if (!dl)
    {
        if (needGame)
        {
            if (settings::aim_enabled || settings::aim_autoshoot)
                pcache::update();
            thenwefuckin::aim::run(width, height);
        }
        thenwefuckin::misc::run();
        return;
    }
    static double lastDebugBuild = -10.0;
    if (Menu::g_internalDebug && now - lastDebugBuild >= 1.0)
    {
        lastDebugBuild = now;
        PrintDebugLines(fps);
    }
    Logger::quiet = true;
    t0 = qnow();
    thenwefuckin::lua::run_actions();
    {
        thenwefuckin::lua::Snapshot snap;
        snap.fps = fps;
        snap.players = (int)thenwefuckin::g_ctx.player_list().size();
        thenwefuckin::lua::set_snapshot(snap);
    }
    if (thenwefuckin::lua::has_render() && thenwefuckin::g_ctx.ready)
    {
        if (!needGame)
            thenwefuckin::visual::wcache::update((float)width, (float)height);
        thenwefuckin::lua::snapshot_players((float)width, (float)height);
        thenwefuckin::lua::fire_render();
    }
    Menu::RenderWatermark(dl, width, fps);
    Menu::RenderKeybinds(dl, width, height);
    Menu::RenderMovement(dl, width, height);
    Menu::RenderInternalDebug(dl, width, height, fps);
    Menu::RenderNavBar(dl, width, height);
    Menu::Render();
    Menu::RenderSettingsPanel();
    Menu::RenderThemePanel();
    Menu::RenderExplorerPanel();
    Menu::RenderPlayerPreviewPanel();
    Menu::RenderPlayersPanel();
    Menu::RenderLuaPanel();
    Menu::g_dbgMsMenu = qema(Menu::g_dbgMsMenu, qms(t0));
    t0 = qnow();
    if (needGame)
    {
        if (settings::aim_enabled || settings::aim_autoshoot)
            pcache::update();
        thenwefuckin::aim::run(width, height);
        LARGE_INTEGER tv0 = qnow();
        thenwefuckin::visual::render(dl, width, height);
        Menu::g_dbgMsVisual = qema(Menu::g_dbgMsVisual, qms(tv0));
        thenwefuckin::ore::tick(dl, width, height);
        thenwefuckin::plants::tick(dl, width, height);
        thenwefuckin::bots::tick(dl, width, height);
        thenwefuckin::animals::tick(dl, width, height);
        thenwefuckin::tools::tick(dl, width, height);
        thenwefuckin::crates::tick(dl, width, height);
        thenwefuckin::keycards::tick(dl, width, height);
        thenwefuckin::worldgeo::tick(dl, width, height);
    }
    for (auto& c : thenwefuckin::lua::take_draws())
    {
        const ImU32 col = static_cast<ImU32>(c.color);
        if (c.kind == 0)
        {
            dl->AddRect(ImVec2(c.x0 - 1.0f, c.y0 - 1.0f), ImVec2(c.x1 + 1.0f, c.y1 + 1.0f), IM_COL32(0, 0, 0, 255));
            dl->AddRect(ImVec2(c.x0, c.y0), ImVec2(c.x1, c.y1), col);
        }
        else if (c.kind == 1)
        {
            dl->AddRectFilled(ImVec2(c.x0, c.y0), ImVec2(c.x1, c.y1), col);
        }
        else if (c.kind == 2)
        {
            if (!c.text.empty())
            {
                ImFont* df = ImGui::GetFont();
                dl->AddText(df, 13.0f, ImVec2(c.x0 + 1.0f, c.y0 + 1.0f), IM_COL32(0, 0, 0, 255), c.text.c_str());
                dl->AddText(df, 13.0f, ImVec2(c.x0, c.y0), col, c.text.c_str());
            }
        }
        else if (c.kind == 3)
        {
            dl->AddLine(ImVec2(c.x0, c.y0), ImVec2(c.x1, c.y1), col);
        }
    }
    Menu::g_dbgMsAim = qema(Menu::g_dbgMsAim, qms(t0));
    t0 = qnow();
    thenwefuckin::misc::run();
    Menu::g_dbgMsMisc = qema(Menu::g_dbgMsMisc, qms(t0));
}
