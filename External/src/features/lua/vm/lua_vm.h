#pragma once
#include <functional>
#include <string>
#include <vector>

struct lua_State;

namespace thenwefuckin::lua
{
enum class OutLevel { Info, Print, Warn, Error, Ok };
struct OutLine
{
    OutLevel level = OutLevel::Info;
    std::string text;
};

struct Snapshot
{
    int players = 0;
    float fps = 0.0f;
};

struct PlayerSnap
{
    std::string name;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    float hp = 0.0f;
    float dist = 0.0f;
};

struct DrawCmd
{
    int kind = 0;
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;
    unsigned color = 0xFFFFFFFFu;
    std::string text;
};

bool initialize();
void shutdown();
bool ready();
lua_State* state();

bool execute(const std::string& source, const char* chunk_name = "script");
void tick(float dt);

void log(OutLevel level, const std::string& text);
void clear_output();
std::vector<OutLine> output_snapshot();

void push_action(std::function<void()> fn);
void run_actions();

void set_snapshot(const Snapshot& s);
Snapshot snapshot();

void snapshot_players(float w, float h);
std::vector<PlayerSnap> players_snapshot();
bool has_render();
void fire_render();

void push_draw(const DrawCmd& d);
std::vector<DrawCmd> take_draws();
}
