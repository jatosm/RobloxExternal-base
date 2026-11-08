#pragma once
#include <string>
#include <vector>
namespace thenwefuckin::misc::hitsound
{
    void run();
    void play();
    void open_folder();
    std::vector<std::string> get_sound_list();
    void refresh_sounds();
}
