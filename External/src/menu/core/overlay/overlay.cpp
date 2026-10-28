#include "overlay.h"
#include "../../../utility/global/globals.h"
#include "../gui/resources/font.h"
#include "../gui/menu.h"
#include <dwmapi.h>
#include <windowsx.h>
#include <dxgi1_2.h>
#include <chrono>
#include <thread>
#include <mmsystem.h>
#ifndef DXGI_PRESENT_ALLOW_TEARING
#define DXGI_PRESENT_ALLOW_TEARING 0x00000200U
#endif
#ifndef DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
#define DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING 512
#endif
#ifndef DXGI_PRESENT_DO_NOT_WAIT
#define DXGI_PRESENT_DO_NOT_WAIT 0x00000001U
#endif
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../../imgui/imgui_impl_dx11.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
Overlay* Overlay::s_instance = nullptr;
bool Overlay::s_renderEnabled = true;
float Overlay::s_frameInterval = 0.0f;
extern bool g_vsync;
extern bool g_streamProof;
extern bool g_menuOpen;
extern int g_fpsCap;
Overlay::Overlay(const OverlayConfig& cfg) : m_cfg(cfg)
{
    m_fpsTimer = std::chrono::steady_clock::now();
    m_lastFrameStart = m_fpsTimer;
}
void Overlay::SetFrameInterval(float seconds)
{
    s_frameInterval = (seconds > 0.0f) ? seconds : 0.0f;
}
Overlay::~Overlay()
{
    Shutdown();
}
bool Overlay::Initialize()
{
    if (!CreateOverlayWindow())
    {
        return false;
    }
    if (!InitD3D())
    {
        return false;
    }
    if (!InitImGui())
    {
        return false;
    }
    if (!InstallKeyboardHook())
    {
        return false;
    }
    m_running = true;
    return true;
}
void Overlay::Run()
{
    timeBeginPeriod(1);
    MSG msg = {};
    while (m_running)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
            {
                m_running = false;
            }
        }
        if (!m_running)
        {
            break;
        }
        if (!m_cfg.targetWindow.empty())
        {
            SnapToWindow(m_cfg.targetWindow);
        }
        SetVSync(g_vsync);
        SetStreamProof(g_streamProof);
        {
            bool vis = m_menuOpen || g_menuOpen || !Menu::g_navHidden || Menu::g_settingsOpen || Menu::g_themeOpen || Menu::g_explorerOpen || Menu::g_playersOpen || Menu::g_playerPreviewOpen || Menu::g_luaOpen;
            static bool lastVis = false;
            static bool firstVis = true;
            if (firstVis || vis != lastVis)
            {
                firstVis = false;
                lastVis = vis;
                SetClickThrough(!vis);
            }
        }
        if (!s_renderEnabled && !g_menuOpen)
        {
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
                if (msg.message == WM_QUIT)
                {
                    m_running = false;
                }
            }
            if (!m_running)
            {
                break;
            }
            if (m_renderCb)
            {
                try { m_renderCb(nullptr, m_width, m_height, m_fps, false); }
                catch (...) {}
                if (s_renderEnabled)
                    continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
            continue;
        }
        s_renderEnabled = true;
        BeginFrame();
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        float imguiFPS = ImGui::GetIO().Framerate;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (m_renderCb)
        {
            m_renderCb(dl, m_width, m_height, imguiFPS, m_menuOpen);
        }
        ImGui::Render();
        EndFrame();
        {
            auto frameStart = std::chrono::steady_clock::now();
            if (g_fpsCap >= 30)
            {

                auto target = std::chrono::microseconds(1000000 / g_fpsCap);
                auto elapsed = frameStart - m_lastFrameStart;
                if (elapsed < target)
                {
                    auto remain = target - elapsed;
                    if (remain > std::chrono::milliseconds(2))
                        std::this_thread::sleep_for(remain - std::chrono::milliseconds(1));
                    while (std::chrono::steady_clock::now() - m_lastFrameStart < target) { }
                }
            }
            m_lastFrameStart = frameStart;
            if (s_frameInterval > 0.0f)
            {
                auto elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - frameStart).count();
                float remaining = s_frameInterval - elapsed;
                if (remaining > 0.0005f)
                {
                    std::this_thread::sleep_for(std::chrono::duration<float>(remaining));
                }
            }
        }
    }
    timeEndPeriod(1);
}
void Overlay::Shutdown()
{
    RemoveKeyboardHook();
    CleanupImGui();
    CleanupD3D();
    DestroyOverlayWindow();
    m_running = false;
}
bool Overlay::InstallKeyboardHook()
{
    s_instance = this;
    m_kbHook = SetWindowsHookExW(WH_KEYBOARD_LL, LLKeyboardProc, GetModuleHandleW(nullptr), 0);
    return m_kbHook != nullptr;
}
void Overlay::RemoveKeyboardHook()
{
    if (m_kbHook)
    {
        UnhookWindowsHookEx(m_kbHook);
        m_kbHook = nullptr;
        s_instance = nullptr;
    }
}
void Overlay::ToggleMenu()
{
    if (!s_instance)
    {
        return;
    }
    bool anyVisible = s_instance->m_menuOpen || g_menuOpen || !Menu::g_navHidden || Menu::g_settingsOpen || Menu::g_themeOpen || Menu::g_explorerOpen || Menu::g_playersOpen || Menu::g_playerPreviewOpen || Menu::g_luaOpen;
    bool open = !anyVisible;
    if (!open)
    {
        Menu::g_settingsOpen = false;
        Menu::g_themeOpen = false;
        Menu::g_explorerOpen = false;
        Menu::g_playersOpen = false;
        Menu::g_playerPreviewOpen = false;
        Menu::g_luaOpen = false;
        Menu::g_navHidden = true;
    }
    else
    {
        Menu::g_navHidden = false;
    }
    SetMenuOpen(open);
}
void Overlay::SetMenuOpen(bool open)
{
    if (s_instance)
    {
        s_instance->m_menuOpen = open;
        s_instance->SetClickThrough(!open);
        g_menuOpen = open;
    }
}
LRESULT CALLBACK Overlay::LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && s_instance)
    {
        KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            if (kb->vkCode == VK_INSERT)
            {
                static ULONGLONG s_lastToggle = 0;
                ULONGLONG now = GetTickCount64();
                if (now - s_lastToggle > 250)
                {
                    s_lastToggle = now;
                    ToggleMenu();
                }
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
RECT Overlay::GetPrimaryMonitorRect()
{
    struct Ctx
    {
        RECT rect{};
    };
    Ctx ctx{};
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC hDc, LPRECT lpRect, LPARAM lp) -> BOOL {
        (void)hMon;
        (void)hDc;
        (void)lpRect;
        MONITORINFO mi{ sizeof(MONITORINFO) };
        if (GetMonitorInfoW((HMONITOR)lp, &mi) && (mi.dwFlags & MONITORINFOF_PRIMARY))
        {
            reinterpret_cast<Ctx*>(lp)->rect = mi.rcMonitor;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&ctx));
    if (ctx.rect.right == 0)
    {
        ctx.rect.right = GetSystemMetrics(SM_CXSCREEN);
        ctx.rect.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    return ctx.rect;
}
bool Overlay::CreateOverlayWindow()
{
    RECT primary = GetPrimaryMonitorRect();
    m_posX = primary.left;
    m_posY = primary.top;
    m_width = primary.right - primary.left;
    m_height = primary.bottom - primary.top;
    m_wc = {};
    m_wc.cbSize = sizeof(WNDCLASSEXW);
    m_wc.style = CS_HREDRAW | CS_VREDRAW;
    m_wc.lpfnWndProc = WndProc;
    m_wc.hInstance = GetModuleHandleW(nullptr);
    m_wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    m_wc.lpszClassName = L"DX11_OV_CLS";
    if (!RegisterClassExW(&m_wc))
    {
        return false;
    }
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    m_hwnd = CreateWindowExW(exStyle, m_wc.lpszClassName, m_cfg.windowTitle.c_str(), WS_POPUP, m_posX, m_posY, m_width, m_height, nullptr, nullptr, m_wc.hInstance, this);
    if (!m_hwnd)
    {
        return false;
    }
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    MARGINS margins{ -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);
    DWM_BLURBEHIND bb{ DWM_BB_ENABLE, TRUE, nullptr, FALSE };
    DwmEnableBlurBehindWindow(m_hwnd, &bb);
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = m_hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    return true;
}
static std::atomic<long> s_rawDx{ 0 };
static std::atomic<long> s_rawDy{ 0 };
void Overlay::ConsumeMouseDelta(long& dx, long& dy)
{
    dx = s_rawDx.exchange(0, std::memory_order_relaxed);
    dy = s_rawDy.exchange(0, std::memory_order_relaxed);
}
void Overlay::DestroyOverlayWindow()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}
static void ForceFocusWindow(HWND hwnd)
{
    if (!hwnd)
    {
        return;
    }
    HWND fg = GetForegroundWindow();
    if (fg != hwnd)
    {
        DWORD fgThread = fg ? GetWindowThreadProcessId(fg, nullptr) : 0;
        DWORD curThread = GetCurrentThreadId();
        if (fgThread && fgThread != curThread)
        {
            AttachThreadInput(curThread, fgThread, TRUE);
            BringWindowToTop(hwnd);
            SetForegroundWindow(hwnd);
            SetActiveWindow(hwnd);
            SetFocus(hwnd);
            AttachThreadInput(curThread, fgThread, FALSE);
        }
        else
        {
            BringWindowToTop(hwnd);
            SetForegroundWindow(hwnd);
            SetActiveWindow(hwnd);
            SetFocus(hwnd);
        }
    }
}
void Overlay::SetClickThrough(bool enabled)
{
    if (!m_hwnd)
    {
        return;
    }
    if (enabled == m_clickThrough)
    {
        LONG_PTR ex = GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE);
        bool actual = (ex & WS_EX_TRANSPARENT) != 0;
        if (actual == enabled)
        {
            return;
        }
    }
    m_clickThrough = enabled;
    LONG_PTR ex = GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE);
    if (enabled)
    {
        ex |= WS_EX_TRANSPARENT;
        ex |= WS_EX_NOACTIVATE;
        SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, ex);
        SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        if (m_prevFg && m_prevFg != m_hwnd && IsWindow(m_prevFg))
        {
            ForceFocusWindow(m_prevFg);
        }
        else
        {
            HWND top = GetWindow(m_hwnd, GW_HWNDNEXT);
            while (top && (!IsWindowVisible(top) || top == m_hwnd))
            {
                top = GetWindow(top, GW_HWNDNEXT);
            }
            if (top)
            {
                SetForegroundWindow(top);
            }
        }
        m_prevFg = nullptr;
    }
    else
    {
        HWND fg = GetForegroundWindow();
        if (fg && fg != m_hwnd)
        {
            m_prevFg = fg;
        }
        ex &= ~WS_EX_TRANSPARENT;
        ex &= ~WS_EX_NOACTIVATE;
        SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, ex);
        SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ForceFocusWindow(m_hwnd);
    }
}
LRESULT CALLBACK Overlay::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return TRUE;
    }
    Overlay* ov = reinterpret_cast<Overlay*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    if (!ov)
    {
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    bool menuActive = ov->m_menuOpen || g_menuOpen;
    bool dockLive = !Menu::g_navHidden;
    bool feedImgui = menuActive;
    if (!feedImgui && dockLive && (msg == WM_SETCURSOR || (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)))
    {
        int lx = 0;
        int ly = 0;
        if (msg == WM_SETCURSOR)
        {
            DWORD mp = GetMessagePos();
            POINT pt{ GET_X_LPARAM(mp), GET_Y_LPARAM(mp) };
            lx = pt.x - ov->m_posX;
            ly = pt.y - ov->m_posY;
        }
        else
        {
            lx = GET_X_LPARAM(lParam);
            ly = GET_Y_LPARAM(lParam);
        }
        if (Menu::NavHitTest(lx, ly, ov->m_width, ov->m_height) || Menu::PanelHitTest(lx, ly))
        {
            feedImgui = true;
        }
    }
    if (feedImgui)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        {
            return true;
        }
    }
    switch (msg)
    {
    case WM_NCHITTEST:
    {
        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int lx = pt.x - ov->m_posX;
        int ly = pt.y - ov->m_posY;
        if ((dockLive && Menu::NavHitTest(lx, ly, ov->m_width, ov->m_height)) || Menu::PanelHitTest(lx, ly))
        {
            return HTCLIENT;
        }
        return menuActive ? HTCLIENT : HTTRANSPARENT;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_MOUSEACTIVATE:
        if (menuActive)
        {
            return MA_ACTIVATE;
        }
        return MA_NOACTIVATE;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        if (menuActive && GetForegroundWindow() != hWnd)
        {
            ForceFocusWindow(hWnd);
        }
        break;
    case WM_INPUT:
    {
        UINT sz = 0;
        if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &sz, sizeof(RAWINPUTHEADER)) == 0 && sz >= sizeof(RAWINPUTHEADER))
        {
            BYTE buf[sizeof(RAWINPUT)];
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, buf, &sz, sizeof(RAWINPUTHEADER)) == sz)
            {
                const RAWINPUT* ri = reinterpret_cast<const RAWINPUT*>(buf);
                if (ri->header.dwType == RIM_TYPEMOUSE)
                {
                    s_rawDx.fetch_add(ri->data.mouse.lLastX, std::memory_order_relaxed);
                    s_rawDy.fetch_add(ri->data.mouse.lLastY, std::memory_order_relaxed);
                }
            }
        }
        break;
    }
    case WM_SIZE:
        if (ov && ov->m_device && wParam != SIZE_MINIMIZED)
        {
            ov->ResizeBuffers(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
bool Overlay::InitD3D()
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = (UINT)m_width;
    sd.BufferDesc.Height = (UINT)m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate = { 0, 1 };
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc = { 1, 0 };
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL fl;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &fl, &m_ctx);
    if (FAILED(hr))
    {
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, &fl, &m_ctx);
    }
    if (FAILED(hr))
    {
        return false;
    }
    if (IDXGIDevice1* dxgi1 = nullptr)
    {
        if (SUCCEEDED(m_device->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgi1))) && dxgi1)
        {
            dxgi1->SetMaximumFrameLatency(1);
            dxgi1->Release();
        }
    }
    ID3D11Texture2D* bb = nullptr;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&bb));
    if (!bb)
    {
        return false;
    }
    m_device->CreateRenderTargetView(bb, nullptr, &m_rtv);
    bb->Release();
    return true;
}
void Overlay::CleanupD3D()
{
    if (m_rtv)
    {
        m_rtv->Release();
        m_rtv = nullptr;
    }
    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }
    if (m_ctx)
    {
        m_ctx->Release();
        m_ctx = nullptr;
    }
    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }
}
bool Overlay::ResizeBuffers(int w, int h)
{
    if (m_rtv)
    {
        m_rtv->Release();
        m_rtv = nullptr;
    }
    HRESULT hr = m_swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
    {
        return false;
    }
    ID3D11Texture2D* bb = nullptr;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&bb));
    if (FAILED(hr) || !bb)
    {
        return false;
    }
    hr = m_device->CreateRenderTargetView(bb, nullptr, &m_rtv);
    bb->Release();
    if (FAILED(hr))
    {
        return false;
    }
    m_width = w;
    m_height = h;
    return true;
}
bool Overlay::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    if (!ImGui_ImplWin32_Init(m_hwnd))
    {
        return false;
    }
    if (!ImGui_ImplDX11_Init(m_device, m_ctx))
    {
        return false;
    }
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ID3D11SamplerState* linearSampler = nullptr;
    if (SUCCEEDED(m_device->CreateSamplerState(&sampDesc, &linearSampler)))
    {
        m_ctx->PSSetSamplers(0, 1, &linearSampler);
        linearSampler->Release();
    }
    LoadFonts();
    return true;
}
void Overlay::CleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    if (ImGui::GetCurrentContext())
    {
        ImGui::DestroyContext();
    }
}
void Overlay::BeginFrame()
{
    const float clear[4] = { 0.f, 0.f, 0.f, 0.f };
    m_ctx->OMSetRenderTargets(1, &m_rtv, nullptr);
    m_ctx->ClearRenderTargetView(m_rtv, clear);
}
void Overlay::EndFrame()
{
    m_ctx->OMSetRenderTargets(1, &m_rtv, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    if (m_vsync)
    {
        m_swapChain->Present(1, 0);
    }
    else
    {
        HRESULT pr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING | DXGI_PRESENT_DO_NOT_WAIT);
        if (pr == DXGI_ERROR_INVALID_CALL)
            pr = m_swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
        if (pr == DXGI_ERROR_INVALID_CALL)
            m_swapChain->Present(0, 0);
    }
    UpdateFPS();
}
void Overlay::UpdateFPS()
{
    ++m_frameCount;
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - m_fpsTimer).count();
    if (dt >= 0.5f)
    {
        m_fps = m_frameCount / dt;
        m_frameCount = 0;
        m_fpsTimer = now;
    }
}
void Overlay::SnapToWindow(const std::wstring& title)
{
    HWND target = FindWindowW(nullptr, title.c_str());
    if (!target)
    {
        m_targetHwnd = nullptr;
        return;
    }
    m_targetHwnd = target;
    RECT r{};
    if (!GetWindowRect(target, &r))
    {
        return;
    }
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    SetWindowPos(m_hwnd, HWND_TOPMOST, r.left, r.top, w, h, SWP_NOACTIVATE);
    if (w != m_width || h != m_height)
    {
        ResizeBuffers(w, h);
    }
}
void Overlay::SetVSync(bool enabled)
{
    m_vsync = enabled;
}
void Overlay::SetStreamProof(bool enabled)
{
    if (!m_hwnd || enabled == m_streamProof)
    {
        return;
    }
    m_streamProof = enabled;
    DWORD affinity = enabled ? WDA_EXCLUDEFROMCAPTURE : 0;
    SetWindowDisplayAffinity(m_hwnd, affinity);
}
#define STB_IMAGE_IMPLEMENTATION
#include "../../../utility/stb/stb_image.h"
ID3D11ShaderResourceView* Menu::LoadIconTexture(const unsigned char* png, int len)
{
    int tw = 0;
    int th = 0;
    unsigned char* px = stbi_load_from_memory(png, len, &tw, &th, nullptr, 4);
    if (!px || tw <= 0 || th <= 0)
    {
        if (px) stbi_image_free(px);
        return nullptr;
    }
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11Device* dev = Overlay::GetDeviceStatic();
    if (dev)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = (UINT)tw;
        desc.Height = (UINT)th;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init = {};
        init.pSysMem = px;
        init.SysMemPitch = (UINT)(tw * 4);
        ID3D11Texture2D* tex = nullptr;
        if (SUCCEEDED(dev->CreateTexture2D(&desc, &init, &tex)))
        {
            dev->CreateShaderResourceView(tex, nullptr, &srv);
            tex->Release();
        }
    }
    stbi_image_free(px);
    return srv;
}
