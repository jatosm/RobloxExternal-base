#include "env.h"
#include "../../../../utility/struct/offsets.h"
#include "../../../../menu/core/gui/menu.h"
#include <cmath>
namespace thenwefuckin::world::env
{
    namespace
    {
        constexpr float k_pi = 3.1415927f;
        constexpr float k_hour = 3600.0f;
        constexpr float k_day = 86400.0f;
        constexpr float k_sunrise = 6.0f * k_hour;
        constexpr float k_sunset = 18.0f * k_hour;
        constexpr float k_rise_set = k_hour;
        constexpr float k_solar_year = 365.2564f * k_day;
        constexpr float k_half_solar = 182.6282f;
        inline float deg_to_rad(float deg)
        {
            return deg * (k_pi / 180.0f);
        }
        inline vec3 vcross(const vec3& a, const vec3& b)
        {
            return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
        }
        inline vec3 spline(float t, const float* times, const vec3* colors, int count)
        {
            if (count <= 0)
                return { 1.0f, 1.0f, 1.0f };
            if (t <= times[0])
                return colors[0];
            if (t >= times[count - 1])
                return colors[count - 1];
            for (int i = 0; i < count - 1; i++)
            {
                if (t < times[i] || t > times[i + 1])
                    continue;
                float span = times[i + 1] - times[i];
                float a = span > 0.0f ? (t - times[i]) / span : 0.0f;
                return (colors[i] * (1.0f - a)) + (colors[i + 1] * a);
            }
            return colors[0];
        }
        inline vec3 rotate_axis_angle(const vec3& v, const vec3& axis, float angle)
        {
            float cos_a = std::cos(angle);
            float sin_a = std::sin(angle);
            return (v * cos_a) + (vcross(axis, v) * sin_a) + (axis * (axis.dot(v) * (1.0f - cos_a)));
        }
        inline vec3 true_sun_position(float source_angle, float abs_seconds, float latitude_deg)
        {
            vec3 sun{ std::sin(source_angle), -std::cos(source_angle), 0.0f };
            float day_of_year = (abs_seconds - std::floor(abs_seconds / k_solar_year) * k_solar_year) / k_day;
            float sun_offset = -deg_to_rad(23.5f) * std::cos(k_pi * (day_of_year - k_half_solar) / k_half_solar) - deg_to_rad(latitude_deg);
            vec3 axis = vcross(vec3{ 0.0f, 0.0f, 1.0f }, sun).normalized();
            return rotate_axis_angle(sun, axis, sun_offset);
        }
    }
    bool keyed(bool en, int key, int mode, const char* id)
    {
        if (!en)
            return false;
        if (key <= 0)
            return true;
        return Menu::IsMenuKeyActive(id, key, mode);
    }
    vec3 sky_ambient(float t)
    {
        static const float times[] = {
            0.0f, k_sunrise - 2.0f * k_hour, k_sunrise - k_hour, k_sunrise - k_hour * 0.5f,
            k_sunrise, k_sunrise + k_rise_set, k_sunset - k_rise_set, k_sunset,
            k_sunset + k_hour / 3.0f, k_day
        };
        static const vec3 colors[] = {
            { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.07f, 0.07f, 0.1f }, { 0.2f, 0.15f, 0.01f },
            { 0.2f, 0.15f, 0.01f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.4f, 0.2f, 0.05f },
            { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
        };
        return spline(t, times, colors, 10);
    }
    vec3 sky_ambient2(float t)
    {
        static const float times[] = {
            0.0f, k_sunrise - 3.0f * k_hour, k_sunrise - 2.0f * k_hour, k_sunrise - k_hour * 0.5f,
            k_sunrise, k_sunrise + k_rise_set, k_sunset - k_rise_set, k_sunset,
            k_sunset + k_hour / 3.0f, k_sunset + 2.0f * k_hour, k_sunset + 3.0f * k_hour, k_day
        };
        static const vec3 colors[] = {
            { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.21f, 0.21f, 0.28f }, { 0.4f, 0.3f, 0.3f },
            { 0.3f, 0.2f, 0.3f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.4f, 0.3f, 0.2f },
            { 0.3f, 0.2f, 0.3f }, { 0.3f, 0.2f, 0.3f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
        };
        return spline(t, times, colors, 12);
    }
    vec3 light_color_for_time(float t)
    {
        const vec3 day{ 0.75f, 0.75f, 0.75f };
        static const float times[] = {
            0.0f, k_sunrise - k_hour, k_sunrise, k_sunrise + k_rise_set * 0.25f,
            k_sunrise + k_rise_set, k_sunset - k_rise_set, k_sunset - k_rise_set * 0.5f,
            k_sunset, k_sunset + k_hour * 0.5f, k_day
        };
        static const vec3 colors[] = {
            { 0.2f, 0.2f, 0.2f }, { 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.6f, 0.6f, 0.0f },
            day, day, { 0.1f, 0.1f, 0.075f }, { 0.1f, 0.05f, 0.05f }, { 0.1f, 0.1f, 0.1f }, { 0.2f, 0.2f, 0.2f }
        };
        return spline(t, times, colors, 10);
    }
    void apply_clock_time(uintptr_t addr, float hours)
    {
        float seconds = hours * k_hour;
        float time_of_day = seconds - std::floor(seconds / k_day) * k_day;
        float source_angle = (time_of_day * 2.0f * k_pi) / k_day;
        float latitude = g_proc->read<float>(addr + Offsets::Lighting::GeographicLatitude);
        vec3 sun = true_sun_position(source_angle, seconds, latitude);
        vec3 moon = sun * -1.0f;
        bool use_sun = sun.y > -0.3f;
        vec3 light_direction = use_sun ? sun : moon;
        uint32_t source = use_sun ? 0u : 1u;
        g_proc->write<float>(addr + Offsets::Lighting::ClockTime, hours);
        g_proc->write<vec3>(addr + Offsets::Lighting::GradientTop, sky_ambient(time_of_day));
        g_proc->write<vec3>(addr + Offsets::Lighting::GradientBottom, sky_ambient2(time_of_day));
        g_proc->write<vec3>(addr + Offsets::Lighting::LightColor, light_color_for_time(time_of_day));
        g_proc->write<vec3>(addr + Offsets::Lighting::SunPosition, sun);
        g_proc->write<vec3>(addr + Offsets::Lighting::MoonPosition, moon);
        if (Offsets::Lighting::LightDirection)
            g_proc->write<vec3>(addr + Offsets::Lighting::LightDirection, light_direction);
        g_proc->write<uint32_t>(addr + Offsets::Lighting::Source, source);
    }
    lighting_backup capture_lighting(uintptr_t addr)
    {
        lighting_backup b{};
        b.address = addr;
        b.clock_time = g_proc->read<float>(addr + Offsets::Lighting::ClockTime);
        b.source = g_proc->read<uint32_t>(addr + Offsets::Lighting::Source);
        b.light_direction = Offsets::Lighting::LightDirection ? g_proc->read<vec3>(addr + Offsets::Lighting::LightDirection) : vec3{};
        b.light_color = g_proc->read<vec3>(addr + Offsets::Lighting::LightColor);
        b.sun_position = g_proc->read<vec3>(addr + Offsets::Lighting::SunPosition);
        b.moon_position = g_proc->read<vec3>(addr + Offsets::Lighting::MoonPosition);
        b.gradient_top = g_proc->read<vec3>(addr + Offsets::Lighting::GradientTop);
        b.gradient_bottom = g_proc->read<vec3>(addr + Offsets::Lighting::GradientBottom);
        b.ambient = g_proc->read<vec3>(addr + Offsets::Lighting::Ambient);
        b.outdoor_ambient = g_proc->read<vec3>(addr + Offsets::Lighting::OutdoorAmbient);
        b.color_shift_top = g_proc->read<vec3>(addr + Offsets::Lighting::ColorShift_Top);
        b.color_shift_bottom = g_proc->read<vec3>(addr + Offsets::Lighting::ColorShift_Bottom);
        b.fog_start = g_proc->read<float>(addr + Offsets::Lighting::FogStart);
        b.fog_end = g_proc->read<float>(addr + Offsets::Lighting::FogEnd);
        b.fog_color = g_proc->read<vec3>(addr + Offsets::Lighting::FogColor);
        b.exposure = g_proc->read<float>(addr + Offsets::Lighting::ExposureCompensation);
            b.brightness = g_proc->read<float>(addr + Offsets::Lighting::Brightness);
            b.shadows = g_proc->read<uint8_t>(addr + Offsets::Lighting::GlobalShadows);
            b.shadow_soft = Offsets::Lighting::ShadowSoftness ? g_proc->read<float>(addr + Offsets::Lighting::ShadowSoftness) : 0.0f;
            b.captured = true;
            return b;
        }
    terrain_backup capture_terrain(uintptr_t addr)
    {
        terrain_backup b{};
        b.address = addr;
            b.grass = g_proc->read<float>(addr + Offsets::Terrain::GrassLength);
            b.captured = true;
            return b;
        }
    void restore_clock_params(const lighting_backup& b)
    {
        if (!b.captured || !b.address)
            return;
        g_proc->write<float>(b.address + Offsets::Lighting::ClockTime, b.clock_time);
        g_proc->write<uint32_t>(b.address + Offsets::Lighting::Source, b.source);
        if (Offsets::Lighting::LightDirection)
            g_proc->write<vec3>(b.address + Offsets::Lighting::LightDirection, b.light_direction);
        g_proc->write<vec3>(b.address + Offsets::Lighting::LightColor, b.light_color);
        g_proc->write<vec3>(b.address + Offsets::Lighting::SunPosition, b.sun_position);
        g_proc->write<vec3>(b.address + Offsets::Lighting::MoonPosition, b.moon_position);
        g_proc->write<vec3>(b.address + Offsets::Lighting::GradientTop, b.gradient_top);
        g_proc->write<vec3>(b.address + Offsets::Lighting::GradientBottom, b.gradient_bottom);
    }
    void restore_ambient(const lighting_backup& b)
    {
        if (!b.captured || !b.address)
            return;
        g_proc->write<vec3>(b.address + Offsets::Lighting::Ambient, b.ambient);
        g_proc->write<vec3>(b.address + Offsets::Lighting::OutdoorAmbient, b.outdoor_ambient);
        g_proc->write<vec3>(b.address + Offsets::Lighting::ColorShift_Top, b.color_shift_top);
        g_proc->write<vec3>(b.address + Offsets::Lighting::ColorShift_Bottom, b.color_shift_bottom);
        g_proc->write<vec3>(b.address + Offsets::Lighting::GradientTop, b.gradient_top);
        g_proc->write<vec3>(b.address + Offsets::Lighting::GradientBottom, b.gradient_bottom);
        g_proc->write<float>(b.address + Offsets::Lighting::Brightness, b.brightness);
    }
    void force_lighting_dirty()
    {
        uintptr_t rv = s_cachedRenderView;
        uintptr_t ve = g_proc->read<uintptr_t>(g_proc->base() + Offsets::VisualEngine::Pointer);
        if (ve < 0x10000 || ve > 0x00007FFFFFFFFFFF)
            return;
        if (rv < 0x10000 || rv > 0x00007FFFFFFFFFFF ||
            g_proc->read<uintptr_t>(rv + Offsets::RenderView::VisualEngine) != ve)
        {
            rv = g_proc->read<uintptr_t>(ve + Offsets::VisualEngine::RenderView);
            if (rv < 0x10000 || rv > 0x00007FFFFFFFFFFF ||
                g_proc->read<uintptr_t>(rv + Offsets::RenderView::VisualEngine) != ve)
            {
                s_cachedRenderView = 0;
                return;
            }
            s_cachedRenderView = rv;
        }
        if (rv < 0x10000 || rv > 0x00007FFFFFFFFFFF)
            return;
        if (Offsets::RenderView::LightingValid)
            g_proc->write<bool>(rv + Offsets::RenderView::LightingValid, false);
        if (Offsets::RenderView::SkyValid)
            g_proc->write<bool>(rv + Offsets::RenderView::SkyValid, false);
    }
    bool resolve_lighting(bool any_active)
    {
        if (!g_proc || !g_proc->connected() || !g_ctx.datamodel)
        {
            reset_all();
            return false;
        }
        if (!s_lightingAddress || g_proc->read<uintptr_t>(s_lightingAddress + Offsets::Instance::Parent) != g_ctx.datamodel.addr)
        {
            reset_all();
            node l = g_ctx.datamodel.find_child_of_class("Lighting");
            if (!l)
                l = g_ctx.datamodel.find_child("Lighting");
            s_lightingAddress = l ? l.addr : 0;
        }
        if (!s_lightingAddress)
            return false;
        if (any_active && (!s_backup.captured || s_backup.address != s_lightingAddress))
            s_backup = capture_lighting(s_lightingAddress);
        return true;
    }
    bool resolve_terrain()
    {
        if (!s_terrainAddress || !g_ctx.workspace ||
            g_proc->read<uintptr_t>(s_terrainAddress + Offsets::Instance::Parent) != g_ctx.workspace.addr)
        {
            s_terrainAddress = 0;
            s_terrainBackup = {};
            s_grassActive = false;
            if (g_ctx.workspace)
            {
                node t = g_ctx.workspace.find_child_of_class("Terrain");
                if (!t)
                    t = g_ctx.workspace.find_child("Terrain");
                s_terrainAddress = t ? t.addr : 0;
            }
        }
        if (!s_terrainAddress)
            return false;
        if (!s_terrainBackup.captured || s_terrainBackup.address != s_terrainAddress)
            s_terrainBackup = capture_terrain(s_terrainAddress);
        return true;
    }
    void reset_all()
    {
        if (s_backup.captured && s_lightingAddress)
        {
            if (s_clockActive || s_nightActive) restore_clock_params(s_backup);
            if (s_ambientActive || s_fullbrightActive || s_nightActive)
                restore_ambient(s_backup);
            if (s_brightnessActive || s_fullbrightActive || s_nightActive)
                g_proc->write<float>(s_lightingAddress + Offsets::Lighting::Brightness, s_backup.brightness);
            if (s_shadowActive)
            {
                g_proc->write<uint8_t>(s_lightingAddress + Offsets::Lighting::GlobalShadows, s_backup.shadows);
                if (Offsets::Lighting::ShadowSoftness)
                    g_proc->write<float>(s_lightingAddress + Offsets::Lighting::ShadowSoftness, s_backup.shadow_soft);
            }
            if (s_fogActive)
            {
                g_proc->write<float>(s_lightingAddress + Offsets::Lighting::FogStart, s_backup.fog_start);
                g_proc->write<float>(s_lightingAddress + Offsets::Lighting::FogEnd, s_backup.fog_end);
                g_proc->write<vec3>(s_lightingAddress + Offsets::Lighting::FogColor, s_backup.fog_color);
            }
            if (s_exposureActive)
            {
                g_proc->write<float>(s_lightingAddress + Offsets::Lighting::ExposureCompensation, s_backup.exposure);
            }
            force_lighting_dirty();
        }
        if (s_terrainBackup.captured && s_terrainAddress)
        {
            if (s_grassActive)
                g_proc->write<float>(s_terrainAddress + Offsets::Terrain::GrassLength, s_terrainBackup.grass);
        }
        s_lightingAddress = 0;
        s_backup = {};
        s_terrainAddress = 0;
        s_terrainBackup = {};
        s_clockActive = false;
        s_brightnessActive = false;
        s_ambientActive = false;
        s_fogActive = false;
        s_exposureActive = false;
        s_fullbrightActive = false;
        s_shadowActive = false;
        s_grassActive = false;
        s_nightActive = false;
        s_cachedRenderView = 0;
    }
}
