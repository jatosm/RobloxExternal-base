#pragma once
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shlobj.h>
#include <mmsystem.h>
#include "../../imgui/imgui.h"
#include "../../../utility/global/globals.h"
#include "../../../utility/logger/logger.h"
#include "../overlay/overlay.h"
#include "resources/icons.h"
#include "../../../menu/resources/font.h"
#include "../../../menu/overlay/settings.h"
#include "../../../features/visual/preview/player_preview.h"
#include "../../../features/misc/hitsound/hitsound.h"
#include "../../../features/misc/explorer/explorer.h"
#include "./mascot.h"
#include "../../../features/aim/raycast/raycast.h"
#include "../../../features/aim/predict/predict.h"
#include "../../../features/visual/players/avatars.h"
#include "../../../features/visual/players/friends.h"
#include "../../../features/misc/spectate/spectate.h"
#include "../../../features/lua/lua_editor.h"
#include "../../../features/lua/vm/lua_vm.h"
#include "../../../utility/cache/cache.h"
namespace S = thenwefuckin::settings;
namespace Menu
{
inline int g_activeTab = 0;
inline bool g_navBarHov = false;
inline bool g_navHidden = false;
inline bool g_settingsOpen = false;
inline float g_setAnim = 0.0f;
inline ImVec2 g_setPos = ImVec2(-1.0f, -1.0f);
inline ImVec2 g_setSize = ImVec2(340.0f, 366.0f);
inline bool g_setDragging = false;
inline ImVec2 g_setDragOff = ImVec2(0.0f, 0.0f);
inline bool g_themeOpen = false;
inline float g_themeAnim = 0.0f;
inline ImVec2 g_themePos = ImVec2(-1.0f, -1.0f);
inline ImVec2 g_themeSize = ImVec2(340.0f, 280.0f);
inline bool g_themeDragging = false;
inline ImVec2 g_themeDragOff = ImVec2(0.0f, 0.0f);
inline bool g_explorerOpen = false;
inline ImVec2 g_explorerPos = ImVec2(-1.0f, -1.0f);
inline ImVec2 g_explorerSize = ImVec2(680.0f, 560.0f);
inline bool g_explorerDragging = false;
inline ImVec2 g_explorerDragOff = ImVec2(0.0f, 0.0f);
inline bool g_playerPreviewOpen = false;
inline ImVec2 g_playerPreviewPos = ImVec2(-1.0f, -1.0f);
inline ImVec2 g_playerPreviewSize = ImVec2(280.0f, 380.0f);
inline bool g_playerPreviewDragging = false;
inline ImVec2 g_playerPreviewDragOff = ImVec2(0.0f, 0.0f);
inline bool g_playersOpen = false;
inline ImVec2 g_playersPos = ImVec2(-1.0f, -1.0f);
inline ImVec2 g_playersSize = ImVec2(400.0f, 640.0f);
inline bool g_playersDragging = false;
inline ImVec2 g_playersDragOff = ImVec2(0.0f, 0.0f);
inline bool g_luaOpen = false;
inline ImVec2 g_luaPos = ImVec2(-1.0f, -1.0f);
inline ImVec2 g_luaSize = ImVec2(560.0f, 520.0f);
inline bool g_luaDragging = false;
inline ImVec2 g_luaDragOff = ImVec2(0.0f, 0.0f);
inline bool g_previewSpin = false;
inline bool g_previewAnimateHp = true;
inline ImVec2 g_pos = ImVec2(-1.0f, -1.0f);
inline float g_menuAnim = 0.0f;
inline bool g_dragging = false;
inline ImVec2 g_dragOff = ImVec2(0.0f, 0.0f);
inline ImVec2 g_menuSize = ImVec2(540.0f, 740.0f);
inline bool g_checkbox1 = false;
inline bool g_checkbox2 = false;
inline bool g_checkbox3 = false;
inline bool g_checkbox4 = false;
inline bool g_checkbox5 = false;
inline bool g_subOption1 = false;
inline bool g_watermark = true;
inline bool g_watermarkFps = true;
inline bool g_watermarkPlayers = true;
inline bool g_watermarkDate = false;
inline bool g_keybinds = false;
inline ImVec2 g_bindsPos = ImVec2(20.0f, 260.0f);
inline ImVec2 g_bindsSize = ImVec2(160.0f, 20.0f);
inline bool g_bindsDragging = false;
inline ImVec2 g_bindsDragOff = ImVec2(0.0f, 0.0f);
inline bool g_internalDebug = false;
inline double g_dbgMsRefresh = 0.0;
inline double g_dbgMsPf = 0.0;
inline double g_dbgMsMenu = 0.0;
inline double g_dbgMsAim = 0.0;
inline double g_dbgMsVisual = 0.0;
inline double g_dbgMsMisc = 0.0;
inline ImVec2 g_debugPos = ImVec2(20.0f, 520.0f);
inline ImVec2 g_debugSize = ImVec2(270.0f, 180.0f);
inline bool g_debugDragging = false;
inline ImVec2 g_debugDragOff = ImVec2(0.0f, 0.0f);
inline bool g_movement = false;
inline bool g_movementVelocityOpt = true;
inline bool g_movementGraphOpt = true;
inline bool g_movementKeysOpt = true;
inline float g_movementVelocity = -1.0f;
inline ImVec2 g_movementPos = ImVec2(20.0f, 320.0f);
inline ImVec2 g_movementSize = ImVec2(190.0f, 190.0f);
inline bool g_movementDragging = false;
inline ImVec2 g_movementDragOff = ImVec2(0.0f, 0.0f);
inline int g_keybind1_key = 0;
inline int g_keybind1_mode = 0;
inline int g_slider1 = 50;
inline int g_sliderMin = 20;
inline int g_sliderMax = 80;
inline int g_dropdown1 = (1 << 0);
inline char g_textinput1[64] = "";
inline char g_configName[64] = "default";
inline ImVec4 g_color1 = ImVec4(112.0f / 255.0f, 96.0f / 255.0f, 128.0f / 255.0f, 1.0f);
inline ImVec4 g_colorTeam = ImVec4(110.0f / 255.0f, 220.0f / 255.0f, 120.0f / 255.0f, 1.0f);
inline ImVec4 g_colorEnemy = ImVec4(240.0f / 255.0f, 110.0f / 255.0f, 120.0f / 255.0f, 1.0f);
inline int g_themeMask = (1 << 6);
inline ImVec4 g_accent1 = ImVec4(112.0f / 255.0f, 96.0f / 255.0f, 128.0f / 255.0f, 1.0f);
inline ImVec4 g_colFrame = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 26.0f / 255.0f, 1.0f);
inline ImVec4 g_colGroup = ImVec4(32.0f / 255.0f, 32.0f / 255.0f, 37.0f / 255.0f, 1.0f);
inline ImVec4 g_colCtrl = ImVec4(16.0f / 255.0f, 16.0f / 255.0f, 20.0f / 255.0f, 1.0f);
inline ImVec4 g_colCtrlHov = ImVec4(28.0f / 255.0f, 28.0f / 255.0f, 34.0f / 255.0f, 1.0f);
inline ImVec4 g_colBorder = ImVec4(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 1.0f);
inline ImVec4 g_colText = ImVec4(235.0f / 255.0f, 235.0f / 255.0f, 238.0f / 255.0f, 1.0f);
inline ImVec4 g_colTextDim = ImVec4(150.0f / 255.0f, 150.0f / 255.0f, 158.0f / 255.0f, 1.0f);
inline const char* g_activeBindId = nullptr;
inline int g_bindStartFrame = 0;
inline bool g_inQuestionPopup = false;
inline bool g_modalAtFrameStart = false;
inline bool IsModalActive()
{
    if (g_inQuestionPopup) return false;
    if (g_modalAtFrameStart) return true;
    if (g_activeBindId != nullptr) return true;
    return ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
}
inline ImU32 AccentU32() { return ImGui::ColorConvertFloat4ToU32(g_accent1); }
inline ImU32 AccentDimU32()
{
    int r = (int)(g_accent1.x * 178.0f);
    int g = (int)(g_accent1.y * 178.0f);
    int b = (int)(g_accent1.z * 178.0f);
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    return IM_COL32(r, g, b, 255);
}
inline ImU32 FrameU32() { return ImGui::ColorConvertFloat4ToU32(g_colFrame); }
inline ImU32 GroupU32() { return ImGui::ColorConvertFloat4ToU32(g_colGroup); }
inline ImU32 CtrlBgU32() { return ImGui::ColorConvertFloat4ToU32(g_colCtrl); }
inline ImU32 CtrlHovU32() { return ImGui::ColorConvertFloat4ToU32(g_colCtrlHov); }
inline ImU32 BorderU32() { return ImGui::ColorConvertFloat4ToU32(g_colBorder); }
inline ImU32 TextU32() { return ImGui::ColorConvertFloat4ToU32(g_colText); }
inline ImU32 TextDimU32() { return ImGui::ColorConvertFloat4ToU32(g_colTextDim); }
inline ImU32 BrightenU32(const ImVec4& c, float f)
{
    int r = (int)(c.x * 255.0f * f);
    int g = (int)(c.y * 255.0f * f);
    int b = (int)(c.z * 255.0f * f);
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    return IM_COL32(r, g, b, 255);
}
inline ImU32 BorderHovU32() { return BrightenU32(g_colBorder, 1.2f); }
inline ImU32 FadeU32(ImU32 col, float mul)
{
    if (mul < 0.0f) mul = 0.0f;
    if (mul > 1.0f) mul = 1.0f;
    ImVec4 c = ImGui::ColorConvertU32ToFloat4(col);
    c.w *= mul;
    if (c.w < 0.0f) c.w = 0.0f;
    if (c.w > 1.0f) c.w = 1.0f;
    return ImGui::ColorConvertFloat4ToU32(c);
}
inline void DrawGlowLine(ImDrawList* dl, float x0, float x1, float y, ImU32 col)
{
    float xi0 = floorf(x0);
    float xi1 = floorf(x1);
    if (xi1 <= xi0 + 1.0f || !dl)
        return;
    ImVec4 c = ImGui::ColorConvertU32ToFloat4(col);
    ImU32 soft0 = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, 0.10f));
    ImU32 soft1 = ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, 0.18f));
    dl->AddRectFilled(ImVec2(xi0, floorf(y - 2.0f)), ImVec2(xi1, floorf(y + 3.0f)), soft0);
    dl->AddRectFilled(ImVec2(xi0, floorf(y - 1.0f)), ImVec2(xi1, floorf(y + 2.0f)), soft1);
    const int segs = 24;
    float w = xi1 - xi0;
    float yc = floorf(y);
    for (int i = 0; i < segs; i++)
    {
        float t0 = (float)i / (float)segs;
        float t1 = (float)(i + 1) / (float)segs;
        float a0 = sinf(t0 * 3.14159265f);
        float a1 = sinf(t1 * 3.14159265f);
        float am = (a0 + a1) * 0.5f;
        am *= am;
        if (am < 0.02f)
            continue;
        dl->AddRectFilled(ImVec2(xi0 + t0 * w, yc), ImVec2(xi0 + t1 * w, yc + 1.0f), FadeU32(col, am));
    }
}
inline ImU32 AccentLightU32() { return BrightenU32(g_accent1, 1.18f); }
inline ImU32 AccentAlphaU32(int a)
{
    int r = (int)(g_accent1.x * 255.0f);
    int g = (int)(g_accent1.y * 255.0f);
    int b = (int)(g_accent1.z * 255.0f);
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    if (a < 0) a = 0; if (a > 255) a = 255;
    return IM_COL32(r, g, b, a);
}
inline void ResetThemeColors()
{
    g_accent1 = ImVec4(112.0f / 255.0f, 96.0f / 255.0f, 128.0f / 255.0f, 1.0f);
    g_color1 = ImVec4(112.0f / 255.0f, 96.0f / 255.0f, 128.0f / 255.0f, 1.0f);
    g_colFrame = ImVec4(22.0f / 255.0f, 22.0f / 255.0f, 26.0f / 255.0f, 1.0f);
    g_colGroup = ImVec4(32.0f / 255.0f, 32.0f / 255.0f, 37.0f / 255.0f, 1.0f);
    g_colCtrl = ImVec4(16.0f / 255.0f, 16.0f / 255.0f, 20.0f / 255.0f, 1.0f);
    g_colCtrlHov = ImVec4(28.0f / 255.0f, 28.0f / 255.0f, 34.0f / 255.0f, 1.0f);
    g_colBorder = ImVec4(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 1.0f);
    g_colText = ImVec4(235.0f / 255.0f, 235.0f / 255.0f, 238.0f / 255.0f, 1.0f);
    g_colTextDim = ImVec4(150.0f / 255.0f, 150.0f / 255.0f, 158.0f / 255.0f, 1.0f);
}
inline void ApplyThemePalette(int sel)
{
    ResetThemeColors();
    if (sel == 1)
    {
        g_accent1 = ImVec4(56.0f / 255.0f, 189.0f / 255.0f, 248.0f / 255.0f, 1.0f);
        g_colFrame = ImVec4(40.0f / 255.0f, 48.0f / 255.0f, 64.0f / 255.0f, 1.0f);
        g_colGroup = ImVec4(50.0f / 255.0f, 60.0f / 255.0f, 80.0f / 255.0f, 1.0f);
        g_colCtrl = ImVec4(26.0f / 255.0f, 32.0f / 255.0f, 44.0f / 255.0f, 1.0f);
        g_colCtrlHov = ImVec4(36.0f / 255.0f, 44.0f / 255.0f, 60.0f / 255.0f, 1.0f);
        g_colBorder = ImVec4(95.0f / 255.0f, 120.0f / 255.0f, 160.0f / 255.0f, 1.0f);
        g_colText = ImVec4(235.0f / 255.0f, 240.0f / 255.0f, 255.0f / 255.0f, 1.0f);
        g_colTextDim = ImVec4(175.0f / 255.0f, 175.0f / 255.0f, 180.0f / 255.0f, 1.0f);
    }
    else if (sel == 2)
    {
        g_accent1 = ImVec4(170.0f / 255.0f, 130.0f / 255.0f, 255.0f / 255.0f, 1.0f);
        g_colFrame = ImVec4(48.0f / 255.0f, 40.0f / 255.0f, 64.0f / 255.0f, 1.0f);
        g_colGroup = ImVec4(60.0f / 255.0f, 50.0f / 255.0f, 80.0f / 255.0f, 1.0f);
        g_colCtrl = ImVec4(32.0f / 255.0f, 26.0f / 255.0f, 44.0f / 255.0f, 1.0f);
        g_colCtrlHov = ImVec4(44.0f / 255.0f, 36.0f / 255.0f, 60.0f / 255.0f, 1.0f);
        g_colBorder = ImVec4(120.0f / 255.0f, 95.0f / 255.0f, 160.0f / 255.0f, 1.0f);
        g_colText = ImVec4(240.0f / 255.0f, 235.0f / 255.0f, 255.0f / 255.0f, 1.0f);
        g_colTextDim = ImVec4(170.0f / 255.0f, 150.0f / 255.0f, 190.0f / 255.0f, 1.0f);
    }
    else if (sel == 3)
    {
        g_accent1 = ImVec4(90.0f / 255.0f, 210.0f / 255.0f, 140.0f / 255.0f, 1.0f);
        g_colFrame = ImVec4(40.0f / 255.0f, 60.0f / 255.0f, 50.0f / 255.0f, 1.0f);
        g_colGroup = ImVec4(50.0f / 255.0f, 78.0f / 255.0f, 62.0f / 255.0f, 1.0f);
        g_colCtrl = ImVec4(26.0f / 255.0f, 42.0f / 255.0f, 32.0f / 255.0f, 1.0f);
        g_colCtrlHov = ImVec4(36.0f / 255.0f, 58.0f / 255.0f, 44.0f / 255.0f, 1.0f);
        g_colBorder = ImVec4(95.0f / 255.0f, 160.0f / 255.0f, 125.0f / 255.0f, 1.0f);
        g_colText = ImVec4(235.0f / 255.0f, 250.0f / 255.0f, 240.0f / 255.0f, 1.0f);
        g_colTextDim = ImVec4(150.0f / 255.0f, 190.0f / 255.0f, 165.0f / 255.0f, 1.0f);
    }
    else if (sel == 4)
    {
        g_accent1 = ImVec4(240.0f / 255.0f, 115.0f / 255.0f, 125.0f / 255.0f, 1.0f);
        g_colFrame = ImVec4(64.0f / 255.0f, 40.0f / 255.0f, 44.0f / 255.0f, 1.0f);
        g_colGroup = ImVec4(80.0f / 255.0f, 50.0f / 255.0f, 56.0f / 255.0f, 1.0f);
        g_colCtrl = ImVec4(44.0f / 255.0f, 26.0f / 255.0f, 30.0f / 255.0f, 1.0f);
        g_colCtrlHov = ImVec4(60.0f / 255.0f, 36.0f / 255.0f, 40.0f / 255.0f, 1.0f);
        g_colBorder = ImVec4(160.0f / 255.0f, 95.0f / 255.0f, 105.0f / 255.0f, 1.0f);
        g_colText = ImVec4(255.0f / 255.0f, 235.0f / 255.0f, 238.0f / 255.0f, 1.0f);
        g_colTextDim = ImVec4(190.0f / 255.0f, 150.0f / 255.0f, 158.0f / 255.0f, 1.0f);
    }
    else if (sel == 5)
    {
        g_accent1 = ImVec4(245.0f / 255.0f, 175.0f / 255.0f, 90.0f / 255.0f, 1.0f);
        g_colFrame = ImVec4(62.0f / 255.0f, 50.0f / 255.0f, 38.0f / 255.0f, 1.0f);
        g_colGroup = ImVec4(80.0f / 255.0f, 62.0f / 255.0f, 48.0f / 255.0f, 1.0f);
        g_colCtrl = ImVec4(44.0f / 255.0f, 34.0f / 255.0f, 26.0f / 255.0f, 1.0f);
        g_colCtrlHov = ImVec4(60.0f / 255.0f, 46.0f / 255.0f, 36.0f / 255.0f, 1.0f);
        g_colBorder = ImVec4(160.0f / 255.0f, 130.0f / 255.0f, 95.0f / 255.0f, 1.0f);
        g_colText = ImVec4(255.0f / 255.0f, 242.0f / 255.0f, 230.0f / 255.0f, 1.0f);
        g_colTextDim = ImVec4(190.0f / 255.0f, 170.0f / 255.0f, 150.0f / 255.0f, 1.0f);
    }
    else if (sel == 6)
    {
        g_accent1 = ImVec4(255.0f / 255.0f, 122.0f / 255.0f, 0.0f / 255.0f, 1.0f);
        g_colFrame = ImVec4(46.0f / 255.0f, 20.0f / 255.0f, 10.0f / 255.0f, 1.0f);
        g_colGroup = ImVec4(58.0f / 255.0f, 26.0f / 255.0f, 12.0f / 255.0f, 1.0f);
        g_colCtrl = ImVec4(30.0f / 255.0f, 15.0f / 255.0f, 10.0f / 255.0f, 1.0f);
        g_colCtrlHov = ImVec4(52.0f / 255.0f, 28.0f / 255.0f, 16.0f / 255.0f, 1.0f);
        g_colBorder = ImVec4(255.0f / 255.0f, 122.0f / 255.0f, 0.0f / 255.0f, 1.0f);
        g_colText = ImVec4(255.0f / 255.0f, 236.0f / 255.0f, 190.0f / 255.0f, 1.0f);
        g_colTextDim = ImVec4(205.0f / 255.0f, 150.0f / 255.0f, 100.0f / 255.0f, 1.0f);
    }
    g_color1 = g_accent1;
}
inline float Clamp(float v, float mn, float mx) { return (v < mn) ? mn : (v > mx) ? mx : v; }
inline void DrawPixelBox(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 bg_col, ImU32 border_light, ImU32 border_dark = IM_COL32(0, 0, 0, 255))
{
    float x0 = floorf(min.x);
    float y0 = floorf(min.y);
    float x1 = floorf(max.x);
    float y1 = floorf(max.y);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), bg_col);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y0 + 1.0f), border_dark);
    dl->AddRectFilled(ImVec2(x0, y1 - 1.0f), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + 1.0f, y1), border_dark);
    dl->AddRectFilled(ImVec2(x1 - 1.0f, y0), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y0 + 2.0f), border_light);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y1 - 2.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x0 + 2.0f, y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(x1 - 2.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
}
inline void DrawOutlinedText(ImDrawList* dl, ImFont* font, float font_size, ImVec2 pos, ImU32 text_col, ImU32 outline_col, const char* text)
{

    ImVec2 p(floorf(pos.x), floorf(pos.y));
    dl->AddText(font, font_size, ImVec2(p.x + 1.0f, p.y + 1.0f), outline_col, text);
    dl->AddText(font, font_size, p, text_col, text);
}
inline void DrawOutlinedText(ImDrawList* dl, ImVec2 pos, ImU32 text_col, ImU32 outline_col, const char* text)
{
    DrawOutlinedText(dl, nullptr, 0.0f, pos, text_col, outline_col, text);
}
inline void DrawWaveText(ImDrawList* dl, ImFont* font, float font_size, ImVec2 pos, ImU32 text_col, ImU32 outline_col, const char* text)
{
    if (!text || text[0] == '\0') return;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : ImGui::GetFontSize();
    float time = (float)ImGui::GetTime() * 2.8f;
    char chars[64];
    ImVec2 pts[64];
    int count = 0;
    float cur_x = pos.x;
    for (int i = 0; text[i] != '\0' && count < 63; i++)
    {
        chars[count] = text[i];
        float y_off = sinf(time - (float)i * 0.65f) * 1.5f;
        pts[count] = ImVec2(floorf(cur_x), pos.y + y_off);
        char buf[2] = { text[i], '\0' };
        cur_x += f->CalcTextSizeA(fs, FLT_MAX, 0.0f, buf).x;
        count++;
    }
    for (int i = 0; i < count; i++)
    {
        char buf[2] = { chars[i], '\0' };
        ImVec2 p = pts[i];
        dl->AddText(f, fs, ImVec2(p.x + 1.0f, p.y + 1.0f), outline_col, buf);
    }
    for (int i = 0; i < count; i++)
    {
        char buf[2] = { chars[i], '\0' };
        dl->AddText(f, fs, pts[i], text_col, buf);
    }
}
inline void DrawWaveText(ImDrawList* dl, ImVec2 pos, ImU32 text_col, ImU32 outline_col, const char* text)
{
    DrawWaveText(dl, nullptr, 0.0f, pos, text_col, outline_col, text);
}
#pragma comment(lib, "winmm.lib")
#include "../../resources/ui_click.h"
#include "../../resources/ui_slider.h"
#include <vector>
inline void UiClick()
{
    struct ClickBuf { const unsigned char* data = nullptr; unsigned len = 0; };
    static ClickBuf buf = []() -> ClickBuf {
        ClickBuf out{ UiClick_wav, UiClick_wav_len };
        constexpr float kGain = 0.12f;
        if (UiClick_wav_len < 44 || std::memcmp(UiClick_wav, "RIFF", 4) != 0 || std::memcmp(UiClick_wav + 8, "WAVE", 4) != 0)
            return out;
        uint32_t fmtOff = 0, fmtLen = 0, dataOff = 0;
        uint32_t dataLen = 0;
        for (uint32_t p = 12; p + 8 <= UiClick_wav_len; )
        {
            uint32_t ckLen = 0;
            std::memcpy(&ckLen, UiClick_wav + p + 4, 4);
            if (std::memcmp(UiClick_wav + p, "fmt ", 4) == 0) { fmtOff = p + 8; fmtLen = ckLen; }
            else if (std::memcmp(UiClick_wav + p, "data", 4) == 0) { dataOff = p + 8; dataLen = ckLen; }
            if (fmtOff && dataOff)
                break;
            uint32_t step = 8 + ckLen + (ckLen & 1u);
            if (step == 0 || p + step <= p || p + step > UiClick_wav_len)
                break;
            p += step;
        }
        if (!fmtOff || fmtLen < 16 || !dataOff || !dataLen || dataOff + dataLen > UiClick_wav_len)
            return out;
        uint16_t audioFormat = 0, channels = 0, bits = 0;
        std::memcpy(&audioFormat, UiClick_wav + fmtOff, 2);
        std::memcpy(&channels, UiClick_wav + fmtOff + 2, 2);
        std::memcpy(&bits, UiClick_wav + fmtOff + 14, 2);
        if (audioFormat != 1 || channels == 0)
            return out;
        static std::vector<unsigned char> quiet;
        quiet.assign(UiClick_wav, UiClick_wav + UiClick_wav_len);
        if (bits == 16)
        {
            int16_t* s = reinterpret_cast<int16_t*>(quiet.data() + dataOff);
            size_t n = dataLen / 2;
            for (size_t i = 0; i < n; ++i)
                s[i] = (int16_t)(s[i] * kGain);
        }
        else if (bits == 8)
        {
            unsigned char* s = quiet.data() + dataOff;
            for (uint32_t i = 0; i < dataLen; ++i)
            {
                int v = (int)s[i] - 128;
                s[i] = (unsigned char)(128 + (int)(v * kGain));
            }
        }
        else
        {
            return out;
        }
        out.data = quiet.data();
        out.len = (unsigned)quiet.size();
        return out;
    }();
    if (!g_menuOpen)
        return;
    static ULONGLONG s_lastMs = 0;
    ULONGLONG now = GetTickCount64();
    if (now - s_lastMs < 40)
        return;
    s_lastMs = now;
    PlaySoundA((LPCSTR)buf.data, NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}
inline void UiSliderTick()
{
    struct TickBuf { const unsigned char* data = nullptr; unsigned len = 0; };
    static TickBuf buf = []() -> TickBuf {
        TickBuf out{ UiSlider_wav, UiSlider_wav_len };
        constexpr float kGain = 0.08f;
        if (UiSlider_wav_len < 44 || std::memcmp(UiSlider_wav, "RIFF", 4) != 0 || std::memcmp(UiSlider_wav + 8, "WAVE", 4) != 0)
            return out;
        uint32_t fmtOff = 0, fmtLen = 0, dataOff = 0;
        uint32_t dataLen = 0;
        for (uint32_t p = 12; p + 8 <= UiSlider_wav_len; )
        {
            uint32_t ckLen = 0;
            std::memcpy(&ckLen, UiSlider_wav + p + 4, 4);
            if (std::memcmp(UiSlider_wav + p, "fmt ", 4) == 0) { fmtOff = p + 8; fmtLen = ckLen; }
            else if (std::memcmp(UiSlider_wav + p, "data", 4) == 0) { dataOff = p + 8; dataLen = ckLen; }
            if (fmtOff && dataOff)
                break;
            uint32_t step = 8 + ckLen + (ckLen & 1u);
            if (step == 0 || p + step <= p || p + step > UiSlider_wav_len)
                break;
            p += step;
        }
        if (!fmtOff || fmtLen < 16 || !dataOff || !dataLen || dataOff + dataLen > UiSlider_wav_len)
            return out;
        uint16_t audioFormat = 0, channels = 0, bits = 0;
        std::memcpy(&audioFormat, UiSlider_wav + fmtOff, 2);
        std::memcpy(&channels, UiSlider_wav + fmtOff + 2, 2);
        std::memcpy(&bits, UiSlider_wav + fmtOff + 14, 2);
        if (audioFormat != 1 || channels == 0)
            return out;
        static std::vector<unsigned char> quiet;
        quiet.assign(UiSlider_wav, UiSlider_wav + UiSlider_wav_len);
        if (bits == 16)
        {
            int16_t* s = reinterpret_cast<int16_t*>(quiet.data() + dataOff);
            size_t n = dataLen / 2;
            for (size_t i = 0; i < n; ++i)
                s[i] = (int16_t)(s[i] * kGain);
        }
        else if (bits == 8)
        {
            unsigned char* s = quiet.data() + dataOff;
            for (uint32_t i = 0; i < dataLen; ++i)
            {
                int v = (int)s[i] - 128;
                s[i] = (unsigned char)(128 + (int)(v * kGain));
            }
        }
        else
        {
            return out;
        }
        out.data = quiet.data();
        out.len = (unsigned)quiet.size();
        return out;
    }();
    if (!g_menuOpen)
        return;
    static ULONGLONG s_lastTickMs = 0;
    ULONGLONG now = GetTickCount64();
    if (now - s_lastTickMs < 35)
        return;
    s_lastTickMs = now;
    PlaySoundA((LPCSTR)buf.data, NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}
inline void DrawTabbedContainer(ImDrawList* dl, ImVec2 min, ImVec2 max, float tab_min_x, float tab_max_x, float tab_top_y, ImU32 bg_col, ImU32 border_light, ImU32 border_dark, ImU32 accent_col)
{
    float x0 = floorf(min.x);
    float y0 = floorf(min.y);
    float x1 = floorf(max.x);
    float y1 = floorf(max.y);
    float tx0 = floorf(tab_min_x);
    float tx1 = floorf(tab_max_x);
    float ty0 = floorf(tab_top_y);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), bg_col);
    dl->AddRectFilled(ImVec2(tx0, ty0), ImVec2(tx1, y0 + 2.0f), bg_col);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + 1.0f, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x0 + 2.0f, y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(x1 - 1.0f, y0), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x1 - 2.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(x0, y1 - 1.0f), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y1 - 2.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
    if (tx0 > x0)
    {
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(tx0 + 1.0f, y0 + 1.0f), border_dark);
        dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(tx0 + 1.0f, y0 + 2.0f), border_light);
    }
    if (tx1 < x1)
    {
        dl->AddRectFilled(ImVec2(tx1 - 1.0f, y0), ImVec2(x1, y0 + 1.0f), border_dark);
        dl->AddRectFilled(ImVec2(tx1 - 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y0 + 2.0f), border_light);
    }
    dl->AddRectFilled(ImVec2(tx0, ty0), ImVec2(tx0 + 1.0f, y0 + 1.0f), border_dark);
    dl->AddRectFilled(ImVec2(tx0 + 1.0f, ty0 + 1.0f), ImVec2(tx0 + 2.0f, y0 + 2.0f), border_light);
    dl->AddRectFilled(ImVec2(tx1 - 1.0f, ty0), ImVec2(tx1, y0 + 1.0f), border_dark);
    dl->AddRectFilled(ImVec2(tx1 - 2.0f, ty0 + 1.0f), ImVec2(tx1 - 1.0f, y0 + 2.0f), border_light);
    dl->AddRectFilled(ImVec2(tx0, ty0), ImVec2(tx1, ty0 + 1.0f), border_dark);
    dl->AddRectFilled(ImVec2(tx0 + 1.0f, ty0 + 1.0f), ImVec2(tx1 - 1.0f, ty0 + 2.0f), accent_col);
}
inline void DrawGroupBox(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* title, ImFont* font, float font_size, ImU32 bg_col, ImU32 border_light, ImU32 border_dark = IM_COL32(0, 0, 0, 255), ImU32 text_col = TextU32())
{
    float x0 = floorf(min.x);
    float y0 = floorf(min.y);
    float x1 = floorf(max.x);
    float y1 = floorf(max.y);
    dl->AddRectFilled(ImVec2(x0, y0 + 1.0f), ImVec2(x1, y1), bg_col);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + 1.0f, y1), border_dark);
    dl->AddRectFilled(ImVec2(x1 - 1.0f, y0), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0, y1 - 1.0f), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x0 + 2.0f, y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(x1 - 2.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y1 - 2.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
    if (title && title[0] != '\0')
    {
        ImFont* f = font ? font : ImGui::GetFont();
        float fs = (font_size > 0.0f) ? font_size : 13.0f;
        ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, title);
        float text_offset_x = 12.0f;
        float text_x = x0 + text_offset_x;
        float cut_start = text_x - 4.0f;
        float cut_end = text_x + text_sz.x + 4.0f;
        float text_y = floorf(y0 - text_sz.y * 0.5f);
        if (cut_start > x0)
        {
            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(cut_start, y0 + 1.0f), border_dark);
            dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(cut_start, y0 + 2.0f), border_light);
        }
        if (cut_end < x1)
        {
            dl->AddRectFilled(ImVec2(cut_end, y0), ImVec2(x1, y0 + 1.0f), border_dark);
            dl->AddRectFilled(ImVec2(cut_end, y0 + 1.0f), ImVec2(x1 - 1.0f, y0 + 2.0f), border_light);
        }
        DrawOutlinedText(dl, f, fs, ImVec2(text_x, text_y), text_col, border_dark, title);
    }
    else
    {
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y0 + 1.0f), border_dark);
        dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y0 + 2.0f), border_light);
    }
}
inline constexpr float kCheckboxLabelGap = 3.0f;
inline constexpr float kLabelFontSize = 13.0f;
inline constexpr float kLabelBandH    = kLabelFontSize;
inline constexpr float kLabeledCtrlH = kLabelBandH + 14.0f;
inline constexpr float kLabeledBoxH  = kLabelBandH + 20.0f;
inline constexpr float kCheckRowH     = 14.0f;
inline constexpr float kRowGapCb      = 4.0f;
inline constexpr float kCheckRowPitch = kCheckRowH + kRowGapCb;
inline constexpr float kLabelTextRise = 2.0f;
inline bool DrawCheckbox(ImDrawList* dl, ImVec2 pos, bool* value, const char* label = nullptr, ImFont* font = nullptr, float font_size = 13.0f, bool ignore_modal = false)
{
    float x0 = floorf(pos.x);
    float y0 = floorf(pos.y);
    float w = 14.0f;
    float h = 14.0f;
    float x1 = x0 + w;
    float y1 = y0 + h;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    float total_w = w;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    ImVec2 text_sz(0.0f, 0.0f);
    if (label && label[0] != '\0')
    {
        text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, label);
        total_w += kCheckboxLabelGap + text_sz.x;
    }
    bool is_hov = (m.x >= x0 && m.x <= x0 + total_w && m.y >= y0 && m.y <= y1);
    if (!ignore_modal && IsModalActive()) is_hov = false;
    if (is_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsMouseClicked(0))
        {
            *value = !*value;
            UiClick();
        }
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), border_dark);
    if (*value)
    {
        dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), AccentU32());
        dl->AddRectFilled(ImVec2(x0 + 2.0f, y0 + 2.0f), ImVec2(x1 - 2.0f, y1 - 2.0f), AccentLightU32());
    }
    else
    {
        dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_light);
        dl->AddRectFilled(ImVec2(x0 + 2.0f, y0 + 2.0f), ImVec2(x1 - 2.0f, y1 - 2.0f), CtrlBgU32());
    }
    if (label && label[0] != '\0')
    {
        float text_x = x1 + kCheckboxLabelGap;
        float text_y = floorf(y0 + (h - text_sz.y) * 0.5f);
        ImU32 col_label = *value ? TextU32() : TextDimU32();
        DrawOutlinedText(dl, f, fs, ImVec2(text_x, text_y), col_label, border_dark, label);
    }
    return is_hov;
}
inline int HexVal(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
inline bool DrawColorPicker(const char* id, ImDrawList* dl, ImVec2 pos, ImVec2 size, ImVec4* col, bool* outline_opt = nullptr, ImVec4* outline_col = nullptr)
{
    bool out_res = false;
    if (outline_col != nullptr && outline_opt != nullptr && *outline_opt)
    {
        char out_id[64];
        snprintf(out_id, sizeof(out_id), "%s_out", id ? id : "##cp");
        float out_x = pos.x - (size.x + 6.0f);
        float out_y = pos.y;
        out_res = DrawColorPicker(out_id, dl, ImVec2(out_x, out_y), size, outline_col, outline_opt, nullptr);
    }
    float x0 = floorf(pos.x);
    float y0 = floorf(pos.y);
    float x1 = floorf(x0 + size.x);
    float y1 = floorf(y0 + size.y);
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    char ctx_popup_id[64];
    snprintf(ctx_popup_id, sizeof(ctx_popup_id), "%s##cp_ctx", id);
    bool is_hov = (m.x >= x0 && m.x <= x1 && m.y >= y0 && m.y <= y1);
    bool own_open_cp = ImGui::IsPopupOpen(id) || ImGui::IsPopupOpen(ctx_popup_id);
    if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) && !own_open_cp && !g_inQuestionPopup)
        is_hov = false;
    if (is_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsMouseClicked(0))
        {
            ImGui::OpenPopup(id);
            UiClick();
        }
        else if (ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup(ctx_popup_id);
        }
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    float inline_factor = is_hov ? 0.90f : 0.75f;
    int inline_floor = is_hov ? 40 : 30;
    int in_r = (int)(col->x * 255.0f * inline_factor);
    int in_g = (int)(col->y * 255.0f * inline_factor);
    int in_b = (int)(col->z * 255.0f * inline_factor);
    if (in_r < inline_floor) in_r = inline_floor; if (in_r > 255) in_r = 255;
    if (in_g < inline_floor) in_g = inline_floor; if (in_g > 255) in_g = 255;
    if (in_b < inline_floor) in_b = inline_floor; if (in_b > 255) in_b = 255;
    const ImU32 border_inline = IM_COL32(in_r, in_g, in_b, 255);
    ImU32 col_u32 = ImGui::ColorConvertFloat4ToU32(*col);
    dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), border_inline);
    dl->AddRectFilled(ImVec2(x0 + 2.0f, y0 + 2.0f), ImVec2(x1 - 2.0f, y1 - 2.0f), col_u32);
    const float sv_w = 160.0f;
    const float sv_h = 160.0f;
    const float bar_h = 10.0f;
    const float pad = 8.0f;
    const float margin = 8.0f;
    float popup_w = sv_w + margin * 2.0f;
    float popup_h = sv_h + pad + bar_h + pad + bar_h + margin * 2.0f;
    ImVec2 popup_pos(floorf(x1 - popup_w), floorf(y1 + 4.0f));
    ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(popup_w, popup_h), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(30.0f / 255.0f, 36.0f / 255.0f, 48.0f / 255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(margin, margin));
    if (ImGui::BeginPopup(id, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* pdl = ImGui::GetWindowDrawList();
        ImVec2 w_min = ImGui::GetWindowPos();
        w_min.x = floorf(w_min.x);
        w_min.y = floorf(w_min.y);
        ImVec2 w_max = ImVec2(w_min.x + popup_w, w_min.y + popup_h);
        DrawPixelBox(pdl, w_min, w_max, CtrlBgU32(), border_light, border_dark);
        ImGui::Dummy(ImVec2(sv_w, sv_h + pad + bar_h + pad + bar_h));
        float sv_x0 = w_min.x + margin;
        float sv_y0 = w_min.y + margin;
        float sv_x1 = sv_x0 + sv_w;
        float sv_y1 = sv_y0 + sv_h;
        float hue_y0 = sv_y1 + pad;
        float hue_y1 = hue_y0 + bar_h;
        float alpha_y0 = hue_y1 + pad;
        float alpha_y1 = alpha_y0 + bar_h;
        float h = 0.0f, s = 0.0f, v = 0.0f;
        ImGui::ColorConvertRGBtoHSV(col->x, col->y, col->z, h, s, v);
        static std::unordered_map<std::string, float> s_savedH;
        std::string hkey = id ? id : "##cp";
        auto hIt = s_savedH.find(hkey);
        if (hIt == s_savedH.end())
        {
            s_savedH[hkey] = h;
            hIt = s_savedH.find(hkey);
        }
        if (s > 0.001f)
        {
            hIt->second = h;
        }
        else
        {
            h = hIt->second;
        }
        static std::string s_activePid;
        static int s_activeHandle = 0;
        if (ImGui::IsMouseClicked(0))
        {
            if (m.x >= sv_x0 && m.x <= sv_x1 && m.y >= sv_y0 && m.y <= sv_y1)
            {
                s_activePid = id ? id : "";
                s_activeHandle = 1;
            }
            else if (m.x >= sv_x0 && m.x <= sv_x1 && m.y >= hue_y0 && m.y <= hue_y1)
            {
                s_activePid = id ? id : "";
                s_activeHandle = 2;
            }
            else if (m.x >= sv_x0 && m.x <= sv_x1 && m.y >= alpha_y0 && m.y <= alpha_y1)
            {
                s_activePid = id ? id : "";
                s_activeHandle = 3;
            }
        }
        if (!ImGui::IsMouseDown(0))
        {
            s_activePid.clear();
            s_activeHandle = 0;
        }
        bool own_drag = (!s_activePid.empty() && id != nullptr && s_activePid == id);
        int active_drag = own_drag ? s_activeHandle : 0;
        if (active_drag == 1)
        {
            s = Clamp((m.x - sv_x0) / sv_w, 0.0f, 1.0f);
            v = Clamp(1.0f - (m.y - sv_y0) / sv_h, 0.0f, 1.0f);
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
            col->x = r;
            col->y = g;
            col->z = b;
        }
        else if (active_drag == 2)
        {
            h = Clamp((m.x - sv_x0) / sv_w, 0.0f, 0.999f);
            hIt->second = h;
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
            col->x = r;
            col->y = g;
            col->z = b;
        }
        else if (active_drag == 3)
        {
            col->w = Clamp((m.x - sv_x0) / sv_w, 0.0f, 1.0f);
        }
        float pure_r, pure_g, pure_b;
        ImGui::ColorConvertHSVtoRGB(h, 1.0f, 1.0f, pure_r, pure_g, pure_b);
        ImU32 col_pure_hue = ImGui::ColorConvertFloat4ToU32(ImVec4(pure_r, pure_g, pure_b, 1.0f));
        ImU32 col_white = IM_COL32(255, 255, 255, 255);
        pdl->AddRectFilledMultiColor(ImVec2(sv_x0, sv_y0), ImVec2(sv_x1, sv_y1), col_white, col_pure_hue, col_pure_hue, col_white);
        pdl->AddRectFilledMultiColor(ImVec2(sv_x0, sv_y0), ImVec2(sv_x1, sv_y1), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));
        pdl->AddRectFilled(ImVec2(sv_x0, sv_y0), ImVec2(sv_x1, sv_y0 + 1.0f), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x0, sv_y1 - 1.0f), ImVec2(sv_x1, sv_y1), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x0, sv_y0), ImVec2(sv_x0 + 1.0f, sv_y1), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x1 - 1.0f, sv_y0), ImVec2(sv_x1, sv_y1), border_dark);
        float cur_x = Clamp(sv_x0 + s * sv_w, sv_x0, sv_x1);
        float cur_y = Clamp(sv_y0 + (1.0f - v) * sv_h, sv_y0, sv_y1);
        pdl->AddCircle(ImVec2(cur_x, cur_y), 4.5f, IM_COL32(0, 0, 0, 255), 16, 2.5f);
        pdl->AddCircle(ImVec2(cur_x, cur_y), 4.5f, IM_COL32(255, 255, 255, 255), 16, 1.5f);
        float seg_w = sv_w / 6.0f;
        for (int i = 0; i < 6; i++)
        {
            float x_a = floorf(sv_x0 + (float)i * seg_w);
            float x_b = (i == 5) ? sv_x1 : floorf(sv_x0 + (float)(i + 1) * seg_w);
            float r0, g0, b0, r1, g1, b1;
            ImGui::ColorConvertHSVtoRGB((float)i / 6.0f, 1.0f, 1.0f, r0, g0, b0);
            ImGui::ColorConvertHSVtoRGB((float)(i + 1) / 6.0f, 1.0f, 1.0f, r1, g1, b1);
            ImU32 c0 = ImGui::ColorConvertFloat4ToU32(ImVec4(r0, g0, b0, 1.0f));
            ImU32 c1 = ImGui::ColorConvertFloat4ToU32(ImVec4(r1, g1, b1, 1.0f));
            pdl->AddRectFilledMultiColor(ImVec2(x_a, hue_y0), ImVec2(x_b, hue_y1), c0, c1, c1, c0);
        }
        pdl->AddRectFilled(ImVec2(sv_x0, hue_y0), ImVec2(sv_x1, hue_y0 + 1.0f), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x0, hue_y1 - 1.0f), ImVec2(sv_x1, hue_y1), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x0, hue_y0), ImVec2(sv_x0 + 1.0f, hue_y1), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x1 - 1.0f, hue_y0), ImVec2(sv_x1, hue_y1), border_dark);
        float hue_cur_x = Clamp(sv_x0 + h * sv_w, sv_x0, sv_x1);
        float hue_cur_y = floorf((hue_y0 + hue_y1) * 0.5f);
        pdl->AddCircle(ImVec2(hue_cur_x, hue_cur_y), 4.5f, IM_COL32(0, 0, 0, 255), 16, 2.5f);
        pdl->AddCircle(ImVec2(hue_cur_x, hue_cur_y), 4.5f, IM_COL32(255, 255, 255, 255), 16, 1.5f);
        pdl->AddRectFilled(ImVec2(sv_x0, alpha_y0), ImVec2(sv_x1, alpha_y1), IM_COL32(20, 24, 32, 255));
        ImU32 c_a0 = ImGui::ColorConvertFloat4ToU32(ImVec4(col->x, col->y, col->z, 0.0f));
        ImU32 c_a1 = ImGui::ColorConvertFloat4ToU32(ImVec4(col->x, col->y, col->z, 1.0f));
        pdl->AddRectFilledMultiColor(ImVec2(sv_x0, alpha_y0), ImVec2(sv_x1, alpha_y1), c_a0, c_a1, c_a1, c_a0);
        pdl->AddRectFilled(ImVec2(sv_x0, alpha_y0), ImVec2(sv_x1, alpha_y0 + 1.0f), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x0, alpha_y1 - 1.0f), ImVec2(sv_x1, alpha_y1), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x0, alpha_y0), ImVec2(sv_x0 + 1.0f, alpha_y1), border_dark);
        pdl->AddRectFilled(ImVec2(sv_x1 - 1.0f, alpha_y0), ImVec2(sv_x1, alpha_y1), border_dark);
        float alpha_cur_x = Clamp(sv_x0 + col->w * sv_w, sv_x0, sv_x1);
        float alpha_cur_y = floorf((alpha_y0 + alpha_y1) * 0.5f);
        pdl->AddCircle(ImVec2(alpha_cur_x, alpha_cur_y), 4.5f, IM_COL32(0, 0, 0, 255), 16, 2.5f);
        pdl->AddCircle(ImVec2(alpha_cur_x, alpha_cur_y), 4.5f, IM_COL32(255, 255, 255, 255), 16, 1.5f);
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    const float ctx_w = (outline_opt != nullptr) ? 96.0f : 85.0f;
    const float ctx_item_h = 18.0f;
    const float ctx_pad_y = 2.0f;
    int ctx_count = (outline_opt != nullptr) ? 3 : 2;
    float ctx_h = ctx_pad_y * 2.0f + ctx_item_h * (float)ctx_count;
    ImVec2 ctx_pos(floorf(x0), floorf(y1 + 4.0f));
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_sz = ImGui::GetWindowSize();
    if (ctx_pos.x + ctx_w > win_pos.x + win_sz.x - 10.0f)
    {
        ctx_pos.x = floorf(x1 - ctx_w);
    }
    if (ctx_pos.y + ctx_h > win_pos.y + win_sz.y - 10.0f)
    {
        ctx_pos.y = floorf(y0 - ctx_h - 4.0f);
    }
    ImGui::SetNextWindowPos(ctx_pos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(ctx_w, ctx_h), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(30.0f / 255.0f, 36.0f / 255.0f, 48.0f / 255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::BeginPopup(ctx_popup_id, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* pdl = ImGui::GetWindowDrawList();
        ImVec2 w_min = ImGui::GetWindowPos();
        w_min.x = floorf(w_min.x);
        w_min.y = floorf(w_min.y);
        ImVec2 w_max = ImVec2(w_min.x + ctx_w, w_min.y + ctx_h);
        DrawPixelBox(pdl, w_min, w_max, CtrlBgU32(), border_light, border_dark);
        ImGui::Dummy(ImVec2(ctx_w, ctx_h));
        const char* ctx_items[] = { "Copy hex", "Paste hex" };
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        float fs = 13.0f;
        for (int i = 0; i < ctx_count; i++)
        {
            float it_y0 = floorf(w_min.y + ctx_pad_y + (float)i * ctx_item_h);
            float it_y1 = it_y0 + ctx_item_h;
            float it_x0 = w_min.x + 2.0f;
            float it_x1 = w_max.x - 2.0f;
            bool it_hov = (m.x >= it_x0 && m.x <= it_x1 && m.y >= it_y0 && m.y <= it_y1);
            if (it_hov)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                pdl->AddRectFilled(ImVec2(it_x0, it_y0), ImVec2(it_x1, it_y1), CtrlHovU32());
                if (ImGui::IsMouseClicked(0))
                {
                    if (i == 0)
                    {
                        float cr = Clamp(col->x, 0.0f, 1.0f);
                        float cg = Clamp(col->y, 0.0f, 1.0f);
                        float cb = Clamp(col->z, 0.0f, 1.0f);
                        float ca = Clamp(col->w, 0.0f, 1.0f);
                        int ir = (int)(cr * 255.0f + 0.5f);
                        int ig = (int)(cg * 255.0f + 0.5f);
                        int ib = (int)(cb * 255.0f + 0.5f);
                        int ia = (int)(ca * 255.0f + 0.5f);
                        char hex_buf[16];
                        if (ia < 255)
                        {
                            snprintf(hex_buf, sizeof(hex_buf), "#%02X%02X%02X%02X", ir, ig, ib, ia);
                        }
                        else
                        {
                            snprintf(hex_buf, sizeof(hex_buf), "#%02X%02X%02X", ir, ig, ib);
                        }
                        ImGui::SetClipboardText(hex_buf);
                    }
                    else if (i == 1)
                    {
                        const char* clip = ImGui::GetClipboardText();
                        if (clip)
                        {
                            while (*clip == ' ' || *clip == '\t' || *clip == '\r' || *clip == '\n') clip++;
                            if (*clip == '#') clip++;
                            else if (clip[0] == '0' && (clip[1] == 'x' || clip[1] == 'X')) clip += 2;
                            int hlen = 0;
                            while (HexVal(clip[hlen]) >= 0) hlen++;
                            if (hlen == 6)
                            {
                                int r = (HexVal(clip[0]) << 4) | HexVal(clip[1]);
                                int g = (HexVal(clip[2]) << 4) | HexVal(clip[3]);
                                int b = (HexVal(clip[4]) << 4) | HexVal(clip[5]);
                                col->x = (float)r / 255.0f;
                                col->y = (float)g / 255.0f;
                                col->z = (float)b / 255.0f;
                                col->w = 1.0f;
                            }
                            else if (hlen == 8)
                            {
                                int r = (HexVal(clip[0]) << 4) | HexVal(clip[1]);
                                int g = (HexVal(clip[2]) << 4) | HexVal(clip[3]);
                                int b = (HexVal(clip[4]) << 4) | HexVal(clip[5]);
                                int a = (HexVal(clip[6]) << 4) | HexVal(clip[7]);
                                col->x = (float)r / 255.0f;
                                col->y = (float)g / 255.0f;
                                col->z = (float)b / 255.0f;
                                col->w = (float)a / 255.0f;
                            }
                            else if (hlen == 3)
                            {
                                int r = HexVal(clip[0]) * 17;
                                int g = HexVal(clip[1]) * 17;
                                int b = HexVal(clip[2]) * 17;
                                col->x = (float)r / 255.0f;
                                col->y = (float)g / 255.0f;
                                col->z = (float)b / 255.0f;
                                col->w = 1.0f;
                            }
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
                else if (i == 2 && outline_opt)
                {
                    *outline_opt = !*outline_opt;
                    if (outline_col == nullptr && !*outline_opt)
                    {
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            if (i < 2)
            {
                ImU32 it_text_col = it_hov ? TextU32() : TextDimU32();
                ImVec2 it_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, ctx_items[i]);
                float it_tx = floorf(w_min.x + (ctx_w - it_sz.x) * 0.5f);
                float it_ty = floorf(it_y0 + (ctx_item_h - it_sz.y) * 0.5f);
                DrawOutlinedText(pdl, f, fs, ImVec2(it_tx, it_ty), it_text_col, border_dark, ctx_items[i]);
            }
            else if (i == 2 && outline_opt)
            {
                float cb_sz = 10.0f;
                float cb_x0 = it_x0 + 6.0f;
                float cb_y0 = floorf(it_y0 + (ctx_item_h - cb_sz) * 0.5f);
                float cb_x1 = cb_x0 + cb_sz;
                float cb_y1 = cb_y0 + cb_sz;
                pdl->AddRectFilled(ImVec2(cb_x0, cb_y0), ImVec2(cb_x1, cb_y1), border_dark);
                if (*outline_opt)
                {
                    pdl->AddRectFilled(ImVec2(cb_x0 + 1.0f, cb_y0 + 1.0f), ImVec2(cb_x1 - 1.0f, cb_y1 - 1.0f), AccentDimU32());
                    pdl->AddRectFilled(ImVec2(cb_x0 + 2.0f, cb_y0 + 2.0f), ImVec2(cb_x1 - 2.0f, cb_y1 - 2.0f), border_light);
                }
                else
                {
                    pdl->AddRectFilled(ImVec2(cb_x0 + 1.0f, cb_y0 + 1.0f), ImVec2(cb_x1 - 1.0f, cb_y1 - 1.0f), border_light);
                    pdl->AddRectFilled(ImVec2(cb_x0 + 2.0f, cb_y0 + 2.0f), ImVec2(cb_x1 - 2.0f, cb_y1 - 2.0f), CtrlBgU32());
                }
                ImVec2 t_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "Outline");
                float tx = cb_x1 + 6.0f;
                float ty = floorf(it_y0 + (ctx_item_h - t_sz.y) * 0.5f);
                ImU32 it_text_col = *outline_opt ? TextU32() : TextDimU32();
                DrawOutlinedText(pdl, f, fs, ImVec2(tx, ty), it_text_col, border_dark, "Outline");
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    return is_hov || own_open_cp || out_res;
}
inline const char* GetKeyDisplayName(int key)
{
    if (key == ImGuiKey_MouseLeft) return "M1";
    if (key == ImGuiKey_MouseRight) return "M2";
    if (key == ImGuiKey_MouseMiddle) return "M3";
    if (key == ImGuiKey_MouseX1) return "X1";
    if (key == ImGuiKey_MouseX2) return "X2";
    if (key == ImGuiKey_LeftShift) return "LShift";
    if (key == ImGuiKey_RightShift) return "RShift";
    if (key == ImGuiKey_LeftCtrl) return "LCtrl";
    if (key == ImGuiKey_RightCtrl) return "RCtrl";
    if (key == ImGuiKey_LeftAlt) return "LAlt";
    if (key == ImGuiKey_RightAlt) return "RAlt";
    const char* name = ImGui::GetKeyName((ImGuiKey)key);
    return name ? name : "None";
}
inline bool DrawKeybind(const char* id, ImDrawList* dl, ImVec2 right_edge, int* key, int* mode, ImFont* font = nullptr, float font_size = 13.0f)
{
    if (!g_menuOpen)
    {
        g_activeBindId = nullptr;
    }
    bool is_binding = (g_activeBindId != nullptr && (g_activeBindId == id || strcmp(g_activeBindId, id) == 0));
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    if (is_binding && ImGui::GetFrameCount() > g_bindStartFrame)
    {
        bool kb_any_popup = !g_inQuestionPopup && ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            *key = 0;
            g_activeBindId = nullptr;
            is_binding = false;
        }
        else if (!kb_any_popup && ImGui::IsMouseClicked(0))
        {
            *key = ImGuiKey_MouseLeft;
            g_activeBindId = nullptr;
            is_binding = false;
        }
        else if (!kb_any_popup && ImGui::IsMouseClicked(1))
        {
            *key = ImGuiKey_MouseRight;
            g_activeBindId = nullptr;
            is_binding = false;
        }
        else if (!kb_any_popup && ImGui::IsMouseClicked(2))
        {
            *key = ImGuiKey_MouseMiddle;
            g_activeBindId = nullptr;
            is_binding = false;
        }
        else if (!kb_any_popup && ImGui::IsMouseClicked(3))
        {
            *key = ImGuiKey_MouseX1;
            g_activeBindId = nullptr;
            is_binding = false;
        }
        else if (!kb_any_popup && ImGui::IsMouseClicked(4))
        {
            *key = ImGuiKey_MouseX2;
            g_activeBindId = nullptr;
            is_binding = false;
        }
        else
        {
            for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
            {
                if (k == ImGuiKey_ReservedForModCtrl || k == ImGuiKey_ReservedForModShift ||
                    k == ImGuiKey_ReservedForModAlt || k == ImGuiKey_ReservedForModSuper ||
                    k == ImGuiKey_MouseWheelX || k == ImGuiKey_MouseWheelY)
                {
                    continue;
                }
                if (ImGui::IsKeyPressed((ImGuiKey)k))
                {
                    *key = k;
                    g_activeBindId = nullptr;
                    is_binding = false;
                    break;
                }
            }
        }
    }
    char display_buf[64];
    if (is_binding)
    {
        snprintf(display_buf, sizeof(display_buf), "[ ]");
    }
    else if (*key == 0)
    {
        snprintf(display_buf, sizeof(display_buf), "[ . . . ]");
    }
    else
    {
        snprintf(display_buf, sizeof(display_buf), "[ %s ]", GetKeyDisplayName(*key));
    }
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, display_buf);
    float right_x = floorf(right_edge.x);
    float text_x = floorf(right_x - text_sz.x);
    float y0 = floorf(right_edge.y);
    float text_y = floorf(y0 + (14.0f - text_sz.y) * 0.5f);
    float hit_x0 = text_x - 3.0f;
    float hit_x1 = right_x + 3.0f;
    float hit_y0 = y0 - 2.0f;
    float hit_y1 = y0 + 16.0f;
    bool is_hov = (m.x >= hit_x0 && m.x <= hit_x1 && m.y >= hit_y0 && m.y <= hit_y1);
    char popup_id[64];
    snprintf(popup_id, sizeof(popup_id), "##kb_pop_%s", id);
    bool kb_own_popup = ImGui::IsPopupOpen(popup_id);
    if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) && !kb_own_popup && !g_inQuestionPopup)
        is_hov = false;
    if (is_hov && !is_binding)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsMouseClicked(0))
        {
            g_activeBindId = id;
            g_bindStartFrame = ImGui::GetFrameCount();
            is_binding = true;
            UiClick();
            snprintf(display_buf, sizeof(display_buf), "[ ]");
            text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, display_buf);
            text_x = floorf(right_x - text_sz.x);
        }
        else if (ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup(popup_id);
        }
    }
    const ImU32 border_dark = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    ImU32 text_col;
    if (is_binding)
    {
        text_col = AccentU32();
    }
    else if (*key != 0)
    {
        text_col = TextU32();
    }
    else
    {
        text_col = TextDimU32();
    }
    DrawOutlinedText(dl, f, fs, ImVec2(text_x, text_y), text_col, border_dark, display_buf);
    const float popup_w = 75.0f;
    const float item_h = 18.0f;
    const float pad_y = 2.0f;
    const float popup_h = pad_y * 2.0f + item_h * 3.0f;
    ImVec2 popup_pos(floorf(right_x - popup_w), floorf(y0 + 18.0f));
    ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(popup_w, popup_h), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(30.0f / 255.0f, 36.0f / 255.0f, 48.0f / 255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::BeginPopup(popup_id, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* pdl = ImGui::GetWindowDrawList();
        ImVec2 w_min = ImGui::GetWindowPos();
        w_min.x = floorf(w_min.x);
        w_min.y = floorf(w_min.y);
        ImVec2 w_max = ImVec2(w_min.x + popup_w, w_min.y + popup_h);
        DrawPixelBox(pdl, w_min, w_max, CtrlBgU32(), border_light, border_dark);
        ImGui::Dummy(ImVec2(popup_w, popup_h));
        const char* modes[] = { "Always", "Toggle", "Hold" };
        for (int i = 0; i < 3; i++)
        {
            float it_y0 = floorf(w_min.y + pad_y + (float)i * item_h);
            float it_y1 = it_y0 + item_h;
            float it_x0 = w_min.x + 2.0f;
            float it_x1 = w_max.x - 2.0f;
            bool it_hov = (m.x >= it_x0 && m.x <= it_x1 && m.y >= it_y0 && m.y <= it_y1);
            if (it_hov)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                pdl->AddRectFilled(ImVec2(it_x0, it_y0), ImVec2(it_x1, it_y1), CtrlHovU32());
                if (ImGui::IsMouseClicked(0))
                {
                    *mode = i;
                    UiClick();
                    ImGui::CloseCurrentPopup();
                }
            }
            bool is_selected = (*mode == i);
            ImU32 it_text_col = is_selected ? AccentU32() : TextDimU32();
            ImVec2 t_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, modes[i]);
            float tx = it_x0 + 8.0f;
            float ty = floorf(it_y0 + (item_h - t_sz.y) * 0.5f);
            DrawOutlinedText(pdl, f, fs, ImVec2(tx, ty), it_text_col, border_dark, modes[i]);
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    bool popup_open = ImGui::IsPopupOpen(popup_id);
    return is_hov || popup_open || is_binding;
}
inline bool DrawSlider(const char* id, ImDrawList* dl, ImVec2 pos, float width, int* val, int min_val, int max_val, const char* label = nullptr, const char* format = "%d", ImFont* font = nullptr, float font_size = 13.0f, const ImVec4* fillCol = nullptr)
{
    static const char* s_active_slider_id = nullptr;
    static float s_repeat_timer = 0.0f;
    if (!g_menuOpen && !g_settingsOpen && !g_themeOpen)
    {
        s_active_slider_id = nullptr;
    }
    const int old_slider_val = val ? *val : 0;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float x1 = floorf(pos.x + width);
    bool has_label = (label && label[0] != '\0');
    float bar_y0 = has_label ? floorf(pos.y + kLabelBandH) : floorf(pos.y);
    float bar_y1 = bar_y0 + 14.0f;
    float btn_minus_x0 = x0;
    float btn_minus_x1 = x0 + 14.0f;
    float btn_plus_x0 = x1 - 14.0f;
    float btn_plus_x1 = x1;
    float bar_x0 = btn_minus_x1 + 4.0f;
    float bar_x1 = btn_plus_x0 - 4.0f;
    bool is_minus_hov = (m.x >= btn_minus_x0 && m.x <= btn_minus_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool is_plus_hov  = (m.x >= btn_plus_x0 && m.x <= btn_plus_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool is_bar_hov   = (m.x >= bar_x0 && m.x <= bar_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool slider_modal = IsModalActive();
    if (slider_modal)
    {

        is_minus_hov = false;
        is_plus_hov = false;
        is_bar_hov = false;
    }
    if (is_minus_hov || is_plus_hov || is_bar_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if (is_minus_hov && ImGui::IsMouseDown(0))
    {
        if (ImGui::IsMouseClicked(0))
        {
            if (*val > min_val)
            {
                *val -= 1;
            }
            s_repeat_timer = 0.0f;
        }
        else if (io.MouseDownDuration[0] > 0.35f)
        {
            s_repeat_timer += io.DeltaTime;
            if (s_repeat_timer >= 0.05f)
            {
                if (*val > min_val)
                {
                    *val -= 1;
                }
                s_repeat_timer = 0.0f;
            }
        }
    }
    else if (is_plus_hov && ImGui::IsMouseDown(0))
    {
        if (ImGui::IsMouseClicked(0))
        {
            if (*val < max_val)
            {
                *val += 1;
            }
            s_repeat_timer = 0.0f;
        }
        else if (io.MouseDownDuration[0] > 0.35f)
        {
            s_repeat_timer += io.DeltaTime;
            if (s_repeat_timer >= 0.05f)
            {
                if (*val < max_val)
                {
                    *val += 1;
                }
                s_repeat_timer = 0.0f;
            }
        }
    }
    else if (!ImGui::IsMouseDown(0))
    {
        s_repeat_timer = 0.0f;
    }
    if (is_bar_hov && ImGui::IsMouseClicked(0))
    {
        s_active_slider_id = id;
    }
    bool bar_active = (s_active_slider_id != nullptr && (s_active_slider_id == id || strcmp(s_active_slider_id, id) == 0));
    float inner_w = (bar_x1 - 2.0f) - (bar_x0 + 2.0f);
    if (bar_active)
    {
        if (ImGui::IsMouseDown(0))
        {
            if (!slider_modal)
            {
                float frac = (m.x - (bar_x0 + 2.0f)) / inner_w;
                frac = Clamp(frac, 0.0f, 1.0f);
                *val = min_val + (int)floorf(frac * (float)(max_val - min_val) + 0.5f);
                *val = (*val < min_val) ? min_val : (*val > max_val) ? max_val : *val;
            }
        }
        else
        {
            s_active_slider_id = nullptr;
            bar_active = false;
        }
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    char val_buf[32];
    snprintf(val_buf, sizeof(val_buf), format ? format : "%d", *val);
    ImVec2 val_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, val_buf);
    if (has_label)
    {
        float text_y = floorf(pos.y - kLabelTextRise);
        ImU32 col_lbl = TextU32();
        DrawOutlinedText(dl, f, fs, ImVec2(x0, text_y), col_lbl, border_dark, label);
    }
    dl->AddRectFilled(ImVec2(btn_minus_x0, bar_y0), ImVec2(btn_minus_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(btn_minus_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(btn_minus_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(btn_minus_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(btn_minus_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    ImVec2 minus_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "-");
    float mx = floorf(btn_minus_x0 + (14.0f - minus_sz.x) * 0.5f);
    float my = floorf(bar_y0 + (14.0f - minus_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(mx, my), TextDimU32(), border_dark, "-");
    dl->AddRectFilled(ImVec2(btn_plus_x0, bar_y0), ImVec2(btn_plus_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(btn_plus_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(btn_plus_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(btn_plus_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(btn_plus_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    ImVec2 plus_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "+");
    float px = floorf(btn_plus_x0 + (14.0f - plus_sz.x) * 0.5f);
    float py = floorf(bar_y0 + (14.0f - plus_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(px, py), TextDimU32(), border_dark, "+");
    dl->AddRectFilled(ImVec2(bar_x0, bar_y0), ImVec2(bar_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(bar_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(bar_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(bar_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(bar_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    float frac = (max_val > min_val) ? ((float)(*val - min_val) / (float)(max_val - min_val)) : 0.0f;
    frac = Clamp(frac, 0.0f, 1.0f);
    static ImGuiStorage s_slider_storage;
    ImGuiID sid = ImGui::GetID(id);
    float current_anim = s_slider_storage.GetFloat(sid, -1.0f);
    if (current_anim < 0.0f)
    {
        current_anim = frac;
    }
    else
    {
        float dt = io.DeltaTime;
        if (dt > 0.05f) dt = 0.05f;
        float speed = 14.0f;
        current_anim += (frac - current_anim) * (1.0f - expf(-speed * dt));
        if (fabsf(frac - current_anim) < 0.0005f)
        {
            current_anim = frac;
        }
    }
    current_anim = Clamp(current_anim, 0.0f, 1.0f);
    s_slider_storage.SetFloat(sid, current_anim);
    float fill_w = floorf(inner_w * current_anim);
    if (fill_w > 0.0f)
    {
        float fill_x1 = bar_x0 + 2.0f + fill_w;
        if (fill_x1 > bar_x1 - 1.0f)
        {
            fill_x1 = bar_x1 - 1.0f;
        }
        ImU32 fillLow = fillCol ? BrightenU32(*fillCol, 0.7f) : AccentDimU32();
        ImU32 fillDark = fillCol ? BrightenU32(*fillCol, 0.55f) : BrightenU32(g_accent1, 0.55f);
        dl->AddRectFilled(ImVec2(bar_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(fill_x1, bar_y1 - 1.0f), fillLow);
        dl->AddRectFilled(ImVec2(bar_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(fill_x1, bar_y1 - 2.0f), fillDark);
        if (fill_x1 < bar_x1 - 2.0f)
        {
            dl->AddRectFilled(ImVec2(fill_x1, bar_y0 + 1.0f), ImVec2(fill_x1 + 1.0f, bar_y1 - 1.0f), border_dark);
        }
    }
    float vx = floorf(bar_x0 + (bar_x1 - bar_x0 - val_sz.x) * 0.5f);
    float vy = floorf(bar_y0 + (14.0f - val_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(vx, vy), TextU32(), border_dark, val_buf);
    float total_h = has_label ? kLabeledCtrlH : 14.0f;
    bool is_total_hov = (m.x >= x0 && m.x <= x1 && m.y >= pos.y - kLabelTextRise && m.y <= pos.y + total_h);
    if (slider_modal) return false;
    if (val && *val != old_slider_val)
        UiSliderTick();
    return is_total_hov || bar_active || is_minus_hov || is_plus_hov;
}
inline bool DrawSliderFloat(const char* id, ImDrawList* dl, ImVec2 pos, float width, float* val, float min_val, float max_val, const char* label = nullptr, const char* format = "%.1f", ImFont* font = nullptr, float font_size = 13.0f, const ImVec4* fillCol = nullptr, float step = 1.0f)
{
    static const char* s_active_sliderf_id = nullptr;
    static float s_repeat_timer_f = 0.0f;
    if (!g_menuOpen && !g_settingsOpen && !g_themeOpen)
    {
        s_active_sliderf_id = nullptr;
    }
    const float old_slider_val = val ? *val : 0.0f;
    if (val)
    {
        *val = Clamp(*val, min_val, max_val);
    }
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float x1 = floorf(pos.x + width);
    bool has_label = (label && label[0] != '\0');
    float bar_y0 = has_label ? floorf(pos.y + kLabelBandH) : floorf(pos.y);
    float bar_y1 = bar_y0 + 14.0f;
    float btn_minus_x0 = x0;
    float btn_minus_x1 = x0 + 14.0f;
    float btn_plus_x0 = x1 - 14.0f;
    float btn_plus_x1 = x1;
    float bar_x0 = btn_minus_x1 + 4.0f;
    float bar_x1 = btn_plus_x0 - 4.0f;
    bool is_minus_hov = (m.x >= btn_minus_x0 && m.x <= btn_minus_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool is_plus_hov  = (m.x >= btn_plus_x0 && m.x <= btn_plus_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool is_bar_hov   = (m.x >= bar_x0 && m.x <= bar_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool slider_modal = IsModalActive();
    if (slider_modal)
    {

        is_minus_hov = false;
        is_plus_hov = false;
        is_bar_hov = false;
    }
    if (is_minus_hov || is_plus_hov || is_bar_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if (is_minus_hov && ImGui::IsMouseDown(0))
    {
        if (ImGui::IsMouseClicked(0))
        {
            *val = Clamp(*val - step, min_val, max_val);
            s_repeat_timer_f = 0.0f;
        }
        else if (io.MouseDownDuration[0] > 0.35f)
        {
            s_repeat_timer_f += io.DeltaTime;
            if (s_repeat_timer_f >= 0.05f)
            {
                *val = Clamp(*val - step, min_val, max_val);
                s_repeat_timer_f = 0.0f;
            }
        }
    }
    else if (is_plus_hov && ImGui::IsMouseDown(0))
    {
        if (ImGui::IsMouseClicked(0))
        {
            *val = Clamp(*val + step, min_val, max_val);
            s_repeat_timer_f = 0.0f;
        }
        else if (io.MouseDownDuration[0] > 0.35f)
        {
            s_repeat_timer_f += io.DeltaTime;
            if (s_repeat_timer_f >= 0.05f)
            {
                *val = Clamp(*val + step, min_val, max_val);
                s_repeat_timer_f = 0.0f;
            }
        }
    }
    else if (!ImGui::IsMouseDown(0))
    {
        s_repeat_timer_f = 0.0f;
    }
    if (is_bar_hov && ImGui::IsMouseClicked(0))
    {
        s_active_sliderf_id = id;
    }
    bool bar_active = (s_active_sliderf_id != nullptr && (s_active_sliderf_id == id || strcmp(s_active_sliderf_id, id) == 0));
    float inner_w = (bar_x1 - 2.0f) - (bar_x0 + 2.0f);
    if (bar_active)
    {
        if (ImGui::IsMouseDown(0))
        {
            if (!slider_modal)
            {
                float frac = (inner_w > 0.0f) ? (m.x - (bar_x0 + 2.0f)) / inner_w : 0.0f;
                frac = Clamp(frac, 0.0f, 1.0f);
                *val = min_val + frac * (max_val - min_val);
                *val = Clamp(*val, min_val, max_val);
            }
        }
        else
        {
            s_active_sliderf_id = nullptr;
            bar_active = false;
        }
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    char val_buf[32];
    snprintf(val_buf, sizeof(val_buf), format ? format : "%.1f", (double)*val);
    ImVec2 val_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, val_buf);
    if (has_label)
    {
        float text_y = floorf(pos.y - kLabelTextRise);
        ImU32 col_lbl = TextU32();
        DrawOutlinedText(dl, f, fs, ImVec2(x0, text_y), col_lbl, border_dark, label);
    }
    dl->AddRectFilled(ImVec2(btn_minus_x0, bar_y0), ImVec2(btn_minus_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(btn_minus_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(btn_minus_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(btn_minus_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(btn_minus_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    ImVec2 minus_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "-");
    float mx = floorf(btn_minus_x0 + (14.0f - minus_sz.x) * 0.5f);
    float my = floorf(bar_y0 + (14.0f - minus_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(mx, my), TextDimU32(), border_dark, "-");
    dl->AddRectFilled(ImVec2(btn_plus_x0, bar_y0), ImVec2(btn_plus_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(btn_plus_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(btn_plus_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(btn_plus_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(btn_plus_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    ImVec2 plus_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "+");
    float px = floorf(btn_plus_x0 + (14.0f - plus_sz.x) * 0.5f);
    float py = floorf(bar_y0 + (14.0f - plus_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(px, py), TextDimU32(), border_dark, "+");
    dl->AddRectFilled(ImVec2(bar_x0, bar_y0), ImVec2(bar_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(bar_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(bar_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(bar_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(bar_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    float frac = (max_val > min_val) ? ((*val - min_val) / (max_val - min_val)) : 0.0f;
    frac = Clamp(frac, 0.0f, 1.0f);
    static ImGuiStorage s_sliderf_storage;
    ImGuiID sid = ImGui::GetID(id);
    float current_anim = s_sliderf_storage.GetFloat(sid, -1.0f);
    if (current_anim < 0.0f)
    {
        current_anim = frac;
    }
    else
    {
        float dt = io.DeltaTime;
        if (dt > 0.05f) dt = 0.05f;
        float speed = 14.0f;
        current_anim += (frac - current_anim) * (1.0f - expf(-speed * dt));
        if (fabsf(frac - current_anim) < 0.0005f)
        {
            current_anim = frac;
        }
    }
    current_anim = Clamp(current_anim, 0.0f, 1.0f);
    s_sliderf_storage.SetFloat(sid, current_anim);
    float fill_w = floorf(inner_w * current_anim);
    if (fill_w > 0.0f)
    {
        float fill_x1 = bar_x0 + 2.0f + fill_w;
        if (fill_x1 > bar_x1 - 1.0f)
        {
            fill_x1 = bar_x1 - 1.0f;
        }
        ImU32 fillLow = fillCol ? BrightenU32(*fillCol, 0.7f) : AccentDimU32();
        ImU32 fillDark = fillCol ? BrightenU32(*fillCol, 0.55f) : BrightenU32(g_accent1, 0.55f);
        dl->AddRectFilled(ImVec2(bar_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(fill_x1, bar_y1 - 1.0f), fillLow);
        dl->AddRectFilled(ImVec2(bar_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(fill_x1, bar_y1 - 2.0f), fillDark);
        if (fill_x1 < bar_x1 - 2.0f)
        {
            dl->AddRectFilled(ImVec2(fill_x1, bar_y0 + 1.0f), ImVec2(fill_x1 + 1.0f, bar_y1 - 1.0f), border_dark);
        }
    }
    float vx = floorf(bar_x0 + (bar_x1 - bar_x0 - val_sz.x) * 0.5f);
    float vy = floorf(bar_y0 + (14.0f - val_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(vx, vy), TextU32(), border_dark, val_buf);
    float total_h = has_label ? kLabeledCtrlH : 14.0f;
    bool is_total_hov = (m.x >= x0 && m.x <= x1 && m.y >= pos.y - kLabelTextRise && m.y <= pos.y + total_h);
    if (slider_modal) return false;
    if (val && *val != old_slider_val)
        UiSliderTick();
    return is_total_hov || bar_active || is_minus_hov || is_plus_hov;
}
inline bool DrawRangeSlider(const char* id, ImDrawList* dl, ImVec2 pos, float width, int* val_min, int* val_max, int min_val, int max_val, const char* label = nullptr, const char* format = "%d - %d", ImFont* font = nullptr, float font_size = 13.0f, const ImVec4* fillCol = nullptr)
{
    static const char* s_active_rslider_id = nullptr;
    static int s_active_rslider_handle = 0;
    static float s_repeat_timer = 0.0f;
    if (!g_menuOpen && !g_settingsOpen && !g_themeOpen)
    {
        s_active_rslider_id = nullptr;
        s_active_rslider_handle = 0;
    }
    const int old_rslider_min = val_min ? *val_min : 0;
    const int old_rslider_max = val_max ? *val_max : 0;
    if (*val_min < min_val) *val_min = min_val;
    if (*val_min > max_val) *val_min = max_val;
    if (*val_max < *val_min) *val_max = *val_min;
    if (*val_max > max_val) *val_max = max_val;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float x1 = floorf(pos.x + width);
    bool has_label = (label && label[0] != '\0');
    float bar_y0 = has_label ? floorf(pos.y + kLabelBandH) : floorf(pos.y);
    float bar_y1 = bar_y0 + 14.0f;
    float btn_minus_x0 = x0;
    float btn_minus_x1 = x0 + 14.0f;
    float btn_plus_x0 = x1 - 14.0f;
    float btn_plus_x1 = x1;
    float bar_x0 = btn_minus_x1 + 4.0f;
    float bar_x1 = btn_plus_x0 - 4.0f;
    bool is_minus_hov = (m.x >= btn_minus_x0 && m.x <= btn_minus_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool is_plus_hov  = (m.x >= btn_plus_x0 && m.x <= btn_plus_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool is_bar_hov   = (m.x >= bar_x0 && m.x <= bar_x1 && m.y >= bar_y0 && m.y <= bar_y1);
    bool slider_modal = IsModalActive();
    if (slider_modal)
    {

        is_minus_hov = false;
        is_plus_hov = false;
        is_bar_hov = false;
    }
    if (is_minus_hov || is_plus_hov || is_bar_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if (is_minus_hov && ImGui::IsMouseDown(0))
    {
        if (ImGui::IsMouseClicked(0))
        {
            if (*val_min > min_val) *val_min -= 1;
            s_repeat_timer = 0.0f;
        }
        else if (io.MouseDownDuration[0] > 0.35f)
        {
            s_repeat_timer += io.DeltaTime;
            if (s_repeat_timer >= 0.05f)
            {
                if (*val_min > min_val) *val_min -= 1;
                s_repeat_timer = 0.0f;
            }
        }
    }
    else if (is_minus_hov && ImGui::IsMouseClicked(1))
    {
        if (*val_min < *val_max) *val_min += 1;
    }
    else if (is_plus_hov && ImGui::IsMouseDown(0))
    {
        if (ImGui::IsMouseClicked(0))
        {
            if (*val_max < max_val) *val_max += 1;
            s_repeat_timer = 0.0f;
        }
        else if (io.MouseDownDuration[0] > 0.35f)
        {
            s_repeat_timer += io.DeltaTime;
            if (s_repeat_timer >= 0.05f)
            {
                if (*val_max < max_val) *val_max += 1;
                s_repeat_timer = 0.0f;
            }
        }
    }
    else if (is_plus_hov && ImGui::IsMouseClicked(1))
    {
        if (*val_max > *val_min) *val_max -= 1;
    }
    else if (!ImGui::IsMouseDown(0))
    {
        s_repeat_timer = 0.0f;
    }
    float inner_w = (bar_x1 - 2.0f) - (bar_x0 + 2.0f);
    float range_span = (max_val > min_val) ? (float)(max_val - min_val) : 1.0f;
    float frac_min = Clamp((float)(*val_min - min_val) / range_span, 0.0f, 1.0f);
    float frac_max = Clamp((float)(*val_max - min_val) / range_span, 0.0f, 1.0f);
    if (is_bar_hov && ImGui::IsMouseClicked(0))
    {
        s_active_rslider_id = id;
        float click_frac = Clamp((m.x - (bar_x0 + 2.0f)) / inner_w, 0.0f, 1.0f);
        float d_min = fabsf(click_frac - frac_min);
        float d_max = fabsf(click_frac - frac_max);
        if (d_min < d_max)
        {
            s_active_rslider_handle = 1;
            *val_min = min_val + (int)floorf(click_frac * range_span + 0.5f);
            if (*val_min > *val_max) *val_min = *val_max;
            if (*val_min < min_val) *val_min = min_val;
        }
        else if (d_max < d_min)
        {
            s_active_rslider_handle = 2;
            *val_max = min_val + (int)floorf(click_frac * range_span + 0.5f);
            if (*val_max < *val_min) *val_max = *val_min;
            if (*val_max > max_val) *val_max = max_val;
        }
        else
        {
            if (click_frac >= frac_max)
            {
                s_active_rslider_handle = 2;
                *val_max = min_val + (int)floorf(click_frac * range_span + 0.5f);
            }
            else
            {
                s_active_rslider_handle = 1;
                *val_min = min_val + (int)floorf(click_frac * range_span + 0.5f);
            }
        }
    }
    bool bar_active = (s_active_rslider_id != nullptr && (s_active_rslider_id == id || strcmp(s_active_rslider_id, id) == 0));
    if (bar_active)
    {
        if (ImGui::IsMouseDown(0))
        {
            if (!slider_modal)
            {
                float drag_frac = Clamp((m.x - (bar_x0 + 2.0f)) / inner_w, 0.0f, 1.0f);
                int new_val = min_val + (int)floorf(drag_frac * range_span + 0.5f);
                if (s_active_rslider_handle == 1)
                {
                    if (new_val > *val_max) new_val = *val_max;
                    if (new_val < min_val) new_val = min_val;
                    *val_min = new_val;
                }
                else if (s_active_rslider_handle == 2)
                {
                    if (new_val < *val_min) new_val = *val_min;
                    if (new_val > max_val) new_val = max_val;
                    *val_max = new_val;
                }
            }
        }
        else
        {
            s_active_rslider_id = nullptr;
            s_active_rslider_handle = 0;
            bar_active = false;
        }
    }
    if (is_bar_hov && io.MouseWheel != 0.0f)
    {
        float mid_x = bar_x0 + 2.0f + inner_w * (frac_min + frac_max) * 0.5f;
        int wheel_delta = (io.MouseWheel > 0.0f) ? 1 : -1;
        if (m.x < mid_x)
        {
            *val_min = (int)Clamp((float)(*val_min + wheel_delta), (float)min_val, (float)*val_max);
        }
        else
        {
            *val_max = (int)Clamp((float)(*val_max + wheel_delta), (float)*val_min, (float)max_val);
        }
    }
    frac_min = Clamp((float)(*val_min - min_val) / range_span, 0.0f, 1.0f);
    frac_max = Clamp((float)(*val_max - min_val) / range_span, 0.0f, 1.0f);
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    char val_buf[32];
    snprintf(val_buf, sizeof(val_buf), format ? format : "%d - %d", *val_min, *val_max);
    ImVec2 val_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, val_buf);
    if (has_label)
    {
        float text_y = floorf(pos.y - kLabelTextRise);
        ImU32 col_lbl = TextU32();
        DrawOutlinedText(dl, f, fs, ImVec2(x0, text_y), col_lbl, border_dark, label);
    }
    dl->AddRectFilled(ImVec2(btn_minus_x0, bar_y0), ImVec2(btn_minus_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(btn_minus_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(btn_minus_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(btn_minus_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(btn_minus_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    ImVec2 minus_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "-");
    float mx = floorf(btn_minus_x0 + (14.0f - minus_sz.x) * 0.5f);
    float my = floorf(bar_y0 + (14.0f - minus_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(mx, my), TextDimU32(), border_dark, "-");
    dl->AddRectFilled(ImVec2(btn_plus_x0, bar_y0), ImVec2(btn_plus_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(btn_plus_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(btn_plus_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(btn_plus_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(btn_plus_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    ImVec2 plus_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "+");
    float px = floorf(btn_plus_x0 + (14.0f - plus_sz.x) * 0.5f);
    float py = floorf(bar_y0 + (14.0f - plus_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(px, py), TextDimU32(), border_dark, "+");
    dl->AddRectFilled(ImVec2(bar_x0, bar_y0), ImVec2(bar_x1, bar_y1), border_dark);
    dl->AddRectFilled(ImVec2(bar_x0 + 1.0f, bar_y0 + 1.0f), ImVec2(bar_x1 - 1.0f, bar_y1 - 1.0f), border_light);
    dl->AddRectFilled(ImVec2(bar_x0 + 2.0f, bar_y0 + 2.0f), ImVec2(bar_x1 - 2.0f, bar_y1 - 2.0f), CtrlBgU32());
    static ImGuiStorage s_rslider_storage;
    ImGuiID sid_min = ImGui::GetID(id);
    ImGuiID sid_max = sid_min + 1;
    float cur_anim_min = s_rslider_storage.GetFloat(sid_min, -1.0f);
    float cur_anim_max = s_rslider_storage.GetFloat(sid_max, -1.0f);
    if (cur_anim_min < 0.0f) cur_anim_min = frac_min;
    if (cur_anim_max < 0.0f) cur_anim_max = frac_max;
    float dt = io.DeltaTime;
    if (dt > 0.05f) dt = 0.05f;
    float speed = 14.0f;
    cur_anim_min += (frac_min - cur_anim_min) * (1.0f - expf(-speed * dt));
    cur_anim_max += (frac_max - cur_anim_max) * (1.0f - expf(-speed * dt));
    if (fabsf(frac_min - cur_anim_min) < 0.0005f) cur_anim_min = frac_min;
    if (fabsf(frac_max - cur_anim_max) < 0.0005f) cur_anim_max = frac_max;
    cur_anim_min = Clamp(cur_anim_min, 0.0f, 1.0f);
    cur_anim_max = Clamp(cur_anim_max, 0.0f, 1.0f);
    if (cur_anim_min > cur_anim_max) cur_anim_min = cur_anim_max;
    s_rslider_storage.SetFloat(sid_min, cur_anim_min);
    s_rslider_storage.SetFloat(sid_max, cur_anim_max);
    float fill_x0 = floorf(bar_x0 + 2.0f + inner_w * cur_anim_min);
    float fill_x1 = floorf(bar_x0 + 2.0f + inner_w * cur_anim_max);
    if (fill_x0 < bar_x0 + 2.0f) fill_x0 = bar_x0 + 2.0f;
    if (fill_x1 > bar_x1 - 1.0f) fill_x1 = bar_x1 - 1.0f;
    if (fill_x0 > fill_x1) fill_x0 = fill_x1;
    ImU32 fillLow = fillCol ? BrightenU32(*fillCol, 0.7f) : AccentDimU32();
    ImU32 fillDark = fillCol ? BrightenU32(*fillCol, 0.55f) : BrightenU32(g_accent1, 0.55f);
    if (fill_x1 > fill_x0)
    {
        dl->AddRectFilled(ImVec2(fill_x0, bar_y0 + 1.0f), ImVec2(fill_x1, bar_y1 - 1.0f), fillLow);
        dl->AddRectFilled(ImVec2(fill_x0, bar_y0 + 2.0f), ImVec2(fill_x1, bar_y1 - 2.0f), fillDark);
    }
    if (fill_x0 > bar_x0 + 2.0f)
    {
        dl->AddRectFilled(ImVec2(fill_x0 - 1.0f, bar_y0 + 1.0f), ImVec2(fill_x0, bar_y1 - 1.0f), border_dark);
    }
    if (fill_x1 < bar_x1 - 2.0f)
    {
        dl->AddRectFilled(ImVec2(fill_x1, bar_y0 + 1.0f), ImVec2(fill_x1 + 1.0f, bar_y1 - 1.0f), border_dark);
    }
    float vx = floorf(bar_x0 + (bar_x1 - bar_x0 - val_sz.x) * 0.5f);
    float vy = floorf(bar_y0 + (14.0f - val_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, fs, ImVec2(vx, vy), TextU32(), border_dark, val_buf);
    float total_h = has_label ? kLabeledCtrlH : 14.0f;
    bool is_total_hov = (m.x >= x0 && m.x <= x1 && m.y >= pos.y - kLabelTextRise && m.y <= pos.y + total_h);
    if (slider_modal) return false;
    if ((val_min && *val_min != old_rslider_min) || (val_max && *val_max != old_rslider_max))
        UiSliderTick();
    return is_total_hov || bar_active || is_minus_hov || is_plus_hov;
}
inline bool DrawMinMaxSlider(const char* id, ImDrawList* dl, ImVec2 pos, float width, int* val_min, int* val_max, int min_val, int max_val, const char* label = nullptr, const char* format = "%d - %d", ImFont* font = nullptr, float font_size = 13.0f, const ImVec4* fillCol = nullptr)
{
    return DrawRangeSlider(id, dl, pos, width, val_min, val_max, min_val, max_val, label, format, font, font_size, fillCol);
}
inline bool DrawButton(ImDrawList* dl, ImVec2 pos, float width, float height, const char* text, ImFont* font = nullptr, float font_size = 13.0f, bool* out_hovered = nullptr)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float y0 = floorf(pos.y);
    float x1 = floorf(x0 + width);
    float y1 = floorf(y0 + height);
    bool is_hov = (m.x >= x0 && m.x <= x1 && m.y >= y0 && m.y <= y1);
    bool is_down = is_hov && ImGui::IsMouseDown(0);
    bool is_clicked = is_hov && ImGui::IsMouseClicked(0);
    if (IsModalActive())
    {
        is_hov = false;
        is_down = false;
        is_clicked = false;
    }
    if (is_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    ImU32 bg_col = CtrlBgU32();
    ImU32 light_col = border_light;
    DrawPixelBox(dl, ImVec2(x0, y0), ImVec2(x1, y1), bg_col, light_col, border_dark);
    ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, text);
    float tx = floorf(x0 + (width - text_sz.x) * 0.5f);
    float ty = floorf(y0 + (height - text_sz.y) * 0.5f + (is_down ? 1.0f : 0.0f));
    DrawOutlinedText(dl, f, fs, ImVec2(tx, ty), TextDimU32(), border_dark, text);
    if (out_hovered) *out_hovered = is_hov;
    if (is_clicked) UiClick();
    return is_clicked;
}
inline bool DrawTextInput(const char* id, ImDrawList* dl, ImVec2 pos, float width, char* buf, int buf_size, const char* label = nullptr, const char* hint = nullptr, ImFont* font = nullptr, float font_size = 13.0f)
{
    static const char* s_active_input_id = nullptr;
    static int s_cursor_pos = 0;
    static int s_sel_start = -1;
    static int s_sel_end = -1;
    if (!g_menuOpen)
    {
        s_active_input_id = nullptr;
        s_sel_start = -1;
        s_sel_end = -1;
    }
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float x1 = floorf(pos.x + width);
    bool has_label = (label && label[0] != '\0');
    float box_y0 = has_label ? floorf(pos.y + kLabelBandH) : floorf(pos.y);
    float box_h = 20.0f;
    float box_y1 = box_y0 + box_h;
    bool is_active = (s_active_input_id != nullptr && (s_active_input_id == id || strcmp(s_active_input_id, id) == 0));
    bool raw_box_hov = (m.x >= x0 && m.x <= x1 && m.y >= box_y0 && m.y <= box_y1);
    bool text_modal = IsModalActive();
    bool is_box_hov = raw_box_hov && !text_modal;
    if (is_box_hov && !is_active)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
    }
    if (is_box_hov && is_active)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
    }
    if (ImGui::IsMouseClicked(0))
    {
        if (raw_box_hov && !text_modal)
        {
            s_active_input_id = id;
            s_cursor_pos = (int)strlen(buf);
            s_sel_start = -1;
            s_sel_end = -1;
        }
        else if (is_active && !raw_box_hov)
        {
            s_active_input_id = nullptr;
            is_active = false;
            s_sel_start = -1;
            s_sel_end = -1;
        }
    }
    bool has_sel = (is_active && s_sel_start >= 0 && s_sel_end >= 0 && s_sel_start != s_sel_end);
    int sel_min = 0;
    int sel_max = 0;
    if (has_sel)
    {
        sel_min = s_sel_start < s_sel_end ? s_sel_start : s_sel_end;
        sel_max = s_sel_start > s_sel_end ? s_sel_start : s_sel_end;
    }
    if (is_active && text_modal)
    {
        io.InputQueueCharacters.resize(0);
    }
    if (is_active && !text_modal)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            s_active_input_id = nullptr;
            is_active = false;
            s_sel_start = -1;
            s_sel_end = -1;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            s_active_input_id = nullptr;
            is_active = false;
            s_sel_start = -1;
            s_sel_end = -1;
        }
        else
        {
            int len = (int)strlen(buf);
            bool ctrl = io.KeyCtrl;
            bool shift = io.KeyShift;
            if (ctrl && ImGui::IsKeyPressed(ImGuiKey_A))
            {
                s_sel_start = 0;
                s_sel_end = len;
                s_cursor_pos = len;
            }
            else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C))
            {
                if (has_sel)
                {
                    char clip[256];
                    int clip_len = sel_max - sel_min;
                    if (clip_len > (int)sizeof(clip) - 1)
                    {
                        clip_len = (int)sizeof(clip) - 1;
                    }
                    memcpy(clip, buf + sel_min, (size_t)clip_len);
                    clip[clip_len] = '\0';
                    ImGui::SetClipboardText(clip);
                }
            }
            else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_X))
            {
                if (has_sel)
                {
                    char clip[256];
                    int clip_len = sel_max - sel_min;
                    if (clip_len > (int)sizeof(clip) - 1)
                    {
                        clip_len = (int)sizeof(clip) - 1;
                    }
                    memcpy(clip, buf + sel_min, (size_t)clip_len);
                    clip[clip_len] = '\0';
                    ImGui::SetClipboardText(clip);
                    for (int j = sel_min; j <= len - (sel_max - sel_min); j++)
                    {
                        buf[j] = buf[j + (sel_max - sel_min)];
                    }
                    s_cursor_pos = sel_min;
                    s_sel_start = -1;
                    s_sel_end = -1;
                    has_sel = false;
                    len = (int)strlen(buf);
                }
            }
            else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V))
            {
                const char* clip = ImGui::GetClipboardText();
                if (clip && clip[0] != '\0')
                {
                    if (has_sel)
                    {
                        for (int j = sel_min; j <= len - (sel_max - sel_min); j++)
                        {
                            buf[j] = buf[j + (sel_max - sel_min)];
                        }
                        s_cursor_pos = sel_min;
                        s_sel_start = -1;
                        s_sel_end = -1;
                        has_sel = false;
                        len = (int)strlen(buf);
                    }
                    int clip_len = (int)strlen(clip);
                    int avail = buf_size - 1 - len;
                    if (clip_len > avail)
                    {
                        clip_len = avail;
                    }
                    if (clip_len > 0)
                    {
                        for (int j = len; j >= s_cursor_pos; j--)
                        {
                            buf[j + clip_len] = buf[j];
                        }
                        for (int j = 0; j < clip_len; j++)
                        {
                            unsigned char ch = (unsigned char)clip[j];
                            if (ch < 32 || ch >= 127)
                            {
                                ch = '?';
                            }
                            buf[s_cursor_pos + j] = (char)ch;
                        }
                        s_cursor_pos += clip_len;
                        len = (int)strlen(buf);
                    }
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Backspace))
            {
                if (has_sel)
                {
                    for (int j = sel_min; j <= len - (sel_max - sel_min); j++)
                    {
                        buf[j] = buf[j + (sel_max - sel_min)];
                    }
                    s_cursor_pos = sel_min;
                    s_sel_start = -1;
                    s_sel_end = -1;
                    has_sel = false;
                }
                else if (s_cursor_pos > 0 && len > 0)
                {
                    for (int j = s_cursor_pos - 1; j < len; j++)
                    {
                        buf[j] = buf[j + 1];
                    }
                    s_cursor_pos--;
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                if (has_sel)
                {
                    for (int j = sel_min; j <= len - (sel_max - sel_min); j++)
                    {
                        buf[j] = buf[j + (sel_max - sel_min)];
                    }
                    s_cursor_pos = sel_min;
                    s_sel_start = -1;
                    s_sel_end = -1;
                    has_sel = false;
                }
                else if (s_cursor_pos < len)
                {
                    for (int j = s_cursor_pos; j < len; j++)
                    {
                        buf[j] = buf[j + 1];
                    }
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            {
                if (has_sel && !shift)
                {
                    s_cursor_pos = sel_min;
                    s_sel_start = -1;
                    s_sel_end = -1;
                }
                else
                {
                    if (shift && s_sel_start < 0)
                    {
                        s_sel_start = s_cursor_pos;
                        s_sel_end = s_cursor_pos;
                    }
                    if (s_cursor_pos > 0)
                    {
                        s_cursor_pos--;
                    }
                    if (shift)
                    {
                        s_sel_end = s_cursor_pos;
                    }
                    else
                    {
                        s_sel_start = -1;
                        s_sel_end = -1;
                    }
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            {
                if (has_sel && !shift)
                {
                    s_cursor_pos = sel_max;
                    s_sel_start = -1;
                    s_sel_end = -1;
                }
                else
                {
                    if (shift && s_sel_start < 0)
                    {
                        s_sel_start = s_cursor_pos;
                        s_sel_end = s_cursor_pos;
                    }
                    if (s_cursor_pos < len)
                    {
                        s_cursor_pos++;
                    }
                    if (shift)
                    {
                        s_sel_end = s_cursor_pos;
                    }
                    else
                    {
                        s_sel_start = -1;
                        s_sel_end = -1;
                    }
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Home))
            {
                if (shift && s_sel_start < 0)
                {
                    s_sel_start = s_cursor_pos;
                    s_sel_end = s_cursor_pos;
                }
                s_cursor_pos = 0;
                if (shift)
                {
                    s_sel_end = s_cursor_pos;
                }
                else
                {
                    s_sel_start = -1;
                    s_sel_end = -1;
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_End))
            {
                if (shift && s_sel_start < 0)
                {
                    s_sel_start = s_cursor_pos;
                    s_sel_end = s_cursor_pos;
                }
                s_cursor_pos = len;
                if (shift)
                {
                    s_sel_end = s_cursor_pos;
                }
                else
                {
                    s_sel_start = -1;
                    s_sel_end = -1;
                }
            }
            if (io.InputQueueCharacters.Size > 0 && !ctrl)
            {
                if (has_sel)
                {
                    for (int j = sel_min; j <= len - (sel_max - sel_min); j++)
                    {
                        buf[j] = buf[j + (sel_max - sel_min)];
                    }
                    s_cursor_pos = sel_min;
                    s_sel_start = -1;
                    s_sel_end = -1;
                    has_sel = false;
                    len = (int)strlen(buf);
                }
                for (int n = 0; n < io.InputQueueCharacters.Size; n++)
                {
                    unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                    if (c >= 32 && c < 127 && len < buf_size - 1)
                    {
                        for (int j = len; j >= s_cursor_pos; j--)
                        {
                            buf[j + 1] = buf[j];
                        }
                        buf[s_cursor_pos] = (char)c;
                        s_cursor_pos++;
                        len++;
                    }
                }
                io.InputQueueCharacters.resize(0);
            }
            else if (ctrl)
            {
                io.InputQueueCharacters.resize(0);
            }
            len = (int)strlen(buf);
            if (s_cursor_pos > len)
            {
                s_cursor_pos = len;
            }
        }
    }
    has_sel = (is_active && s_sel_start >= 0 && s_sel_end >= 0 && s_sel_start != s_sel_end);
    if (has_sel)
    {
        sel_min = s_sel_start < s_sel_end ? s_sel_start : s_sel_end;
        sel_max = s_sel_start > s_sel_end ? s_sel_start : s_sel_end;
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    if (has_label)
    {
        float text_y = floorf(pos.y - kLabelTextRise);
        ImU32 col_lbl = (is_active || is_box_hov) ? TextU32() : TextU32();
        DrawOutlinedText(dl, f, fs, ImVec2(x0, text_y), col_lbl, border_dark, label);
    }
    dl->AddRectFilled(ImVec2(x0, box_y0), ImVec2(x1, box_y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, box_y0 + 1.0f), ImVec2(x1 - 1.0f, box_y1 - 1.0f), is_active ? BorderHovU32() : border_light);
    dl->AddRectFilled(ImVec2(x0 + 2.0f, box_y0 + 2.0f), ImVec2(x1 - 2.0f, box_y1 - 2.0f), CtrlBgU32());
    float text_pad = 6.0f;
    float text_area_x0 = x0 + text_pad;
    float text_area_x1 = x1 - text_pad;
    float max_text_w = text_area_x1 - text_area_x0;
    int len = (int)strlen(buf);
    bool is_empty = (len == 0);
    if (is_empty && !is_active && hint && hint[0] != '\0')
    {
        ImVec2 hint_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, hint);
        float hy = floorf(box_y0 + (box_h - hint_sz.y) * 0.5f);
        DrawOutlinedText(dl, f, fs, ImVec2(text_area_x0, hy), TextDimU32(), border_dark, hint);
    }
    else
    {
        ImVec2 full_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, buf);
        float scroll_off = 0.0f;
        if (is_active)
        {
            char tmp[128];
            int cp = s_cursor_pos < (int)sizeof(tmp) - 1 ? s_cursor_pos : (int)sizeof(tmp) - 1;
            memcpy(tmp, buf, (size_t)cp);
            tmp[cp] = '\0';
            ImVec2 cursor_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, tmp);
            if (cursor_sz.x > max_text_w)
            {
                scroll_off = cursor_sz.x - max_text_w + 2.0f;
            }
        }
        ImVec4 clip_rect(text_area_x0, box_y0 + 2.0f, text_area_x1, box_y1 - 2.0f);
        dl->PushClipRect(ImVec2(clip_rect.x, clip_rect.y), ImVec2(clip_rect.z, clip_rect.w), true);
        float draw_x = text_area_x0 - scroll_off;
        float draw_y = floorf(box_y0 + (box_h - full_sz.y) * 0.5f);
        if (has_sel)
        {
            char tmp_min[128];
            int cm = sel_min < (int)sizeof(tmp_min) - 1 ? sel_min : (int)sizeof(tmp_min) - 1;
            memcpy(tmp_min, buf, (size_t)cm);
            tmp_min[cm] = '\0';
            ImVec2 sm_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, tmp_min);
            char tmp_max[128];
            int cx = sel_max < (int)sizeof(tmp_max) - 1 ? sel_max : (int)sizeof(tmp_max) - 1;
            memcpy(tmp_max, buf, (size_t)cx);
            tmp_max[cx] = '\0';
            ImVec2 sx_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, tmp_max);
            float s_x0 = floorf(draw_x + sm_sz.x);
            float s_x1 = floorf(draw_x + sx_sz.x);
            dl->AddRectFilled(ImVec2(s_x0, box_y0 + 3.0f), ImVec2(s_x1, box_y1 - 3.0f), AccentAlphaU32(180));
        }
        ImU32 text_col = is_active ? TextU32() : TextU32();
        DrawOutlinedText(dl, f, fs, ImVec2(draw_x, draw_y), text_col, border_dark, buf);
        if (is_active)
        {
            char tmp[128];
            int cp = s_cursor_pos < (int)sizeof(tmp) - 1 ? s_cursor_pos : (int)sizeof(tmp) - 1;
            memcpy(tmp, buf, (size_t)cp);
            tmp[cp] = '\0';
            ImVec2 cursor_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, tmp);
            float cursor_x = floorf(draw_x + cursor_sz.x);
            bool blink = (fmodf((float)ImGui::GetTime() * 2.0f, 2.0f) < 1.2f);
            if (blink)
            {
                dl->AddRectFilled(ImVec2(cursor_x, box_y0 + 4.0f), ImVec2(cursor_x + 1.0f, box_y1 - 4.0f), TextU32());
            }
        }
        dl->PopClipRect();
    }
    float total_h = has_label ? kLabeledBoxH : 20.0f;
    bool is_total_hov = (m.x >= x0 && m.x <= x1 && m.y >= pos.y - kLabelTextRise && m.y <= pos.y + total_h);
    if (text_modal) return false;
    return is_total_hov || is_active;
}
inline bool DrawDropdown(const char* id, ImDrawList* dl, ImVec2 pos, float width, int* selected_mask, const char* const items[], int items_count, const char* label = nullptr, ImFont* font = nullptr, float font_size = 13.0f)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float x1 = floorf(pos.x + width);
    bool has_label = (label && label[0] != '\0');
    float box_y0 = has_label ? floorf(pos.y + kLabelBandH) : floorf(pos.y);
    float box_h = 20.0f;
    float box_y1 = box_y0 + box_h;
    bool raw_dd_hov = (m.x >= x0 && m.x <= x1 && m.y >= box_y0 && m.y <= box_y1);
    char popup_id[64];
    snprintf(popup_id, sizeof(popup_id), "##pop_%s", id);
    bool popup_open = ImGui::IsPopupOpen(popup_id);
    bool dd_modal = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) && !popup_open && !g_inQuestionPopup;
    bool is_box_hov = raw_dd_hov && !dd_modal;
    if (is_box_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if (is_box_hov && ImGui::IsMouseClicked(0) && !popup_open)
    {
        ImGui::OpenPopup(popup_id);
        UiClick();
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    if (has_label)
    {
        float text_y = floorf(pos.y - kLabelTextRise);
        ImU32 col_lbl = popup_open ? TextU32() : TextDimU32();
        DrawOutlinedText(dl, f, fs, ImVec2(x0, text_y), col_lbl, border_dark, label);
    }
    DrawPixelBox(dl, ImVec2(x0, box_y0), ImVec2(x1, box_y1),
        popup_open ? CtrlHovU32() : CtrlBgU32(), BorderU32(), border_dark);
    char display_buf[128] = "";
    int count = 0;
    int first_idx = -1;
    int second_idx = -1;
    int third_idx = -1;
    for (int i = 0; i < items_count; i++)
    {
        if (*selected_mask & (1 << i))
        {
            count++;
            if (count == 1)
            {
                first_idx = i;
            }
            else if (count == 2)
            {
                second_idx = i;
            }
            else if (count == 3)
            {
                third_idx = i;
            }
        }
    }
    if (count == 0)
    {
        snprintf(display_buf, sizeof(display_buf), "-");
    }
    else if (count == 1)
    {
        snprintf(display_buf, sizeof(display_buf), "%s", items[first_idx]);
    }
    else if (count == 2)
    {
        snprintf(display_buf, sizeof(display_buf), "%s, %s", items[first_idx], items[second_idx]);
    }
    else if (count == 3)
    {
        snprintf(display_buf, sizeof(display_buf), "%s, %s, %s", items[first_idx], items[second_idx], items[third_idx]);
    }
    else
    {
        snprintf(display_buf, sizeof(display_buf), "%s, %s, %s, ...", items[first_idx], items[second_idx], items[third_idx]);
    }
    ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, display_buf);
    float tx = x0 + 8.0f;
    float ty = floorf(box_y0 + (box_h - text_sz.y) * 0.5f);
    dl->PushClipRect(ImVec2(tx, box_y0 + 2.0f), ImVec2(x1 - 18.0f, box_y1 - 2.0f), true);
    DrawOutlinedText(dl, f, fs, ImVec2(tx, ty), popup_open ? TextU32() : TextDimU32(), border_dark, display_buf);
    dl->PopClipRect();
    float ax = floorf(x1 - 10.0f);
    float ay = floorf(box_y0 + (box_h - 3.0f) * 0.5f);
    ImU32 arrow_col = popup_open ? TextU32() : TextDimU32();
    dl->AddRectFilled(ImVec2(ax - 2.0f, ay), ImVec2(ax + 3.0f, ay + 1.0f), arrow_col);
    dl->AddRectFilled(ImVec2(ax - 1.0f, ay + 1.0f), ImVec2(ax + 2.0f, ay + 2.0f), arrow_col);
    dl->AddRectFilled(ImVec2(ax, ay + 2.0f), ImVec2(ax + 1.0f, ay + 3.0f), arrow_col);
    float item_h = 18.0f;
    float pad_y = 2.0f;
    float popup_h = pad_y * 2.0f + (float)items_count * item_h;
    float popup_w = width;
    ImVec2 popup_pos(x0, floorf(box_y1 + 2.0f));
    ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(popup_w, popup_h), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(30.0f / 255.0f, 36.0f / 255.0f, 48.0f / 255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::BeginPopup(popup_id, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* pdl = ImGui::GetWindowDrawList();
        ImVec2 w_min = ImGui::GetWindowPos();
        w_min.x = floorf(w_min.x);
        w_min.y = floorf(w_min.y);
        ImVec2 w_max = ImVec2(w_min.x + popup_w, w_min.y + popup_h);
        DrawPixelBox(pdl, w_min, w_max, CtrlBgU32(), BorderU32(), border_dark);
        ImGui::Dummy(ImVec2(popup_w, popup_h));
        for (int i = 0; i < items_count; i++)
        {
            float it_y0 = floorf(w_min.y + pad_y + (float)i * item_h);
            float it_y1 = it_y0 + item_h;
            float it_x0 = w_min.x + 2.0f;
            float it_x1 = w_max.x - 2.0f;
            bool is_selected = ((*selected_mask & (1 << i)) != 0);
            bool it_hov = (m.x >= it_x0 && m.x <= it_x1 && m.y >= it_y0 && m.y <= it_y1);
            if (it_hov)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                pdl->AddRectFilled(ImVec2(it_x0, it_y0), ImVec2(it_x1, it_y1), is_selected ? CtrlHovU32() : CtrlBgU32());
                if (ImGui::IsMouseClicked(0))
                {
                    *selected_mask ^= (1 << i);
                    UiClick();
                }
            }
            else if (is_selected)
            {
                pdl->AddRectFilled(ImVec2(it_x0, it_y0), ImVec2(it_x1, it_y1), CtrlHovU32());
            }
            ImU32 it_text_col = (is_selected || it_hov) ? TextU32() : TextDimU32();
            ImVec2 it_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, items[i]);
            float it_tx = it_x0 + 8.0f;
            float it_ty = floorf(it_y0 + (item_h - it_sz.y) * 0.5f);
            DrawOutlinedText(pdl, f, fs, ImVec2(it_tx, it_ty), it_text_col, border_dark, items[i]);
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    popup_open = ImGui::IsPopupOpen(popup_id);
    float total_h = has_label ? kLabeledBoxH : 20.0f;
    bool is_total_hov = (m.x >= x0 && m.x <= x1 && m.y >= pos.y - kLabelTextRise && m.y <= pos.y + total_h);
    return popup_open || (!dd_modal && is_total_hov);
}
inline bool DrawDropdownSingle(const char* id, ImDrawList* dl, ImVec2 pos, float width, int* selected_idx, const char* const items[], int items_count, const char* label = nullptr, ImFont* font = nullptr, float font_size = 13.0f)
{
    int mask = (*selected_idx >= 0 && *selected_idx < items_count) ? (1 << *selected_idx) : 0;
    int before = mask;
    bool hov = DrawDropdown(id, dl, pos, width, &mask, items, items_count, label, font, font_size);
    if (mask != before)
    {
        int added = mask & ~before;
        int sel = -1;
        if (added != 0) { for (int i = 0; i < items_count; i++) if (added & (1 << i)) { sel = i; break; } }
        else { for (int i = 0; i < items_count; i++) if (mask & (1 << i)) { sel = i; break; } }
        if (sel < 0) sel = 0;
        *selected_idx = sel;
    }
    if (*selected_idx < 0 || *selected_idx >= items_count) *selected_idx = 0;
    return hov;
}
enum QuestionItemType
{
    QITEM_CHECKBOX = 0,
    QITEM_SLIDER,
    QITEM_SLIDERF,
    QITEM_DROPDOWN,
    QITEM_DROPDOWNSINGLE,
    QITEM_COLORPICKER,
    QITEM_COLORPICKER_PAIR,
    QITEM_KEYBIND,
    QITEM_TEXTINPUT,
    QITEM_LABEL,
    QITEM_SUBMENU
};
struct QuestionItem
{
    QuestionItemType type;
    const char* id;
    const char* label;
    bool* cond;
    bool* bool_val;
    int* int_val;
    int min_val;
    int max_val;
    const char* format;
    float* float_val;
    float float_min;
    float float_max;
    int* dd_mask;
    const char* const* dd_items;
    int dd_count;
    int* dd_sel;
    ImVec4* col_val;
    bool* outline_opt;
    ImVec4* outline_col;
    const char* id2;
    ImVec4* col_val2;
    bool* cond2;
    int* key_val;
    int* mode_val;
    char* text_buf;
    int text_size;
    const char* hint;
    const QuestionItem* sub_items;
    int sub_count;
    bool* open_val;
};
inline QuestionItem MakeQCheckbox(const char* label, bool* val, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_CHECKBOX;
    it.label = label;
    it.bool_val = val;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQCheckboxColor(const char* id, const char* label, bool* val, ImVec4* col, bool* outline_opt = nullptr, ImVec4* outline_col = nullptr, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_CHECKBOX;
    it.id = id;
    it.label = label;
    it.bool_val = val;
    it.col_val = col;
    it.outline_opt = outline_opt;
    it.outline_col = outline_col;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQCheckboxColor(const char* label, bool* val, ImVec4* col, bool* outline_opt = nullptr, ImVec4* outline_col = nullptr, bool* cond = nullptr)
{
    return MakeQCheckboxColor(nullptr, label, val, col, outline_opt, outline_col, cond);
}
inline QuestionItem MakeQCheckboxColorPair(const char* id, const char* label, bool* val, ImVec4* col, const char* id2, ImVec4* col2, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_CHECKBOX;
    it.id = id;
    it.label = label;
    it.bool_val = val;
    it.col_val = col;
    it.id2 = id2;
    it.col_val2 = col2;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQCheckboxColorPair(const char* label, bool* val, ImVec4* col, ImVec4* col2, bool* cond = nullptr)
{
    return MakeQCheckboxColorPair(nullptr, label, val, col, nullptr, col2, cond);
}
inline QuestionItem MakeQCheckboxKeybind(const char* id, const char* label, bool* val, int* key, int* mode, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_CHECKBOX;
    it.id = id;
    it.label = label;
    it.bool_val = val;
    it.key_val = key;
    it.mode_val = mode;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQSubmenu(const char* id, const char* label, const QuestionItem* sub_items, int sub_count, bool* open_val, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_SUBMENU;
    it.id = id;
    it.label = label;
    it.sub_items = sub_items;
    it.sub_count = sub_count;
    it.open_val = open_val;
    it.cond = cond;
    return it;
}
inline float QSubmenuContentHeight(const QuestionItem* items, int count)
{
    float h = 0.0f;
    if (!items || count <= 0)
        return h;
    for (int i = 0; i < count; ++i)
    {
        if (items[i].cond && !*items[i].cond)
            continue;
        if (items[i].type == QITEM_CHECKBOX || items[i].type == QITEM_COLORPICKER ||
            items[i].type == QITEM_COLORPICKER_PAIR || items[i].type == QITEM_KEYBIND ||
            items[i].type == QITEM_LABEL)
            h += 20.0f;
        else if (items[i].type == QITEM_TEXTINPUT)
            h += (items[i].label && items[i].label[0] != '\0') ? 37.0f : 24.0f;
        else if (items[i].type == QITEM_SLIDER || items[i].type == QITEM_SLIDERF)
            h += (items[i].label && items[i].label[0] != '\0') ? (kLabeledCtrlH + 4.0f) : 17.0f;
        else if (items[i].type == QITEM_DROPDOWN || items[i].type == QITEM_DROPDOWNSINGLE)
            h += (items[i].label && items[i].label[0] != '\0') ? (kLabeledBoxH + 4.0f) : 24.0f;
        else
            h += 20.0f;
    }
    return h;
}
inline QuestionItem MakeQSlider(const char* id, const char* label, int* val, int min_v, int max_v, const char* fmt = "%d", bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_SLIDER;
    it.id = id;
    it.label = label;
    it.int_val = val;
    it.min_val = min_v;
    it.max_val = max_v;
    it.format = fmt;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQSliderFloat(const char* id, const char* label, float* val, float min_v, float max_v, const char* fmt = "%.1f", bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_SLIDERF;
    it.id = id;
    it.label = label;
    it.float_val = val;
    it.float_min = min_v;
    it.float_max = max_v;
    it.format = fmt;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQDropdown(const char* id, const char* label, int* mask, const char* const items[], int count, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_DROPDOWN;
    it.id = id;
    it.label = label;
    it.dd_mask = mask;
    it.dd_items = items;
    it.dd_count = count;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQDropdownSingle(const char* id, const char* label, int* sel, const char* const items[], int count, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_DROPDOWNSINGLE;
    it.id = id;
    it.label = label;
    it.dd_sel = sel;
    it.dd_items = items;
    it.dd_count = count;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQColorPicker(const char* id, const char* label, ImVec4* col, bool* outline_opt, ImVec4* outline_col, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_COLORPICKER;
    it.id = id;
    it.label = label;
    it.col_val = col;
    it.outline_opt = outline_opt;
    it.outline_col = outline_col;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQColorPicker(const char* id, const char* label, ImVec4* col, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_COLORPICKER;
    it.id = id;
    it.label = label;
    it.col_val = col;
    it.outline_opt = nullptr;
    it.outline_col = nullptr;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQColorPickerPair(const char* id, const char* label, ImVec4* col, bool* cond, const char* id2, ImVec4* col2, bool* cond2 = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_COLORPICKER_PAIR;
    it.id = id;
    it.label = label;
    it.col_val = col;
    it.cond = cond;
    it.id2 = id2;
    it.col_val2 = col2;
    it.cond2 = cond2;
    return it;
}
inline QuestionItem MakeQColorPickerPair(const char* label, ImVec4* col, const char* label2, ImVec4* col2, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_COLORPICKER_PAIR;
    it.id = nullptr;
    it.label = label;
    it.col_val = col;
    it.cond = cond;
    it.id2 = nullptr;
    it.col_val2 = col2;
    it.cond2 = cond;
    return it;
}
inline QuestionItem MakeQKeybind(const char* id, const char* label, int* key, int* mode, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_KEYBIND;
    it.id = id;
    it.label = label;
    it.key_val = key;
    it.mode_val = mode;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQTextInput(const char* id, const char* label, char* buf, int buf_size, const char* hint = nullptr, bool* cond = nullptr)
{
    QuestionItem it{};
    it.type = QITEM_TEXTINPUT;
    it.id = id;
    it.label = label;
    it.text_buf = buf;
    it.text_size = buf_size;
    it.hint = hint;
    it.cond = cond;
    return it;
}
inline QuestionItem MakeQLabel(const char* text)
{
    QuestionItem it{};
    it.type = QITEM_LABEL;
    it.label = text;
    return it;
}
struct QuestionOption
{
    bool* val = nullptr;
    const char* label = nullptr;
    ImVec4* col = nullptr;
    bool* outline_opt = nullptr;
    ImVec4* outline_col = nullptr;
    const char* id = nullptr;
    ImVec4* col2 = nullptr;
    const char* id2 = nullptr;
    bool* cond = nullptr;
    QuestionOption() = default;
    QuestionOption(bool* v, const char* lbl, ImVec4* c = nullptr, bool* out_opt = nullptr, ImVec4* out_col = nullptr, const char* i = nullptr, bool* cn = nullptr)
        : val(v), label(lbl), col(c), outline_opt(out_opt), outline_col(out_col), id(i), col2(nullptr), id2(nullptr), cond(cn) {}
    QuestionOption(bool* v, const char* lbl, ImVec4* c1, ImVec4* c2, const char* i1 = nullptr, const char* i2 = nullptr, bool* cn = nullptr)
        : val(v), label(lbl), col(c1), outline_opt(nullptr), outline_col(nullptr), id(i1), col2(c2), id2(i2), cond(cn) {}
};
inline const char* kBannableWarnText = "This feature might be bannable for your current game or kick you.";
inline bool DrawWarnIndicator(const char* id, ImDrawList* dl, ImVec2 right_edge, const char* warn_text, ImFont* font = nullptr, float font_size = 13.0f)
{
    (void)id;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    const char* btn_text = "[ ! ]";
    ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, btn_text);
    float right_x = floorf(right_edge.x);
    float text_x = floorf(right_x - text_sz.x);
    float y0 = floorf(right_edge.y);
    float text_y = floorf(y0 + (14.0f - text_sz.y) * 0.5f);
    float hit_x0 = text_x - 3.0f;
    float hit_x1 = right_x + 3.0f;
    float hit_y0 = y0 - 2.0f;
    float hit_y1 = y0 + 16.0f;
    bool is_hov = (m.x >= hit_x0 && m.x <= hit_x1 && m.y >= hit_y0 && m.y <= hit_y1);
    const ImU32 border_dark = IM_COL32(0, 0, 0, 255);
    ImU32 text_col = is_hov ? IM_COL32(255, 200, 60, 255) : IM_COL32(190, 140, 30, 255);
    DrawOutlinedText(dl, f, fs, ImVec2(text_x, text_y), text_col, border_dark, btn_text);
    if (is_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(24.0f / 255.0f, 24.0f / 255.0f, 24.0f / 255.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::BeginTooltip())
        {
            ImDrawList* tdl = ImGui::GetWindowDrawList();
            ImVec2 tp = ImGui::GetCursorScreenPos();
            const ImU32 tdark = IM_COL32(0, 0, 0, 255);
            const ImU32 tlight = BorderU32();
            const char* title = "Warning";
            const char* msg = warn_text ? warn_text : kBannableWarnText;
            ImVec2 tsz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, title);
            ImVec2 msz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, msg);
            float box_w = (std::max)(tsz.x, msz.x) + 24.0f;
            float box_h = tsz.y + 2.0f + msz.y + 12.0f;
            ImVec2 b0 = ImVec2(floorf(tp.x), floorf(tp.y));
            ImVec2 b1 = ImVec2(floorf(tp.x + box_w), floorf(tp.y + box_h));
            DrawPixelBox(tdl, b0, b1, GroupU32(), tlight, tdark);
            DrawOutlinedText(tdl, f, fs, ImVec2(b0.x + 12.0f, b0.y + 6.0f), IM_COL32(255, 200, 60, 255), tdark, title);
            DrawOutlinedText(tdl, f, fs, ImVec2(b0.x + 12.0f, b0.y + 6.0f + tsz.y + 2.0f), TextU32(), tdark, msg);
            ImGui::Dummy(ImVec2(box_w, box_h));
            ImGui::EndTooltip();
        }
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }
    return is_hov;
}
inline bool DrawQuestionButton(const char* id, ImDrawList* dl, ImVec2 right_edge, const QuestionItem* items, int item_count, ImFont* font = nullptr, float font_size = 13.0f, int depth = 0)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    char popup_id[64];
    snprintf(popup_id, sizeof(popup_id), "##qb_pop_%s", id);
    const char* btn_text = "[ ? ]";
    ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, btn_text);
    float right_x = floorf(right_edge.x);
    float text_x = floorf(right_x - text_sz.x);
    float y0 = floorf(right_edge.y);
    float text_y = floorf(y0 + (14.0f - text_sz.y) * 0.5f);
    float hit_x0 = text_x - 3.0f;
    float hit_x1 = right_x + 3.0f;
    float hit_y0 = y0 - 2.0f;
    float hit_y1 = y0 + 16.0f;
    bool is_hov = (m.x >= hit_x0 && m.x <= hit_x1 && m.y >= hit_y0 && m.y <= hit_y1);
    bool own_popup = ImGui::IsPopupOpen(popup_id);
    bool sub_any = false;
    if ((ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) && !own_popup && depth == 0) || g_activeBindId != nullptr)
        is_hov = false;
    if (is_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsMouseClicked(0))
        {
            if (!own_popup)
            {
                ImGui::OpenPopup(popup_id);
                UiClick();
            }
        }
    }
    const ImU32 border_dark = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    ImU32 text_col = own_popup ? AccentU32() : (is_hov ? TextU32() : TextDimU32());
    DrawOutlinedText(dl, f, fs, ImVec2(text_x, text_y), text_col, border_dark, btn_text);
    float pad_x = 12.0f;
    float pad_y = 6.0f;
    bool has_slider_or_dd = false;
    bool has_picker_or_kb = false;
    bool has_picker_pair = false;
    float max_label_w = 0.0f;
    float pop_h = pad_y * 2.0f;
    for (int i = 0; i < item_count; i++)
    {
        if (items[i].type == QITEM_SLIDER || items[i].type == QITEM_SLIDERF || items[i].type == QITEM_DROPDOWN || items[i].type == QITEM_DROPDOWNSINGLE || items[i].type == QITEM_TEXTINPUT)
        {
            has_slider_or_dd = true;
        }
        else if (items[i].type == QITEM_COLORPICKER || items[i].type == QITEM_COLORPICKER_PAIR || items[i].type == QITEM_KEYBIND)
        {
            has_picker_or_kb = true;
            if (items[i].type == QITEM_COLORPICKER_PAIR || (items[i].outline_opt && items[i].outline_col))
                has_picker_pair = true;
        }
        else if (items[i].type == QITEM_CHECKBOX && (items[i].col_val || items[i].key_val))
        {
            has_picker_or_kb = true;
            if (items[i].col_val2 || (items[i].outline_opt && items[i].outline_col))
                has_picker_pair = true;
        }
        else if (items[i].type == QITEM_SUBMENU)
        {
            has_picker_or_kb = true;
            if (items[i].col_val)
                has_picker_pair = true;
        }
        ImVec2 opt_ts = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, items[i].label ? items[i].label : "");
        if (opt_ts.x > max_label_w) max_label_w = opt_ts.x;
        if (items[i].type == QITEM_SUBMENU && items[i].sub_items && items[i].sub_count > 0)
        {
            for (int k = 0; k < items[i].sub_count; ++k)
            {
                const char* sl = items[i].sub_items[k].label;
                if (!sl || !sl[0])
                    continue;
                ImVec2 sts = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, sl);
                float need = sts.x + 10.0f;
                if (need > max_label_w) max_label_w = need;
            }
        }
        bool pair_visible = items[i].type != QITEM_COLORPICKER_PAIR ||
            ((!items[i].cond || *items[i].cond) || (!items[i].cond2 || *items[i].cond2));
        if ((items[i].cond && !*items[i].cond && items[i].type != QITEM_COLORPICKER_PAIR) || !pair_visible)
        {
            continue;
        }
        if (items[i].type == QITEM_CHECKBOX)
        {
            pop_h += 20.0f;
        }
        else if (items[i].type == QITEM_SUBMENU)
        {
            pop_h += 20.0f;
            if ((!items[i].cond || *items[i].cond) && items[i].open_val && *items[i].open_val)
                pop_h += QSubmenuContentHeight(items[i].sub_items, items[i].sub_count);
        }
        else if (items[i].type == QITEM_SLIDER || items[i].type == QITEM_SLIDERF)
        {
            pop_h += (items[i].label && items[i].label[0] != '\0') ? (kLabeledCtrlH + 4.0f) : 17.0f;
        }
        else if (items[i].type == QITEM_DROPDOWN || items[i].type == QITEM_DROPDOWNSINGLE)
        {
            pop_h += (items[i].label && items[i].label[0] != '\0') ? (kLabeledBoxH + 4.0f) : 24.0f;
        }
        else if (items[i].type == QITEM_COLORPICKER || items[i].type == QITEM_COLORPICKER_PAIR)
        {
            pop_h += 20.0f;
        }
        else if (items[i].type == QITEM_KEYBIND || items[i].type == QITEM_LABEL)
        {
            pop_h += 20.0f;
        }
        else if (items[i].type == QITEM_TEXTINPUT)
        {
            pop_h += (items[i].label && items[i].label[0] != '\0') ? 37.0f : 24.0f;
        }
    }
    float pop_w = 100.0f;
    if (has_slider_or_dd)
    {
        pop_w = (std::max)(230.0f, pad_x * 2.0f + max_label_w + 30.0f);
    }
    else if (has_picker_or_kb)
    {
        float right_reserve = has_picker_pair ? 48.0f : 28.0f;
        float needed_w = pad_x * 2.0f + 14.0f + kCheckboxLabelGap + max_label_w + right_reserve;
        pop_w = (std::max)(170.0f, needed_w);
    }
    else
    {
        pop_w = floorf(pad_x + 14.0f + kCheckboxLabelGap + max_label_w + 14.0f);
        if (pop_w < 100.0f) pop_w = 100.0f;
    }
    ImVec2 pop_pos(floorf(right_x - pop_w), floorf(y0 + 18.0f));
    ImGui::SetNextWindowPos(pop_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(pop_w, pop_h), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(24.0f / 255.0f, 24.0f / 255.0f, 24.0f / 255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::BeginPopup(popup_id, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar))
    {
        ImDrawList* pdl = ImGui::GetWindowDrawList();
        ImVec2 w_min = ImGui::GetWindowPos();
        w_min.x = floorf(w_min.x);
        w_min.y = floorf(w_min.y);
        ImVec2 w_max = ImVec2(w_min.x + pop_w, w_min.y + pop_h);
        DrawPixelBox(pdl, w_min, w_max, GroupU32(), border_light, border_dark);
        ImGui::Dummy(ImVec2(pop_w, pop_h));
        float cur_y = w_min.y + pad_y;
        float item_w = pop_w - pad_x * 2.0f;
        g_inQuestionPopup = true;
        bool is_child_popup_active = false;
        for (int i = 0; i < item_count; i++)
        {
            char auto_cpid[64];
            snprintf(auto_cpid, sizeof(auto_cpid), "##q_cp_%s_%d", id ? id : "qb", i);
            char auto_cpid2[64];
            snprintf(auto_cpid2, sizeof(auto_cpid2), "##q_cp2_%s_%d", id ? id : "qb", i);
            const char* cp1_id = items[i].id ? items[i].id : auto_cpid;
            const char* cp2_id = items[i].id2 ? items[i].id2 : auto_cpid2;
            char ctx_id1[64];
            snprintf(ctx_id1, sizeof(ctx_id1), "%s##cp_ctx", cp1_id);
            char ctx_id2[64];
            snprintf(ctx_id2, sizeof(ctx_id2), "%s##cp_ctx", cp2_id);
            char out_id1[64];
            snprintf(out_id1, sizeof(out_id1), "%s_out", cp1_id);
            if (ImGui::IsPopupOpen(cp1_id) || ImGui::IsPopupOpen(ctx_id1) || ImGui::IsPopupOpen(out_id1) ||
                ImGui::IsPopupOpen(cp2_id) || ImGui::IsPopupOpen(ctx_id2))
            {
                is_child_popup_active = true;
                break;
            }
            if (items[i].type == QITEM_SUBMENU && items[i].id)
            {
                char subpid[64];
                snprintf(subpid, sizeof(subpid), "##qb_pop_%s", items[i].id);
                if (ImGui::IsPopupOpen(subpid))
                {
                    is_child_popup_active = true;
                    break;
                }
            }
        }
        for (int i = 0; i < item_count; i++)
        {
            bool pair_show = items[i].type != QITEM_COLORPICKER_PAIR ||
                ((!items[i].cond || *items[i].cond) || (!items[i].cond2 || *items[i].cond2));
            if ((items[i].cond && !*items[i].cond && items[i].type != QITEM_COLORPICKER_PAIR) || !pair_show)
            {
                continue;
            }
            if (items[i].type == QITEM_CHECKBOX)
            {
                float cb_x = w_min.x + pad_x;
                float cb_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                DrawCheckbox(pdl, ImVec2(cb_x, cb_y), items[i].bool_val, items[i].label, f, fs, !is_child_popup_active);
                char auto_cpid[64];
                snprintf(auto_cpid, sizeof(auto_cpid), "##q_cp_%s_%d", id ? id : "qb", i);
                char auto_cpid2[64];
                snprintf(auto_cpid2, sizeof(auto_cpid2), "##q_cp2_%s_%d", id ? id : "qb", i);
                const char* cp1_id = items[i].id ? items[i].id : auto_cpid;
                const char* cp2_id = items[i].id2 ? items[i].id2 : auto_cpid2;
                if (items[i].col_val && items[i].col_val2)
                {
                    float sx = w_max.x - pad_x - 14.0f;
                    DrawColorPicker(cp1_id, pdl, ImVec2(sx - 20.0f, cb_y), ImVec2(14.0f, 14.0f), items[i].col_val, nullptr, nullptr);
                    DrawColorPicker(cp2_id, pdl, ImVec2(sx, cb_y), ImVec2(14.0f, 14.0f), items[i].col_val2, nullptr, nullptr);
                }
                else if (items[i].col_val)
                {
                    ImVec2 cp_pos(w_max.x - pad_x - 14.0f, cb_y);
                    DrawColorPicker(cp1_id, pdl, cp_pos, ImVec2(14.0f, 14.0f), items[i].col_val, items[i].outline_opt, items[i].outline_col);
                }
                if (items[i].key_val)
                {
                    char auto_kbid[64];
                    snprintf(auto_kbid, sizeof(auto_kbid), "##q_kb_%s_%d", id ? id : "qb", i);
                    DrawKeybind(items[i].id ? items[i].id : auto_kbid, pdl, ImVec2(w_max.x - pad_x, cb_y), items[i].key_val, items[i].mode_val, f, fs);
                }
                cur_y += 20.0f;
            }
            else if (items[i].type == QITEM_SUBMENU)
            {
                float row_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                if (items[i].bool_val)
                {
                    DrawCheckbox(pdl, ImVec2(w_min.x + pad_x, row_y), items[i].bool_val, items[i].label, f, fs, !is_child_popup_active);
                }
                else if (items[i].label && items[i].label[0] != '\0')
                {
                    DrawOutlinedText(pdl, f, fs, ImVec2(w_min.x + pad_x, row_y), TextU32(), border_dark, items[i].label);
                }
                bool is_open = items[i].open_val && *items[i].open_val;
                const char* tg_text = is_open ? "[ - ]" : "[ + ]";
                ImVec2 tg_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, tg_text);
                float sw_right = floorf(w_max.x - pad_x);
                float tg_right = sw_right;
                if (items[i].col_val)
                    tg_right = floorf(sw_right - 14.0f - 6.0f);
                float tg_x = floorf(tg_right - tg_sz.x);
                float tg_y = floorf(row_y + (14.0f - tg_sz.y) * 0.5f);
                bool tg_hov = (m.x >= tg_x - 3.0f && m.x <= tg_right + 3.0f && tg_y - 2.0f <= m.y && m.y <= tg_y + 16.0f);
                if (is_child_popup_active || g_activeBindId != nullptr)
                    tg_hov = false;
                if (tg_hov)
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                    if (ImGui::IsMouseClicked(0) && items[i].open_val)
                    {
                        *items[i].open_val = !*items[i].open_val;
                        UiClick();
                    }
                }
                DrawOutlinedText(pdl, f, fs, ImVec2(tg_x, tg_y), tg_hov ? TextU32() : TextDimU32(), border_dark, tg_text);
                if (items[i].col_val)
                {
                    char auto_subcp[64];
                    snprintf(auto_subcp, sizeof(auto_subcp), "##q_subcp_%s", items[i].id ? items[i].id : "qb");
                    DrawColorPicker(auto_subcp, pdl, ImVec2(sw_right - 14.0f, row_y), ImVec2(14.0f, 14.0f), items[i].col_val, items[i].outline_opt, items[i].outline_col);
                }
                cur_y += 20.0f;
                if (is_open && items[i].sub_items && items[i].sub_count > 0)
                {
                    int tl_rows = 0;
                    for (int c = 0; c < items[i].sub_count; ++c)
                    {
                        const QuestionItem& s = items[i].sub_items[c];
                        if (s.cond && !*s.cond)
                            continue;
                        if (s.type == QITEM_CHECKBOX || s.type == QITEM_COLORPICKER)
                            ++tl_rows;
                    }
                    if (tl_rows > 0)
                    {
                        float tl_x = floorf(w_min.x + pad_x + 10.0f - 6.0f);
                        float tl_top = floorf(cur_y + 3.0f);
                        float tl_bot = floorf(cur_y + (float)(tl_rows - 1) * 20.0f + 17.0f);
                        pdl->AddLine(ImVec2(tl_x - 2.0f, tl_top), ImVec2(tl_x - 2.0f, tl_bot), border_dark, 1.0f);
                        pdl->AddLine(ImVec2(tl_x, tl_top), ImVec2(tl_x, tl_bot), AccentU32(), 2.0f);
                        pdl->AddLine(ImVec2(tl_x + 2.0f, tl_top), ImVec2(tl_x + 2.0f, tl_bot), border_dark, 1.0f);
                    }
                    for (int k = 0; k < items[i].sub_count; ++k)
                    {
                        const QuestionItem& sub = items[i].sub_items[k];
                        if (sub.cond && !*sub.cond)
                            continue;
                        if (sub.type == QITEM_CHECKBOX)
                        {
                            float scb_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                            DrawCheckbox(pdl, ImVec2(w_min.x + pad_x + 10.0f, scb_y), sub.bool_val, sub.label, f, fs, !is_child_popup_active);
                            if (sub.col_val)
                            {
                                char auto_scp[64];
                                snprintf(auto_scp, sizeof(auto_scp), "##q_scp_%s_%d", items[i].id ? items[i].id : "qb", k);
                                const char* scp_id = sub.id ? sub.id : auto_scp;
                                DrawColorPicker(scp_id, pdl, ImVec2(w_max.x - pad_x - 14.0f, scb_y), ImVec2(14.0f, 14.0f), sub.col_val, sub.outline_opt, sub.outline_col);
                            }
                            cur_y += 20.0f;
                        }
                        else if (sub.type == QITEM_COLORPICKER)
                        {
                            float scp_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                            if (sub.label && sub.label[0] != '\0')
                            {
                                DrawOutlinedText(pdl, f, fs, ImVec2(w_min.x + pad_x + 10.0f, scp_y), TextU32(), border_dark, sub.label);
                            }
                            char auto_scp[64];
                            snprintf(auto_scp, sizeof(auto_scp), "##q_scpp_%s_%d", items[i].id ? items[i].id : "qb", k);
                            const char* scp_id = sub.id ? sub.id : auto_scp;
                            DrawColorPicker(scp_id, pdl, ImVec2(w_max.x - pad_x - 14.0f, scp_y), ImVec2(14.0f, 14.0f), sub.col_val, sub.outline_opt, sub.outline_col);
                            cur_y += 20.0f;
                        }
                    }
                }
            }
            else if (items[i].type == QITEM_SLIDER)
            {
                float sl_x = w_min.x + pad_x;
                float sl_h = (items[i].label && items[i].label[0] != '\0') ? (kLabeledCtrlH + 4.0f) : 17.0f;
                DrawSlider(items[i].id ? items[i].id : "##q_sl", pdl, ImVec2(sl_x, cur_y), item_w, items[i].int_val, items[i].min_val, items[i].max_val, items[i].label, items[i].format ? items[i].format : "%d", f, fs);
                cur_y += sl_h;
            }
            else if (items[i].type == QITEM_SLIDERF)
            {
                float sl_x = w_min.x + pad_x;
                float sl_h = (items[i].label && items[i].label[0] != '\0') ? (kLabeledCtrlH + 4.0f) : 17.0f;
                DrawSliderFloat(items[i].id ? items[i].id : "##q_slf", pdl, ImVec2(sl_x, cur_y), item_w, items[i].float_val, items[i].float_min, items[i].float_max, items[i].label, items[i].format ? items[i].format : "%.1f", f, fs);
                cur_y += sl_h;
            }
            else if (items[i].type == QITEM_DROPDOWN)
            {
                float dd_x = w_min.x + pad_x;
                float dd_h = (items[i].label && items[i].label[0] != '\0') ? (kLabeledBoxH + 4.0f) : 24.0f;
                DrawDropdown(items[i].id ? items[i].id : "##q_dd", pdl, ImVec2(dd_x, cur_y), item_w, items[i].dd_mask, items[i].dd_items, items[i].dd_count, items[i].label, f, fs);
                cur_y += dd_h;
            }
            else if (items[i].type == QITEM_DROPDOWNSINGLE)
            {
                float dd_x = w_min.x + pad_x;
                float dd_h = (items[i].label && items[i].label[0] != '\0') ? (kLabeledBoxH + 4.0f) : 24.0f;
                DrawDropdownSingle(items[i].id ? items[i].id : "##q_dds", pdl, ImVec2(dd_x, cur_y), item_w, items[i].dd_sel, items[i].dd_items, items[i].dd_count, items[i].label, f, fs);
                cur_y += dd_h;
            }
            else if (items[i].type == QITEM_COLORPICKER)
            {
                float row_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                if (items[i].label && items[i].label[0] != '\0')
                {
                    DrawOutlinedText(pdl, f, fs, ImVec2(w_min.x + pad_x, row_y), TextU32(), border_dark, items[i].label);
                }
                char auto_cpid[64];
                snprintf(auto_cpid, sizeof(auto_cpid), "##q_cp_%s_%d", id ? id : "qb", i);
                const char* cp_id = items[i].id ? items[i].id : auto_cpid;
                ImVec2 cp_pos(w_max.x - pad_x - 14.0f, row_y);
                DrawColorPicker(cp_id, pdl, cp_pos, ImVec2(14.0f, 14.0f), items[i].col_val, items[i].outline_opt, items[i].outline_col);
                cur_y += 20.0f;
            }
            else if (items[i].type == QITEM_COLORPICKER_PAIR)
            {
                bool show1 = !items[i].cond || *items[i].cond;
                bool show2 = !items[i].cond2 || *items[i].cond2;
                float row_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                if (items[i].label && items[i].label[0] != '\0')
                {
                    DrawOutlinedText(pdl, f, fs, ImVec2(w_min.x + pad_x, row_y), TextU32(), border_dark, items[i].label);
                }
                char auto_cpid1[64];
                snprintf(auto_cpid1, sizeof(auto_cpid1), "##q_cp1_%s_%d", id ? id : "qb", i);
                char auto_cpid2[64];
                snprintf(auto_cpid2, sizeof(auto_cpid2), "##q_cp2_%s_%d", id ? id : "qb", i);
                const char* cp1_id = items[i].id ? items[i].id : auto_cpid1;
                const char* cp2_id = items[i].id2 ? items[i].id2 : auto_cpid2;
                float sx = w_max.x - pad_x - 14.0f;
                if (show1 && show2)
                {
                    DrawColorPicker(cp1_id, pdl, ImVec2(sx - 20.0f, row_y), ImVec2(14.0f, 14.0f), items[i].col_val, nullptr, nullptr);
                    DrawColorPicker(cp2_id, pdl, ImVec2(sx, row_y), ImVec2(14.0f, 14.0f), items[i].col_val2, nullptr, nullptr);
                }
                else if (show1)
                {
                    DrawColorPicker(cp1_id, pdl, ImVec2(sx, row_y), ImVec2(14.0f, 14.0f), items[i].col_val, nullptr, nullptr);
                }
                else if (show2)
                {
                    DrawColorPicker(cp2_id, pdl, ImVec2(sx, row_y), ImVec2(14.0f, 14.0f), items[i].col_val2, nullptr, nullptr);
                }
                cur_y += 20.0f;
            }
            else if (items[i].type == QITEM_KEYBIND)
            {
                float row_y = floorf(cur_y + (20.0f - 14.0f) * 0.5f);
                if (items[i].label && items[i].label[0] != '\0')
                {
                    DrawOutlinedText(pdl, f, fs, ImVec2(w_min.x + pad_x, row_y), TextU32(), border_dark, items[i].label);
                }
                char auto_kbid[64];
                snprintf(auto_kbid, sizeof(auto_kbid), "##q_kb_%s_%d", id ? id : "qb", i);
                const char* kb_id = items[i].id ? items[i].id : auto_kbid;
                DrawKeybind(kb_id, pdl, ImVec2(w_max.x - pad_x, row_y), items[i].key_val, items[i].mode_val, f, fs);
                cur_y += 20.0f;
            }
            else if (items[i].type == QITEM_LABEL)
            {
                if (items[i].label && items[i].label[0] != '\0')
                {
                    float lb_y = floorf(cur_y + (20.0f - fs) * 0.5f);
                    DrawOutlinedText(pdl, f, fs, ImVec2(w_min.x + pad_x, lb_y), TextDimU32(), border_dark, items[i].label);
                }
                cur_y += 20.0f;
            }
            else if (items[i].type == QITEM_TEXTINPUT && items[i].text_buf && items[i].text_size > 0)
            {
                char auto_tid[64];
                snprintf(auto_tid, sizeof(auto_tid), "##q_ti_%s_%d", id ? id : "qb", i);
                const char* ti_id = items[i].id ? items[i].id : auto_tid;
                DrawTextInput(ti_id, pdl, ImVec2(w_min.x + pad_x, cur_y), item_w, items[i].text_buf, items[i].text_size, items[i].label, items[i].hint, f, fs);
                cur_y += (items[i].label && items[i].label[0] != '\0') ? 37.0f : 24.0f;
            }
        }
        g_inQuestionPopup = false;
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    return is_hov || own_popup || sub_any;
}
inline bool DrawQuestionButton(const char* id, ImDrawList* dl, ImVec2 right_edge, QuestionOption* options, int option_count, ImFont* font = nullptr, float font_size = 13.0f)
{
    QuestionItem stack_items[16];
    int n = (option_count < 16) ? option_count : 16;
    for (int i = 0; i < n; i++)
    {
        char auto_cpid[64];
        snprintf(auto_cpid, sizeof(auto_cpid), "%s_qopt_%d", id ? id : "qb", i);
        if (options[i].col && options[i].col2)
        {
            char auto_cpid2[64];
            snprintf(auto_cpid2, sizeof(auto_cpid2), "%s_qopt2_%d", id ? id : "qb", i);
            stack_items[i] = MakeQCheckboxColorPair(options[i].id ? options[i].id : auto_cpid, options[i].label, options[i].val, options[i].col, options[i].id2 ? options[i].id2 : auto_cpid2, options[i].col2, options[i].cond);
        }
        else if (options[i].col)
        {
            stack_items[i] = MakeQCheckboxColor(options[i].id ? options[i].id : auto_cpid, options[i].label, options[i].val, options[i].col, options[i].outline_opt, options[i].outline_col, options[i].cond);
        }
        else
        {
            stack_items[i] = MakeQCheckbox(options[i].label, options[i].val, options[i].cond);
        }
    }
    return DrawQuestionButton(id, dl, right_edge, stack_items, n, font, font_size);
}
inline bool DrawQuestionButton(const char* id, ImDrawList* dl, ImVec2 right_edge, bool* sub_val, const char* sub_label = "Sub-option", ImVec4* col = nullptr, ImFont* font = nullptr, float font_size = 13.0f)
{
    QuestionOption opt(sub_val, sub_label, col);
    return DrawQuestionButton(id, dl, right_edge, &opt, 1, font, font_size);
}
inline float KeybindTextWidth(const char* bind_id, int key, ImFont* font = nullptr, float font_size = 13.0f)
{
    char buf[64];
    if (g_activeBindId != nullptr && bind_id && strcmp(g_activeBindId, bind_id) == 0)
        snprintf(buf, sizeof(buf), "[ ]");
    else if (key == 0)
        snprintf(buf, sizeof(buf), "[ . . . ]");
    else
        snprintf(buf, sizeof(buf), "[ %s ]", GetKeyDisplayName(key));
    ImFont* f = font ? font : ImGui::GetFont();
    return f->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf).x;
}
inline void UpdateThemeFromMask()
{
    static int s_prevTheme = -1;
    if (s_prevTheme < 0) s_prevTheme = 0;
    if (g_themeMask != s_prevTheme)
    {
        int added = g_themeMask & ~s_prevTheme;
        int sel = -1;
        if (added != 0) { for (int i = 0; i < 7; i++) if (added & (1 << i)) { sel = i; break; } }
        else { for (int i = 0; i < 7; i++) if (g_themeMask & (1 << i)) { sel = i; break; } }
        if (sel < 0) sel = 0;
        g_themeMask = (1 << sel);
        s_prevTheme = g_themeMask;
        ApplyThemePalette(sel);
    }
    if (g_themeMask == 0) { g_themeMask = (1 << 0); s_prevTheme = g_themeMask; }
}
inline constexpr float kNavTile = 32.0f;
inline constexpr float kNavPad = 6.0f;
inline constexpr float kNavGap = 6.0f;
inline constexpr float kNavTop = 12.0f;
inline void CalcNavBarRect(int width, int height, ImVec2& outMin, ImVec2& outMax)
{
    (void)height;
    float bw = kNavPad * 2.0f + kNavTile * 7.0f + kNavGap * 6.0f;
    float bh = kNavPad * 2.0f + kNavTile;
    float bx0 = floorf(((float)width - bw) * 0.5f);
    outMin = ImVec2(bx0, kNavTop);
    outMax = ImVec2(bx0 + bw, kNavTop + bh);
}
inline bool NavHitTest(int x, int y, int width, int height)
{
    ImVec2 mn, mx;
    CalcNavBarRect(width, height, mn, mx);
    return (x >= (int)mn.x && x <= (int)mx.x && y >= (int)mn.y && y <= (int)mx.y);
}
inline bool PanelHitTest(int x, int y)
{
    if (g_settingsOpen && g_setPos.x >= 0.0f && x >= (int)g_setPos.x && x <= (int)(g_setPos.x + g_setSize.x) && y >= (int)(g_setPos.y - 16.0f) && y <= (int)(g_setPos.y + g_setSize.y + 16.0f))
    {
        return true;
    }
    if (g_themeOpen && g_themePos.x >= 0.0f && x >= (int)g_themePos.x && x <= (int)(g_themePos.x + g_themeSize.x) && y >= (int)(g_themePos.y - 16.0f) && y <= (int)(g_themePos.y + g_themeSize.y + 16.0f))
    {
        return true;
    }
    if (g_keybinds && g_menuOpen && g_bindsPos.x >= 0.0f && x >= (int)g_bindsPos.x && x <= (int)(g_bindsPos.x + g_bindsSize.x) && y >= (int)g_bindsPos.y && y <= (int)(g_bindsPos.y + g_bindsSize.y))
    {
        return true;
    }
    if (g_movement && g_menuOpen && g_movementPos.x >= 0.0f && x >= (int)g_movementPos.x && x <= (int)(g_movementPos.x + g_movementSize.x) && y >= (int)g_movementPos.y && y <= (int)(g_movementPos.y + g_movementSize.y))
    {
        return true;
    }
    if (g_internalDebug && g_menuOpen && g_debugPos.x >= 0.0f && x >= (int)g_debugPos.x && x <= (int)(g_debugPos.x + g_debugSize.x) && y >= (int)g_debugPos.y && y <= (int)(g_debugPos.y + g_debugSize.y))
    {
        return true;
    }
    if (g_explorerOpen && g_explorerPos.x >= 0.0f && x >= (int)g_explorerPos.x && x <= (int)(g_explorerPos.x + g_explorerSize.x) && y >= (int)g_explorerPos.y && y <= (int)(g_explorerPos.y + g_explorerSize.y))
    {
        return true;
    }
    if (g_playerPreviewOpen && g_playerPreviewPos.x >= 0.0f && x >= (int)g_playerPreviewPos.x && x <= (int)(g_playerPreviewPos.x + g_playerPreviewSize.x) && y >= (int)g_playerPreviewPos.y && y <= (int)(g_playerPreviewPos.y + g_playerPreviewSize.y))
    {
        return true;
    }
    if (g_playersOpen && g_playersPos.x >= 0.0f && x >= (int)g_playersPos.x && x <= (int)(g_playersPos.x + g_playersSize.x) && y >= (int)g_playersPos.y && y <= (int)(g_playersPos.y + g_playersSize.y))
    {
        return true;
    }
    if (g_luaOpen && g_luaPos.x >= 0.0f && x >= (int)g_luaPos.x && x <= (int)(g_luaPos.x + g_luaSize.x) && y >= (int)g_luaPos.y && y <= (int)(g_luaPos.y + g_luaSize.y))
    {
        return true;
    }
    return false;
}
ID3D11ShaderResourceView* LoadIconTexture(const unsigned char* png, int len);
inline void RenderWatermark(ImDrawList* dl, int width, float fps)
{
    if (!g_watermark)
    {
        return;
    }
    char buf[128] = "thenwefuckin";
    int len = (int)strlen(buf);
    static float s_fpsSmooth = -1.0f;
    static float s_fpsLastTime = 0.0f;
    float now_t = (float)ImGui::GetTime();
    float sdt = (s_fpsLastTime > 0.0f) ? (now_t - s_fpsLastTime) : 0.016f;
    if (sdt < 0.0001f) sdt = 0.0001f;
    if (sdt > 0.5f) sdt = 0.5f;
    s_fpsLastTime = now_t;
    float alpha = 1.0f - expf(-sdt / 0.6f);
    if (s_fpsSmooth < 0.0f) s_fpsSmooth = fps;
    else s_fpsSmooth += (fps - s_fpsSmooth) * alpha;
    if (g_watermarkFps && len < (int)sizeof(buf) - 1)
    {
        int disp_fps = (int)(s_fpsSmooth + 0.5f);
        if (disp_fps < 10) disp_fps = (int)((s_fpsSmooth + 0.5f) / 10.0f) * 10;
        else disp_fps = (disp_fps / 10) * 10;
        if (disp_fps < 0) disp_fps = 0;
        if (disp_fps > 9990) disp_fps = 9990;
        len += snprintf(buf + len, sizeof(buf) - len, " | %d fps", disp_fps);
    }
    if (g_watermarkPlayers && len < (int)sizeof(buf) - 1)
    {
        int n = 0;
        if (thenwefuckin::g_proc && thenwefuckin::g_proc->connected())
            n = (int)thenwefuckin::g_ctx.player_list().size();
        if (n < 0) n = 0;
        if (n > 999) n = 999;
        len += snprintf(buf + len, sizeof(buf) - len, " | %d players", n);
    }
    if (g_watermarkDate && len < (int)sizeof(buf) - 1)
    {
        time_t now = time(nullptr);
        struct tm tm_buf;
        localtime_s(&tm_buf, &now);
        len += snprintf(buf + len, sizeof(buf) - len, " | %02d.%02d.%04d", tm_buf.tm_mday, tm_buf.tm_mon + 1, tm_buf.tm_year + 1900);
    }
    ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
    char tmpl[128] = "thenwefuckin";
    int tlen = (int)strlen(tmpl);
    if (g_watermarkFps)
    {
        tlen += snprintf(tmpl + tlen, sizeof(tmpl) - tlen, " | 9999 fps");
    }
    if (g_watermarkPlayers)
    {
        tlen += snprintf(tmpl + tlen, sizeof(tmpl) - tlen, " | 999 players");
    }
    if (g_watermarkDate)
    {
        tlen += snprintf(tmpl + tlen, sizeof(tmpl) - tlen, " | 00.00.0000");
    }
    ImVec2 ref_sz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, tmpl);
    float bw = floorf(ref_sz.x + 16.0f);
    float bh = 20.0f;
    float bx1 = (float)width - 12.0f;
    float bx0 = floorf(bx1 - bw);
    float by0 = 12.0f;
    float by1 = by0 + bh;
    DrawPixelBox(dl, ImVec2(bx0, by0), ImVec2(bx1, by1), CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    float tx = floorf(bx0 + 8.0f);
    float ty = floorf(by0 + (bh - ref_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, 13.0f, ImVec2(tx, ty), TextU32(), IM_COL32(0, 0, 0, 255), buf);
}
inline int ImGuiKeyToVirtualKey(int key)
{
    if (key >= ImGuiKey_0 && key <= ImGuiKey_9) return '0' + (key - ImGuiKey_0);
    if (key >= ImGuiKey_A && key <= ImGuiKey_Z) return 'A' + (key - ImGuiKey_A);
    if (key >= ImGuiKey_F1 && key <= ImGuiKey_F12) return VK_F1 + (key - ImGuiKey_F1);
    if (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9) return VK_NUMPAD0 + (key - ImGuiKey_Keypad0);
    if (key == ImGuiKey_MouseLeft) return VK_LBUTTON;
    if (key == ImGuiKey_MouseRight) return VK_RBUTTON;
    if (key == ImGuiKey_MouseMiddle) return VK_MBUTTON;
    if (key == ImGuiKey_MouseX1) return VK_XBUTTON1;
    if (key == ImGuiKey_MouseX2) return VK_XBUTTON2;
    if (key == ImGuiKey_Tab) return VK_TAB;
    if (key == ImGuiKey_LeftArrow) return VK_LEFT;
    if (key == ImGuiKey_RightArrow) return VK_RIGHT;
    if (key == ImGuiKey_UpArrow) return VK_UP;
    if (key == ImGuiKey_DownArrow) return VK_DOWN;
    if (key == ImGuiKey_PageUp) return VK_PRIOR;
    if (key == ImGuiKey_PageDown) return VK_NEXT;
    if (key == ImGuiKey_Home) return VK_HOME;
    if (key == ImGuiKey_End) return VK_END;
    if (key == ImGuiKey_Insert) return VK_INSERT;
    if (key == ImGuiKey_Delete) return VK_DELETE;
    if (key == ImGuiKey_Backspace) return VK_BACK;
    if (key == ImGuiKey_Space) return VK_SPACE;
    if (key == ImGuiKey_Enter || key == ImGuiKey_KeypadEnter) return VK_RETURN;
    if (key == ImGuiKey_Escape) return VK_ESCAPE;
    if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift) return VK_SHIFT;
    if (key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl) return VK_CONTROL;
    if (key == ImGuiKey_LeftAlt || key == ImGuiKey_RightAlt) return VK_MENU;
    if (key == ImGuiKey_CapsLock) return VK_CAPITAL;
    return 0;
}
inline bool IsKeybindDown(int key)
{
    if (key <= 0) return false;
    int vk = ImGuiKeyToVirtualKey(key);
    if (vk != 0)
    {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
    }
    if (key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END)
    {
        return ImGui::IsKeyDown((ImGuiKey)key);
    }
    return false;
}
struct MenuKeyToggle
{
    bool toggled = false;
    bool prevDown = false;
};
inline std::unordered_map<std::string, MenuKeyToggle> s_menuKeyToggles;
inline std::mutex s_menuKeyMtx;
inline bool IsMenuKeyActive(const char* id, int key, int mode)
{
    if (mode == 0) return true;
    if (key <= 0) return false;
    bool down = IsKeybindDown(key);
    if (mode == 2) return down;
    std::lock_guard<std::mutex> lk(s_menuKeyMtx);
    std::string k = id ? id : "";
    MenuKeyToggle& st = s_menuKeyToggles[k];
    if (down && !st.prevDown) st.toggled = !st.toggled;
    st.prevDown = down;
    return st.toggled;
}
inline void RenderKeybinds(ImDrawList* dl, int width, int height)
{
    (void)width;
    (void)height;
    if (!g_keybinds)
    {
        return;
    }
    struct ActiveBind
    {
        const char* name;
        const char* mode_tag;
    };
    ActiveBind list[32];
    int count = 0;
    auto push_bind = [&](const char* bind_id, const char* name, bool feat_on, int key, int mode)
    {
        if (!feat_on || key <= 0 || count >= 32) return;
        bool active = IsMenuKeyActive(bind_id, key, mode);
        if (!active) return;
        const char* tag = (mode == 0) ? "[always]" : ((mode == 1) ? "[toggle]" : "[hold]");
        list[count].name = name;
        list[count].mode_tag = tag;
        count++;
    };
    push_bind("##aim_key", "Aimbot", S::aim_enabled, S::aim_key, S::aim_key_mode);
    push_bind("##autoshoot_key", "Auto shoot", S::aim_autoshoot, S::aim_autoshoot_key, S::aim_autoshoot_key_mode);
    push_bind("##walkspeed_key", "Walkspeed", S::walkspeed_enabled, S::walkspeed_key, S::walkspeed_key_mode);
    push_bind("##jumppower_key", "Jump power", S::jumppower_enabled, S::jumppower_key, S::jumppower_key_mode);
    push_bind("##gravity_key", "Gravity", S::gravity_enabled, S::gravity_key, S::gravity_key_mode);
    push_bind("##nofall_key", "No fall damage", S::nofall_enabled, S::nofall_key, S::nofall_key_mode);
    push_bind("##fov_key", "Camera FOV", S::fov_enabled, S::fov_key, S::fov_key_mode);
    push_bind("##zoom_key", "Zoom", S::zoom_enabled, S::zoom_key, S::zoom_key_mode);
    push_bind("##noclip_key", "Noclip", S::noclip_enabled, S::noclip_key, S::noclip_key_mode);
    push_bind("##fly_key", "Fly", S::fly_enabled, S::fly_key, S::fly_key_mode);
    push_bind("##freecam_key", "Freecam", S::freecam_enabled, S::freecam_key, S::freecam_key_mode);
    push_bind("##hipheight_key", "Hip height", S::hipheight_enabled, S::hipheight_key, S::hipheight_key_mode);
    push_bind("##infjump_key", "Infinite jump", S::infjump_enabled, S::infjump_key, S::infjump_key_mode);
    push_bind("##time_key", "Shadow changer", S::time_enabled, S::time_key, S::time_key_mode);
    push_bind("##night_key", "Night mode", S::night_enabled, S::night_key, S::night_key_mode);
    push_bind("##fog_key", "Fog", S::world_fog_enabled, S::world_fog_key, S::world_fog_key_mode);
    push_bind("##exposure_key", "Exposure", S::world_exposure_enabled, S::world_exposure_key, S::world_exposure_key_mode);
    push_bind("##ambient_key", "Ambient", S::ambient_enabled, S::ambient_key, S::ambient_key_mode);
    push_bind("##fullbright_key", "Fullbright", S::fullbright_enabled, S::fullbright_key, S::fullbright_key_mode);
    push_bind("##noshadow_key", "No shadows", S::noshadow_enabled, S::noshadow_key, S::noshadow_key_mode);
    push_bind("##nograss_key", "No grass", S::nograss_enabled, S::nograss_key, S::nograss_key_mode);
    push_bind("##ore_key", "Ores", S::world_ore_enabled, S::world_ore_key, S::world_ore_key_mode);
    push_bind("##plant_key", "Plants", S::world_plant_enabled, S::world_plant_key, S::world_plant_key_mode);
    push_bind("##bots_key", "Bots", S::world_bots_enabled, S::world_bots_key, S::world_bots_key_mode);
    push_bind("##animals_key", "Animals", S::world_animals_enabled, S::world_animals_key, S::world_animals_key_mode);
    push_bind("##tools_key", "Tools", S::world_tools_enabled, S::world_tools_key, S::world_tools_key_mode);
    push_bind("##crates_key", "Crates", S::world_crates_enabled, S::world_crates_key, S::world_crates_key_mode);
    push_bind("##keycards_key", "Keycards", S::world_keycards_enabled, S::world_keycards_key, S::world_keycards_key_mode);
    push_bind("##geo_key", "World geo", S::world_geo_enabled, S::world_geo_key, S::world_geo_key_mode);
    if (!g_menuOpen && count == 0)
    {
        return;
    }
    float w = 160.0f;
    float hh = 20.0f;
    float row_h = 18.0f;
    float pad_y = 3.0f;
    float body_h = (count > 0) ? (pad_y * 2.0f + (float)count * row_h) : 0.0f;
    float total_h = hh + (count > 0 ? (body_h + 2.0f) : 0.0f);
    g_bindsSize = ImVec2(w, total_h);
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    if (g_menuOpen)
    {
        bool bHov = (m.x >= g_bindsPos.x && m.x <= g_bindsPos.x + w && m.y >= g_bindsPos.y && m.y <= g_bindsPos.y + total_h);
        bool anyPop = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        if (!g_bindsDragging && ImGui::IsMouseClicked(0) && bHov && !anyPop && !g_movementDragging && g_activeBindId == nullptr)
        {
            g_bindsDragging = true;
            g_bindsDragOff = ImVec2(m.x - g_bindsPos.x, m.y - g_bindsPos.y);
        }
        if (g_bindsDragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_bindsPos.x = m.x - g_bindsDragOff.x;
                g_bindsPos.y = m.y - g_bindsDragOff.y;
            }
            else
            {
                g_bindsDragging = false;
            }
        }
    }
    else
    {
        g_bindsDragging = false;
    }
    ImVec2 h_min = g_bindsPos;
    ImVec2 h_max = ImVec2(g_bindsPos.x + w, g_bindsPos.y + hh);
    DrawPixelBox(dl, h_min, h_max, CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
    ImVec2 title_sz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, "keybinds");
    float tx = floorf(h_min.x + (w - title_sz.x) * 0.5f);
    float ty = floorf(h_min.y + (hh - title_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, 13.0f, ImVec2(tx, ty), TextU32(), IM_COL32(0, 0, 0, 255), "keybinds");
    if (count > 0)
    {
        ImVec2 b_min = ImVec2(g_bindsPos.x, g_bindsPos.y + hh + 2.0f);
        ImVec2 b_max = ImVec2(g_bindsPos.x + w, b_min.y + body_h);
        DrawPixelBox(dl, b_min, b_max, CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
        for (int i = 0; i < count; i++)
        {
            float ry = b_min.y + pad_y + (float)i * row_h;
            float text_y = floorf(ry + (row_h - 13.0f) * 0.5f);
            float name_x = floorf(b_min.x + 8.0f);
            DrawOutlinedText(dl, f, 13.0f, ImVec2(name_x, text_y), TextU32(), IM_COL32(0, 0, 0, 255), list[i].name);
            ImVec2 tag_sz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, list[i].mode_tag);
            float tag_x = floorf(b_max.x - 8.0f - tag_sz.x);
            DrawOutlinedText(dl, f, 13.0f, ImVec2(tag_x, text_y), AccentU32(), IM_COL32(0, 0, 0, 255), list[i].mode_tag);
        }
    }
}
inline void RenderMovement(ImDrawList* dl, int width, int height)
{
    (void)width;
    (void)height;
    if (!g_movement)
    {
        return;
    }
    if (!g_movementVelocityOpt && !g_movementGraphOpt && !g_movementKeysOpt)
    {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    bool typing = io.WantTextInput;
    bool w_down = !typing && ((GetAsyncKeyState('W') & 0x8000) != 0);
    bool a_down = !typing && ((GetAsyncKeyState('A') & 0x8000) != 0);
    bool s_down = !typing && ((GetAsyncKeyState('S') & 0x8000) != 0);
    bool d_down = !typing && ((GetAsyncKeyState('D') & 0x8000) != 0);
    bool space_down = !typing && ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0);
    float curTime = (float)ImGui::GetTime();
    static bool s_havePrev = false;
    static float s_prevSpeed = 0.0f;
    float disp_speed = 0.0f;
    if (g_localSpeed >= 0.0f && std::isfinite(g_localSpeed))
    {
        float sp = g_localSpeed;
        if (s_havePrev && sp > s_prevSpeed + 4000.0f)
            sp = s_prevSpeed;
        s_prevSpeed = sp;
        s_havePrev = true;
        disp_speed = sp;
    }
    else
    {
        s_havePrev = false;
        s_prevSpeed = 0.0f;
        disp_speed = 0.0f;
    }
    static float s_velHistory[120];
    static float s_graphInited = false;
    if (!s_graphInited)
    {
        for (int i = 0; i < 120; i++)
            s_velHistory[i] = 0.0f;
        s_graphInited = true;
    }
    static float s_lastScrollTime = 0.0f;
    if (curTime - s_lastScrollTime >= 0.025f)
    {
        s_lastScrollTime = curTime;
        for (int i = 0; i < 119; i++)
        {
            s_velHistory[i] = s_velHistory[i + 1];
        }
        s_velHistory[119] = disp_speed;
    }
    static float s_peak = 20.0f;
    {
        float inst_peak = 0.0f;
        for (int i = 0; i < 120; i++)
        {
            if (s_velHistory[i] > inst_peak) inst_peak = s_velHistory[i];
        }
        if (inst_peak > s_peak) s_peak = inst_peak;
        else s_peak += (inst_peak - s_peak) * 0.02f;
        if (s_peak < 20.0f) s_peak = 20.0f;
    }
    float w = 190.0f;
    float hh = 20.0f;
    float pad_y = 6.0f;
    float body_h = pad_y;
    float vel_h = 16.0f;
    float graph_h = 56.0f;
    float keys_h = 22.0f + 4.0f + 22.0f + 4.0f + 16.0f;
    if (g_movementVelocityOpt)
    {
        body_h += vel_h;
        if (g_movementGraphOpt || g_movementKeysOpt)
        {
            body_h += 5.0f;
        }
    }
    if (g_movementGraphOpt)
    {
        body_h += graph_h;
        if (g_movementKeysOpt)
        {
            body_h += 6.0f;
        }
    }
    if (g_movementKeysOpt)
    {
        if (g_movementVelocityOpt || g_movementGraphOpt)
        {
            body_h += 7.0f;
        }
        body_h += keys_h;
    }
    body_h += pad_y;
    float total_h = hh + 2.0f + body_h;
    g_movementSize = ImVec2(w, total_h);
    ImVec2 m = io.MousePos;
    if (g_menuOpen)
    {
        bool mHov = (m.x >= g_movementPos.x && m.x <= g_movementPos.x + w && m.y >= g_movementPos.y && m.y <= g_movementPos.y + total_h);
        bool anyPop = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        bool overBinds = g_keybinds && (m.x >= g_bindsPos.x && m.x <= g_bindsPos.x + g_bindsSize.x && m.y >= g_bindsPos.y && m.y <= g_bindsPos.y + g_bindsSize.y);
        if (!g_movementDragging && ImGui::IsMouseClicked(0) && mHov && !anyPop && !overBinds && g_activeBindId == nullptr)
        {
            g_movementDragging = true;
            g_movementDragOff = ImVec2(m.x - g_movementPos.x, m.y - g_movementPos.y);
        }
        if (g_movementDragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_movementPos.x = m.x - g_movementDragOff.x;
                g_movementPos.y = m.y - g_movementDragOff.y;
            }
            else
            {
                g_movementDragging = false;
            }
        }
    }
    else
    {
        g_movementDragging = false;
    }
    ImVec2 h_min = g_movementPos;
    ImVec2 h_max = ImVec2(g_movementPos.x + w, g_movementPos.y + hh);
    DrawPixelBox(dl, h_min, h_max, CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
    ImVec2 title_sz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, "movement");
    float tx = floorf(h_min.x + (w - title_sz.x) * 0.5f);
    float ty = floorf(h_min.y + (hh - title_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, 13.0f, ImVec2(tx, ty), TextU32(), IM_COL32(0, 0, 0, 255), "movement");
    ImVec2 b_min = ImVec2(g_movementPos.x, g_movementPos.y + hh + 2.0f);
    ImVec2 b_max = ImVec2(g_movementPos.x + w, b_min.y + body_h);
    DrawPixelBox(dl, b_min, b_max, CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    float cur_y = b_min.y + pad_y;
    if (g_movementVelocityOpt)
    {
        float text_y = floorf(cur_y + 1.0f);
        DrawOutlinedText(dl, f, 13.0f, ImVec2(b_min.x + 10.0f, text_y), TextU32(), IM_COL32(0, 0, 0, 255), "Velocity");
        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "%d", (int)(disp_speed + 0.5f));
        ImVec2 ssz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, sbuf);
        ImU32 scol = (disp_speed >= 250.0f) ? AccentU32() : ((disp_speed > 10.0f) ? TextU32() : TextDimU32());
        DrawOutlinedText(dl, f, 13.0f, ImVec2(b_max.x - 10.0f - ssz.x, text_y), scol, IM_COL32(0, 0, 0, 255), sbuf);
        cur_y += vel_h;
        if (!g_movementGraphOpt && !g_movementKeysOpt)
        {
            float bar_x0 = b_min.x + 10.0f;
            float bar_x1 = b_max.x - 10.0f;
            float bar_y = cur_y + 1.0f;
            float bar_w = bar_x1 - bar_x0;
            dl->AddRectFilled(ImVec2(bar_x0, bar_y), ImVec2(bar_x1, bar_y + 2.0f), FrameU32());
            float ratio = disp_speed / s_peak;
            if (ratio < 0.0f) ratio = 0.0f;
            if (ratio > 1.0f) ratio = 1.0f;
            if (ratio > 0.0f)
            {
                dl->AddRectFilled(ImVec2(bar_x0, bar_y), ImVec2(bar_x0 + bar_w * ratio, bar_y + 2.0f), AccentU32());
            }
            cur_y += 5.0f;
        }
        else if (g_movementGraphOpt)
        {
            cur_y += 5.0f;
        }
    }
    if (g_movementGraphOpt)
    {
        float gx0 = b_min.x + 10.0f;
        float gx1 = b_max.x - 10.0f;
        float gy0 = cur_y;
        float gy1 = cur_y + graph_h;
        float gw = gx1 - gx0;
        float gh = graph_h;
        dl->AddRectFilled(ImVec2(gx0, gy0), ImVec2(gx1, gy1), FrameU32());
        dl->AddRect(ImVec2(gx0, gy0), ImVec2(gx1, gy1), BorderU32(), 0.0f, 0, 1.0f);
        for (int i = 1; i <= 4; i++)
        {
            float tx_pos = floorf(gx0 + gw * ((float)i / 5.0f));
            dl->AddLine(ImVec2(tx_pos, gy1), ImVec2(tx_pos, gy1 - 3.0f), BorderU32());
            dl->AddLine(ImVec2(tx_pos, gy0), ImVec2(tx_pos, gy0 + 3.0f), BorderU32());
        }
        for (int i = 1; i <= 3; i++)
        {
            float ty_pos = floorf(gy0 + gh * ((float)i / 4.0f));
            dl->AddLine(ImVec2(gx0, ty_pos), ImVec2(gx0 + 3.0f, ty_pos), BorderU32());
            dl->AddLine(ImVec2(gx1, ty_pos), ImVec2(gx1 - 3.0f, ty_pos), BorderU32());
        }
        dl->AddLine(ImVec2(gx0 + 3.0f, gy0 + gh * 0.5f), ImVec2(gx1 - 3.0f, gy0 + gh * 0.5f), IM_COL32(70, 70, 70, 80));
        float max_v = s_peak;
        float min_v = 0.0f;
        float range = max_v - min_v;
        if (range < 1.0f) range = 1.0f;
        ImVec2 pts[120];
        for (int i = 0; i < 120; i++)
        {
            float px = gx0 + gw * ((float)i / 119.0f);
            float norm = (s_velHistory[i] - min_v) / range;
            if (norm < 0.0f) norm = 0.0f;
            if (norm > 1.0f) norm = 1.0f;
            float py = gy1 - 2.0f - norm * (gh - 4.0f);
            pts[i] = ImVec2(px, py);
        }
        ImU32 fill_col = IM_COL32(
            (int)(g_accent1.x * 255.0f),
            (int)(g_accent1.y * 255.0f),
            (int)(g_accent1.z * 255.0f),
            30
        );
        for (int i = 0; i < 119; i++)
        {
            ImVec2 p0 = pts[i];
            ImVec2 p1 = pts[i + 1];
            ImVec2 b0 = ImVec2(p0.x, gy1 - 1.0f);
            ImVec2 b1 = ImVec2(p1.x, gy1 - 1.0f);
            dl->AddQuadFilled(p0, p1, b1, b0, fill_col);
        }
        dl->AddPolyline(pts, 120, AccentU32(), 0, 1.5f);
        dl->AddCircleFilled(pts[119], 2.5f, IM_COL32(255, 255, 255, 255));
        dl->AddCircleFilled(pts[119], 1.5f, AccentU32());
        char max_lbl[16];
        snprintf(max_lbl, sizeof(max_lbl), "%d", (int)(max_v + 0.5f));
        DrawOutlinedText(dl, f, 13.0f, ImVec2(gx0 + 5.0f, gy0 + 3.0f), TextDimU32(), IM_COL32(0, 0, 0, 255), max_lbl);
        DrawOutlinedText(dl, f, 13.0f, ImVec2(gx0 + 5.0f, gy1 - 14.0f), TextDimU32(), IM_COL32(0, 0, 0, 255), "0");
        cur_y += gh;
    }
    if (g_movementKeysOpt)
    {
        if (g_movementVelocityOpt || g_movementGraphOpt)
        {
            cur_y += 4.0f;
            DrawGlowLine(dl, b_min.x + 8.0f, b_max.x - 8.0f, cur_y, AccentU32());
            cur_y += 5.0f;
        }
        auto draw_key_tile = [&](ImVec2 k0, ImVec2 k1, const char* lbl, bool down)
        {
            ImU32 bg = down ? CtrlHovU32() : FrameU32();
            ImU32 bor = down ? BorderHovU32() : BorderU32();
            ImU32 col = down ? AccentU32() : TextDimU32();
            dl->AddRectFilled(k0, k1, bg);
            dl->AddRect(k0, k1, bor, 0.0f, 0, 1.0f);
            if (down)
            {
                dl->AddRectFilled(ImVec2(k0.x + 1.0f, k0.y + 1.0f), ImVec2(k1.x - 1.0f, k0.y + 2.0f), AccentU32());
            }
            ImVec2 lsz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, lbl);
            float lx = floorf(k0.x + (k1.x - k0.x - lsz.x) * 0.5f);
            float ly = floorf(k0.y + (k1.y - k0.y - lsz.y) * 0.5f);
            DrawOutlinedText(dl, f, 13.0f, ImVec2(lx, ly), col, IM_COL32(0, 0, 0, 255), lbl);
        };
        float kw = 32.0f;
        float kh = 22.0f;
        float kgap = 4.0f;
        float total_kw = kw * 3.0f + kgap * 2.0f;
        float start_kx = b_min.x + floorf((w - total_kw) * 0.5f);
        float w_x0 = start_kx + kw + kgap;
        float w_y0 = cur_y;
        draw_key_tile(ImVec2(w_x0, w_y0), ImVec2(w_x0 + kw, w_y0 + kh), "W", w_down);
        cur_y += kh + kgap;
        float a_x0 = start_kx;
        draw_key_tile(ImVec2(a_x0, cur_y), ImVec2(a_x0 + kw, cur_y + kh), "A", a_down);
        float s_x0 = start_kx + kw + kgap;
        draw_key_tile(ImVec2(s_x0, cur_y), ImVec2(s_x0 + kw, cur_y + kh), "S", s_down);
        float d_x0 = start_kx + (kw + kgap) * 2.0f;
        draw_key_tile(ImVec2(d_x0, cur_y), ImVec2(d_x0 + kw, cur_y + kh), "D", d_down);
        cur_y += kh + kgap;
        float sp_h = 16.0f;
        draw_key_tile(ImVec2(start_kx, cur_y), ImVec2(start_kx + total_kw, cur_y + sp_h), "SPACE", space_down);
    }
}
inline bool CopyTextToClipboard(const std::string& text)
{
    if (text.empty())
        return false;
    HWND hwnd = Overlay::GetOverlayWindow();
    if (!hwnd)
        hwnd = GetForegroundWindow();
    if (!hwnd || !OpenClipboard(hwnd))
        return false;
    bool ok = false;
    EmptyClipboard();
    size_t bytes = text.size() + 1;
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (mem)
    {
        void* dst = GlobalLock(mem);
        if (dst)
        {
            memcpy(dst, text.c_str(), bytes);
            GlobalUnlock(mem);
            ok = SetClipboardData(CF_TEXT, mem) != nullptr;
        }
        if (!ok)
            GlobalFree(mem);
    }
    CloseClipboard();
    return ok;
}
inline void RenderInternalDebug(ImDrawList* dl, int width, int height, float fps)
{
    (void)width;
    (void)height;
    if (!g_internalDebug)
    {
        return;
    }
    float w = 270.0f;
    float hh = 20.0f;
    float row_h = 16.0f;
    float pad_y = 5.0f;
    struct DebugRow {
        std::string label;
        std::string val;
        ImU32 col;
    };
    std::vector<DebugRow> rows;
    rows.reserve(14);
    bool connected = thenwefuckin::g_proc && thenwefuckin::g_proc->connected();
    if (connected)
    {
        char b[64];
        snprintf(b, sizeof(b), "attached (PID %u)", (unsigned)thenwefuckin::g_proc->pid());
        rows.push_back({ "Process", b, IM_COL32(70, 220, 100, 255) });
        snprintf(b, sizeof(b), "0x%llX", (unsigned long long)thenwefuckin::g_proc->base());
        rows.push_back({ "Base", b, AccentU32() });
    }
    else
    {
        rows.push_back({ "Process", "waiting...", IM_COL32(230, 80, 80, 255) });
    }
    char bfps[64];
    snprintf(bfps, sizeof(bfps), "%.1f", (double)fps);
    rows.push_back({ "Overlay FPS", bfps, TextU32() });
    {
        char b[96];
        snprintf(b, sizeof(b), "r:%.2f pf:%.2f m:%.2f a:%.2f v:%.2f x:%.2f",
            g_dbgMsRefresh, g_dbgMsPf, g_dbgMsMenu, g_dbgMsAim, g_dbgMsVisual, g_dbgMsMisc);
        rows.push_back({ "Frame ms", b, TextU32() });
    }
    if (thenwefuckin::g_ctx.datamodel)
    {
        char b[64];
        snprintf(b, sizeof(b), "0x%llX", (unsigned long long)thenwefuckin::g_ctx.datamodel.addr);
        rows.push_back({ "DataModel", b, TextU32() });
    }
    else
    {
        rows.push_back({ "DataModel", "not resolved", TextDimU32() });
    }
    if (thenwefuckin::g_ctx.players)
    {
        char b[64];
        std::string sname = thenwefuckin::g_ctx.players.name();
        size_t count = thenwefuckin::g_ctx.player_list().size();
        snprintf(b, sizeof(b), "%zu [%s]", count, sname.c_str());
        rows.push_back({ "Players", b, TextU32() });
    }
    else
    {
        rows.push_back({ "Players", "none", TextDimU32() });
    }
    if (thenwefuckin::g_ctx.local_player)
    {
        char b[64];
        std::string lpName = thenwefuckin::g_ctx.local_player.name();
        snprintf(b, sizeof(b), "%s", lpName.empty() ? "valid" : lpName.c_str());
        rows.push_back({ "LocalPlayer", b, AccentU32() });
    }
    else
    {
        rows.push_back({ "LocalPlayer", "none", TextDimU32() });
    }
    if (thenwefuckin::g_ctx.local_root)
    {
        char b[64];
        thenwefuckin::math::vec3 pos = thenwefuckin::g_ctx.local_pos;
        snprintf(b, sizeof(b), "%.0f, %.0f, %.0f", pos.x, pos.y, pos.z);
        rows.push_back({ "LocalPos", b, TextU32() });
        float sp = g_localSpeed >= 0.0f ? g_localSpeed : 0.0f;
        snprintf(b, sizeof(b), "%.1f studs/s", sp);
        rows.push_back({ "Speed", b, AccentU32() });
    }
    else
    {
        rows.push_back({ "LocalRoot", "none", TextDimU32() });
    }
    if (thenwefuckin::g_ctx.camera)
    {
        char b[64];
        snprintf(b, sizeof(b), "0x%llX", (unsigned long long)thenwefuckin::g_ctx.camera.addr);
        rows.push_back({ "Camera", b, TextU32() });
        if (S::freecam_enabled && thenwefuckin::g_proc && thenwefuckin::g_proc->connected())
        {
            int camType = Offsets::Camera::CameraType ? thenwefuckin::g_proc->read<int>(
                thenwefuckin::g_ctx.camera.addr + Offsets::Camera::CameraType) : -1;
            uintptr_t subj = thenwefuckin::g_proc->read<uintptr_t>(
                thenwefuckin::g_ctx.camera.addr + Offsets::Camera::CameraSubject);
            snprintf(b, sizeof(b), "%d%s", camType, camType == 6 ? " (scriptable)" : " (!!)");
            rows.push_back({ "FCamType", b, camType == 6 ? IM_COL32(70, 220, 100, 255) : IM_COL32(230, 80, 80, 255) });
            snprintf(b, sizeof(b), subj == 0 ? "0 (detached)" : "0x%llX (!!)", (unsigned long long)subj);
            rows.push_back({ "FCamSubj", b, subj == 0 ? IM_COL32(70, 220, 100, 255) : IM_COL32(230, 80, 80, 255) });
        }
    }
    char brdy[64];
    snprintf(brdy, sizeof(brdy), "%s", thenwefuckin::g_ctx.ready ? "ready" : "not ready");
    rows.push_back({ "SDK Cache", brdy, thenwefuckin::g_ctx.ready ? IM_COL32(70, 220, 100, 255) : IM_COL32(230, 80, 80, 255) });
    {
        const char* rst = thenwefuckin::aim::raycast::status();
        bool rok = thenwefuckin::aim::raycast::ready();
        rows.push_back({ "Raycast", rst ? rst : "?", rok ? IM_COL32(70, 220, 100, 255) : TextDimU32() });
        if (rok)
        {
            char brc[64];
            snprintf(brc, sizeof(brc), "%llu", (unsigned long long)thenwefuckin::aim::raycast::calls());
            rows.push_back({ "RayCalls", brc, TextU32() });
        }
    }
    if (PfCache::workspacePlayersAddr)
    {
        char bpf[96];
        snprintf(bpf, sizeof(bpf), "%zu players (%s)", PfCache::players.size(), PfCache::dbg.failStage);
        rows.push_back({ "PF Cache", bpf, TextU32() });
    }
    if (thenwefuckin::g_ctx.local_player)
    {
        char buid[64];
        int64_t luid = thenwefuckin::g_proc && thenwefuckin::g_proc->connected()
            ? thenwefuckin::g_proc->read<int64_t>(thenwefuckin::g_ctx.local_player.addr + Offsets::Player::UserId)
            : 0;
        snprintf(buid, sizeof(buid), "%lld", (long long)luid);
        rows.push_back({ "LocalUID", buid, luid > 0 ? AccentU32() : IM_COL32(230, 80, 80, 255) });
    }
    float body_h = pad_y * 2.0f + (float)rows.size() * row_h + 42.0f;
    float total_h = hh + 2.0f + body_h;
    g_debugSize = ImVec2(w, total_h);
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    if (g_menuOpen)
    {
        bool bHov = (m.x >= g_debugPos.x && m.x <= g_debugPos.x + w && m.y >= g_debugPos.y && m.y <= g_debugPos.y + total_h);
        bool anyPop = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        bool overBinds = g_keybinds && (m.x >= g_bindsPos.x && m.x <= g_bindsPos.x + g_bindsSize.x && m.y >= g_bindsPos.y && m.y <= g_bindsPos.y + g_bindsSize.y);
        bool overMove = g_movement && (m.x >= g_movementPos.x && m.x <= g_movementPos.x + g_movementSize.x && m.y >= g_movementPos.y && m.y <= g_movementPos.y + g_movementSize.y);
        if (!g_debugDragging && ImGui::IsMouseClicked(0) && bHov && !anyPop && !overBinds && !overMove && g_activeBindId == nullptr)
        {
            g_debugDragging = true;
            g_debugDragOff = ImVec2(m.x - g_debugPos.x, m.y - g_debugPos.y);
        }
        if (g_debugDragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_debugPos.x = m.x - g_debugDragOff.x;
                g_debugPos.y = m.y - g_debugDragOff.y;
            }
            else
            {
                g_debugDragging = false;
            }
        }
    }
    else
    {
        g_debugDragging = false;
    }
    ImVec2 h_min = g_debugPos;
    ImVec2 h_max = ImVec2(g_debugPos.x + w, g_debugPos.y + hh);
    DrawPixelBox(dl, h_min, h_max, CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
    ImVec2 title_sz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, "logs");
    float tx = floorf(h_min.x + (w - title_sz.x) * 0.5f);
    float ty = floorf(h_min.y + (hh - title_sz.y) * 0.5f);
    DrawOutlinedText(dl, f, 13.0f, ImVec2(tx, ty), TextU32(), IM_COL32(0, 0, 0, 255), "logs");
    ImVec2 b_min = ImVec2(g_debugPos.x, g_debugPos.y + hh + 2.0f);
    ImVec2 b_max = ImVec2(g_debugPos.x + w, b_min.y + body_h);
    DrawPixelBox(dl, b_min, b_max, CtrlBgU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    for (size_t i = 0; i < rows.size(); i++)
    {
        float ry = b_min.y + pad_y + (float)i * row_h;
        float text_y = floorf(ry + (row_h - 12.0f) * 0.5f);
        float name_x = floorf(b_min.x + 8.0f);
        DrawOutlinedText(dl, f, 12.0f, ImVec2(name_x, text_y), TextDimU32(), IM_COL32(0, 0, 0, 255), rows[i].label.c_str());
        ImVec2 val_sz = f->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, rows[i].val.c_str());
        float val_x = floorf(b_max.x - 8.0f - val_sz.x);
        DrawOutlinedText(dl, f, 12.0f, ImVec2(val_x, text_y), rows[i].col, IM_COL32(0, 0, 0, 255), rows[i].val.c_str());
    }
    {
        float by = b_min.y + pad_y + (float)rows.size() * row_h + 3.0f;
        bool copy_hov = false;
        bool copy_click = g_menuOpen && DrawButton(dl, ImVec2(b_min.x + 8.0f, by), w - 16.0f, 20.0f, "Copy logs", f, 12.0f, &copy_hov);
        static double s_copyMsgUntil = 0.0;
        if (copy_click)
        {
            std::string txt;
            txt.reserve(4096);
            for (auto& r : rows)
            {
                txt += r.label;
                txt += ": ";
                txt += r.val;
                txt += "\r\n";
            }
            {
                std::lock_guard<std::mutex> lk(Logger::mtx);
                for (auto& l : Logger::recent)
                {
                    txt += l;
                    txt += "\r\n";
                }
            }
            if (CopyTextToClipboard(txt))
                s_copyMsgUntil = ImGui::GetTime() + 1.5;
        }
        if (ImGui::GetTime() < s_copyMsgUntil)
            DrawOutlinedText(dl, f, 12.0f, ImVec2(floorf(b_min.x + 8.0f), floorf(by + 22.0f)), IM_COL32(70, 220, 100, 255), IM_COL32(0, 0, 0, 255), "copied!");
    }
}
inline bool RenderNavBar(ImDrawList* dl, int width, int height)
{
    (void)height;
    g_navBarHov = false;
    if (g_navHidden)
    {
        return false;
    }
    const float tile = kNavTile;
    const float pad = kNavPad;
    const float gap = kNavGap;
    ImVec2 barMin, barMax;
    CalcNavBarRect(width, height, barMin, barMax);
    float bx0 = barMin.x;
    float by0 = barMin.y;
    float bx1 = barMax.x;
    float by1 = barMax.y;
    float hx0 = bx0 + pad;
    float hy0 = by0 + pad;
    float plx0 = hx0 + tile + gap;
    float ply0 = hy0;
    float fx0 = plx0 + tile + gap;
    float fy0 = hy0;
    float sx0 = fx0 + tile + gap;
    float sy0 = hy0;
    float px0 = sx0 + tile + gap;
    float py0 = hy0;
    float ux0 = px0 + tile + gap;
    float uy0 = hy0;
    float cox0 = ux0 + tile + gap;
    float coy0 = hy0;
    bool home_active = g_menuOpen;
    bool pl_active = g_playersOpen;
    bool file_active = g_explorerOpen;
    bool set_active = g_settingsOpen;
    bool pal_active = g_themeOpen;
    bool user_active = g_playerPreviewOpen;
    bool code_active = g_luaOpen;
    DrawPixelBox(dl, ImVec2(bx0, by0), ImVec2(bx1, by1), FrameU32(), BorderU32(), IM_COL32(0, 0, 0, 255));
    dl->AddRectFilled(ImVec2(bx0 + 2.0f, by0 + 2.0f), ImVec2(bx1 - 2.0f, by0 + 3.0f), AccentU32());
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    bool home_hov = (m.x >= hx0 && m.x <= hx0 + tile && m.y >= hy0 && m.y <= hy0 + tile);
    bool pl_hov = (m.x >= plx0 && m.x <= plx0 + tile && m.y >= ply0 && m.y <= ply0 + tile);
    bool file_hov = (m.x >= fx0 && m.x <= fx0 + tile && m.y >= fy0 && m.y <= fy0 + tile);
    bool set_hov  = (m.x >= sx0 && m.x <= sx0 + tile && m.y >= sy0 && m.y <= sy0 + tile);
    bool pal_hov  = (m.x >= px0 && m.x <= px0 + tile && m.y >= py0 && m.y <= py0 + tile);
    bool user_hov = (m.x >= ux0 && m.x <= ux0 + tile && m.y >= uy0 && m.y <= uy0 + tile);
    bool code_hov = (m.x >= cox0 && m.x <= cox0 + tile && m.y >= coy0 && m.y <= coy0 + tile);
    if (IsModalActive())
    {
        home_hov = false;
        pl_hov = false;
        file_hov = false;
        set_hov = false;
        pal_hov = false;
        user_hov = false;
        code_hov = false;
    }
    if (home_hov || pl_hov || file_hov || set_hov || pal_hov || user_hov || code_hov)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    g_navBarHov = home_hov || pl_hov || file_hov || set_hov || pal_hov || user_hov || code_hov;
    bool home_click = home_hov && ImGui::IsMouseClicked(0);
    bool pl_click = pl_hov && ImGui::IsMouseClicked(0);
    bool file_click = file_hov && ImGui::IsMouseClicked(0);
    bool set_click  = set_hov && ImGui::IsMouseClicked(0);
    bool pal_click  = pal_hov && ImGui::IsMouseClicked(0);
    bool user_click = user_hov && ImGui::IsMouseClicked(0);
    bool code_click = code_hov && ImGui::IsMouseClicked(0);
    if (home_click || pl_click || file_click || set_click || pal_click || user_click || code_click) UiClick();
    if (home_click)
    {
        if (g_menuOpen)
        {
            g_menuOpen = false;
            g_playersOpen = false;
            g_playerPreviewOpen = false;
            g_settingsOpen = false;
            g_themeOpen = false;
            g_explorerOpen = false;
            g_luaOpen = false;
        }
        else
        {
            g_activeTab = 0;
            g_menuOpen = true;
        }
    }
    if (pl_click)
    {
        g_playersOpen = !g_playersOpen;
    }
    if (file_click)
    {
        g_explorerOpen = !g_explorerOpen;
    }
    if (set_click)
    {
        g_settingsOpen = !g_settingsOpen;
    }
    if (pal_click)
    {
        g_themeOpen = !g_themeOpen;
    }
    if (user_click)
    {
        g_playerPreviewOpen = !g_playerPreviewOpen;
    }
    if (code_click)
    {
        g_luaOpen = !g_luaOpen;
    }
    auto draw_nav_tile = [&](ImVec2 pos, bool active, bool hov) {
        ImU32 bg = (active || hov) ? CtrlHovU32() : CtrlBgU32();
        ImU32 border = (active || hov) ? BorderHovU32() : BorderU32();
        DrawPixelBox(dl, pos, ImVec2(pos.x + tile, pos.y + tile), bg, border, IM_COL32(0, 0, 0, 255));
    };
    draw_nav_tile(ImVec2(hx0, hy0), home_active, home_hov);
    draw_nav_tile(ImVec2(plx0, ply0), pl_active, pl_hov);
    draw_nav_tile(ImVec2(fx0, fy0), file_active, file_hov);
    draw_nav_tile(ImVec2(sx0, sy0), set_active, set_hov);
    draw_nav_tile(ImVec2(px0, py0), pal_active, pal_hov);
    draw_nav_tile(ImVec2(ux0, uy0), user_active, user_hov);
    draw_nav_tile(ImVec2(cox0, coy0), code_active, code_hov);
    ImU32 col_active_icon = AccentU32();
    ImU32 col_hov_icon = IM_COL32(255, 255, 255, 255);
    ImU32 col_idle_icon = TextDimU32();
    ImU32 home_glyph = home_active ? (home_hov ? col_hov_icon : col_active_icon) : (home_hov ? col_hov_icon : col_idle_icon);
    ImU32 pl_glyph = pl_active ? (pl_hov ? col_hov_icon : col_active_icon) : (pl_hov ? col_hov_icon : col_idle_icon);
    ImU32 file_glyph = file_active ? (file_hov ? col_hov_icon : col_active_icon) : (file_hov ? col_hov_icon : col_idle_icon);
    ImU32 set_glyph  = set_active  ? (set_hov  ? col_hov_icon : col_active_icon) : (set_hov  ? col_hov_icon : col_idle_icon);
    ImU32 pal_col    = pal_active  ? (pal_hov  ? col_hov_icon : col_active_icon) : (pal_hov  ? col_hov_icon : col_idle_icon);
    ImU32 user_glyph = user_active ? (user_hov ? col_hov_icon : col_active_icon) : (user_hov ? col_hov_icon : col_idle_icon);
    ImU32 code_glyph = code_active ? (code_hov ? col_hov_icon : col_active_icon) : (code_hov ? col_hov_icon : col_idle_icon);
    ImFont* iconF = ImGui::GetFont();
    auto navIcon = [&](float tx0, float ty0, const char* icon, ImU32 col) {
        ImVec2 isz = iconF->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, icon);
        float ix = floorf(tx0 + (tile - isz.x) * 0.5f);
        float iy = floorf(ty0 + (tile - isz.y) * 0.5f);
        dl->AddText(iconF, 16.0f, ImVec2(ix, iy), col, icon);
    };
    navIcon(hx0, hy0, ICON_FA_HOUSE, home_glyph);
    navIcon(sx0, sy0, ICON_FA_GEAR, set_glyph);
    navIcon(px0, py0, ICON_FA_PALETTE, pal_col);
    navIcon(fx0, fy0, ICON_FA_FOLDER, file_glyph);
    navIcon(ux0, uy0, ICON_FA_EYE, user_glyph);
    navIcon(plx0, ply0, ICON_FA_USERS, pl_glyph);
    navIcon(cox0, coy0, ICON_FA_CODE, code_glyph);
    return g_navBarHov;
}
inline void Render()
{
    if (!g_menuOpen)
    {
        g_menuAnim = 0.0f;
        g_activeBindId = nullptr;
        g_modalAtFrameStart = false;
        return;
    }
    g_modalAtFrameStart = (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) || g_activeBindId != nullptr);
    ImGuiIO& io = ImGui::GetIO();
    if (g_menuAnim < 1.0f)
    {
        float dt = io.DeltaTime;
        if (dt > 0.05f) dt = 0.05f;
        g_menuAnim += dt * 6.0f;
        if (g_menuAnim > 1.0f) g_menuAnim = 1.0f;
    }
    float t = g_menuAnim;
    float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    float y_off = (1.0f - ease) * -14.0f;
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    center.y -= 100.0f;
    if (g_pos.x < 0.0f)
    {
        g_pos = ImVec2(center.x - g_menuSize.x * 0.5f, center.y - g_menuSize.y * 0.5f);
    }
    ImGui::SetNextWindowSize(g_menuSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(g_pos.x, g_pos.y + y_off), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(ease);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ease);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uibase", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline    = IM_COL32(0, 0, 0, 255);
        const ImU32 col_bg_frame   = FrameU32();
        const ImU32 col_bg_group   = GroupU32();
        const ImU32 col_highlight  = BorderU32();
        ImU32 col_accent_top = ImGui::ColorConvertFloat4ToU32(g_accent1);
        DrawPixelBox(dl, pos, max, col_bg_frame, col_highlight, col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), col_accent_top);
        const float header_space = 46.0f;
        const float margin = 10.0f;
        const ImU32 col_text = TextU32();
        DrawWaveText(dl, ImVec2(pos.x + 10.0f, pos.y + 12.0f), col_text, col_outline, "home");
        ImVec2 inner_pos = ImVec2(pos.x + margin, pos.y + margin + header_space);
        ImVec2 inner_max = ImVec2(max.x - margin, max.y - margin);
        ImFont* tab_font = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        const char* sections[] = { "Aimbot", "Visuals", "Misc" };
        const float tab_h = 22.0f;
        const float tab_top_y = inner_pos.y - tab_h;
        const float pad_x = 8.0f;
        const float tab_gap = 4.0f;
        float tab_bounds_min_x[3];
        float tab_bounds_max_x[3];
        float cur_tab_x = inner_pos.x + 12.0f;
        ImVec2 m = io.MousePos;
        bool tab_hovered = false;
        for (int i = 0; i < 3; i++)
        {
            ImVec2 text_sz = tab_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, sections[i]);
            float w = floorf(text_sz.x + pad_x * 2.0f);
            tab_bounds_min_x[i] = cur_tab_x;
            tab_bounds_max_x[i] = cur_tab_x + w;
            bool is_hov = (m.x >= cur_tab_x && m.x <= cur_tab_x + w && m.y >= tab_top_y && m.y <= inner_pos.y);
            if (is_hov)
            {
                tab_hovered = true;
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                if (ImGui::IsMouseClicked(0))
                {
                    g_activeTab = i;
                    UiClick();
                }
            }
            cur_tab_x += w + tab_gap;
        }
        if (g_activeTab < 0 || g_activeTab >= 3)
        {
            g_activeTab = 0;
        }
        float active_min_x = tab_bounds_min_x[g_activeTab];
        float active_max_x = tab_bounds_max_x[g_activeTab];
        DrawTabbedContainer(dl, inner_pos, inner_max, active_min_x, active_max_x, tab_top_y, col_bg_frame, col_highlight, col_outline, col_accent_top);
        for (int i = 0; i < 3; i++)
        {
            float t_min_x = tab_bounds_min_x[i];
            float t_max_x = tab_bounds_max_x[i];
            ImVec2 text_sz = tab_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, sections[i]);
            float tx = floorf(t_min_x + (t_max_x - t_min_x - text_sz.x) * 0.5f);
            float ty = floorf(tab_top_y + (tab_h - text_sz.y) * 0.5f + 1.0f);
            bool is_hov = (m.x >= t_min_x && m.x <= t_max_x && m.y >= tab_top_y && m.y <= inner_pos.y);
            bool is_active = (g_activeTab == i);
            ImU32 col_tab = is_active ? TextU32() : TextDimU32();
            DrawOutlinedText(dl, tab_font, 13.0f, ImVec2(tx, ty), col_tab, col_outline, sections[i]);
        }
        const float group_pad = 12.0f;
        const float group_gap = 10.0f;
        float avail_w = (inner_max.x - inner_pos.x) - (group_pad * 2.0f) - group_gap;
        float col_w = floorf(avail_w * 0.5f);
        ImVec2 left_min = ImVec2(inner_pos.x + group_pad, inner_pos.y + group_pad + 6.0f);
        ImVec2 left_max = ImVec2(left_min.x + col_w, inner_max.y - group_pad);
        ImVec2 right_min = ImVec2(left_max.x + group_gap, inner_pos.y + group_pad + 6.0f);
        ImVec2 right_max = ImVec2(inner_max.x - group_pad, inner_max.y - group_pad);
        const float group_split_h = 265.0f;
        const float group_gap_v = 10.0f;
        ImVec2 v_left_top_min = left_min;
        ImVec2 v_left_top_max = ImVec2(left_max.x, left_min.y + group_split_h);
        ImVec2 v_left_bot_min = ImVec2(left_min.x, v_left_top_max.y + group_gap_v);
        ImVec2 v_left_bot_max = left_max;
        const float world_h = v_left_bot_max.y - v_left_bot_min.y;
        const float misc_h = world_h + 40.0f;
        ImVec2 v_right_bot_max = right_max;
        ImVec2 v_right_bot_min = ImVec2(right_min.x, right_max.y - misc_h);
        ImVec2 v_right_top_min = right_min;
        ImVec2 v_right_top_max = ImVec2(right_max.x, v_right_bot_min.y - group_gap_v);
        if (g_activeTab == 1)
        {
            DrawGroupBox(dl, v_left_top_min, v_left_top_max, "visuals", tab_font, 13.0f, col_bg_group, col_highlight, col_outline);
            DrawGroupBox(dl, v_left_bot_min, v_left_bot_max, "world", tab_font, 13.0f, col_bg_group, col_highlight, col_outline);
            DrawGroupBox(dl, v_right_top_min, v_right_top_max, "other", tab_font, 13.0f, col_bg_group, col_highlight, col_outline);
            DrawGroupBox(dl, v_right_bot_min, v_right_bot_max, "misc", tab_font, 13.0f, col_bg_group, col_highlight, col_outline);
        }
        else
        {
            const char* group_titles_left[]  = { "aimbot", "visuals", "misc" };
            const char* group_titles_right[] = { "other", "other", "other" };
            DrawGroupBox(dl, left_min, left_max, group_titles_left[g_activeTab], tab_font, 13.0f, col_bg_group, col_highlight, col_outline);
            DrawGroupBox(dl, right_min, right_max, group_titles_right[g_activeTab], tab_font, 13.0f, col_bg_group, col_highlight, col_outline);
        }
        bool cb_hov = false;
        bool cp_hov = false;
        bool kb_hov = false;
        bool sl_hov = false;
        bool btn_hov = false;
        bool ti_hov = false;
        bool dd_hov = false;
        ImVec2 picker_sz(14.0f, 14.0f);
        const float swatch_pair_w = picker_sz.x * 2.0f + 6.0f;
        const float qb_right_off = 14.0f + swatch_pair_w + 6.0f;
        const float qb_single_swatch_off = 14.0f + picker_sz.x + 6.0f;
        float col_avail_w = (left_max.x - 14.0f) - (left_min.x + 14.0f);
        float col_avail_w_r = (right_max.x - 14.0f) - (right_min.x + 14.0f);
        if (g_activeTab == 0)
        {
            float y = left_min.y + 20.0f;
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::aim_enabled, "Enabled", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##aim_key", dl, ImVec2(left_max.x - 14.0f, y), &S::aim_key, &S::aim_key_mode, tab_font, 13.0f);
            if (S::aim_method == 3)
            {
                char kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##aim_key") == 0)
                    snprintf(kb_buf, sizeof(kb_buf), "[ ]");
                else if (S::aim_key == 0)
                    snprintf(kb_buf, sizeof(kb_buf), "[ . . . ]");
                else
                    snprintf(kb_buf, sizeof(kb_buf), "[ %s ]", GetKeyDisplayName(S::aim_key));
                ImFont* qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 kb_sz = qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, kb_buf);
                QuestionItem aim_items[] = {
                    MakeQCheckbox("MAGIC BULLET", &S::aim_magicbullet)
                };
                kb_hov |= DrawQuestionButton("##qb_aim", dl, ImVec2(left_max.x - 14.0f - kb_sz.x - 8.0f, y), aim_items, 1, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            {
                const char* method_items[] = { "Memory", "Mouse", "Viewport", "Raycast", "Wraith" };
                dd_hov |= DrawDropdownSingle("##dd_aimmethod", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_method, method_items, 5, "Method", tab_font, 13.0f);
                y += 37.0f;
                const char* part_items[] = { "Head", "Torso", "Pelvis", "Left arm", "Right arm", "Left leg", "Right leg" };
                dd_hov |= DrawDropdownSingle("##dd_aimpart", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_part, part_items, 7, "Target part", tab_font, 13.0f);
                y += 37.0f;
                static const char* const kPriorityItems[] = { "Crosshair", "Distance", "Camera" };
                dd_hov |= DrawDropdownSingle("##dd_priority", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_priority, kPriorityItems, 3, "Priority", tab_font, 13.0f);
                y += 37.0f;
                sl_hov |= DrawSliderFloat("##sl_aimfov", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_fov, 10.0f, 500.0f, "Aim FOV", "%.0f", tab_font, 13.0f);
                y += 33.0f;
                cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::aim_360, "360 mode", tab_font, 13.0f);
                y += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::aim_smoothing_enabled, "Smoothing", tab_font, 13.0f);
                y += kCheckRowPitch;
                bool h_smx = false, h_smy = false;
                if (S::aim_smoothing_enabled)
                {
                    h_smx = DrawSliderFloat("##sl_smoothx", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_smoothing_x, 1.0f, 30.0f, "Smooth X", "%.1f", tab_font, 13.0f);
                    y += 33.0f;
                    h_smy = DrawSliderFloat("##sl_smoothy", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_smoothing_y, 1.0f, 30.0f, "Smooth Y", "%.1f", tab_font, 13.0f);
                    y += 33.0f;
                }
                sl_hov |= h_smx || h_smy;
                cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::aim_deadzone_enabled, "Deadzone", tab_font, 13.0f);
                y += kCheckRowPitch;
                if (S::aim_deadzone_enabled)
                {
                    sl_hov |= DrawMinMaxSlider("##sl_deadzone", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::aim_deadzone_min, &S::aim_deadzone_max, 0, 150, "Deadzone", "%d - %d", tab_font, 13.0f);
                    y += 33.0f;
                }
            }
            float ry = right_min.y + 20.0f;
            {
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_show_fov, "Show FOV", tab_font, 13.0f);
                cp_hov |= DrawColorPicker("##col_fov", dl, ImVec2(right_max.x - 14.0f - picker_sz.x, ry), picker_sz,
                    reinterpret_cast<ImVec4*>(S::aim_fov_color), &S::aim_fov_outline, reinterpret_cast<ImVec4*>(S::aim_fov_outline_color));
                {
                    static const char* const kAimFovStyleItems[] = { "Ring", "Dots", "Box", "Hexagon", "Diamond" };
                    QuestionItem fov_items[] = {
                        MakeQCheckbox("OUTLINE", &S::aim_fov_outline),
                        MakeQCheckboxColor("##q_fovfc", "FILL", &S::aim_fov_fill, reinterpret_cast<ImVec4*>(S::aim_fov_fill_color)),
                        MakeQDropdownSingle("##q_fovstyle", "Style", &S::aim_fov_style, kAimFovStyleItems, 5)
                    };
                    kb_hov |= DrawQuestionButton("##qb_fov", dl, ImVec2(right_max.x - qb_right_off, ry), fov_items, 3, tab_font, 13.0f);
                }
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_tracer, "Aim tracer", tab_font, 13.0f);
                cp_hov |= DrawColorPicker("##col_aimtracer", dl, ImVec2(right_max.x - 14.0f - picker_sz.x, ry), picker_sz,
                    reinterpret_cast<ImVec4*>(S::aim_tracer_color), &S::aim_tracer_outline, reinterpret_cast<ImVec4*>(S::aim_tracer_outline_color));
                {
                    QuestionItem aimtracer_items[] = {
                        MakeQCheckbox("OUTLINE", &S::aim_tracer_outline),
                        MakeQSliderFloat("##q_aimtracerthick", "Thickness", &S::aim_tracer_thickness, 0.5f, 5.0f, "%.1f")
                    };
                    kb_hov |= DrawQuestionButton("##qb_aimtracer", dl, ImVec2(right_max.x - qb_right_off, ry), aimtracer_items, 2, tab_font, 13.0f);
                }
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_prediction, "Prediction", tab_font, 13.0f);
                {
                    static const char* const kPredModeItems[] = { "Global", "Fallen", "Dynamic" };
                    QuestionItem pred_items[8];
                    int pred_n = 0;
                    pred_items[pred_n++] = MakeQDropdownSingle("##q_predmode", "Mode", &S::aim_pred_mode, kPredModeItems, 3);
                    if (S::aim_pred_mode == 1)
                    {
                        pred_items[pred_n++] = MakeQDropdownSingle("##q_predweapon", "Weapon", &S::aim_weapon, thenwefuckin::aim::predict::FALLEN_WEAPON_ITEMS, thenwefuckin::aim::predict::FALLEN_WEAPON_COUNT);
                        pred_items[pred_n++] = MakeQSliderFloat("##q_predbv", "Bullet vel", &S::aim_fallen_bv, -1.0f, 3000.0f, "%.0f");
                        pred_items[pred_n++] = MakeQSliderFloat("##q_predgrav", "Gravity mult", &S::aim_fallen_grav, 0.0f, 2.0f, "%.2f");
                        static char predDetBuf[128];
                        snprintf(predDetBuf, sizeof(predDetBuf), "Detected: %s", thenwefuckin::aim::predict::detected_weapon_name.c_str());
                        pred_items[pred_n++] = MakeQLabel(predDetBuf);
                        const thenwefuckin::aim::predict::FallenWeaponBallistics* ball = nullptr;
                        if (S::aim_weapon > 0 && S::aim_weapon < thenwefuckin::aim::predict::FALLEN_WEAPON_COUNT - 1)
                            ball = thenwefuckin::aim::predict::fallen_get_weapon_data(thenwefuckin::aim::predict::FALLEN_WEAPON_LIST[(size_t)S::aim_weapon]);
                        else if (!thenwefuckin::aim::predict::detected_weapon_name.empty() && thenwefuckin::aim::predict::detected_weapon_name != "None")
                            ball = thenwefuckin::aim::predict::fallen_get_weapon_data(thenwefuckin::aim::predict::detected_weapon_name);
                        if (ball)
                        {
                            static char predBvBuf[128];
                            snprintf(predBvBuf, sizeof(predBvBuf), "BV: %.0f  Gravity: %.2f", (double)ball->bv, (double)ball->grav);
                            pred_items[pred_n++] = MakeQLabel(predBvBuf);
                        }
                    }
                    else
                    {
                        pred_items[pred_n++] = MakeQSliderFloat("##q_predh", "Height", &S::aim_pred_height, -5.0f, 10.0f, "%.1f");
                        pred_items[pred_n++] = MakeQSliderFloat("##q_predf", "Forward", &S::aim_pred_forward, -10.0f, 20.0f, "%.1f");
                    }
                    kb_hov |= DrawQuestionButton("##qb_pred", dl, ImVec2(right_max.x - 14.0f, ry), pred_items, pred_n, tab_font, 13.0f);
                }
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_distance_check, "Distance check", tab_font, 13.0f);
                {
                    QuestionItem dist_items[] = {
                        MakeQSliderFloat("##q_aimdist", "Max distance", &S::aim_distance_value, 10.0f, 2000.0f, "%.0f")
                    };
                    kb_hov |= DrawQuestionButton("##qb_aimdist", dl, ImVec2(right_max.x - 14.0f, ry), dist_items, 1, tab_font, 13.0f);
                }
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_health_check, "Health check", tab_font, 13.0f);
                {
                    QuestionItem hp_items[] = {
                        MakeQSliderFloat("##q_aimhp", "Max health", &S::aim_health_value, 1.0f, 100.0f, "%.0f")
                    };
                    kb_hov |= DrawQuestionButton("##qb_aimhp", dl, ImVec2(right_max.x - 14.0f, ry), hp_items, 1, tab_font, 13.0f);
                }
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_team_check, "Team check", tab_font, 13.0f);
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_visible_check, "Visible check", tab_font, 13.0f);
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_bots, "Include bots", tab_font, 13.0f);
                ry += kCheckRowPitch;
                cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::aim_autoshoot, "Auto shoot", tab_font, 13.0f);
                kb_hov |= DrawKeybind("##autoshoot_key", dl, ImVec2(right_max.x - 14.0f, ry), &S::aim_autoshoot_key, &S::aim_autoshoot_key_mode, tab_font, 13.0f);
                ry += kCheckRowPitch;
                if (S::aim_autoshoot)
                {
                    sl_hov |= DrawSliderFloat("##sl_shootdelay", dl, ImVec2(right_min.x + 14.0f, ry), col_avail_w_r, &S::aim_autoshoot_delay, 0.0f, 500.0f, "Shoot delay (ms)", "%.0f", tab_font, 13.0f);
                    ry += 33.0f;
                }
            }
        }
        else if (g_activeTab == 1)
        {
            float y = v_left_top_min.y + 20.0f;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::box_enabled, "Box", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_box", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::box_color), &S::box_outline, reinterpret_cast<ImVec4*>(S::box_outline_color));
            {
                static const char* const kBoxModeItems[] = { "Static", "Dynamic" };
                static const char* const kBoxStyleItems[] = { "Full", "Corner" };
                QuestionItem box_items[] = {
                    MakeQCheckbox("OUTLINE", &S::box_outline),
                    MakeQCheckboxColorPair("##q_boxfc", "FILLED", &S::box_filled, reinterpret_cast<ImVec4*>(S::box_fill_color),
                        "##q_boxfc2", reinterpret_cast<ImVec4*>(S::box_fill_color2)),
                    MakeQCheckbox("GRADIENT", &S::box_fill_gradient, &S::box_filled),
                    MakeQDropdownSingle("##q_boxmode", "Box mode", &S::box_mode, kBoxModeItems, 2),
                    MakeQDropdownSingle("##q_boxstyle", "Box style", &S::box_style, kBoxStyleItems, 2)
                };
                kb_hov |= DrawQuestionButton("##qb_box", dl, ImVec2(v_left_top_max.x - qb_right_off, y), box_items, 5, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::skeleton_enabled, "Skeleton", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_skel", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::skeleton_color), &S::skeleton_outline, reinterpret_cast<ImVec4*>(S::skeleton_outline_color));
            {
                QuestionItem skel_items[] = {
                    MakeQCheckbox("OUTLINE", &S::skeleton_outline)
                };
                kb_hov |= DrawQuestionButton("##qb_skel", dl, ImVec2(v_left_top_max.x - qb_right_off, y), skel_items, 1, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::health_enabled, "Health bar", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_health", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::health_color), &S::health_outline, reinterpret_cast<ImVec4*>(S::health_outline_color));
            {
                QuestionItem hbar_items[] = {
                    MakeQCheckbox("OUTLINE", &S::health_outline),
                    MakeQCheckbox("GRADIENT", &S::health_gradient),
                    MakeQCheckbox("TEXT", &S::health_text)
                };
                kb_hov |= DrawQuestionButton("##qb_health", dl, ImVec2(v_left_top_max.x - qb_right_off, y), hbar_items, 3, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::name_enabled, "Name", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_name", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::name_color), &S::name_outline, reinterpret_cast<ImVec4*>(S::name_outline_color));
            {
                QuestionItem name_items[] = {
                    MakeQCheckbox("OUTLINE", &S::name_outline)
                };
                kb_hov |= DrawQuestionButton("##qb_name", dl, ImVec2(v_left_top_max.x - qb_right_off, y), name_items, 1, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::headdot_enabled, "Head dot", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_headdot", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::headdot_color), &S::headdot_outline, reinterpret_cast<ImVec4*>(S::headdot_outline_color));
            {
                QuestionItem hd_items[] = {
                    MakeQCheckbox("FILL", &S::headdot_fill),
                    MakeQCheckbox("OUTLINE", &S::headdot_outline),
                    MakeQSliderFloat("##q_headdotsz", "Dot size", &S::headdot_thickness, 1.0f, 10.0f, "%.1f")
                };
                kb_hov |= DrawQuestionButton("##qb_headdot", dl, ImVec2(v_left_top_max.x - qb_right_off, y), hd_items, 3, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::distance_enabled, "Distance", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_dist", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::distance_color), &S::distance_outline, reinterpret_cast<ImVec4*>(S::distance_outline_color));
            {
                QuestionItem dist_items[] = {
                    MakeQCheckbox("OUTLINE", &S::distance_outline),
                    MakeQSliderFloat("##q_vismaxdist", "Max distance", &S::visual_maxdist, 100.0f, 10000.0f, "%.0f")
                };
                kb_hov |= DrawQuestionButton("##qb_dist", dl, ImVec2(v_left_top_max.x - qb_right_off, y), dist_items, 2, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::tool_enabled, "Tool", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_tool", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::tool_color), &S::tool_outline, reinterpret_cast<ImVec4*>(S::tool_outline_color));
            {
                QuestionItem tool_items[] = {
                    MakeQCheckbox("OUTLINE", &S::tool_outline),
                    MakeQCheckbox("DIST CHECK", &S::tool_distance_check),
                    MakeQSliderFloat("##q_tooldist", "Max distance", &S::tool_distance_value, 10.0f, 2000.0f, "%.0f", &S::tool_distance_check)
                };
                kb_hov |= DrawQuestionButton("##qb_tool", dl, ImVec2(v_left_top_max.x - qb_right_off, y), tool_items, 3, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::flags_enabled, "Flags", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_flags", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::flags_color), &S::flags_outline, reinterpret_cast<ImVec4*>(S::flags_outline_color));
            {
                QuestionItem flags_items[] = {
                    MakeQCheckbox("GLOBAL", &S::flags_global),
                    MakeQCheckboxColor("##q_flags_team", "TEAM", &S::flags_team, reinterpret_cast<ImVec4*>(S::flags_team_color), &S::flags_team_outline, reinterpret_cast<ImVec4*>(S::flags_team_outline_color)),
                    MakeQCheckboxColor("##q_flags_state", "STATE", &S::flags_state, reinterpret_cast<ImVec4*>(S::flags_state_color), &S::flags_state_outline, reinterpret_cast<ImVec4*>(S::flags_state_outline_color)),
                    MakeQCheckboxColor("##q_flags_health", "HEALTH", &S::flags_health, reinterpret_cast<ImVec4*>(S::flags_health_color), &S::flags_health_outline, reinterpret_cast<ImVec4*>(S::flags_health_outline_color))
                };
                kb_hov |= DrawQuestionButton("##qb_flags", dl, ImVec2(v_left_top_max.x - qb_right_off, y), flags_items, 4, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_top_min.x + 14.0f, y), &S::tracer_enabled, "Tracer", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_tracer", dl, ImVec2(v_left_top_max.x - 14.0f - picker_sz.x, y), picker_sz,
                reinterpret_cast<ImVec4*>(S::tracer_color), &S::tracer_outline, reinterpret_cast<ImVec4*>(S::tracer_outline_color));
            {
                static const char* const kTracerOriginItems[] = { "Bottom", "Top", "Center" };
                QuestionItem tracer_items[] = {
                    MakeQCheckbox("OUTLINE", &S::tracer_outline),
                    MakeQSliderFloat("##q_tracerthick", "Thickness", &S::tracer_thickness, 0.5f, 5.0f, "%.1f"),
                    MakeQDropdownSingle("##q_tracerorigin", "Origin", &S::tracer_origin, kTracerOriginItems, 3)
                };
                kb_hov |= DrawQuestionButton("##qb_tracer", dl, ImVec2(v_left_top_max.x - qb_right_off, y), tracer_items, 3, tab_font, 13.0f);
            }
            y += kCheckRowPitch;
            y = v_left_bot_min.y + 20.0f;
            static float s_worldScroll = 0.0f;
            static float s_worldMaxScroll = 0.0f;
            if (s_worldScroll < 0.0f) s_worldScroll = 0.0f;
            if (s_worldScroll > s_worldMaxScroll) s_worldScroll = s_worldMaxScroll;
            const float wb_contentTop = y;
            const ImVec2 wb_viewMin(v_left_bot_min.x + 1.0f, v_left_bot_min.y + 18.0f);
            const ImVec2 wb_viewMax(v_left_bot_max.x - 1.0f, v_left_bot_max.y - 5.0f);
            y -= s_worldScroll;
            dl->PushClipRect(wb_viewMin, wb_viewMax, true);
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::time_enabled, "Time changer", tab_font, 13.0f);
            {
                QuestionItem time_items[] = {
                    MakeQSliderFloat("##q_timeval", "Time", &S::time_value, 0.0f, 24.0f, "%.1f")
                };
                kb_hov |= DrawQuestionButton("##qb_time", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##time_key", S::time_key, tab_font) - 8.0f, y), time_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##time_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::time_key, &S::time_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::world_fog_enabled, "Fog", tab_font, 13.0f);
            {
                QuestionItem fog_items[] = {
                    MakeQSliderFloat("##q_fogstart", "Start", &S::world_fog_start, 0.0f, 2000.0f, "%.0f"),
                    MakeQSliderFloat("##q_fogend", "End", &S::world_fog_end, 0.0f, 5000.0f, "%.0f"),
                    MakeQColorPicker("##q_fogcol", "Color", reinterpret_cast<ImVec4*>(S::world_fog_color), nullptr)
                };
                kb_hov |= DrawQuestionButton("##qb_fog", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##fog_key", S::world_fog_key, tab_font) - 8.0f, y), fog_items, 3, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##fog_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::world_fog_key, &S::world_fog_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::world_exposure_enabled, "Exposure", tab_font, 13.0f);
            {
                QuestionItem exp_items[] = {
                    MakeQSliderFloat("##q_expval", "Exposure", &S::world_exposure_value, -5.0f, 5.0f, "%.2f")
                };
                kb_hov |= DrawQuestionButton("##qb_exposure", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##exposure_key", S::world_exposure_key, tab_font) - 8.0f, y), exp_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##exposure_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::world_exposure_key, &S::world_exposure_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::ambient_enabled, "Ambient", tab_font, 13.0f);
            {
                QuestionItem amb_items[] = {
                    MakeQColorPicker("##q_ambcol", "Color", reinterpret_cast<ImVec4*>(S::ambient_color), nullptr)
                };
                kb_hov |= DrawQuestionButton("##qb_ambient", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##ambient_key", S::ambient_key, tab_font) - 8.0f, y), amb_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##ambient_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::ambient_key, &S::ambient_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::fullbright_enabled, "Fullbright", tab_font, 13.0f);
            {
                QuestionItem fb_items[] = {
                    MakeQSliderFloat("##q_fbval", "Brightness", &S::fullbright_value, 1.0f, 5.0f, "%.1f")
                };
                kb_hov |= DrawQuestionButton("##qb_fullbright", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##fullbright_key", S::fullbright_key, tab_font) - 8.0f, y), fb_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##fullbright_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::fullbright_key, &S::fullbright_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::night_enabled, "Night mode", tab_font, 13.0f);
            {
                QuestionItem night_items[] = {
                    MakeQSliderFloat("##q_nightlvl", "Darkness", &S::night_level, 0.0f, 1.0f, "%.2f")
                };
                kb_hov |= DrawQuestionButton("##qb_night", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##night_key", S::night_key, tab_font) - 8.0f, y), night_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##night_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::night_key, &S::night_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::noshadow_enabled, "No shadows", tab_font, 13.0f);
            {
                QuestionItem sh_items[] = {
                    MakeQSliderFloat("##q_shsoft", "Softness", &S::shadow_softness, 0.0f, 1.0f, "%.2f")
                };
                kb_hov |= DrawQuestionButton("##qb_shadow", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##noshadow_key", S::noshadow_key, tab_font) - 8.0f, y), sh_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##noshadow_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::noshadow_key, &S::noshadow_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_left_bot_min.x + 14.0f, y), &S::nograss_enabled, "No grass", tab_font, 13.0f);
            {
                QuestionItem gr_items[] = {
                    MakeQSliderFloat("##q_grassval", "Height", &S::grass_value, 0.0f, 3.0f, "%.2f")
                };
                kb_hov |= DrawQuestionButton("##qb_grass", dl, ImVec2(v_left_bot_max.x - 14.0f - KeybindTextWidth("##nograss_key", S::nograss_key, tab_font) - 8.0f, y), gr_items, 1, tab_font, 13.0f);
            }
            kb_hov |= DrawKeybind("##nograss_key", dl, ImVec2(v_left_bot_max.x - 14.0f, y), &S::nograss_key, &S::nograss_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            {
                float wb_contentH = (y + s_worldScroll) - wb_contentTop;
                float wb_viewH = wb_viewMax.y - wb_viewMin.y;
                s_worldMaxScroll = (wb_contentH > wb_viewH) ? (wb_contentH - wb_viewH) : 0.0f;
                if (s_worldScroll > s_worldMaxScroll) s_worldScroll = s_worldMaxScroll;
                dl->PopClipRect();
                ImGuiIO& wb_io = ImGui::GetIO();
                bool wb_in = (wb_io.MousePos.x >= wb_viewMin.x && wb_io.MousePos.x <= wb_viewMax.x
                    && wb_io.MousePos.y >= wb_viewMin.y && wb_io.MousePos.y <= wb_viewMax.y);
                if (g_menuOpen && wb_in && wb_io.MouseWheel != 0.0f && !sl_hov && g_activeBindId == nullptr
                    && !wb_io.WantTextInput && !IsModalActive() && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup))
                {
                    s_worldScroll -= wb_io.MouseWheel * 28.0f;
                    if (s_worldScroll < 0.0f) s_worldScroll = 0.0f;
                    if (s_worldScroll > s_worldMaxScroll) s_worldScroll = s_worldMaxScroll;
                }
            }
            float ry = right_min.y + 20.0f;
            cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::oof_enabled, "Off-screen", tab_font, 13.0f);
            cp_hov |= DrawColorPicker("##col_oof", dl, ImVec2(right_max.x - 14.0f - picker_sz.x, ry), picker_sz,
                reinterpret_cast<ImVec4*>(S::oof_color), &S::oof_outline, reinterpret_cast<ImVec4*>(S::oof_outline_color));
            {
                static const char* const kOofStyleItems[] = { "Arrow", "Chevron", "Triangle" };
                QuestionItem oof_items[] = {
                    MakeQCheckbox("OUTLINE", &S::oof_outline),
                    MakeQCheckbox("NAMES", &S::oof_names),
                    MakeQCheckbox("HEALTH", &S::oof_health),
                    MakeQCheckbox("DISTANCE", &S::oof_distance),
                    MakeQCheckbox("AVATAR", &S::oof_avatar),
                    MakeQSliderFloat("##q_oofsize", "Arrow size", &S::oof_size, 3.0f, 25.0f, "%.0f"),
                    MakeQSliderFloat("##q_oofradius", "Radius", &S::oof_radius, 30.0f, 500.0f, "%.0f"),
                    MakeQDropdownSingle("##q_oofstyle", "Arrow style", &S::oof_arrow_style, kOofStyleItems, 3)
                };
                kb_hov |= DrawQuestionButton("##qb_oof", dl, ImVec2(right_max.x - qb_right_off, ry), oof_items, 8, tab_font, 13.0f);
            }
            ry += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::vis_team_check, "Team check", tab_font, 13.0f);
            ry += kCheckRowPitch;
            float my = v_right_bot_min.y + 20.0f;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_ore_enabled, "Ores", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##ore_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_ore_key, &S::world_ore_key_mode, tab_font, 13.0f);
            {
                char ore_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##ore_key") == 0)
                    snprintf(ore_kb_buf, sizeof(ore_kb_buf), "[ ]");
                else if (S::world_ore_key == 0)
                    snprintf(ore_kb_buf, sizeof(ore_kb_buf), "[ . . . ]");
                else
                    snprintf(ore_kb_buf, sizeof(ore_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_ore_key));
                ImFont* ore_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 ore_kb_sz = ore_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, ore_kb_buf);
                const char* ore_sub_ids[3] = { "##qb_ore_sub0", "##qb_ore_sub1", "##qb_ore_sub2" };
                const char* ore_labels[3] = { "STONE", "PHOSPHATE", "METAL" };
                static bool s_oreSubOpen[3] = { false, false, false };
                QuestionItem ore_sub[3][2];
                for (int i = 0; i < 3; ++i)
                {
                    ore_sub[i][0] = MakeQCheckbox("NAME", &S::world_ore_name[i]);
                    ore_sub[i][1] = MakeQCheckbox("DIST", &S::world_ore_dist[i]);
                }
                QuestionItem ore_items[3];
                for (int i = 0; i < 3; ++i)
                {
                    ore_items[i] = MakeQSubmenu(ore_sub_ids[i], ore_labels[i], ore_sub[i], 2, &s_oreSubOpen[i]);
                    ore_items[i].bool_val = (i == 0) ? &S::world_ore_stone : (i == 1) ? &S::world_ore_phosphate : &S::world_ore_metal;
                    ore_items[i].col_val = reinterpret_cast<ImVec4*>((i == 0) ? S::world_ore_stone_color : (i == 1) ? S::world_ore_phosphate_color : S::world_ore_metal_color);
                }
                kb_hov |= DrawQuestionButton("##qb_ore", dl, ImVec2(v_right_bot_max.x - 14.0f - ore_kb_sz.x - 8.0f, my), ore_items, 3, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_plant_enabled, "Plants", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##plant_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_plant_key, &S::world_plant_key_mode, tab_font, 13.0f);
            {
                char plant_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##plant_key") == 0)
                    snprintf(plant_kb_buf, sizeof(plant_kb_buf), "[ ]");
                else if (S::world_plant_key == 0)
                    snprintf(plant_kb_buf, sizeof(plant_kb_buf), "[ . . . ]");
                else
                    snprintf(plant_kb_buf, sizeof(plant_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_plant_key));
                ImFont* plant_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 plant_kb_sz = plant_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, plant_kb_buf);
                const char* plant_ids[7] = { "##q_plant0", "##q_plant1", "##q_plant2", "##q_plant3", "##q_plant4", "##q_plant5", "##q_plant6" };
                const char* plant_labels[7] = { "WOOL", "BLUEBERRY", "RASPBERRY", "LEMON", "CORN", "PUMPKIN", "TOMATO" };
                const char* plant_sub_ids[7] = { "##qb_plant_sub0", "##qb_plant_sub1", "##qb_plant_sub2", "##qb_plant_sub3", "##qb_plant_sub4", "##qb_plant_sub5", "##qb_plant_sub6" };
                static bool s_plantSubOpen[7] = { false, false, false, false, false, false, false };
                QuestionItem plant_sub[7][2];
                for (int i = 0; i < 7; ++i)
                {
                    plant_sub[i][0] = MakeQCheckbox("NAME", &S::world_plant_name[i]);
                    plant_sub[i][1] = MakeQCheckbox("DIST", &S::world_plant_dist[i]);
                }
                QuestionItem plant_items[7];
                for (int i = 0; i < 7; ++i)
                {
                    plant_items[i] = MakeQSubmenu(plant_sub_ids[i], plant_labels[i], plant_sub[i], 2, &s_plantSubOpen[i]);
                    plant_items[i].bool_val = &S::world_plant_sel[i];
                    plant_items[i].col_val = reinterpret_cast<ImVec4*>(S::world_plant_color[i]);
                }
                kb_hov |= DrawQuestionButton("##qb_plant", dl, ImVec2(v_right_bot_max.x - 14.0f - plant_kb_sz.x - 8.0f, my), plant_items, 7, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_bots_enabled, "Bots", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##bots_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_bots_key, &S::world_bots_key_mode, tab_font, 13.0f);
            {
                char bots_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##bots_key") == 0)
                    snprintf(bots_kb_buf, sizeof(bots_kb_buf), "[ ]");
                else if (S::world_bots_key == 0)
                    snprintf(bots_kb_buf, sizeof(bots_kb_buf), "[ . . . ]");
                else
                    snprintf(bots_kb_buf, sizeof(bots_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_bots_key));
                ImFont* bots_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 bots_kb_sz = bots_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, bots_kb_buf);
                const char* bots_sub_ids[4] = { "##qb_bots_sub0", "##qb_bots_sub1", "##qb_bots_sub2", "##qb_bots_sub3" };
                const char* bots_labels[4] = { "MILITARY", "ZOMBIE", "BTR", "HELI" };
                static bool s_botsSubOpen[4] = { false, false, false, false };
                QuestionItem bots_sub[4][3];
                for (int i = 0; i < 4; ++i)
                {
                    bots_sub[i][0] = MakeQCheckbox("BOX", &S::world_bots_box[i]);
                    bots_sub[i][1] = MakeQCheckbox("NAME", &S::world_bots_name[i]);
                    bots_sub[i][2] = MakeQCheckbox("DIST", &S::world_bots_dist[i]);
                }
                QuestionItem bots_items[4];
                for (int i = 0; i < 4; ++i)
                {
                    bots_items[i] = MakeQSubmenu(bots_sub_ids[i], bots_labels[i], bots_sub[i], 3, &s_botsSubOpen[i]);
                    bots_items[i].bool_val = &S::world_bots_sel[i];
                    bots_items[i].col_val = reinterpret_cast<ImVec4*>(S::world_bots_color[i]);
                }
                kb_hov |= DrawQuestionButton("##qb_bots", dl, ImVec2(v_right_bot_max.x - 14.0f - bots_kb_sz.x - 8.0f, my), bots_items, 4, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_animals_enabled, "Animals", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##animals_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_animals_key, &S::world_animals_key_mode, tab_font, 13.0f);
            {
                char animals_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##animals_key") == 0)
                    snprintf(animals_kb_buf, sizeof(animals_kb_buf), "[ ]");
                else if (S::world_animals_key == 0)
                    snprintf(animals_kb_buf, sizeof(animals_kb_buf), "[ . . . ]");
                else
                    snprintf(animals_kb_buf, sizeof(animals_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_animals_key));
                ImFont* animals_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 animals_kb_sz = animals_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, animals_kb_buf);
                const char* animals_sub_ids[3] = { "##qb_animals_sub0", "##qb_animals_sub1", "##qb_animals_sub2" };
                const char* animals_labels[3] = { "DEER", "BOAR", "WOLF" };
                static bool s_animalsSubOpen[3] = { false, false, false };
                QuestionItem animals_sub[3][2];
                for (int i = 0; i < 3; ++i)
                {
                    animals_sub[i][0] = MakeQCheckbox("NAME", &S::world_animals_name[i]);
                    animals_sub[i][1] = MakeQCheckbox("DIST", &S::world_animals_dist[i]);
                }
                QuestionItem animals_items[3];
                for (int i = 0; i < 3; ++i)
                {
                    animals_items[i] = MakeQSubmenu(animals_sub_ids[i], animals_labels[i], animals_sub[i], 2, &s_animalsSubOpen[i]);
                    animals_items[i].bool_val = &S::world_animals_sel[i];
                    animals_items[i].col_val = reinterpret_cast<ImVec4*>(S::world_animals_color[i]);
                }
                kb_hov |= DrawQuestionButton("##qb_animals", dl, ImVec2(v_right_bot_max.x - 14.0f - animals_kb_sz.x - 8.0f, my), animals_items, 3, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_tools_enabled, "Tools", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##tools_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_tools_key, &S::world_tools_key_mode, tab_font, 13.0f);
            {
                char tools_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##tools_key") == 0)
                    snprintf(tools_kb_buf, sizeof(tools_kb_buf), "[ ]");
                else if (S::world_tools_key == 0)
                    snprintf(tools_kb_buf, sizeof(tools_kb_buf), "[ . . . ]");
                else
                    snprintf(tools_kb_buf, sizeof(tools_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_tools_key));
                ImFont* tools_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 tools_kb_sz = tools_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, tools_kb_buf);
                const char* tools_sub_ids[7] = { "##qb_tools_sub0", "##qb_tools_sub1", "##qb_tools_sub2", "##qb_tools_sub3", "##qb_tools_sub4", "##qb_tools_sub5", "##qb_tools_sub6" };
                const char* tools_labels[7] = { "CABINET", "SMALL BOX", "LARGE BOX", "ANVIL", "FURNACE", "STORAGE", "BAG" };
                static bool s_toolsSubOpen[7] = { false, false, false, false, false, false, false };
                QuestionItem tools_sub[7][2];
                for (int i = 0; i < 7; ++i)
                {
                    tools_sub[i][0] = MakeQCheckbox("NAME", &S::world_tools_name[i]);
                    tools_sub[i][1] = MakeQCheckbox("DIST", &S::world_tools_dist[i]);
                }
                QuestionItem tools_items[7];
                for (int j = 0; j < 7; ++j)
                {
                    tools_items[j] = MakeQSubmenu(tools_sub_ids[j], tools_labels[j], tools_sub[j], 2, &s_toolsSubOpen[j]);
                    tools_items[j].bool_val = &S::world_tools_sel[j];
                    tools_items[j].col_val = reinterpret_cast<ImVec4*>(S::world_tools_color[j]);
                }
                kb_hov |= DrawQuestionButton("##qb_tools", dl, ImVec2(v_right_bot_max.x - 14.0f - tools_kb_sz.x - 8.0f, my), tools_items, 7, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_crates_enabled, "Crates", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##crates_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_crates_key, &S::world_crates_key_mode, tab_font, 13.0f);
            {
                char crates_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##crates_key") == 0)
                    snprintf(crates_kb_buf, sizeof(crates_kb_buf), "[ ]");
                else if (S::world_crates_key == 0)
                    snprintf(crates_kb_buf, sizeof(crates_kb_buf), "[ . . . ]");
                else
                    snprintf(crates_kb_buf, sizeof(crates_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_crates_key));
                ImFont* crates_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 crates_kb_sz = crates_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, crates_kb_buf);
                const char* crates_sub_ids[8] = { "##qb_crates_sub0", "##qb_crates_sub1", "##qb_crates_sub2", "##qb_crates_sub3", "##qb_crates_sub4", "##qb_crates_sub5", "##qb_crates_sub6", "##qb_crates_sub7" };
                const char* crates_labels[8] = { "CARE", "FOOD", "WOODEN", "LOCKED WD", "LOCKED ST", "LOCKED MT", "TIMED", "BRUTUS" };
                static bool s_cratesSubOpen[8] = { false, false, false, false, false, false, false, false };
                QuestionItem crates_sub[8][2];
                for (int i = 0; i < 8; ++i)
                {
                    crates_sub[i][0] = MakeQCheckbox("NAME", &S::world_crates_name[i]);
                    crates_sub[i][1] = MakeQCheckbox("DIST", &S::world_crates_dist[i]);
                }
                QuestionItem crates_items[8];
                for (int i = 0; i < 8; ++i)
                {
                    crates_items[i] = MakeQSubmenu(crates_sub_ids[i], crates_labels[i], crates_sub[i], 2, &s_cratesSubOpen[i]);
                    crates_items[i].bool_val = &S::world_crates_sel[i];
                    crates_items[i].col_val = reinterpret_cast<ImVec4*>(S::world_crates_color[i]);
                }
                kb_hov |= DrawQuestionButton("##qb_crates", dl, ImVec2(v_right_bot_max.x - 14.0f - crates_kb_sz.x - 8.0f, my), crates_items, 8, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(v_right_bot_min.x + 14.0f, my), &S::world_keycards_enabled, "Keycards", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##keycards_key", dl, ImVec2(v_right_bot_max.x - 14.0f, my), &S::world_keycards_key, &S::world_keycards_key_mode, tab_font, 13.0f);
            {
                char keycards_kb_buf[64];
                if (g_activeBindId != nullptr && strcmp(g_activeBindId, "##keycards_key") == 0)
                    snprintf(keycards_kb_buf, sizeof(keycards_kb_buf), "[ ]");
                else if (S::world_keycards_key == 0)
                    snprintf(keycards_kb_buf, sizeof(keycards_kb_buf), "[ . . . ]");
                else
                    snprintf(keycards_kb_buf, sizeof(keycards_kb_buf), "[ %s ]", GetKeyDisplayName(S::world_keycards_key));
                ImFont* keycards_qf = tab_font ? tab_font : ImGui::GetFont();
                ImVec2 keycards_kb_sz = keycards_qf->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, keycards_kb_buf);
                const char* keycards_sub_ids[5] = { "##qb_keycards_sub0", "##qb_keycards_sub1", "##qb_keycards_sub2", "##qb_keycards_sub3", "##qb_keycards_sub4" };
                const char* keycards_labels[5] = { "YELLOW", "PURPLE", "RED", "PINK", "BLACK" };
                static bool s_keycardsSubOpen[5] = { false, false, false, false, false };
                QuestionItem keycards_sub[5][2];
                for (int i = 0; i < 5; ++i)
                {
                    keycards_sub[i][0] = MakeQCheckbox("NAME", &S::world_keycards_name[i]);
                    keycards_sub[i][1] = MakeQCheckbox("DIST", &S::world_keycards_dist[i]);
                }
                QuestionItem keycards_items[5];
                for (int j = 0; j < 5; ++j)
                {
                    keycards_items[j] = MakeQSubmenu(keycards_sub_ids[j], keycards_labels[j], keycards_sub[j], 2, &s_keycardsSubOpen[j]);
                    keycards_items[j].bool_val = &S::world_keycards_sel[j];
                    keycards_items[j].col_val = reinterpret_cast<ImVec4*>(S::world_keycards_color[j]);
                }
                kb_hov |= DrawQuestionButton("##qb_keycards", dl, ImVec2(v_right_bot_max.x - 14.0f - keycards_kb_sz.x - 8.0f, my), keycards_items, 5, tab_font, 13.0f);
            }
            my += kCheckRowPitch;
        }
        else if (g_activeTab == 2)
        {
            float y = left_min.y + 20.0f;
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::walkspeed_enabled, "Walkspeed", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##walkspeed_key", dl, ImVec2(left_max.x - 14.0f, y), &S::walkspeed_key, &S::walkspeed_key_mode, tab_font, 13.0f);
            kb_hov |= DrawWarnIndicator("##warn_walkspeed", dl, ImVec2(left_max.x - 14.0f - KeybindTextWidth("##walkspeed_key", S::walkspeed_key, tab_font) - 8.0f, y), kBannableWarnText, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::walkspeed_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_walkspeed", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::walkspeed_value, 8.0f, 250.0f, "Speed", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::jumppower_enabled, "Jump power", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##jumppower_key", dl, ImVec2(left_max.x - 14.0f, y), &S::jumppower_key, &S::jumppower_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::jumppower_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_jumppower", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::jumppower_value, 10.0f, 300.0f, "Power", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::gravity_enabled, "Gravity", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##gravity_key", dl, ImVec2(left_max.x - 14.0f, y), &S::gravity_key, &S::gravity_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::gravity_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_gravity", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::gravity_value, 0.0f, 1000.0f, "Gravity", "%.1f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::nofall_enabled, "No fall damage", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##nofall_key", dl, ImVec2(left_max.x - 14.0f, y), &S::nofall_key, &S::nofall_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::nofall_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_nofall", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::nofall_value, 0.0f, 200.0f, "Max fall speed", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::fov_enabled, "Camera FOV", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##fov_key", dl, ImVec2(left_max.x - 14.0f, y), &S::fov_key, &S::fov_key_mode, tab_font, 13.0f);
            kb_hov |= DrawWarnIndicator("##warn_fov", dl, ImVec2(left_max.x - 14.0f - KeybindTextWidth("##fov_key", S::fov_key, tab_font) - 8.0f, y), kBannableWarnText, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::fov_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_camerafov", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::fov_value, 30.0f, 120.0f, "FOV", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::zoom_enabled, "Zoom", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##zoom_key", dl, ImVec2(left_max.x - 14.0f, y), &S::zoom_key, &S::zoom_key_mode, tab_font, 13.0f);
            kb_hov |= DrawWarnIndicator("##warn_zoom", dl, ImVec2(left_max.x - 14.0f - KeybindTextWidth("##zoom_key", S::zoom_key, tab_font) - 8.0f, y), kBannableWarnText, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::zoom_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_zoomfov", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::zoom_value, 5.0f, 60.0f, "Zoom", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::noclip_enabled, "Noclip", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##noclip_key", dl, ImVec2(left_max.x - 14.0f, y), &S::noclip_key, &S::noclip_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::fly_enabled, "Fly", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##fly_key", dl, ImVec2(left_max.x - 14.0f, y), &S::fly_key, &S::fly_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::fly_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_flyspeed", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::fly_speed, 5.0f, 500.0f, "Speed", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::freecam_enabled, "Freecam", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##freecam_key", dl, ImVec2(left_max.x - 14.0f, y), &S::freecam_key, &S::freecam_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::freecam_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_freecamspeed", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::freecam_speed, 5.0f, 500.0f, "Speed", "%.0f", tab_font, 13.0f);
                y += 33.0f;
                sl_hov |= DrawSliderFloat("##sl_freecamsens", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::freecam_sensitivity, 0.0005f, 0.01f, "Sensitivity", "%.4f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::hipheight_enabled, "Hip height", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##hipheight_key", dl, ImVec2(left_max.x - 14.0f, y), &S::hipheight_key, &S::hipheight_key_mode, tab_font, 13.0f);
            kb_hov |= DrawWarnIndicator("##warn_hipheight", dl, ImVec2(left_max.x - 14.0f - KeybindTextWidth("##hipheight_key", S::hipheight_key, tab_font) - 8.0f, y), kBannableWarnText, tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::hipheight_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_hipheight", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::hipheight_value, 0.0f, 20.0f, "Height", "%.1f", tab_font, 13.0f);
                y += 33.0f;
            }
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::infjump_enabled, "Infinite jump", tab_font, 13.0f);
            kb_hov |= DrawKeybind("##infjump_key", dl, ImVec2(left_max.x - 14.0f, y), &S::infjump_key, &S::infjump_key_mode, tab_font, 13.0f);
            y += kCheckRowPitch;
            cb_hov |= DrawCheckbox(dl, ImVec2(left_min.x + 14.0f, y), &S::antiafk_enabled, "Anti-AFK", tab_font, 13.0f);
            y += kCheckRowPitch;
            if (S::antiafk_enabled)
            {
                sl_hov |= DrawSliderFloat("##sl_antiafk", dl, ImVec2(left_min.x + 14.0f, y), col_avail_w, &S::antiafk_interval, 1.0f, 15.0f, "Minutes", "%.0f", tab_font, 13.0f);
                y += 33.0f;
            }
            float ry = right_min.y + 20.0f;
            cb_hov |= DrawCheckbox(dl, ImVec2(right_min.x + 14.0f, ry), &S::hitsound_enabled, "Hitsound", tab_font, 13.0f);
            ry += kCheckRowPitch;
            if (S::hitsound_enabled)
            {
                std::vector<std::string> sndList = thenwefuckin::misc::hitsound::get_sound_list();
                const char* sndItems[64];
                char sndBuf[64][64];
                int sndCount = (int)sndList.size();
                if (sndCount > 64) sndCount = 64;
                for (int i = 0; i < sndCount; i++)
                {
                    snprintf(sndBuf[i], sizeof(sndBuf[i]), "%s", sndList[i].c_str());
                    sndItems[i] = sndBuf[i];
                }
                if (sndCount == 0)
                {
                    sndItems[0] = "-";
                    sndCount = 1;
                }
                if (S::hitsound_selected < 0 || S::hitsound_selected >= sndCount) S::hitsound_selected = 0;
                dd_hov |= DrawDropdownSingle("##dd_hitsound", dl, ImVec2(right_min.x + 14.0f, ry), col_avail_w_r, &S::hitsound_selected, sndItems, sndCount, "Sound", tab_font, 13.0f);
                ry += 37.0f;
                const char* hitsoundmode_items[] = { "On damage", "On shoot" };
                dd_hov |= DrawDropdownSingle("##dd_hitsoundmode", dl, ImVec2(right_min.x + 14.0f, ry), col_avail_w_r, &S::hitsound_mode, hitsoundmode_items, 2, "Mode", tab_font, 13.0f);
                ry += 37.0f;
                sl_hov |= DrawSliderFloat("##sl_hitsoundvol", dl, ImVec2(right_min.x + 14.0f, ry), col_avail_w_r, &S::hitsound_volume, 0.0f, 100.0f, "Volume", "%.0f", tab_font, 13.0f);
                ry += 33.0f;
            }
            {
                float b3_w = floorf((col_avail_w_r - 8.0f) / 3.0f);
                bool ph = false;
                if (DrawButton(dl, ImVec2(right_min.x + 14.0f, ry), b3_w, 20.0f, "Play", tab_font, 13.0f, &ph))
                {
                    thenwefuckin::misc::hitsound::play();
                }
                btn_hov |= ph;
                bool fh = false;
                if (DrawButton(dl, ImVec2(right_min.x + 14.0f + b3_w + 4.0f, ry), b3_w, 20.0f, "Folder", tab_font, 13.0f, &fh))
                {
                    thenwefuckin::misc::hitsound::open_folder();
                }
                btn_hov |= fh;
                bool rh = false;
                if (DrawButton(dl, ImVec2(right_min.x + 14.0f + (b3_w + 4.0f) * 2.0f, ry), col_avail_w_r - (b3_w + 4.0f) * 2.0f, 20.0f, "Refresh", tab_font, 13.0f, &rh))
                {
                    thenwefuckin::misc::hitsound::refresh_sounds();
                }
                btn_hov |= rh;
                ry += 24.0f;
            }
        }
        UpdateThemeFromMask();
        bool popup_open = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        bool hovering = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= max.y);
        bool overPanel = g_settingsOpen && g_setPos.x >= 0.0f && (m.x >= g_setPos.x && m.x <= g_setPos.x + g_setSize.x && m.y >= g_setPos.y && m.y <= g_setPos.y + g_setSize.y);
        bool overTheme = g_themeOpen && g_themePos.x >= 0.0f && (m.x >= g_themePos.x && m.x <= g_themePos.x + g_themeSize.x && m.y >= g_themePos.y && m.y <= g_themePos.y + g_themeSize.y);
        bool overBinds = g_keybinds && (m.x >= g_bindsPos.x && m.x <= g_bindsPos.x + g_bindsSize.x && m.y >= g_bindsPos.y && m.y <= g_bindsPos.y + g_bindsSize.y);
        bool overMove = g_movement && (m.x >= g_movementPos.x && m.x <= g_movementPos.x + g_movementSize.x && m.y >= g_movementPos.y && m.y <= g_movementPos.y + g_movementSize.y);
        bool overDebug = g_internalDebug && (m.x >= g_debugPos.x && m.x <= g_debugPos.x + g_debugSize.x && m.y >= g_debugPos.y && m.y <= g_debugPos.y + g_debugSize.y);
        if (!g_dragging && ImGui::IsMouseClicked(0) && hovering && !popup_open && !overPanel && !overTheme && !overBinds && !overMove && !overDebug && g_activeBindId == nullptr && !tab_hovered && !g_navBarHov && !cb_hov && !cp_hov && !kb_hov && !sl_hov && !btn_hov && !ti_hov && !dd_hov)
        {
            g_dragging = true;
            g_menuAnim = 1.0f;
            g_dragOff = ImVec2(m.x - g_pos.x, m.y - g_pos.y);
        }
        if (g_dragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_pos.x = m.x - g_dragOff.x;
                g_pos.y = m.y - g_dragOff.y;
            }
            else
            {
                g_dragging = false;
            }
        }
        if (ease < 0.999f)
        {
            for (int i = 0; i < dl->VtxBuffer.Size; i++)
            {
                ImDrawVert& v = dl->VtxBuffer.Data[i];
                ImU32 a = (v.col >> 24) & 0xFF;
                a = (ImU32)((float)a * ease);
                v.col = (v.col & 0x00FFFFFF) | (a << 24);
            }
        }
    }
    ImGui::End();
    DrawHalloweenMascot();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(6);
}
inline std::string CfgEscapeStr(const char* s)
{
    std::string out;
    if (!s) return out;
    for (const char* p = s; *p; ++p)
    {
        unsigned char c = (unsigned char)*p;
        if (c == '\"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else if (c < 0x20) { char b[8]; snprintf(b, sizeof(b), "\\u%04x", c); out += b; }
        else out += (char)c;
    }
    return out;
}
struct CfgJsonWriter
{
    std::string s;
    bool first = true;
    void key(const char* k)
    {
        if (!first) s += ",";
        first = false;
        s += "\"";
        s += CfgEscapeStr(k);
        s += "\":";
    }
    void b(const char* k, bool v) { key(k); s += v ? "true" : "false"; }
    void i(const char* k, int v) { key(k); char b[32]; snprintf(b, sizeof(b), "%d", v); s += b; }
    void f(const char* k, double v) { key(k); char b[32]; snprintf(b, sizeof(b), "%.9g", v); s += b; }
    void str(const char* k, const char* v) { key(k); s += "\""; s += CfgEscapeStr(v ? v : ""); s += "\""; }
    void arr4(const char* k, const float v[4]) { key(k); char b[128]; snprintf(b, sizeof(b), "[%.9g,%.9g,%.9g,%.9g]", (double)v[0], (double)v[1], (double)v[2], (double)v[3]); s += b; }
    void arrB(const char* k, const bool* v, int n) { key(k); s += "["; for (int j = 0; j < n; ++j) { if (j) s += ","; s += v[j] ? "true" : "false"; } s += "]"; }
    void arrV2(const char* k, float x, float y) { key(k); char b[64]; snprintf(b, sizeof(b), "[%.9g,%.9g]", (double)x, (double)y); s += b; }
    std::string finish() { return "{" + s + "}"; }
};
struct CfgJsonDoc
{
    std::unordered_map<std::string, std::string> kv;
    bool ok = false;
    static void skipWs(const char* t, size_t n, size_t& i) { while (i < n && (t[i] == ' ' || t[i] == '\t' || t[i] == '\n' || t[i] == '\r')) ++i; }
    static bool parseStr(const char* t, size_t n, size_t& i, std::string& out)
    {
        out.clear();
        if (i >= n || t[i] != '\"') return false;
        ++i;
        while (i < n)
        {
            char c = t[i];
            if (c == '\"') { ++i; return true; }
            if (c == '\\' && i + 1 < n)
            {
                char e = t[i + 1];
                if (e == '\"') out += '\"';
                else if (e == '\\') out += '\\';
                else if (e == 'n') out += '\n';
                else if (e == 'r') out += '\r';
                else if (e == 't') out += '\t';
                else if (e == 'u' && i + 5 < n) { i += 6; continue; }
                else out += e;
                i += 2;
                continue;
            }
            out += c;
            ++i;
        }
        return false;
    }
    static bool parseVal(const char* t, size_t n, size_t& i, std::string& out, bool& wasStr)
    {
        out.clear();
        wasStr = false;
        if (i >= n) return false;
        if (t[i] == '\"') { wasStr = true; return parseStr(t, n, i, out); }
        if (t[i] == '[')
        {
            size_t start = i;
            int depth = 0;
            bool instr = false;
            while (i < n)
            {
                char c = t[i];
                if (instr)
                {
                    if (c == '\\') { i += 2; continue; }
                    if (c == '\"') instr = false;
                    ++i;
                    continue;
                }
                if (c == '\"') instr = true;
                else if (c == '[') ++depth;
                else if (c == ']') { --depth; ++i; if (depth == 0) break; continue; }
                ++i;
            }
            if (depth != 0) return false;
            out.assign(t + start, i - start);
            return true;
        }
        size_t start = i;
        while (i < n && t[i] != ',' && t[i] != '}' && t[i] != '\n' && t[i] != '\r') ++i;
        while (i > start && (t[i - 1] == ' ' || t[i - 1] == '\t')) --i;
        out.assign(t + start, i - start);
        return !out.empty();
    }
    bool parse(const std::string& text)
    {
        kv.clear();
        ok = false;
        const char* t = text.c_str();
        size_t n = text.size(), i = 0;
        skipWs(t, n, i);
        if (i >= n || t[i] != '{') return false;
        ++i;
        for (;;)
        {
            skipWs(t, n, i);
            if (i >= n) return false;
            if (t[i] == '}') { ++i; ok = true; return true; }
            std::string k;
            if (!parseStr(t, n, i, k)) return false;
            skipWs(t, n, i);
            if (i >= n || t[i] != ':') return false;
            ++i;
            skipWs(t, n, i);
            std::string v;
            bool wasStr = false;
            if (!parseVal(t, n, i, v, wasStr)) return false;
            kv[k] = v;
            skipWs(t, n, i);
            if (i < n && t[i] == ',') { ++i; continue; }
        }
    }
    const std::string* find(const char* k) const
    {
        auto it = kv.find(k ? k : "");
        return (it == kv.end()) ? nullptr : &it->second;
    }
    bool getBool(const char* k, bool& out) const
    {
        const std::string* v = find(k);
        if (!v || v->empty()) return false;
        if (*v == "true" || *v == "1") { out = true; return true; }
        if (*v == "false" || *v == "0") { out = false; return true; }
        char* end = nullptr;
        long n = strtol(v->c_str(), &end, 10);
        if (end == v->c_str()) return false;
        out = (n != 0);
        return true;
    }
    bool getInt(const char* k, int& out) const
    {
        const std::string* v = find(k);
        if (!v || v->empty()) return false;
        char* end = nullptr;
        long n = strtol(v->c_str(), &end, 10);
        if (end == v->c_str()) return false;
        out = (int)n;
        return true;
    }
    bool getFloat(const char* k, float& out) const
    {
        const std::string* v = find(k);
        if (!v || v->empty()) return false;
        char* end = nullptr;
        double d = strtod(v->c_str(), &end);
        if (end == v->c_str()) return false;
        out = (float)d;
        return true;
    }
    bool getStr(const char* k, char* out, size_t max_l) const
    {
        const std::string* v = find(k);
        if (!v || max_l == 0) return false;
        size_t l = v->size();
        if (l >= max_l) l = max_l - 1;
        memcpy(out, v->data(), l);
        out[l] = '\0';
        return true;
    }
    bool getNumArr(const char* k, float* out, int n) const
    {
        const std::string* v = find(k);
        if (!v || v->empty() || (*v)[0] != '[') return false;
        const char* s = v->c_str() + 1;
        for (int j = 0; j < n; ++j)
        {
            while (*s == ' ' || *s == '\t') ++s;
            if (*s == ']' || *s == '\0') return j > 0;
            char* end = nullptr;
            double d = strtod(s, &end);
            if (end == s) return j > 0;
            out[j] = (float)d;
            s = end;
            while (*s == ' ' || *s == '\t') ++s;
            if (*s == ',') ++s;
        }
        return true;
    }
    bool getBoolArr(const char* k, bool* out, int n) const
    {
        const std::string* v = find(k);
        if (!v || v->empty() || (*v)[0] != '[') return false;
        const char* s = v->c_str() + 1;
        for (int j = 0; j < n; ++j)
        {
            while (*s == ' ' || *s == '\t') ++s;
            if (*s == ']' || *s == '\0') return j > 0;
            if (strncmp(s, "true", 4) == 0) { out[j] = true; s += 4; }
            else if (strncmp(s, "false", 5) == 0) { out[j] = false; s += 5; }
            else
            {
                char* end = nullptr;
                long num = strtol(s, &end, 10);
                if (end == s) return j > 0;
                out[j] = (num != 0);
                s = end;
            }
            while (*s == ' ' || *s == '\t') ++s;
            if (*s == ',') ++s;
        }
        return true;
    }
};
inline void GetDesktopConfigDir(char* out_path, size_t max_len)
{
    char desktop[MAX_PATH] = "";
    if (SHGetFolderPathA(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktop) != S_OK)
    {
        char user_prof[MAX_PATH] = "";
        GetEnvironmentVariableA("USERPROFILE", user_prof, sizeof(user_prof));
        if (user_prof[0] != '\0')
        {
            snprintf(desktop, sizeof(desktop), "%s\\Desktop", user_prof);
        }
        else
        {
            snprintf(desktop, sizeof(desktop), ".");
        }
    }
    snprintf(out_path, max_len, "%s\\saved-configs", desktop);
    CreateDirectoryA(out_path, nullptr);
}
inline std::vector<std::string> g_configFiles;
inline int g_configSelectedIdx = 0;
inline void RefreshConfigList()
{
    g_configFiles.clear();
    char dir[MAX_PATH];
    GetDesktopConfigDir(dir, sizeof(dir));
    char search_pattern[MAX_PATH];
    snprintf(search_pattern, sizeof(search_pattern), "%s\\*.json", dir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_pattern, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::string fname = fd.cFileName;
                size_t dot = fname.rfind(".json");
                if (dot != std::string::npos)
                {
                    fname = fname.substr(0, dot);
                }
                if (!fname.empty())
                {
                    g_configFiles.push_back(fname);
                }
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    if (g_configFiles.empty())
    {
        g_configFiles.push_back("default");
    }
    if (g_configSelectedIdx < 0) g_configSelectedIdx = 0;
    if (g_configSelectedIdx >= (int)g_configFiles.size())
    {
        g_configSelectedIdx = 0;
    }
}
inline bool DeleteConfigFile(const char* name)
{
    if (!name || name[0] == '\0')
    {
        return false;
    }
    char dir[MAX_PATH];
    GetDesktopConfigDir(dir, sizeof(dir));
    char file_name[128];
    if (strstr(name, ".json"))
    {
        snprintf(file_name, sizeof(file_name), "%s", name);
    }
    else
    {
        snprintf(file_name, sizeof(file_name), "%s.json", name);
    }
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s\\%s", dir, file_name);
    BOOL ok = DeleteFileA(full_path);
    RefreshConfigList();
    return ok != FALSE;
}
inline std::string BuildConfigPayload(const char* name)
{
    CfgJsonWriter j;
    j.str("type", "thenwefuckin");
    j.i("version", 1);
    j.str("name", (name && name[0] != '\0') ? name : "default");
    j.b("aim_enabled", S::aim_enabled);
    j.i("aim_key", S::aim_key);
    j.i("aim_key_mode", S::aim_key_mode);
    j.i("aim_method", S::aim_method);
    j.i("aim_part", S::aim_part);
    j.i("aim_priority", S::aim_priority);
    j.f("aim_fov", (double)S::aim_fov);
    j.b("aim_360", S::aim_360);
    j.b("aim_smoothing_enabled", S::aim_smoothing_enabled);
    j.f("aim_smoothing", (double)S::aim_smoothing);
    j.f("aim_smoothing_x", (double)S::aim_smoothing_x);
    j.f("aim_smoothing_y", (double)S::aim_smoothing_y);
    j.b("aim_show_fov", S::aim_show_fov);
    j.b("aim_fov_fill", S::aim_fov_fill);
    j.b("aim_fov_outline", S::aim_fov_outline);
    j.b("aim_tracer_outline", S::aim_tracer_outline);
    j.i("aim_fov_style", S::aim_fov_style);
    j.b("aim_team_check", S::aim_team_check);
    j.b("aim_bots", S::aim_bots);
    j.b("aim_visible_check", S::aim_visible_check);
    j.b("aim_magicbullet", S::aim_magicbullet);
    j.b("aim_tracer", S::aim_tracer);
    j.f("aim_tracer_thickness", (double)S::aim_tracer_thickness);
    j.b("vis_team_check", S::vis_team_check);
    j.b("aim_health_check", S::aim_health_check);
    j.f("aim_health_value", (double)S::aim_health_value);
    j.b("aim_distance_check", S::aim_distance_check);
    j.f("aim_distance_value", (double)S::aim_distance_value);
    j.b("aim_autoshoot", S::aim_autoshoot);
    j.i("aim_autoshoot_key", S::aim_autoshoot_key);
    j.i("aim_autoshoot_key_mode", S::aim_autoshoot_key_mode);
    j.f("aim_autoshoot_delay", (double)S::aim_autoshoot_delay);
    j.b("aim_deadzone_enabled", S::aim_deadzone_enabled);
    j.i("aim_deadzone_min", S::aim_deadzone_min);
    j.i("aim_deadzone_max", S::aim_deadzone_max);
    j.b("aim_prediction", S::aim_prediction);
    j.i("aim_pred_mode", S::aim_pred_mode);
    j.f("aim_pred_height", (double)S::aim_pred_height);
    j.f("aim_pred_forward", (double)S::aim_pred_forward);
    j.f("aim_fallen_bv", (double)S::aim_fallen_bv);
    j.f("aim_fallen_grav", (double)S::aim_fallen_grav);
    j.i("aim_weapon", S::aim_weapon);
    j.b("box_enabled", S::box_enabled);
    j.i("box_mode", S::box_mode);
    j.i("box_style", S::box_style);
    j.b("box_outline", S::box_outline);
    j.b("box_filled", S::box_filled);
    j.b("box_fill_gradient", S::box_fill_gradient);
    j.b("skeleton_enabled", S::skeleton_enabled);
    j.b("skeleton_outline", S::skeleton_outline);
    j.b("health_enabled", S::health_enabled);
    j.b("health_outline", S::health_outline);
    j.b("health_gradient", S::health_gradient);
    j.b("health_text", S::health_text);
    j.b("name_enabled", S::name_enabled);
    j.b("name_outline", S::name_outline);
    j.b("headdot_enabled", S::headdot_enabled);
    j.b("headdot_fill", S::headdot_fill);
    j.b("headdot_outline", S::headdot_outline);
    j.f("headdot_thickness", (double)S::headdot_thickness);
    j.b("distance_enabled", S::distance_enabled);
    j.b("distance_outline", S::distance_outline);
    j.f("visual_maxdist", (double)S::visual_maxdist);
    j.b("tool_enabled", S::tool_enabled);
    j.b("tool_outline", S::tool_outline);
    j.b("tool_distance_check", S::tool_distance_check);
    j.f("tool_distance_value", (double)S::tool_distance_value);
    j.b("flags_enabled", S::flags_enabled);
    j.b("flags_global", S::flags_global);
    j.b("flags_outline", S::flags_outline);
    j.b("flags_team", S::flags_team);
    j.b("flags_team_outline", S::flags_team_outline);
    j.b("flags_state", S::flags_state);
    j.b("flags_state_outline", S::flags_state_outline);
    j.b("flags_health", S::flags_health);
    j.b("flags_health_outline", S::flags_health_outline);
    j.arr4("flags_color", S::flags_color);
    j.arr4("flags_outline_color", S::flags_outline_color);
    j.arr4("flags_team_color", S::flags_team_color);
    j.arr4("flags_team_outline_color", S::flags_team_outline_color);
    j.arr4("flags_state_color", S::flags_state_color);
    j.arr4("flags_state_outline_color", S::flags_state_outline_color);
    j.arr4("flags_health_color", S::flags_health_color);
    j.arr4("flags_health_outline_color", S::flags_health_outline_color);
    j.b("tracer_enabled", S::tracer_enabled);
    j.b("tracer_outline", S::tracer_outline);
    j.i("tracer_origin", S::tracer_origin);
    j.f("tracer_thickness", (double)S::tracer_thickness);
    j.b("oof_enabled", S::oof_enabled);
    j.b("oof_outline", S::oof_outline);
    j.f("oof_size", (double)S::oof_size);
    j.f("oof_radius", (double)S::oof_radius);
    j.i("oof_arrow_style", S::oof_arrow_style);
    j.b("oof_names", S::oof_names);
    j.b("oof_health", S::oof_health);
    j.b("oof_distance", S::oof_distance);
    j.b("oof_avatar", S::oof_avatar);
    j.b("walkspeed_enabled", S::walkspeed_enabled);
    j.f("walkspeed_value", (double)S::walkspeed_value);
    j.i("walkspeed_key", S::walkspeed_key);
    j.i("walkspeed_key_mode", S::walkspeed_key_mode);
    j.b("jumppower_enabled", S::jumppower_enabled);
    j.f("jumppower_value", (double)S::jumppower_value);
    j.i("jumppower_key", S::jumppower_key);
    j.i("jumppower_key_mode", S::jumppower_key_mode);
    j.b("gravity_enabled", S::gravity_enabled);
    j.f("gravity_value", (double)S::gravity_value);
    j.i("gravity_key", S::gravity_key);
    j.i("gravity_key_mode", S::gravity_key_mode);
    j.b("nofall_enabled", S::nofall_enabled);
    j.f("nofall_value", (double)S::nofall_value);
    j.i("nofall_key", S::nofall_key);
    j.i("nofall_key_mode", S::nofall_key_mode);
    j.b("antiafk_enabled", S::antiafk_enabled);
    j.f("antiafk_interval", (double)S::antiafk_interval);
    j.b("hitsound_enabled", S::hitsound_enabled);
    j.i("hitsound_selected", S::hitsound_selected);
    j.i("hitsound_mode", S::hitsound_mode);
    j.f("hitsound_volume", (double)S::hitsound_volume);
    j.b("fov_enabled", S::fov_enabled);
    j.f("fov_value", (double)S::fov_value);
    j.i("fov_key", S::fov_key);
    j.i("fov_key_mode", S::fov_key_mode);
    j.b("zoom_enabled", S::zoom_enabled);
    j.f("zoom_value", (double)S::zoom_value);
    j.i("zoom_key", S::zoom_key);
    j.i("zoom_key_mode", S::zoom_key_mode);
    j.b("noclip_enabled", S::noclip_enabled);
    j.i("noclip_key", S::noclip_key);
    j.i("noclip_key_mode", S::noclip_key_mode);
    j.b("fly_enabled", S::fly_enabled);
    j.i("fly_key", S::fly_key);
    j.i("fly_key_mode", S::fly_key_mode);
    j.f("fly_speed", (double)S::fly_speed);
    j.b("freecam_enabled", S::freecam_enabled);
    j.i("freecam_key", S::freecam_key);
    j.i("freecam_key_mode", S::freecam_key_mode);
    j.f("freecam_speed", (double)S::freecam_speed);
    j.f("freecam_sensitivity", (double)S::freecam_sensitivity);
    j.b("hipheight_enabled", S::hipheight_enabled);
    j.f("hipheight_value", (double)S::hipheight_value);
    j.i("hipheight_key", S::hipheight_key);
    j.i("hipheight_key_mode", S::hipheight_key_mode);
    j.b("infjump_enabled", S::infjump_enabled);
    j.i("infjump_key", S::infjump_key);
    j.i("infjump_key_mode", S::infjump_key_mode);
    j.b("time_enabled", S::time_enabled);
    j.f("time_value", (double)S::time_value);
    j.i("time_key", S::time_key);
    j.i("time_key_mode", S::time_key_mode);
    j.b("night_enabled", S::night_enabled);
    j.i("night_key", S::night_key);
    j.i("night_key_mode", S::night_key_mode);
    j.f("night_level", (double)S::night_level);
    j.b("world_fog_enabled", S::world_fog_enabled);
    j.f("world_fog_start", (double)S::world_fog_start);
    j.f("world_fog_end", (double)S::world_fog_end);
    j.i("world_fog_key", S::world_fog_key);
    j.i("world_fog_key_mode", S::world_fog_key_mode);
    j.arr4("world_fog_color", S::world_fog_color);
    j.b("world_exposure_enabled", S::world_exposure_enabled);
    j.f("world_exposure_value", (double)S::world_exposure_value);
    j.i("world_exposure_key", S::world_exposure_key);
    j.i("world_exposure_key_mode", S::world_exposure_key_mode);
    j.b("fullbright_enabled", S::fullbright_enabled);
    j.f("fullbright_value", (double)S::fullbright_value);
    j.i("fullbright_key", S::fullbright_key);
    j.i("fullbright_key_mode", S::fullbright_key_mode);
    j.b("noshadow_enabled", S::noshadow_enabled);
    j.i("noshadow_key", S::noshadow_key);
    j.i("noshadow_key_mode", S::noshadow_key_mode);
    j.f("shadow_softness", (double)S::shadow_softness);
    j.b("nograss_enabled", S::nograss_enabled);
    j.i("nograss_key", S::nograss_key);
    j.i("nograss_key_mode", S::nograss_key_mode);
    j.f("grass_value", (double)S::grass_value);
    j.b("world_geo_enabled", S::world_geo_enabled);
    j.i("world_geo_key", S::world_geo_key);
    j.i("world_geo_key_mode", S::world_geo_key_mode);
    j.b("ambient_enabled", S::ambient_enabled);
    j.i("ambient_key", S::ambient_key);
    j.i("ambient_key_mode", S::ambient_key_mode);
    j.arr4("ambient_color", S::ambient_color);
    j.b("world_ore_enabled", S::world_ore_enabled);
    j.i("world_ore_key", S::world_ore_key);
    j.i("world_ore_key_mode", S::world_ore_key_mode);
    j.arrB("world_ore_name", S::world_ore_name, 3);
    j.arrB("world_ore_dist", S::world_ore_dist, 3);
    j.b("world_ore_stone", S::world_ore_stone);
    j.b("world_ore_phosphate", S::world_ore_phosphate);
    j.b("world_ore_metal", S::world_ore_metal);
    j.arr4("world_ore_stone_color", S::world_ore_stone_color);
    j.arr4("world_ore_phosphate_color", S::world_ore_phosphate_color);
    j.arr4("world_ore_metal_color", S::world_ore_metal_color);
    j.b("world_plant_enabled", S::world_plant_enabled);
    j.i("world_plant_key", S::world_plant_key);
    j.i("world_plant_key_mode", S::world_plant_key_mode);
    j.arrB("world_plant_sel", S::world_plant_sel, 7);
    j.arrB("world_plant_name", S::world_plant_name, 7);
    j.arrB("world_plant_dist", S::world_plant_dist, 7);
    {
        char k[48];
        for (int i = 0; i < 7; ++i) { snprintf(k, sizeof(k), "world_plant_color%d", i); j.arr4(k, S::world_plant_color[i]); }
    }
    j.b("world_bots_enabled", S::world_bots_enabled);
    j.i("world_bots_key", S::world_bots_key);
    j.i("world_bots_key_mode", S::world_bots_key_mode);
    j.arrB("world_bots_sel", S::world_bots_sel, 4);
    j.arrB("world_bots_box", S::world_bots_box, 4);
    j.arrB("world_bots_name", S::world_bots_name, 4);
    j.arrB("world_bots_dist", S::world_bots_dist, 4);
    {
        char k[48];
        for (int i = 0; i < 4; ++i) { snprintf(k, sizeof(k), "world_bots_color%d", i); j.arr4(k, S::world_bots_color[i]); }
    }
    j.b("world_animals_enabled", S::world_animals_enabled);
    j.i("world_animals_key", S::world_animals_key);
    j.i("world_animals_key_mode", S::world_animals_key_mode);
    j.arrB("world_animals_sel", S::world_animals_sel, 3);
    j.arrB("world_animals_name", S::world_animals_name, 3);
    j.arrB("world_animals_dist", S::world_animals_dist, 3);
    {
        char k[48];
        for (int i = 0; i < 3; ++i) { snprintf(k, sizeof(k), "world_animals_color%d", i); j.arr4(k, S::world_animals_color[i]); }
    }
    j.b("world_tools_enabled", S::world_tools_enabled);
    j.i("world_tools_key", S::world_tools_key);
    j.i("world_tools_key_mode", S::world_tools_key_mode);
    j.arrB("world_tools_sel", S::world_tools_sel, 7);
    j.arrB("world_tools_name", S::world_tools_name, 7);
    j.arrB("world_tools_dist", S::world_tools_dist, 7);
    {
        char k[48];
        for (int i = 0; i < 7; ++i) { snprintf(k, sizeof(k), "world_tools_color%d", i); j.arr4(k, S::world_tools_color[i]); }
    }
    j.b("world_crates_enabled", S::world_crates_enabled);
    j.i("world_crates_key", S::world_crates_key);
    j.i("world_crates_key_mode", S::world_crates_key_mode);
    j.arrB("world_crates_sel", S::world_crates_sel, 8);
    j.arrB("world_crates_name", S::world_crates_name, 8);
    j.arrB("world_crates_dist", S::world_crates_dist, 8);
    {
        char k[48];
        for (int i = 0; i < 8; ++i) { snprintf(k, sizeof(k), "world_crates_color%d", i); j.arr4(k, S::world_crates_color[i]); }
    }
    j.b("world_keycards_enabled", S::world_keycards_enabled);
    j.i("world_keycards_key", S::world_keycards_key);
    j.i("world_keycards_key_mode", S::world_keycards_key_mode);
    j.arrB("world_keycards_sel", S::world_keycards_sel, 5);
    j.arrB("world_keycards_name", S::world_keycards_name, 5);
    j.arrB("world_keycards_dist", S::world_keycards_dist, 5);
    {
        char k[48];
        for (int i = 0; i < 5; ++i) { snprintf(k, sizeof(k), "world_keycards_color%d", i); j.arr4(k, S::world_keycards_color[i]); }
    }
    j.arr4("box_color", S::box_color);
    j.arr4("box_outline_color", S::box_outline_color);
    j.arr4("box_fill_color", S::box_fill_color);
    j.arr4("box_fill_color2", S::box_fill_color2);
    j.arr4("skeleton_color", S::skeleton_color);
    j.arr4("skeleton_outline_color", S::skeleton_outline_color);
    j.arr4("health_color", S::health_color);
    j.arr4("health_outline_color", S::health_outline_color);
    j.arr4("name_color", S::name_color);
    j.arr4("name_outline_color", S::name_outline_color);
    j.arr4("headdot_color", S::headdot_color);
    j.arr4("headdot_outline_color", S::headdot_outline_color);
    j.arr4("distance_color", S::distance_color);
    j.arr4("distance_outline_color", S::distance_outline_color);
    j.arr4("tool_color", S::tool_color);
    j.arr4("tool_outline_color", S::tool_outline_color);
    j.arr4("tracer_color", S::tracer_color);
    j.arr4("tracer_outline_color", S::tracer_outline_color);
    j.arr4("oof_color", S::oof_color);
    j.arr4("oof_outline_color", S::oof_outline_color);
    j.arr4("oof_name_color", S::oof_name_color);
    j.arr4("oof_health_color", S::oof_health_color);
    j.arr4("oof_distance_color", S::oof_distance_color);
    j.arr4("aim_fov_color", S::aim_fov_color);
    j.arr4("aim_fov_outline_color", S::aim_fov_outline_color);
    j.arr4("aim_fov_fill_color", S::aim_fov_fill_color);
    j.arr4("aim_tracer_color", S::aim_tracer_color);
    j.arr4("aim_tracer_outline_color", S::aim_tracer_outline_color);
    j.str("user_name", g_textinput1);
    j.b("vsync", g_vsync);
    j.b("stream_proof", g_streamProof);
    j.i("fps_cap", g_fpsCap);
    j.b("watermark", g_watermark);
    j.b("watermark_fps", g_watermarkFps);
    j.b("watermark_players", g_watermarkPlayers);
    j.b("watermark_date", g_watermarkDate);
    j.b("keybinds", g_keybinds);
    j.b("movement", g_movement);
    j.b("movement_vel", g_movementVelocityOpt);
    j.b("movement_graph", g_movementGraphOpt);
    j.b("movement_keys", g_movementKeysOpt);
    j.b("internal_debug", g_internalDebug);
    j.arrV2("binds_pos", g_bindsPos.x, g_bindsPos.y);
    j.arrV2("movement_pos", g_movementPos.x, g_movementPos.y);
    j.arrV2("debug_pos", g_debugPos.x, g_debugPos.y);
    j.i("theme_mask", g_themeMask);
    j.arr4("accent", reinterpret_cast<const float*>(&g_accent1));
    j.arr4("col_frame", reinterpret_cast<const float*>(&g_colFrame));
    j.arr4("col_group", reinterpret_cast<const float*>(&g_colGroup));
    j.arr4("col_ctrl", reinterpret_cast<const float*>(&g_colCtrl));
    j.arr4("col_ctrl_hov", reinterpret_cast<const float*>(&g_colCtrlHov));
    j.arr4("col_border", reinterpret_cast<const float*>(&g_colBorder));
    j.arr4("col_text", reinterpret_cast<const float*>(&g_colText));
    j.arr4("col_text_dim", reinterpret_cast<const float*>(&g_colTextDim));
    return j.finish();
}

inline bool SaveConfig(const char* name)
{
    char dir[MAX_PATH];
    GetDesktopConfigDir(dir, sizeof(dir));
    char file_name[128];
    if (!name || name[0] == '\0')
    {
        snprintf(file_name, sizeof(file_name), "default.json");
    }
    else
    {
        if (strstr(name, ".json"))
        {
            snprintf(file_name, sizeof(file_name), "%s", name);
        }
        else
        {
            snprintf(file_name, sizeof(file_name), "%s.json", name);
        }
    }
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s\\%s", dir, file_name);
    std::string json = BuildConfigPayload(file_name);
    FILE* fp = nullptr;
    if (fopen_s(&fp, full_path, "wb") != 0 || !fp)
    {
        return false;
    }
    fwrite(json.data(), 1, json.size(), fp);
    fclose(fp);
    RefreshConfigList();
    return true;
}
inline bool ExportConfig(const char* name)
{
    std::string json = BuildConfigPayload(name);
    ImGui::SetClipboardText(json.c_str());
    return true;
}
inline bool ApplyConfigRaw(const std::string& raw)
{
    if (raw.empty())
    {
        return false;
    }
    CfgJsonDoc doc;
    if (!doc.parse(raw))
    {
        return false;
    }
    const std::string* type = doc.find("type");
    if (!type || *type != "thenwefuckin")
    {
        return false;
    }
    auto get_int = [&](const char* key, int& out) {
        doc.getInt(key, out);
    };
    auto get_bool = [&](const char* key, bool& out) {
        doc.getBool(key, out);
    };
    auto get_col = [&](const char* key, ImVec4& out) {
        float v[4] = { out.x, out.y, out.z, out.w };
        if (doc.getNumArr(key, v, 4))
            out = ImVec4(v[0], v[1], v[2], v[3]);
    };
    auto get_str = [&](const char* key, char* out, size_t max_l) {
        doc.getStr(key, out, max_l);
    };
    auto get_float = [&](const char* key, float& out) {
        doc.getFloat(key, out);
    };
    auto get_bool_arr = [&](const char* key, bool* out, int n) {
        doc.getBoolArr(key, out, n);
    };
    get_bool("aim_enabled", S::aim_enabled);
    get_int("aim_key", S::aim_key);
    get_int("aim_key_mode", S::aim_key_mode);
    get_int("aim_method", S::aim_method);
    if (S::aim_method < 0 || S::aim_method > 4) S::aim_method = 0;
    get_int("aim_part", S::aim_part);
    get_int("aim_priority", S::aim_priority);
    if (S::aim_priority < 0 || S::aim_priority > 2) S::aim_priority = 0;
    get_float("aim_fov", S::aim_fov);
    get_bool("aim_360", S::aim_360);
    get_bool("aim_smoothing_enabled", S::aim_smoothing_enabled);
    get_float("aim_smoothing", S::aim_smoothing);
    get_float("aim_smoothing_x", S::aim_smoothing_x);
    get_float("aim_smoothing_y", S::aim_smoothing_y);
    get_bool("aim_show_fov", S::aim_show_fov);
    get_bool("aim_fov_fill", S::aim_fov_fill);
    get_bool("aim_fov_outline", S::aim_fov_outline);
    get_bool("aim_tracer_outline", S::aim_tracer_outline);
    get_int("aim_fov_style", S::aim_fov_style);
    if (S::aim_fov_style < 0 || S::aim_fov_style > 4) S::aim_fov_style = 0;
    get_bool("aim_team_check", S::aim_team_check);
    get_bool("aim_bots", S::aim_bots);
    get_bool("aim_visible_check", S::aim_visible_check);
    get_bool("aim_magicbullet", S::aim_magicbullet);
    get_bool("aim_tracer", S::aim_tracer);
    get_float("aim_tracer_thickness", S::aim_tracer_thickness);
    get_bool("vis_team_check", S::vis_team_check);
    get_bool("aim_health_check", S::aim_health_check);
    get_float("aim_health_value", S::aim_health_value);
    get_bool("aim_distance_check", S::aim_distance_check);
    get_float("aim_distance_value", S::aim_distance_value);
    get_bool("aim_autoshoot", S::aim_autoshoot);
    get_int("aim_autoshoot_key", S::aim_autoshoot_key);
    get_int("aim_autoshoot_key_mode", S::aim_autoshoot_key_mode);
    get_float("aim_autoshoot_delay", S::aim_autoshoot_delay);
    get_bool("aim_deadzone_enabled", S::aim_deadzone_enabled);
    get_int("aim_deadzone_min", S::aim_deadzone_min);
    get_int("aim_deadzone_max", S::aim_deadzone_max);
    get_bool("aim_prediction", S::aim_prediction);
    get_int("aim_pred_mode", S::aim_pred_mode);
    if (S::aim_pred_mode < 0 || S::aim_pred_mode > 2) S::aim_pred_mode = 0;
    get_float("aim_pred_height", S::aim_pred_height);
    get_float("aim_pred_forward", S::aim_pred_forward);
    get_float("aim_fallen_bv", S::aim_fallen_bv);
    get_float("aim_fallen_grav", S::aim_fallen_grav);
    get_int("aim_weapon", S::aim_weapon);
    if (S::aim_weapon < 0 || S::aim_weapon >= thenwefuckin::aim::predict::FALLEN_WEAPON_COUNT) S::aim_weapon = 0;
    get_bool("box_enabled", S::box_enabled);
    get_int("box_mode", S::box_mode);
    get_int("box_style", S::box_style);
    get_bool("box_outline", S::box_outline);
    get_bool("box_filled", S::box_filled);
    get_bool("box_fill_gradient", S::box_fill_gradient);
    get_bool("skeleton_enabled", S::skeleton_enabled);
    get_bool("skeleton_outline", S::skeleton_outline);
    get_bool("health_enabled", S::health_enabled);
    get_bool("health_outline", S::health_outline);
    get_bool("health_gradient", S::health_gradient);
    get_bool("health_text", S::health_text);
    get_bool("name_enabled", S::name_enabled);
    get_bool("name_outline", S::name_outline);
    get_bool("headdot_enabled", S::headdot_enabled);
    get_bool("headdot_fill", S::headdot_fill);
    get_bool("headdot_outline", S::headdot_outline);
    get_float("headdot_thickness", S::headdot_thickness);
    get_bool("distance_enabled", S::distance_enabled);
    get_bool("distance_outline", S::distance_outline);
    get_float("visual_maxdist", S::visual_maxdist);
    get_bool("tool_enabled", S::tool_enabled);
    get_bool("tool_outline", S::tool_outline);
    get_bool("tool_distance_check", S::tool_distance_check);
    get_float("tool_distance_value", S::tool_distance_value);
    get_bool("flags_team", S::flags_team);
    get_bool("flags_global", S::flags_global);
    get_bool("flags_outline", S::flags_outline);
    get_bool("flags_team_outline", S::flags_team_outline);
    get_bool("flags_state", S::flags_state);
    get_bool("flags_state_outline", S::flags_state_outline);
    get_bool("flags_health", S::flags_health);
    get_bool("flags_health_outline", S::flags_health_outline);
    if (doc.find("flags_enabled") != nullptr) {
        get_bool("flags_enabled", S::flags_enabled);
    } else {
        S::flags_enabled = (S::flags_team || S::flags_state || S::flags_health);
    }
    get_bool("tracer_enabled", S::tracer_enabled);
    get_bool("tracer_outline", S::tracer_outline);
    get_int("tracer_origin", S::tracer_origin);
    get_float("tracer_thickness", S::tracer_thickness);
    get_bool("oof_enabled", S::oof_enabled);
    get_bool("oof_outline", S::oof_outline);
    get_float("oof_size", S::oof_size);
    get_float("oof_radius", S::oof_radius);
    get_int("oof_arrow_style", S::oof_arrow_style);
    get_bool("oof_names", S::oof_names);
    get_bool("oof_health", S::oof_health);
    get_bool("oof_distance", S::oof_distance);
    get_bool("oof_avatar", S::oof_avatar);
    get_bool("walkspeed_enabled", S::walkspeed_enabled);
    get_float("walkspeed_value", S::walkspeed_value);
    get_int("walkspeed_key", S::walkspeed_key);
    get_int("walkspeed_key_mode", S::walkspeed_key_mode);
    get_bool("jumppower_enabled", S::jumppower_enabled);
    get_float("jumppower_value", S::jumppower_value);
    get_int("jumppower_key", S::jumppower_key);
    get_int("jumppower_key_mode", S::jumppower_key_mode);
    get_bool("gravity_enabled", S::gravity_enabled);
    get_float("gravity_value", S::gravity_value);
    get_int("gravity_key", S::gravity_key);
    get_int("gravity_key_mode", S::gravity_key_mode);
    get_bool("nofall_enabled", S::nofall_enabled);
    get_float("nofall_value", S::nofall_value);
    get_int("nofall_key", S::nofall_key);
    get_int("nofall_key_mode", S::nofall_key_mode);
    get_bool("antiafk_enabled", S::antiafk_enabled);
    get_float("antiafk_interval", S::antiafk_interval);
    get_bool("fov_enabled", S::fov_enabled);
    get_float("fov_value", S::fov_value);
    get_int("fov_key", S::fov_key);
    get_int("fov_key_mode", S::fov_key_mode);
    get_bool("zoom_enabled", S::zoom_enabled);
    get_float("zoom_value", S::zoom_value);
    get_int("zoom_key", S::zoom_key);
    get_int("zoom_key_mode", S::zoom_key_mode);
    get_bool("noclip_enabled", S::noclip_enabled);
    get_int("noclip_key", S::noclip_key);
    get_int("noclip_key_mode", S::noclip_key_mode);
    get_bool("fly_enabled", S::fly_enabled);
    get_int("fly_key", S::fly_key);
    get_int("fly_key_mode", S::fly_key_mode);
    get_float("fly_speed", S::fly_speed);
    get_bool("freecam_enabled", S::freecam_enabled);
    get_int("freecam_key", S::freecam_key);
    get_int("freecam_key_mode", S::freecam_key_mode);
    get_float("freecam_speed", S::freecam_speed);
    get_float("freecam_sensitivity", S::freecam_sensitivity);
    get_bool("hipheight_enabled", S::hipheight_enabled);
    get_float("hipheight_value", S::hipheight_value);
    get_int("hipheight_key", S::hipheight_key);
    get_int("hipheight_key_mode", S::hipheight_key_mode);
    get_bool("infjump_enabled", S::infjump_enabled);
    get_int("infjump_key", S::infjump_key);
    get_int("infjump_key_mode", S::infjump_key_mode);
    get_bool("hitsound_enabled", S::hitsound_enabled);
    get_int("hitsound_selected", S::hitsound_selected);
    get_int("hitsound_mode", S::hitsound_mode);
    get_float("hitsound_volume", S::hitsound_volume);
    get_bool("time_enabled", S::time_enabled);
    get_float("time_value", S::time_value);
    get_int("time_key", S::time_key);
    get_int("time_key_mode", S::time_key_mode);
    get_bool("night_enabled", S::night_enabled);
    get_int("night_key", S::night_key);
    get_int("night_key_mode", S::night_key_mode);
    get_float("night_level", S::night_level);
    get_bool("world_fog_enabled", S::world_fog_enabled);
    get_float("world_fog_start", S::world_fog_start);
    get_float("world_fog_end", S::world_fog_end);
    get_int("world_fog_key", S::world_fog_key);
    get_int("world_fog_key_mode", S::world_fog_key_mode);
    get_bool("world_exposure_enabled", S::world_exposure_enabled);
    get_float("world_exposure_value", S::world_exposure_value);
    get_int("world_exposure_key", S::world_exposure_key);
    get_int("world_exposure_key_mode", S::world_exposure_key_mode);
    get_bool("fullbright_enabled", S::fullbright_enabled);
    get_float("fullbright_value", S::fullbright_value);
    get_int("fullbright_key", S::fullbright_key);
    get_int("fullbright_key_mode", S::fullbright_key_mode);
    get_bool("noshadow_enabled", S::noshadow_enabled);
    get_int("noshadow_key", S::noshadow_key);
    get_int("noshadow_key_mode", S::noshadow_key_mode);
    get_float("shadow_softness", S::shadow_softness);
    get_bool("nograss_enabled", S::nograss_enabled);
    get_int("nograss_key", S::nograss_key);
    get_int("nograss_key_mode", S::nograss_key_mode);
    get_float("grass_value", S::grass_value);
    get_bool("world_geo_enabled", S::world_geo_enabled);
    get_int("world_geo_key", S::world_geo_key);
    get_int("world_geo_key_mode", S::world_geo_key_mode);
    get_bool("ambient_enabled", S::ambient_enabled);
    get_int("ambient_key", S::ambient_key);
    get_int("ambient_key_mode", S::ambient_key_mode);
    get_bool("world_ore_enabled", S::world_ore_enabled);
    get_int("world_ore_key", S::world_ore_key);
    get_int("world_ore_key_mode", S::world_ore_key_mode);
    get_int("world_plant_key", S::world_plant_key);
    get_int("world_plant_key_mode", S::world_plant_key_mode);
    get_bool_arr("world_ore_name", S::world_ore_name, 3);
    get_bool_arr("world_ore_dist", S::world_ore_dist, 3);
    get_bool("world_ore_stone", S::world_ore_stone);
    get_bool("world_ore_phosphate", S::world_ore_phosphate);
    get_bool("world_ore_metal", S::world_ore_metal);
    get_bool("world_plant_enabled", S::world_plant_enabled);
    get_bool("world_bots_enabled", S::world_bots_enabled);
    get_int("world_bots_key", S::world_bots_key);
    get_int("world_bots_key_mode", S::world_bots_key_mode);
    get_bool("world_animals_enabled", S::world_animals_enabled);
    get_int("world_animals_key", S::world_animals_key);
    get_int("world_animals_key_mode", S::world_animals_key_mode);
    get_bool("world_tools_enabled", S::world_tools_enabled);
    get_int("world_tools_key", S::world_tools_key);
    get_int("world_tools_key_mode", S::world_tools_key_mode);
    get_bool("world_crates_enabled", S::world_crates_enabled);
    get_int("world_crates_key", S::world_crates_key);
    get_int("world_crates_key_mode", S::world_crates_key_mode);
    get_bool("world_keycards_enabled", S::world_keycards_enabled);
    get_int("world_keycards_key", S::world_keycards_key);
    get_int("world_keycards_key_mode", S::world_keycards_key_mode);
    get_bool_arr("world_animals_name", S::world_animals_name, 3);
    get_bool_arr("world_animals_dist", S::world_animals_dist, 3);
    get_bool_arr("world_animals_sel", S::world_animals_sel, 3);
    get_bool_arr("world_plant_sel", S::world_plant_sel, 7);
    get_bool_arr("world_plant_name", S::world_plant_name, 7);
    get_bool_arr("world_plant_dist", S::world_plant_dist, 7);
    auto get_col_arr = [&](const char* key, float arr[4]) {
        doc.getNumArr(key, arr, 4);
    };
    get_col_arr("box_color", S::box_color);
    get_col_arr("box_outline_color", S::box_outline_color);
    get_col_arr("box_fill_color", S::box_fill_color);
    get_col_arr("box_fill_color2", S::box_fill_color2);
    get_col_arr("skeleton_color", S::skeleton_color);
    get_col_arr("skeleton_outline_color", S::skeleton_outline_color);
    get_col_arr("health_color", S::health_color);
    get_col_arr("health_outline_color", S::health_outline_color);
    get_col_arr("name_color", S::name_color);
    get_col_arr("name_outline_color", S::name_outline_color);
    get_col_arr("headdot_color", S::headdot_color);
    get_col_arr("headdot_outline_color", S::headdot_outline_color);
    get_col_arr("distance_color", S::distance_color);
    get_col_arr("distance_outline_color", S::distance_outline_color);
    get_col_arr("tool_color", S::tool_color);
    get_col_arr("tool_outline_color", S::tool_outline_color);
    get_col_arr("flags_color", S::flags_color);
    get_col_arr("flags_outline_color", S::flags_outline_color);
    get_col_arr("flags_team_color", S::flags_team_color);
    get_col_arr("flags_team_outline_color", S::flags_team_outline_color);
    get_col_arr("flags_state_color", S::flags_state_color);
    get_col_arr("flags_state_outline_color", S::flags_state_outline_color);
    get_col_arr("flags_health_color", S::flags_health_color);
    get_col_arr("flags_health_outline_color", S::flags_health_outline_color);
    get_col_arr("tracer_color", S::tracer_color);
    get_col_arr("tracer_outline_color", S::tracer_outline_color);
    get_col_arr("oof_color", S::oof_color);
    get_col_arr("oof_outline_color", S::oof_outline_color);
    get_col_arr("oof_name_color", S::oof_name_color);
    get_col_arr("oof_health_color", S::oof_health_color);
    get_col_arr("oof_distance_color", S::oof_distance_color);
    get_col_arr("aim_fov_color", S::aim_fov_color);
    get_col_arr("aim_fov_outline_color", S::aim_fov_outline_color);
    get_col_arr("aim_fov_fill_color", S::aim_fov_fill_color);
    get_col_arr("aim_tracer_color", S::aim_tracer_color);
    get_col_arr("aim_tracer_outline_color", S::aim_tracer_outline_color);
    get_col_arr("world_fog_color", S::world_fog_color);
    get_col_arr("ambient_color", S::ambient_color);
    get_col_arr("world_ore_stone_color", S::world_ore_stone_color);
    get_col_arr("world_ore_phosphate_color", S::world_ore_phosphate_color);
    get_col_arr("world_ore_metal_color", S::world_ore_metal_color);
    for (int i = 0; i < 7; ++i)
    {
        char k[48];
        snprintf(k, sizeof(k), "world_plant_color%d", i);
        get_col_arr(k, S::world_plant_color[i]);
    }
    get_bool_arr("world_bots_sel", S::world_bots_sel, 4);
    for (int i = 0; i < 4; ++i)
    {
        char k[48];
        snprintf(k, sizeof(k), "world_bots_color%d", i);
        get_col_arr(k, S::world_bots_color[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        char k[48];
        snprintf(k, sizeof(k), "world_animals_color%d", i);
        get_col_arr(k, S::world_animals_color[i]);
    }
    get_bool_arr("world_tools_sel", S::world_tools_sel, 7);
    get_bool_arr("world_tools_name", S::world_tools_name, 7);
    get_bool_arr("world_tools_dist", S::world_tools_dist, 7);
    for (int i = 0; i < 7; ++i)
    {
        char k[48];
        snprintf(k, sizeof(k), "world_tools_color%d", i);
        get_col_arr(k, S::world_tools_color[i]);
    }
    get_bool_arr("world_crates_sel", S::world_crates_sel, 8);
    get_bool_arr("world_crates_name", S::world_crates_name, 8);
    get_bool_arr("world_crates_dist", S::world_crates_dist, 8);
    for (int i = 0; i < 8; ++i)
    {
        char k[48];
        snprintf(k, sizeof(k), "world_crates_color%d", i);
        get_col_arr(k, S::world_crates_color[i]);
    }
    get_bool_arr("world_keycards_sel", S::world_keycards_sel, 5);
    get_bool_arr("world_keycards_name", S::world_keycards_name, 5);
    get_bool_arr("world_keycards_dist", S::world_keycards_dist, 5);
    for (int i = 0; i < 5; ++i)
    {
        char k[48];
        snprintf(k, sizeof(k), "world_keycards_color%d", i);
        get_col_arr(k, S::world_keycards_color[i]);
    }
    get_bool_arr("world_bots_box", S::world_bots_box, 4);
    get_bool_arr("world_bots_name", S::world_bots_name, 4);
    get_bool_arr("world_bots_dist", S::world_bots_dist, 4);
    get_str("user_name", g_textinput1, sizeof(g_textinput1));
    get_bool("vsync", g_vsync);
    get_bool("stream_proof", g_streamProof);
    get_int("fps_cap", g_fpsCap);
    get_bool("watermark", g_watermark);
    get_bool("watermark_fps", g_watermarkFps);
    get_bool("watermark_players", g_watermarkPlayers);
    get_bool("watermark_date", g_watermarkDate);
    get_bool("keybinds", g_keybinds);
    get_bool("internal_debug", g_internalDebug);
    auto get_vec2 = [&](const char* key, ImVec2& out) {
        float v[2] = { out.x, out.y };
        if (doc.getNumArr(key, v, 2))
            out = ImVec2(v[0], v[1]);
    };
    get_vec2("binds_pos", g_bindsPos);
    get_vec2("movement_pos", g_movementPos);
    get_vec2("debug_pos", g_debugPos);
    get_bool("movement", g_movement);
    get_bool("movement_vel", g_movementVelocityOpt);
    get_bool("movement_graph", g_movementGraphOpt);
    get_bool("movement_keys", g_movementKeysOpt);
    get_int("theme_mask", g_themeMask);
    get_col("accent", g_accent1);
    get_col("col_frame", g_colFrame);
    get_col("col_group", g_colGroup);
    get_col("col_ctrl", g_colCtrl);
    get_col("col_ctrl_hov", g_colCtrlHov);
    get_col("col_border", g_colBorder);
    get_col("col_text", g_colText);
    get_col("col_text_dim", g_colTextDim);
    return true;
}

inline bool LoadConfig(const char* name)
{
    char dir[MAX_PATH];
    GetDesktopConfigDir(dir, sizeof(dir));
    char file_name[128];
    if (!name || name[0] == '\0')
    {
        snprintf(file_name, sizeof(file_name), "default.json");
    }
    else
    {
        if (strstr(name, ".json"))
        {
            snprintf(file_name, sizeof(file_name), "%s", name);
        }
        else
        {
            snprintf(file_name, sizeof(file_name), "%s.json", name);
        }
    }
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s\\%s", dir, file_name);
    FILE* fp = nullptr;
    if (fopen_s(&fp, full_path, "rb") != 0 || !fp)
    {
        return false;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0 || sz > 1048576)
    {
        fclose(fp);
        return false;
    }
    std::string raw;
    raw.resize((size_t)sz);
    fread(&raw[0], 1, (size_t)sz, fp);
    fclose(fp);
    return ApplyConfigRaw(raw);
}
inline bool ImportConfig()
{
    const char* clip = ImGui::GetClipboardText();
    if (!clip || clip[0] == '\0')
    {
        return false;
    }
    std::string raw = clip;
    if (raw.find("\"type\"") == std::string::npos || raw.find("thenwefuckin") == std::string::npos)
    {
        return false;
    }
    return ApplyConfigRaw(raw);
}
inline bool DrawConfigDropdown(const char* id, ImDrawList* dl, ImVec2 pos, float width, int* selected_idx, const std::vector<std::string>& items, const char* label = nullptr, ImFont* font = nullptr, float font_size = 13.0f)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 m = io.MousePos;
    ImFont* f = font ? font : ImGui::GetFont();
    float fs = (font_size > 0.0f) ? font_size : 13.0f;
    float x0 = floorf(pos.x);
    float x1 = floorf(pos.x + width);
    bool has_label = (label && label[0] != '\0');
    float box_y0 = has_label ? floorf(pos.y + kLabelBandH) : floorf(pos.y);
    float box_h = 20.0f;
    float box_y1 = box_y0 + box_h;
    const char* popup_id = id;
    bool popup_open = ImGui::IsPopupOpen(popup_id);
    bool is_box_hov = (m.x >= x0 && m.x <= x1 && m.y >= box_y0 && m.y <= box_y1);
    bool dd_modal = IsModalActive();
    if (is_box_hov && !dd_modal)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (ImGui::IsMouseClicked(0))
        {
            RefreshConfigList();
            ImGui::OpenPopup(popup_id);
            UiClick();
        }
    }
    const ImU32 border_dark  = IM_COL32(0, 0, 0, 255);
    const ImU32 border_light = BorderU32();
    if (has_label)
    {
        float text_y = floorf(pos.y - kLabelTextRise);
        ImU32 col_lbl = popup_open ? TextU32() : TextDimU32();
        DrawOutlinedText(dl, f, fs, ImVec2(x0, text_y), col_lbl, border_dark, label);
    }
    dl->AddRectFilled(ImVec2(x0, box_y0), ImVec2(x1, box_y1), border_dark);
    dl->AddRectFilled(ImVec2(x0 + 1.0f, box_y0 + 1.0f), ImVec2(x1 - 1.0f, box_y1 - 1.0f), popup_open ? BorderHovU32() : border_light);
    dl->AddRectFilled(ImVec2(x0 + 2.0f, box_y0 + 2.0f), ImVec2(x1 - 2.0f, box_y1 - 2.0f), popup_open ? CtrlHovU32() : CtrlBgU32());
    const char* display_str = "-";
    if (!items.empty() && *selected_idx >= 0 && *selected_idx < (int)items.size())
    {
        display_str = items[*selected_idx].c_str();
    }
    ImVec2 text_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, display_str);
    float tx = x0 + 8.0f;
    float ty = floorf(box_y0 + (box_h - text_sz.y) * 0.5f);
    dl->PushClipRect(ImVec2(tx, box_y0 + 2.0f), ImVec2(x1 - 18.0f, box_y1 - 2.0f), true);
    DrawOutlinedText(dl, f, fs, ImVec2(tx, ty), popup_open ? TextU32() : TextDimU32(), border_dark, display_str);
    dl->PopClipRect();
    float ax = floorf(x1 - 10.0f);
    float ay = floorf(box_y0 + (box_h - 3.0f) * 0.5f);
    ImU32 arrow_col = popup_open ? TextU32() : TextDimU32();
    dl->AddRectFilled(ImVec2(ax - 2.0f, ay), ImVec2(ax + 3.0f, ay + 1.0f), arrow_col);
    dl->AddRectFilled(ImVec2(ax - 1.0f, ay + 1.0f), ImVec2(ax + 2.0f, ay + 2.0f), arrow_col);
    dl->AddRectFilled(ImVec2(ax, ay + 2.0f), ImVec2(ax + 1.0f, ay + 3.0f), arrow_col);
    float item_h = 18.0f;
    float pad_y = 2.0f;
    int show_count = (int)items.size();
    if (show_count < 1) show_count = 1;
    if (show_count > 6) show_count = 6;
    float popup_h = pad_y * 2.0f + (float)show_count * item_h;
    float popup_w = width;
    ImVec2 popup_pos(x0, floorf(box_y1 + 2.0f));
    ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(popup_w, popup_h), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(30.0f / 255.0f, 36.0f / 255.0f, 48.0f / 255.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::BeginPopup(popup_id, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* pdl = ImGui::GetWindowDrawList();
        ImVec2 w_min = ImGui::GetWindowPos();
        w_min.x = floorf(w_min.x);
        w_min.y = floorf(w_min.y);
        ImVec2 w_max = ImVec2(w_min.x + popup_w, w_min.y + popup_h);
        DrawPixelBox(pdl, w_min, w_max, CtrlBgU32(), border_light, border_dark);
        ImGui::Dummy(ImVec2(popup_w, popup_h));
        for (int i = 0; i < (int)items.size(); i++)
        {
            float it_y0 = floorf(w_min.y + pad_y + (float)i * item_h);
            float it_y1 = it_y0 + item_h;
            float it_x0 = w_min.x + 2.0f;
            float it_x1 = w_max.x - 2.0f;
            bool is_selected = (*selected_idx == i);
            bool it_hov = (m.x >= it_x0 && m.x <= it_x1 && m.y >= it_y0 && m.y <= it_y1);
            if (it_hov)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                pdl->AddRectFilled(ImVec2(it_x0, it_y0), ImVec2(it_x1, it_y1), is_selected ? CtrlHovU32() : CtrlBgU32());
                if (ImGui::IsMouseClicked(0))
                {
                    *selected_idx = i;
                    snprintf(g_configName, sizeof(g_configName), "%s", items[i].c_str());
                    UiClick();
                    ImGui::CloseCurrentPopup();
                }
            }
            else if (is_selected)
            {
                pdl->AddRectFilled(ImVec2(it_x0, it_y0), ImVec2(it_x1, it_y1), CtrlHovU32());
            }
            ImU32 it_text_col = (is_selected || it_hov) ? TextU32() : TextDimU32();
            ImVec2 it_sz = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, items[i].c_str());
            float it_tx = it_x0 + 8.0f;
            float it_ty = floorf(it_y0 + (item_h - it_sz.y) * 0.5f);
            DrawOutlinedText(pdl, f, fs, ImVec2(it_tx, it_ty), it_text_col, border_dark, items[i].c_str());
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    popup_open = ImGui::IsPopupOpen(popup_id);
    float total_h = has_label ? kLabeledBoxH : 20.0f;
    bool is_total_hov = (m.x >= x0 && m.x <= x1 && m.y >= pos.y - kLabelTextRise && m.y <= pos.y + total_h);
    return popup_open || (!dd_modal && is_total_hov);
}
inline void RenderSettingsPanel()
{
    if (!g_settingsOpen)
    {
        g_setAnim = 0.0f;
        g_setDragging = false;
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (g_setAnim < 1.0f)
    {
        float dt = io.DeltaTime;
        if (dt > 0.05f) dt = 0.05f;
        g_setAnim += dt * 6.0f;
        if (g_setAnim > 1.0f) g_setAnim = 1.0f;
    }
    float t = g_setAnim;
    float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    float y_off = (1.0f - ease) * -14.0f;
    if (g_setPos.x < 0.0f)
    {
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        g_setPos = ImVec2(center.x - g_setSize.x * 0.5f + 300.0f, center.y - g_setSize.y * 0.5f - 250.0f);
    }
    ImGui::SetNextWindowSize(g_setSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(g_setPos.x, g_setPos.y + y_off), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(ease);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ease);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uisettings", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
        DrawPixelBox(dl, pos, max, FrameU32(), BorderU32(), col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), AccentU32());
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        ImVec2 gmin = ImVec2(pos.x + 14.0f, pos.y + 24.0f);
        ImVec2 gmax = ImVec2(max.x - 14.0f, max.y - 10.0f);
        DrawGroupBox(dl, gmin, gmax, "settings", f, 13.0f, GroupU32(), BorderU32(), col_outline);
        float cw = (gmax.x - 14.0f) - (gmin.x + 14.0f);
        float r1_y = gmin.y + 16.0f;
        float r2_y = r1_y + kCheckRowPitch;
        float r3_y = r2_y + kCheckRowPitch;
        float r4_y = r3_y + kLabeledCtrlH + 6.0f;
        float r5_y = r4_y + kCheckRowPitch;
        float r6_y = r5_y + kCheckRowPitch;
        bool s1 = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r1_y), &g_vsync, "VSync", f, 13.0f);
        bool s2 = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r2_y), &g_streamProof, "Stream proof", f, 13.0f);
        bool s3 = DrawSlider("##sl_fpscap", dl, ImVec2(gmin.x + 14.0f, r3_y), cw, &g_fpsCap, 0, 240, "FPS Cap (0=off)", "%d", f, 13.0f);
        if (g_fpsCap > 0 && g_fpsCap < 30) g_fpsCap = 30;
        bool s4 = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r4_y), &g_watermark, "Watermark", f, 13.0f);
        QuestionOption wm_opts[] = {
            { &g_watermarkFps, "FPS" },
            { &g_watermarkPlayers, "PLAYERS" },
            { &g_watermarkDate, "DATE" }
        };
        bool s_wm_qb = DrawQuestionButton("##qb_wm", dl, ImVec2(gmax.x - 14.0f, r4_y), wm_opts, 3, f, 13.0f);
        bool s5 = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r5_y), &g_keybinds, "Keybinds", f, 13.0f);
        bool s6 = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r6_y), &g_movement, "Movement", f, 13.0f);
        float r6b_y = r6_y + kCheckRowPitch;
        bool s6b = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r6b_y), &g_internalDebug, "Logs", f, 13.0f);
        QuestionOption mov_opts[] = {
            { &g_movementVelocityOpt, "VELOCITY" },
            { &g_movementGraphOpt, "GRAPH" },
            { &g_movementKeysOpt, "KEYS" }
        };
        bool s_mov_qb = DrawQuestionButton("##qb_mov", dl, ImVec2(gmax.x - 14.0f, r6_y), mov_opts, 3, f, 13.0f);
        float r6c_y = r6b_y + kCheckRowPitch;
        bool s6c = DrawCheckbox(dl, ImVec2(gmin.x + 14.0f, r6c_y), &S::world_geo_enabled, "World geo", f, 13.0f);
        bool s6c_kb = DrawKeybind("##geo_key", dl, ImVec2(gmax.x - 14.0f, r6c_y), &S::world_geo_key, &S::world_geo_key_mode, f, 13.0f);
        bool s6c_warn = DrawWarnIndicator("##warn_geo", dl, ImVec2(gmax.x - 14.0f - KeybindTextWidth("##geo_key", S::world_geo_key, f) - 8.0f, r6c_y), "This feature may cause lag.", f, 13.0f);
        float r7_y = r6c_y + kCheckRowPitch + 2.0f;
        static bool s_cfgInited = false;
        if (!s_cfgInited)
        {
            RefreshConfigList();
            s_cfgInited = true;
        }
        bool s_dd = DrawConfigDropdown("##dd_cfgsel", dl, ImVec2(gmin.x + 14.0f, r7_y), cw, &g_configSelectedIdx, g_configFiles, "Select config", f, 13.0f);
        float r8_y = r7_y + 33.0f;
        bool s7 = DrawTextInput("##ti_cfg", dl, ImVec2(gmin.x + 14.0f, r8_y), cw, g_configName, sizeof(g_configName), "Config name", "default", f, 13.0f);
        float r9_y = r8_y + 37.0f;
        float b3_w = floorf((cw - 8.0f) / 3.0f);
        static float s_saveTime = -10.0f;
        static float s_loadTime = -10.0f;
        static float s_delTime  = -10.0f;
        static float s_expTime  = -10.0f;
        static float s_impTime  = -10.0f;
        bool save_hov = false, load_hov = false, del_hov = false, exp_hov = false, imp_hov = false;
        const char* save_lbl = ((float)ImGui::GetTime() - s_saveTime < 1.0f) ? "Saved!" : "Save";
        const char* load_lbl = ((float)ImGui::GetTime() - s_loadTime < 1.0f) ? "Loaded!" : "Load";
        const char* del_lbl  = ((float)ImGui::GetTime() - s_delTime < 1.0f) ? "Deleted!" : "Delete";
        const char* exp_lbl  = ((float)ImGui::GetTime() - s_expTime < 1.0f) ? "Exported!" : "Export";
        const char* imp_lbl  = ((float)ImGui::GetTime() - s_impTime < 1.0f) ? "Imported!" : "Import";
        bool save_click = DrawButton(dl, ImVec2(gmin.x + 14.0f, r9_y), b3_w, 20.0f, save_lbl, f, 13.0f, &save_hov);
        bool load_click = DrawButton(dl, ImVec2(gmin.x + 14.0f + b3_w + 4.0f, r9_y), b3_w, 20.0f, load_lbl, f, 13.0f, &load_hov);
        bool del_click  = DrawButton(dl, ImVec2(gmin.x + 14.0f + (b3_w + 4.0f) * 2.0f, r9_y), cw - (b3_w + 4.0f) * 2.0f, 20.0f, del_lbl, f, 13.0f, &del_hov);
        float r10_y = r9_y + 24.0f;
        float b2_w = floorf((cw - 6.0f) * 0.5f);
        bool exp_click  = DrawButton(dl, ImVec2(gmin.x + 14.0f, r10_y), b2_w, 20.0f, exp_lbl, f, 13.0f, &exp_hov);
        bool imp_click  = DrawButton(dl, ImVec2(gmin.x + 14.0f + b2_w + 6.0f, r10_y), b2_w, 20.0f, imp_lbl, f, 13.0f, &imp_hov);
        if (save_click)
        {
            if (SaveConfig(g_configName))
            {
                s_saveTime = (float)ImGui::GetTime();
            }
        }
        if (load_click)
        {
            if (LoadConfig(g_configName))
            {
                s_loadTime = (float)ImGui::GetTime();
            }
        }
        if (del_click)
        {
            if (DeleteConfigFile(g_configName))
            {
                s_delTime = (float)ImGui::GetTime();
            }
        }
        if (exp_click)
        {
            if (ExportConfig(g_configName))
            {
                s_expTime = (float)ImGui::GetTime();
            }
        }
        if (imp_click)
        {
            if (ImportConfig())
            {
                s_impTime = (float)ImGui::GetTime();
            }
        }
        ImVec2 m = io.MousePos;
        bool pHover = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= max.y);
        bool anyPop = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        bool overTheme = g_themeOpen && g_themePos.x >= 0.0f && (m.x >= g_themePos.x && m.x <= g_themePos.x + g_themeSize.x && m.y >= g_themePos.y && m.y <= g_themePos.y + g_themeSize.y);
        bool overBinds = g_keybinds && (m.x >= g_bindsPos.x && m.x <= g_bindsPos.x + g_bindsSize.x && m.y >= g_bindsPos.y && m.y <= g_bindsPos.y + g_bindsSize.y);
        bool overMove = g_movement && (m.x >= g_movementPos.x && m.x <= g_movementPos.x + g_movementSize.x && m.y >= g_movementPos.y && m.y <= g_movementPos.y + g_movementSize.y);
        bool overDebug = g_internalDebug && (m.x >= g_debugPos.x && m.x <= g_debugPos.x + g_debugSize.x && m.y >= g_debugPos.y && m.y <= g_debugPos.y + g_debugSize.y);
        bool pcb = s1 || s2 || s4 || s5 || s6 || s6c || s6c_kb || s6c_warn || s_wm_qb || s_mov_qb;
        bool psl = s3;
        bool pdd = s_dd;
        bool pti = s7;
        bool pbtn = save_hov || load_hov || del_hov || exp_hov || imp_hov;
        if (!g_setDragging && ImGui::IsMouseClicked(0) && pHover && !anyPop && !overTheme && !overBinds && !overMove && !overDebug && !g_navBarHov && g_activeBindId == nullptr && !pcb && !psl && !pdd && !pti && !pbtn)
        {
            g_setDragging = true;
            g_setAnim = 1.0f;
            g_setDragOff = ImVec2(m.x - g_setPos.x, m.y - g_setPos.y);
        }
        if (g_setDragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_setPos.x = m.x - g_setDragOff.x;
                g_setPos.y = m.y - g_setDragOff.y;
            }
            else
            {
                g_setDragging = false;
            }
        }
        if (ease < 0.999f)
        {
            for (int i = 0; i < dl->VtxBuffer.Size; i++)
            {
                ImDrawVert& v = dl->VtxBuffer.Data[i];
                ImU32 a = (v.col >> 24) & 0xFF;
                a = (ImU32)((float)a * ease);
                v.col = (v.col & 0x00FFFFFF) | (a << 24);
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(6);
}
inline void RenderThemePanel()
{
    if (!g_themeOpen)
    {
        g_themeAnim = 0.0f;
        g_themeDragging = false;
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (g_themeAnim < 1.0f)
    {
        float dt = io.DeltaTime;
        if (dt > 0.05f) dt = 0.05f;
        g_themeAnim += dt * 6.0f;
        if (g_themeAnim > 1.0f) g_themeAnim = 1.0f;
    }
    float t = g_themeAnim;
    float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    float y_off = (1.0f - ease) * -14.0f;
    if (g_themePos.x < 0.0f)
    {
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        g_themePos = ImVec2(center.x - g_themeSize.x * 0.5f - 300.0f, center.y - g_themeSize.y * 0.5f - 250.0f);
    }
    ImGui::SetNextWindowSize(g_themeSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(g_themePos.x, g_themePos.y + y_off), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(ease);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ease);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uitheme", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
        DrawPixelBox(dl, pos, max, FrameU32(), BorderU32(), col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), AccentU32());
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        ImVec2 gmin = ImVec2(pos.x + 14.0f, pos.y + 24.0f);
        ImVec2 gmax = ImVec2(max.x - 14.0f, max.y - 10.0f);
        DrawGroupBox(dl, gmin, gmax, "theme", f, 13.0f, GroupU32(), BorderU32(), col_outline);
        float cw = (gmax.x - 14.0f) - (gmin.x + 14.0f);
        float t1_y = gmin.y + 18.0f;
        float t2_y = t1_y + 37.0f;
        const char* theme_items[] = { "Basic", "Cyan", "Purple", "Mint", "Red", "Amber", "Halloween" };
        bool s4 = DrawDropdown("##dd_theme", dl, ImVec2(gmin.x + 14.0f, t1_y), cw, &g_themeMask, theme_items, 7, "Theme", f, 13.0f);
        ImVec2 pk_sz(14.0f, 14.0f);
        ImU32 lab_col = TextU32();
        ImU32 lab_out = IM_COL32(0, 0, 0, 255);
        struct ThemeRow { const char* id; ImVec4* col; const char* label; };
        ThemeRow theme_rows[] = {
            { "##c_th0", &g_accent1, "Accent" },
            { "##c_th1", &g_colFrame, "Window" },
            { "##c_th2", &g_colGroup, "Groups" },
            { "##c_th3", &g_colCtrl, "Controls" },
            { "##c_th4", &g_colCtrlHov, "Ctrl hover" },
            { "##c_th5", &g_colBorder, "Borders" },
            { "##c_th6", &g_colText, "Text" },
            { "##c_th7", &g_colTextDim, "Text dim" },
        };
        bool cp_any = false;
        float pr_y = t2_y;
        for (int i = 0; i < 8; i++)
        {
            bool ch = DrawColorPicker(theme_rows[i].id, dl, ImVec2(gmin.x + 14.0f, pr_y), pk_sz, theme_rows[i].col);
            ImVec2 lbsz = f->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, theme_rows[i].label);
            float lby = floorf(pr_y + (14.0f - lbsz.y) * 0.5f);
            DrawOutlinedText(dl, f, 13.0f, ImVec2(gmin.x + 14.0f + 22.0f, lby), lab_col, lab_out, theme_rows[i].label);
            cp_any = cp_any || ch || ImGui::IsPopupOpen(theme_rows[i].id);
            pr_y += kCheckRowPitch;
        }
        bool rs_hov = false;
        bool rs_click = DrawButton(dl, ImVec2(gmin.x + 14.0f, pr_y + 4.0f), cw, 20.0f, "Reset", f, 13.0f, &rs_hov);
        if (rs_click)
        {
            ApplyThemePalette(0);
            g_themeMask = (1 << 0);
        }
        ImVec2 m = io.MousePos;
        bool pHover = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= max.y);
        bool anyPop = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
        bool overSet = g_settingsOpen && g_setPos.x >= 0.0f && (m.x >= g_setPos.x && m.x <= g_setPos.x + g_setSize.x && m.y >= g_setPos.y && m.y <= g_setPos.y + g_setSize.y);
        bool overBinds = g_keybinds && (m.x >= g_bindsPos.x && m.x <= g_bindsPos.x + g_bindsSize.x && m.y >= g_bindsPos.y && m.y <= g_bindsPos.y + g_bindsSize.y);
        bool overMove = g_movement && (m.x >= g_movementPos.x && m.x <= g_movementPos.x + g_movementSize.x && m.y >= g_movementPos.y && m.y <= g_movementPos.y + g_movementSize.y);
        bool overDebug = g_internalDebug && (m.x >= g_debugPos.x && m.x <= g_debugPos.x + g_debugSize.x && m.y >= g_debugPos.y && m.y <= g_debugPos.y + g_debugSize.y);
        if (!g_themeDragging && ImGui::IsMouseClicked(0) && pHover && !anyPop && !overSet && !overBinds && !overMove && !overDebug && !g_navBarHov && g_activeBindId == nullptr && !s4 && !cp_any && !rs_hov)
        {
            g_themeDragging = true;
            g_themeAnim = 1.0f;
            g_themeDragOff = ImVec2(m.x - g_themePos.x, m.y - g_themePos.y);
        }
        if (g_themeDragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_themePos.x = m.x - g_themeDragOff.x;
                g_themePos.y = m.y - g_themeDragOff.y;
            }
            else
            {
                g_themeDragging = false;
            }
        }
        if (ease < 0.999f)
        {
            for (int i = 0; i < dl->VtxBuffer.Size; i++)
            {
                ImDrawVert& v = dl->VtxBuffer.Data[i];
                ImU32 a = (v.col >> 24) & 0xFF;
                a = (ImU32)((float)a * ease);
                v.col = (v.col & 0x00FFFFFF) | (a << 24);
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(6);
}
inline void RenderExplorerPanel()
{
    static bool s_wasOpen = false;
    if (!g_explorerOpen)
    {
        if (s_wasOpen)
        {
            thenwefuckin::explorer::on_close();
            s_wasOpen = false;
        }
        g_explorerDragging = false;
        return;
    }
    s_wasOpen = true;
    ImGuiIO& io = ImGui::GetIO();
    if (g_explorerPos.x < 0.0f)
    {
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        g_explorerPos = ImVec2(center.x - g_menuSize.x * 0.5f - g_explorerSize.x - 20.0f, center.y - g_explorerSize.y * 0.5f);
    }
    ImGui::SetNextWindowSize(g_explorerSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(g_explorerPos, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uiexplorer", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
        DrawPixelBox(dl, pos, max, FrameU32(), BorderU32(), col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), AccentU32());
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        (void)f;
        thenwefuckin::explorer::render(dl, pos, max);
        ImVec2 m = io.MousePos;
        bool titleHover = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= pos.y + 22.0f);
        if (!g_explorerDragging && ImGui::IsMouseClicked(0) && titleHover &&
            !IsModalActive() && !g_navBarHov)
        {
            g_explorerDragging = true;
            g_explorerDragOff = ImVec2(m.x - g_explorerPos.x, m.y - g_explorerPos.y);
        }
        if (g_explorerDragging)
        {
            if (ImGui::IsMouseDown(0))
            {
                g_explorerPos.x = m.x - g_explorerDragOff.x;
                g_explorerPos.y = m.y - g_explorerDragOff.y;
            }
            else
            {
                g_explorerDragging = false;
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(5);
}
inline void RenderPlayerPreviewPanel()
{
    if (!g_playerPreviewOpen)
    {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (g_playerPreviewPos.x < 0.0f)
    {
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        g_playerPreviewPos = ImVec2(center.x + g_menuSize.x * 0.5f + 20.0f, center.y - g_playerPreviewSize.y * 0.5f);
    }
    ImGui::SetNextWindowSize(g_playerPreviewSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(g_playerPreviewPos, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uiplayerpreview", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
        DrawPixelBox(dl, pos, max, FrameU32(), BorderU32(), col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), AccentU32());
        {
            ImVec2 m = io.MousePos;
            bool onBar = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= pos.y + 20.0f);
            if (!g_playerPreviewDragging && onBar && !IsModalActive() && g_activeBindId == nullptr && ImGui::IsMouseClicked(0))
            {
                g_playerPreviewDragging = true;
                g_playerPreviewDragOff = ImVec2(m.x - g_playerPreviewPos.x, m.y - g_playerPreviewPos.y);
            }
            if (g_playerPreviewDragging)
            {
                if (ImGui::IsMouseDown(0) && !IsModalActive())
                {
                    g_playerPreviewPos.x = floorf(m.x - g_playerPreviewDragOff.x);
                    g_playerPreviewPos.y = floorf(m.y - g_playerPreviewDragOff.y);
                }
                else
                {
                    g_playerPreviewDragging = false;
                }
            }
        }
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        const float optH = 66.0f;
        ImVec2 omin = ImVec2(pos.x + 14.0f, max.y - 10.0f - optH);
        ImVec2 omax = ImVec2(max.x - 14.0f, max.y - 10.0f);
        ImVec2 gmin = ImVec2(pos.x + 14.0f, pos.y + 24.0f);
        ImVec2 gmax = ImVec2(max.x - 14.0f, omin.y - 8.0f);
        DrawGroupBox(dl, gmin, gmax, "player", f, 13.0f, GroupU32(), BorderU32(), col_outline);
        thenwefuckin::visual::preview::render(dl, ImVec2(gmin.x + 6.0f, gmin.y + 8.0f), ImVec2(gmax.x - 6.0f, gmax.y - 6.0f));
        DrawGroupBox(dl, omin, omax, "options", f, 13.0f, GroupU32(), BorderU32(), col_outline);
        DrawCheckbox(dl, ImVec2(omin.x + 12.0f, omin.y + 18.0f), &g_previewSpin, "Spin", f, 13.0f);
        DrawCheckbox(dl, ImVec2(omin.x + 12.0f, omin.y + 38.0f), &g_previewAnimateHp, "Animate HP", f, 13.0f);
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(5);
}
inline void RenderPlayersPanel()
{
    if (!g_playersOpen)
    {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (g_playersPos.x < 0.0f)
    {
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        g_playersPos = ImVec2(center.x + g_menuSize.x * 0.5f + 20.0f, center.y - g_playersSize.y * 0.5f);
    }
    ImGui::SetNextWindowSize(g_playersSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(g_playersPos, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uiplayers", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
        DrawPixelBox(dl, pos, max, FrameU32(), BorderU32(), col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), AccentU32());
        {
            ImVec2 m = io.MousePos;
            bool onBar = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= pos.y + 20.0f);
            if (!g_playersDragging && onBar && !IsModalActive() && g_activeBindId == nullptr && ImGui::IsMouseClicked(0))
            {
                g_playersDragging = true;
                g_playersDragOff = ImVec2(m.x - g_playersPos.x, m.y - g_playersPos.y);
            }
            if (g_playersDragging)
            {
                if (ImGui::IsMouseDown(0) && !IsModalActive())
                {
                    g_playersPos.x = floorf(m.x - g_playersDragOff.x);
                    g_playersPos.y = floorf(m.y - g_playersDragOff.y);
                }
                else
                {
                    g_playersDragging = false;
                }
            }
        }
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        ImVec2 gmin = ImVec2(pos.x + 14.0f, pos.y + 24.0f);
        ImVec2 gmax = ImVec2(max.x - 14.0f, max.y - 10.0f);
        std::vector<std::string> names;
        std::vector<long long> uids;
        std::vector<uintptr_t> addrs;
        std::vector<bool> isLocal;
        if (thenwefuckin::g_proc && thenwefuckin::g_proc->connected())
        {
            auto plist = thenwefuckin::g_ctx.player_list();
            for (auto& pl : plist)
            {
                if (!pl.addr)
                    continue;
                std::string nm = pl.name();
                if (nm.empty())
                    continue;
                long long uid = thenwefuckin::g_proc->read<long long>(pl.addr + Offsets::Player::UserId);
                names.push_back(nm);
                uids.push_back(uid);
                addrs.push_back(pl.addr);
                isLocal.push_back(thenwefuckin::g_ctx.local_player.addr && pl.addr == thenwefuckin::g_ctx.local_player.addr);
            }
        }
        static uintptr_t s_selPlayer = 0;
        static bool s_selInit = false;
        {
            bool stillThere = false;
            size_t localIdx = names.size();
            for (size_t i = 0; i < addrs.size(); ++i)
            {
                if (addrs[i] == s_selPlayer)
                    stillThere = true;
                if (isLocal[i])
                    localIdx = i;
            }
            if (!s_selInit || !stillThere)
            {
                if (localIdx < addrs.size())
                    s_selPlayer = addrs[localIdx];
                else if (!addrs.empty())
                    s_selPlayer = addrs[0];
                else
                    s_selPlayer = 0;
                s_selInit = true;
            }
        }
        size_t selIdx = names.size();
        for (size_t i = 0; i < addrs.size(); ++i)
        {
            if (addrs[i] == s_selPlayer)
            {
                selIdx = i;
                break;
            }
        }
        char ptitle[64];
        snprintf(ptitle, sizeof(ptitle), "players (%d)", (int)names.size());
        ImVec2 listMax(gmax.x, gmin.y + 290.0f);
        DrawGroupBox(dl, gmin, listMax, ptitle, f, 13.0f, GroupU32(), BorderU32(), col_outline);
        {
            const float rowH = 20.0f;
            ImVec2 viewMin(gmin.x + 1.0f, gmin.y + 18.0f);
            ImVec2 viewMax(listMax.x - 1.0f, listMax.y - 5.0f);
            float contentH = (float)names.size() * rowH;
            float viewH = viewMax.y - viewMin.y;
            static float s_plScroll = 0.0f;
            float maxScroll = (contentH > viewH) ? (contentH - viewH) : 0.0f;
            if (s_plScroll < 0.0f) s_plScroll = 0.0f;
            if (s_plScroll > maxScroll) s_plScroll = maxScroll;
            ImVec2 m2 = io.MousePos;
            bool inView = (m2.x >= viewMin.x && m2.x <= viewMax.x && m2.y >= viewMin.y && m2.y <= viewMax.y);
            if (inView && io.MouseWheel != 0.0f && !IsModalActive() && g_activeBindId == nullptr)
            {
                s_plScroll -= io.MouseWheel * 28.0f;
                if (s_plScroll < 0.0f) s_plScroll = 0.0f;
                if (s_plScroll > maxScroll) s_plScroll = maxScroll;
            }
            dl->PushClipRect(viewMin, viewMax, true);
            float ry = viewMin.y + 2.0f - s_plScroll;
            for (size_t i = 0; i < names.size(); ++i)
            {
                if (ry + rowH >= viewMin.y && ry <= viewMax.y)
                {
                    bool rowHov = (m2.x >= viewMin.x && m2.x <= viewMax.x && m2.y >= ry && m2.y <= ry + rowH);
                    if (rowHov && ImGui::IsMouseClicked(0) && !IsModalActive() && g_activeBindId == nullptr)
                    {
                        s_selPlayer = addrs[i];
                        selIdx = i;
                        UiClick();
                    }
                    if (addrs[i] == s_selPlayer)
                        dl->AddRectFilled(ImVec2(viewMin.x, ry), ImVec2(viewMax.x, ry + rowH), IM_COL32(255, 255, 255, 18));
                    float ay = floorf(ry + (rowH - 16.0f) * 0.5f);
                    ID3D11ShaderResourceView* srv = thenwefuckin::visual::players::avatars::avatar_for(uids[i]);
                    if (srv)
                    {
                        dl->AddImage((ImTextureID)srv, ImVec2(gmin.x + 12.0f, ay), ImVec2(gmin.x + 28.0f, ay + 16.0f));
                    }
                    else
                    {
                        dl->AddCircle(ImVec2(gmin.x + 20.0f, ay + 8.0f), 6.0f, TextDimU32(), 20, 1.5f);
                    }
                    ImU32 tcol = (addrs[i] == s_selPlayer || isLocal[i]) ? AccentU32() : TextU32();
                    DrawOutlinedText(dl, f, 13.0f, ImVec2(gmin.x + 34.0f, floorf(ry + (rowH - 13.0f) * 0.5f)), tcol, col_outline, names[i].c_str());
                }
                ry += rowH;
            }
            dl->PopClipRect();
        }
        {
            float cardY = listMax.y + 45.0f;
            float cardX = gmin.x + 12.0f;
            float row2Y = gmax.y - 10.0f - 20.0f;
            float row1Y = row2Y - 6.0f - 20.0f;
            DrawGroupBox(dl, ImVec2(gmin.x, cardY - 20.0f), ImVec2(gmax.x, row2Y + 20.0f + 8.0f), "Previewed player", f, 13.0f, GroupU32(), BorderU32(), col_outline);
            std::string selName = (selIdx < names.size()) ? names[selIdx] : "-";
            long long selUid = (selIdx < uids.size()) ? uids[selIdx] : 0;
            bool selLocal = (selIdx < isLocal.size()) ? isLocal[selIdx] : false;
            float selMeters = -1.0f;
            if (selIdx < addrs.size() && addrs[selIdx] && thenwefuckin::g_proc && thenwefuckin::g_proc->connected())
            {
                thenwefuckin::node ch = thenwefuckin::g_ctx.get_character(thenwefuckin::node{ addrs[selIdx] });
                if (ch)
                {
                    thenwefuckin::node hrp = ch.find_child("HumanoidRootPart");
                    if (!hrp)
                        hrp = ch.find_child("Head");
                    if (hrp)
                    {
                        uintptr_t prim = hrp.primitive();
                        if (prim)
                        {
                            thenwefuckin::math::vec3 p = thenwefuckin::g_proc->read<thenwefuckin::math::vec3>(prim + Offsets::Primitive::Position);
                            if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                                selMeters = (p - thenwefuckin::g_ctx.local_pos).length() * 0.28f;
                        }
                    }
                }
            }
            ImVec2 v3Min(cardX, cardY);
            ImVec2 v3Max(cardX + 200.0f, row1Y - 8.0f);
            if (v3Max.y < v3Min.y + 60.0f) v3Max.y = v3Min.y + 60.0f;
            thenwefuckin::visual::preview::render_for(selUid, dl, v3Min, v3Max);
            float tx = cardX + 200.0f + 10.0f;
            float rx = gmax.x - 8.0f;
            {
                ID3D11ShaderResourceView* headSrv = thenwefuckin::visual::players::avatars::avatar_for(selUid);
                if (headSrv)
                {
                    dl->AddImage((ImTextureID)headSrv, ImVec2(tx, cardY + 2.0f), ImVec2(tx + 20.0f, cardY + 22.0f));
                    DrawOutlinedText(dl, f, 13.0f, ImVec2(tx + 26.0f, cardY + 4.0f), selLocal ? AccentU32() : TextU32(), col_outline, selName.c_str());
                }
                else
                {
                    DrawOutlinedText(dl, f, 13.0f, ImVec2(tx, cardY + 2.0f), selLocal ? AccentU32() : TextU32(), col_outline, selName.c_str());
                }
            }
            uintptr_t selPlayerAddr = (selIdx < addrs.size()) ? addrs[selIdx] : 0;
            char subBuf[64];
            bool spectatingSub = selPlayerAddr && thenwefuckin::misc::spectate::is_spectating(selPlayerAddr);
            if (selMeters >= 0.0f && selLocal)
                snprintf(subBuf, sizeof(subBuf), "%.1fm  local", (double)selMeters);
            else if (selMeters >= 0.0f && spectatingSub)
                snprintf(subBuf, sizeof(subBuf), "%.1fm  spectating", (double)selMeters);
            else if (selMeters >= 0.0f)
                snprintf(subBuf, sizeof(subBuf), "%.1fm", (double)selMeters);
            else if (spectatingSub)
                snprintf(subBuf, sizeof(subBuf), "spectating");
            else if (selLocal)
                snprintf(subBuf, sizeof(subBuf), "local");
            else
                snprintf(subBuf, sizeof(subBuf), "-");
            DrawOutlinedText(dl, f, 12.0f, ImVec2(tx, cardY + 26.0f), TextDimU32(), col_outline, subBuf);

            float selHp = -1.0f, selMaxHp = 100.0f, selSpeed = -1.0f;
            std::string selState = "-";
            std::string selToolStr = "-";
            std::string selTeamStr = "-";
            bool selShield = false;
            bool selDead = false;
            uintptr_t selPlayerAddr2 = selPlayerAddr;
            if (selPlayerAddr2 && thenwefuckin::g_proc && thenwefuckin::g_proc->connected())
            {
                thenwefuckin::node selPl{ selPlayerAddr2 };
                thenwefuckin::node selCh = thenwefuckin::g_ctx.get_character(selPl);
                if (selCh)
                {
                    float hp = 0.0f, mx = 0.0f;
                    thenwefuckin::node hum = thenwefuckin::find_live_humanoid(selCh, &hp, &mx);
                    if (hum)
                    {
                        selHp = hp;
                        selMaxHp = (mx > 0.0f) ? mx : 100.0f;
                        selDead = (hp <= 0.0f);
                    }
                    thenwefuckin::node tool = selCh.find_child_of_class("Tool");
                    if (tool && !tool.name().empty())
                        selToolStr = tool.name();
                    if (selCh.find_child_of_class("ForceField"))
                        selShield = true;
                    uintptr_t teamAddr = thenwefuckin::g_proc->read<uintptr_t>(selPl.addr + Offsets::Player::Team);
                    if (teamAddr)
                    {
                        std::string tm = thenwefuckin::node{ teamAddr }.name();
                        if (!tm.empty())
                            selTeamStr = tm;
                    }
                    thenwefuckin::node hrp = selCh.find_child("HumanoidRootPart");
                    uintptr_t humAddr = hum.addr;
                    if (humAddr)
                    {
                        uintptr_t seat = thenwefuckin::g_proc->read<uintptr_t>(humAddr + Offsets::Humanoid::SeatPart);
                        uint8_t plat = thenwefuckin::g_proc->read<uint8_t>(humAddr + Offsets::Humanoid::PlatformStand);
                        if (selDead)
                            selState = "Dead";
                        else if (seat != 0)
                            selState = "Seated";
                        else if (plat != 0)
                            selState = "Platform";
                        else if (hrp)
                        {
                            uintptr_t prim = hrp.primitive();
                            if (prim)
                            {
                                thenwefuckin::math::vec3 vel = thenwefuckin::g_proc->read<thenwefuckin::math::vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
                                if (vel.x != 0.0f || vel.y != 0.0f || vel.z != 0.0f)
                                {
                                    float hs = sqrtf(vel.x * vel.x + vel.z * vel.z);
                                    selSpeed = hs;
                                    if (fabsf(vel.y) > 8.0f)
                                        selState = (vel.y > 0.0f) ? "Jumping" : "Falling";
                                    else if (hs < 1.0f)
                                        selState = "Idle";
                                    else if (hs < 9.0f)
                                        selState = "Walking";
                                    else
                                        selState = "Running";
                                }
                                else
                                {
                                    selState = "Idle";
                                    selSpeed = 0.0f;
                                }
                            }
                        }
                    }
                }
            }
            auto trunc_stat = [](const std::string& v, size_t n) -> std::string {
                if (v.size() <= n) return v;
                if (n <= 2) return v.substr(0, n);
                return v.substr(0, n - 2) + "..";
            };
            float sy = cardY + 44.0f;
            float statW = rx - tx;
            if (statW < 60.0f) statW = 60.0f;

            {
                char hpTxt[32];
                float frac = -1.0f;
                if (selHp >= 0.0f && selMaxHp > 0.0f)
                {
                    frac = selHp / selMaxHp;
                    if (frac < 0.0f) frac = 0.0f;
                    if (frac > 1.0f) frac = 1.0f;
                    snprintf(hpTxt, sizeof(hpTxt), "%.0f/%.0f", (double)selHp, (double)selMaxHp);
                }
                else
                {
                    snprintf(hpTxt, sizeof(hpTxt), "-");
                }
                DrawOutlinedText(dl, f, 11.0f, ImVec2(tx, sy), TextDimU32(), col_outline, "HP & ARMOR");
                ImVec2 hts = f->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, hpTxt);
                DrawOutlinedText(dl, f, 11.0f, ImVec2(rx - hts.x, sy), TextU32(), col_outline, hpTxt);
                ImVec2 b0(tx, sy + 14.0f), b1(rx, sy + 20.0f);
                dl->AddRectFilled(b0, b1, IM_COL32(12, 12, 14, 255));
                if (frac >= 0.0f)
                {
                    ImU32 fc = (frac > 0.5f) ? IM_COL32(90, 220, 120, 255) : (frac > 0.25f ? IM_COL32(240, 200, 80, 255) : IM_COL32(235, 90, 90, 255));
                    if (frac > 0.0f)
                        dl->AddRectFilled(b0, ImVec2(b0.x + (b1.x - b0.x) * frac, b1.y), fc);
                }
                dl->AddRect(b0, b1, IM_COL32(0, 0, 0, 255));

                ImVec2 a0(tx, sy + 22.0f), a1(rx, sy + 28.0f);
                dl->AddRectFilled(a0, a1, IM_COL32(12, 12, 14, 255));
                if (selShield)
                    dl->AddRectFilled(a0, a1, IM_COL32(90, 200, 255, 255));
                dl->AddRect(a0, a1, IM_COL32(0, 0, 0, 255));
                if (selShield)
                {
                    ImVec2 sts = f->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, "Shield");
                    DrawOutlinedText(dl, f, 10.0f, ImVec2(rx - sts.x, sy + 30.0f), IM_COL32(140, 220, 255, 255), col_outline, "Shield");
                }
                sy += 44.0f;
            }

            {
                char stTxt[64];
                if (selSpeed >= 0.0f && (selState == "Walking" || selState == "Running" || selState == "Idle"))
                    snprintf(stTxt, sizeof(stTxt), "%s %.1f", selState.c_str(), (double)selSpeed);
                else
                    snprintf(stTxt, sizeof(stTxt), "%s", selState.c_str());
                DrawOutlinedText(dl, f, 11.0f, ImVec2(tx, sy), TextDimU32(), col_outline, "STATE");
                ImU32 stCol = selDead ? IM_COL32(235, 90, 90, 255) : TextU32();
                DrawOutlinedText(dl, f, 12.0f, ImVec2(tx, sy + 11.0f), stCol, col_outline, stTxt);
                sy += 27.0f;
            }

            {
                DrawOutlinedText(dl, f, 11.0f, ImVec2(tx, sy), TextDimU32(), col_outline, "TOOL");
                std::string tv = trunc_stat(selToolStr, 18);
                DrawOutlinedText(dl, f, 12.0f, ImVec2(tx, sy + 11.0f), TextU32(), col_outline, tv.c_str());
                sy += 27.0f;
            }

            {
                DrawOutlinedText(dl, f, 11.0f, ImVec2(tx, sy), TextDimU32(), col_outline, "TEAM");
                std::string tv = trunc_stat(selTeamStr, 18);
                DrawOutlinedText(dl, f, 12.0f, ImVec2(tx, sy + 11.0f), TextU32(), col_outline, tv.c_str());
                sy += 27.0f;
            }
            float by = row1Y;
            float bw = floorf((gmax.x - 12.0f - cardX - 3.0f * 6.0f) / 4.0f);
            bool bFr = false, bSp = false, bTp = false, bTg = false;
            bool selFriend = selPlayerAddr && thenwefuckin::visual::players::friends::is_friend(selPlayerAddr);
            if (DrawButton(dl, ImVec2(cardX, by), bw, 20.0f, selFriend ? "unfriend" : "friend", f, 13.0f, &bFr))
            {
                if (selPlayerAddr)
                {
                    uintptr_t selModel = 0;
                    thenwefuckin::node ch = thenwefuckin::g_ctx.get_character(thenwefuckin::node{ selPlayerAddr });
                    if (ch)
                        selModel = ch.addr;
                    thenwefuckin::visual::players::friends::set_friend(selPlayerAddr, selModel, !selFriend);
                }
                UiClick();
            }
            bool spectatingSel = selPlayerAddr && thenwefuckin::misc::spectate::is_spectating(selPlayerAddr);
            if (DrawButton(dl, ImVec2(cardX + (bw + 6.0f), by), bw, 20.0f, spectatingSel ? "unspectate" : "spectate", f, 13.0f, &bSp))
            {
                if (selPlayerAddr)
                    thenwefuckin::misc::spectate::toggle(selPlayerAddr);
                UiClick();
            }
            if (DrawButton(dl, ImVec2(cardX + (bw + 6.0f) * 2.0f, by), bw, 20.0f, "teleport", f, 13.0f, &bTp))
            {
                if (selPlayerAddr)
                    thenwefuckin::misc::spectate::teleport_to(selPlayerAddr);
                UiClick();
            }
            DrawButton(dl, ImVec2(cardX + (bw + 6.0f) * 3.0f, by), bw, 20.0f, "target", f, 13.0f, &bTg);
            float by2 = row2Y;
            float bw2 = floorf((gmax.x - 12.0f - cardX - 6.0f) * 0.5f);
            bool bCp = false, bRf = false;
            if (DrawButton(dl, ImVec2(cardX, by2), bw2, 20.0f, "copy id", f, 13.0f, &bCp))
            {
                if (selUid > 0)
                {
                    char idBuf[32];
                    snprintf(idBuf, sizeof(idBuf), "%lld", selUid);
                    ImGui::SetClipboardText(idBuf);
                }
                else if (!selName.empty() && selName != "-")
                {
                    ImGui::SetClipboardText(selName.c_str());
                }
                UiClick();
            }
            if (DrawButton(dl, ImVec2(cardX + bw2 + 6.0f, by2), bw2, 20.0f, "refresh", f, 13.0f, &bRf))
            {
                thenwefuckin::visual::preview::refresh();
                UiClick();
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(5);
}
inline void RenderLuaPanel()
{
    if (!g_luaOpen)
    {
        g_luaDragging = false;
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (g_luaPos.x < 0.0f)
    {
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        g_luaPos = ImVec2(center.x - g_menuSize.x * 0.5f - g_luaSize.x - 20.0f, center.y - g_luaSize.y * 0.5f + 40.0f);
    }
    ImGui::SetNextWindowSize(g_luaSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(g_luaPos, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, g_colFrame);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("##uilua", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        pos.x = floorf(pos.x);
        pos.y = floorf(pos.y);
        ImVec2 max = ImVec2(floorf(pos.x + size.x), floorf(pos.y + size.y));
        const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
        DrawPixelBox(dl, pos, max, FrameU32(), BorderU32(), col_outline);
        dl->AddRectFilled(ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, pos.y + 3.0f), AccentU32());
        ImVec2 m = io.MousePos;
        bool titleHover = (m.x >= pos.x && m.x <= max.x && m.y >= pos.y && m.y <= pos.y + 22.0f);
        if (!g_luaDragging && ImGui::IsMouseClicked(0) && titleHover &&
            !IsModalActive() && !g_navBarHov && g_activeBindId == nullptr)
        {
            g_luaDragging = true;
            g_luaDragOff = ImVec2(m.x - g_luaPos.x, m.y - g_luaPos.y);
        }
        if (g_luaDragging)
        {
            if (ImGui::IsMouseDown(0) && !IsModalActive())
            {
                g_luaPos.x = floorf(m.x - g_luaDragOff.x);
                g_luaPos.y = floorf(m.y - g_luaDragOff.y);
            }
            else
            {
                g_luaDragging = false;
            }
        }
        ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
        ImVec2 gmin = ImVec2(pos.x + 14.0f, pos.y + 24.0f);
        ImVec2 gmax = ImVec2(max.x - 14.0f, max.y - 10.0f);
        const std::string& src = thenwefuckin::lua_editor::source();
        size_t nlines = 1;
        for (char c : src) if (c == '\n') ++nlines;
        char gtitle[64];
        snprintf(gtitle, sizeof(gtitle), "lua (%d lines)", (int)nlines);
        DrawGroupBox(dl, gmin, gmax, gtitle, f, 13.0f, GroupU32(), BorderU32(), col_outline);
        float cw = (gmax.x - 14.0f) - (gmin.x + 14.0f);
        float exec_h = 20.0f;
        float row_h = 18.0f;
        float gap = 6.0f;
        float out_h = 104.0f;
        float by_exec = gmax.y - 10.0f - exec_h;
        float by_row = by_exec - gap - row_h;
        float out_y1 = by_row - gap;
        float out_y0 = out_y1 - out_h;
        float ed_y0 = gmin.y + 18.0f;
        float ed_y1 = out_y0 - gap;
        ImGui::SetCursorScreenPos(ImVec2(floorf(gmin.x + 14.0f), floorf(ed_y0)));
        ImVec2 avail(floorf(cw), floorf(ed_y1 - ed_y0));
        bool can_edit = !IsModalActive() && g_activeBindId == nullptr;
        thenwefuckin::lua_editor::EditorColors ec;
        ec.bg = CtrlBgU32();
        ec.gutter_bg = GroupU32();
        ec.line_num = TextDimU32();
        ec.line_num_active = AccentU32();
        ec.cur_line = CtrlHovU32();
        ec.sel = AccentDimU32();
        ec.cursor = TextU32();
        ec.text = IM_COL32(212, 212, 212, 255);
        ec.keyword = IM_COL32(86, 156, 214, 255);
        ec.func = IM_COL32(220, 220, 170, 255);
        ec.str = IM_COL32(206, 145, 120, 255);
        ec.comment = IM_COL32(106, 153, 85, 255);
        ec.number = IM_COL32(181, 206, 168, 255);
        ec.outline = col_outline;
        ec.frame_border = BorderU32();
        thenwefuckin::lua_editor::render_editor_child(avail, f, 12.0f, can_edit, ec);

        ImVec2 omin = ImVec2(gmin.x + 14.0f, floorf(out_y0));
        ImVec2 omax = ImVec2(gmax.x - 14.0f, floorf(out_y1));
        DrawGroupBox(dl, omin, omax, "output", f, 13.0f, GroupU32(), BorderU32(), col_outline);
        ImVec2 out_pos = ImVec2(floorf(omin.x + 6.0f), floorf(omin.y + 16.0f));
        ImVec2 out_sz = ImVec2(floorf(omax.x - 6.0f - out_pos.x), floorf(omax.y - 4.0f - out_pos.y));
        ImGui::SetCursorScreenPos(out_pos);
        ImGui::BeginChild("##lua_out", out_sz, false, ImGuiWindowFlags_NoMove);
        {
            std::vector<thenwefuckin::lua::OutLine> lines = thenwefuckin::lua::output_snapshot();
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + out_sz.x - 4.0f);
            for (auto& ln : lines)
            {
                ImU32 col = TextDimU32();
                if (ln.level == thenwefuckin::lua::OutLevel::Print) col = TextU32();
                else if (ln.level == thenwefuckin::lua::OutLevel::Warn) col = IM_COL32(240, 200, 80, 255);
                else if (ln.level == thenwefuckin::lua::OutLevel::Error) col = IM_COL32(235, 90, 90, 255);
                else if (ln.level == thenwefuckin::lua::OutLevel::Ok) col = IM_COL32(90, 220, 120, 255);
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(col));
                ImGui::TextUnformatted(ln.text.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::PopTextWrapPos();
            static size_t s_out_n = 0;
            if (lines.size() != s_out_n)
            {
                s_out_n = lines.size();
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();

        float bw = floorf((cw - 16.0f) / 5.0f);
        float bx = gmin.x + 14.0f;
        bool h1 = false, h2 = false, h3 = false, h4 = false, h5 = false, h6 = false;
        if (DrawButton(dl, ImVec2(bx, by_row), bw, row_h, "copy", f, 12.0f, &h1))
        {
            ImGui::SetClipboardText(thenwefuckin::lua_editor::source().c_str());
        }
        if (DrawButton(dl, ImVec2(bx + (bw + 4.0f), by_row), bw, row_h, "paste", f, 12.0f, &h2))
        {
            const char* clip = ImGui::GetClipboardText();
            if (clip && clip[0])
            {
                thenwefuckin::lua_editor::focus_request() = true;
                thenwefuckin::lua_editor::insert_text(clip);
            }
        }
        if (DrawButton(dl, ImVec2(bx + (bw + 4.0f) * 2.0f, by_row), bw, row_h, "clear", f, 12.0f, &h3))
        {
            thenwefuckin::lua_editor::clear_all();
            thenwefuckin::lua_editor::focus_request() = true;
        }
        if (DrawButton(dl, ImVec2(bx + (bw + 4.0f) * 3.0f, by_row), bw, row_h, "sample", f, 12.0f, &h4))
        {
            thenwefuckin::lua_editor::load_sample();
            thenwefuckin::lua_editor::focus_request() = true;
        }
        if (DrawButton(dl, ImVec2(bx + (bw + 4.0f) * 4.0f, by_row), bw, row_h, "out", f, 12.0f, &h6))
        {
            std::string txt;
            for (auto& ln : thenwefuckin::lua::output_snapshot())
            {
                txt += ln.text;
                txt += "\r\n";
            }
            if (!txt.empty())
                ImGui::SetClipboardText(txt.c_str());
        }
        if (DrawButton(dl, ImVec2(bx, by_exec), cw, exec_h, "execute", f, 13.0f, &h5))
        {
            const std::string src_now = thenwefuckin::lua_editor::source();
            if (thenwefuckin::lua::ready() && thenwefuckin::lua::execute(src_now))
            {
                char eb[128];
                snprintf(eb, sizeof(eb), "lua executed: %d chars, %d lines",
                    (int)src_now.size(), (int)nlines);
                LOG("lua", eb);
            }
            else
            {
                thenwefuckin::lua::log(thenwefuckin::lua::OutLevel::Error, "vm not ready");
                LOG("error", "lua execute failed: vm not ready");
            }
        }
        (void)h1; (void)h2; (void)h3; (void)h4; (void)h5; (void)h6;
    }
    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(5);
}
}
