#pragma once
struct ImDrawList;
namespace thenwefuckin::frame
{
    void tick(ImDrawList* dl, int width, int height, float fps, bool menuOpen);
}
