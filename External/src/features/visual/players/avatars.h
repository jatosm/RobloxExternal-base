#pragma once
#include <cstdint>
struct ID3D11ShaderResourceView;
namespace thenwefuckin::visual::players::avatars
{
    ID3D11ShaderResourceView* avatar_for(long long userId);
    void clear();
}
