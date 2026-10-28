#include "mascot.h"
#include "menu.h"
#include "../../resources/halloween_cat.h"
#include "../../../utility/stb/stb_image.h"
namespace Menu
{
    namespace
    {
        ID3D11ShaderResourceView* s_tex = nullptr;
        int s_w = 0;
        int s_h = 0;
        bool s_tried = false;
    }
    bool IsHalloweenActive()
    {
        return (g_themeMask & (1 << kHalloweenThemeBit)) != 0;
    }
    bool EnsureHalloweenTex()
    {
        if (s_tex)
            return true;
        if (s_tried)
            return false;
        s_tried = true;
        ID3D11Device* dev = ::Overlay::GetDeviceStatic();
        if (!dev)
            return false;
        int w = 0, h = 0, ch = 0;
        unsigned char* px = stbi_load_from_memory(HalloweenCat_png, (int)HalloweenCat_png_len, &w, &h, &ch, 4);
        if (!px || w <= 0 || h <= 0)
        {
            if (px)
                stbi_image_free(px);
            return false;
        }
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = (UINT)w;
        desc.Height = (UINT)h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sub{};
        sub.pSysMem = px;
        sub.SysMemPitch = (UINT)(w * 4);
        ID3D11Texture2D* t = nullptr;
        HRESULT hr = dev->CreateTexture2D(&desc, &sub, &t);
        if (FAILED(hr) || !t)
        {
            stbi_image_free(px);
            return false;
        }
        ID3D11ShaderResourceView* srv = nullptr;
        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = desc.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MipLevels = 1;
        hr = dev->CreateShaderResourceView(t, &sd, &srv);
        t->Release();
        stbi_image_free(px);
        if (FAILED(hr) || !srv)
            return false;
        s_tex = srv;
        s_w = w;
        s_h = h;
        return true;
    }
    ID3D11ShaderResourceView* GetHalloweenTex()
    {
        if (!EnsureHalloweenTex())
            return nullptr;
        return s_tex;
    }
    void GetHalloweenTexSize(int& w, int& h)
    {
        w = s_w;
        h = s_h;
    }
    void DrawHalloweenMascot()
    {
        if (!IsHalloweenActive())
            return;
        ID3D11ShaderResourceView* tex = GetHalloweenTex();
        if (!tex)
            return;
        int tw = 0, th = 0;
        GetHalloweenTexSize(tw, th);
        if (tw <= 0 || th <= 0)
            return;
        const float drawW = 120.0f;
        const float drawH = drawW * (float)th / (float)tw;
        ImVec2 br(g_pos.x + g_menuSize.x - 6.0f, g_pos.y + 1.0f);
        ImVec2 tl(br.x - drawW, br.y - drawH);
        ImGui::GetForegroundDrawList()->AddImage((ImTextureID)tex, tl, br);
    }
}
