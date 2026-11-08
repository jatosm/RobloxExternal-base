#pragma once

struct ImDrawList;

namespace thenwefuckin::performance
{
    void update(float fps);
    void render(ImDrawList* dl);
    bool open();
    void toggle();
    void close();
    bool hit_test(int x, int y);
}
