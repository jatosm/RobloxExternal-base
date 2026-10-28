#pragma once
struct ID3D11ShaderResourceView;
namespace Menu
{
    constexpr int kHalloweenThemeBit = 6;
    bool EnsureHalloweenTex();
    ID3D11ShaderResourceView* GetHalloweenTex();
    void GetHalloweenTexSize(int& w, int& h);
    bool IsHalloweenActive();
    void DrawHalloweenMascot();
}
