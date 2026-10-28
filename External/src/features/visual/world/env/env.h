#pragma once
#include "../../../../utility/cache/cache.h"
#include "../../../../utility/cache/math.h"
#include "../../../../menu/overlay/settings.h"
namespace thenwefuckin::world::env
{
    using math::vec3;
    struct lighting_backup
    {
        uintptr_t address = 0;
        float clock_time = 14.0f;
        uint32_t source = 0;
        vec3 light_direction{};
        vec3 light_color{};
        vec3 sun_position{};
        vec3 moon_position{};
        vec3 gradient_top{};
        vec3 gradient_bottom{};
        vec3 ambient{};
        vec3 outdoor_ambient{};
        vec3 color_shift_top{};
        vec3 color_shift_bottom{};
        float fog_start = 0.0f;
        float fog_end = 100000.0f;
        vec3 fog_color{};
            float exposure = 0.0f;
            float brightness = 2.0f;
            uint8_t shadows = 1;
            float shadow_soft = 0.2f;
            bool captured = false;
        };
        struct terrain_backup
        {
            uintptr_t address = 0;
            float grass = 0.0f;
            bool captured = false;
        };
    inline uintptr_t s_lightingAddress = 0;
    inline lighting_backup s_backup{};
    inline uintptr_t s_terrainAddress = 0;
    inline terrain_backup s_terrainBackup{};
    inline bool s_clockActive = false;
    inline bool s_brightnessActive = false;
    inline bool s_ambientActive = false;
    inline bool s_fogActive = false;
    inline bool s_exposureActive = false;
    inline bool s_fullbrightActive = false;
    inline bool s_shadowActive = false;
    inline bool s_grassActive = false;
    inline bool s_nightActive = false;
    inline uintptr_t s_cachedRenderView = 0;
    bool keyed(bool en, int key, int mode, const char* id);
    vec3 sky_ambient(float t);
    vec3 sky_ambient2(float t);
    vec3 light_color_for_time(float t);
    void apply_clock_time(uintptr_t addr, float hours);
    lighting_backup capture_lighting(uintptr_t addr);
    terrain_backup capture_terrain(uintptr_t addr);
    void restore_clock_params(const lighting_backup& b);
    void restore_ambient(const lighting_backup& b);
    void force_lighting_dirty();
    bool resolve_lighting(bool any_active);
    bool resolve_terrain();
    void reset_all();
}
