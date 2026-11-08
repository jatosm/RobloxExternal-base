#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace thenwefuckin::serverbrowser
{
    struct server_entry
    {
        std::string id;
        int max_players = 0;
        int playing = 0;
        int ping = 0;
        float fps = 0.0f;
    };

    struct state_t
    {
        char place_id[32] = "";
        char filter[64] = "";
        int sort_mode = 0;
        bool hide_full = false;
        bool loading = false;
        std::string status = "ready";
        uint64_t active_place = 0;
        std::string active_job;
        uint64_t resolved_place = 0;
        std::vector<server_entry> servers;
        std::string next_cursor;
        std::vector<std::string> cursors;
        int page = 1;
        int selected = -1;
        float scroll = 0.0f;
    };

    std::mutex& mtx();
    state_t& state();

    void refresh_current_game();
    uint64_t target_place_id();
    void fetch(uint64_t place_id, const std::string& cursor = "");
    void apply_filter();
    void join(uint64_t place_id, const std::string& job_id);
    void copy_job_id(const std::string& job_id);
    void copy_teleport_script(uint64_t place_id, const std::string& job_id);
    void next_page();
    void prev_page();
}
