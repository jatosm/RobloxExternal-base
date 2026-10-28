#pragma once
#include "lua_vm.h"
#include <mutex>
struct lua_State;
namespace thenwefuckin::lua::detail
{
inline constexpr const char* kTaskMt = "thenwefuckin.Task";
struct DelayToken
{
    int idx = -1;
    unsigned seq = 0;
};
std::recursive_mutex& mutex();
lua_State* state();
void set_busy(bool);
void schedule_wait(lua_State* L, float sec);
int alloc_delay(lua_State* L, float sec, int fn_idx);
unsigned delay_seq(int idx);
bool cancel_delay(lua_State* L, int idx, unsigned seq);
void reset_render();
void clear_render_callbacks();
}
