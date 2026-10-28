#pragma once
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <string>
#include <atomic>
#include <functional>
#include <chrono>
struct OverlayConfig {
    std::wstring windowTitle = L" ";
    std::wstring targetWindow = L"";
    bool vsync = false;
};
struct ImDrawList;
class Overlay {
public:
    explicit Overlay(const OverlayConfig& cfg = {});
    ~Overlay();
    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;
    bool Initialize();
    void Run();
    void Shutdown();
    void SetVSync(bool enabled);
    void SetStreamProof(bool enabled);
    void SnapToWindow(const std::wstring& title);
    using RenderCallback = std::function<void(ImDrawList*, int, int, float, bool)>;
    void SetRenderCallback(RenderCallback cb) { m_renderCb = std::move(cb); }
    bool  IsRunning()  const { return m_running; }
    bool  IsMenuOpen() const { return m_menuOpen; }
    float GetFPS()     const { return m_fps; }
    int   GetWidth()   const { return m_width; }
    int   GetHeight()  const { return m_height; }
    ID3D11Device* GetDevice()  const { return m_device; }
    ID3D11DeviceContext* GetContext() const { return m_ctx; }
    ID3D11RenderTargetView* GetRTV() const { return m_rtv; }
    static HWND GetOverlayWindow() { return s_instance ? s_instance->m_hwnd : nullptr; }
    static HWND GetTargetWindow() { return s_instance ? s_instance->m_targetHwnd : nullptr; }
    static ID3D11Device* GetDeviceStatic() { return s_instance ? s_instance->m_device : nullptr; }
    static ID3D11DeviceContext* GetContextStatic() { return s_instance ? s_instance->m_ctx : nullptr; }
    static void ToggleMenu();
    static void SetMenuOpen(bool open);
    static void SetFrameInterval(float seconds);
    static void ConsumeMouseDelta(long& dx, long& dy);
    static void SetRenderEnabled(bool on) { s_renderEnabled = on; }
private:
    static RECT GetPrimaryMonitorRect();
    bool CreateOverlayWindow();
    void DestroyOverlayWindow();
    void SetClickThrough(bool enabled);
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    bool InstallKeyboardHook();
    void RemoveKeyboardHook();
    static LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    bool InitD3D();
    void CleanupD3D();
    bool ResizeBuffers(int w, int h);
    bool InitImGui();
    void CleanupImGui();
    bool WantsRender() const { return s_renderEnabled; }
    void BeginFrame();
    void EndFrame();
    void UpdateFPS();
    OverlayConfig            m_cfg;
    HWND                     m_hwnd = nullptr;
    HWND                     m_targetHwnd = nullptr;
    WNDCLASSEXW              m_wc = {};
    int                      m_width = 0;
    int                      m_height = 0;
    int                      m_posX = 0;
    int                      m_posY = 0;
    std::atomic<bool>        m_running{ false };
    bool                     m_menuOpen = false;
    HWND                     m_prevFg = nullptr;
    bool                     m_clickThrough = false;
    bool                     m_vsync = false;
    bool                     m_streamProof = false;
    static bool               s_renderEnabled;
    HHOOK                    m_kbHook = nullptr;
    static Overlay* s_instance;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_ctx = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_rtv = nullptr;
    float  m_fps = 0.f;
    int    m_frameCount = 0;
    std::chrono::steady_clock::time_point m_fpsTimer;
    std::chrono::steady_clock::time_point m_lastFrameStart{};
    static float s_frameInterval;
    RenderCallback m_renderCb;
};
