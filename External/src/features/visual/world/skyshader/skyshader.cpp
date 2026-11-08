#include "skyshader.h"

#include "../env/env.h"
#include "../../../../menu/overlay/settings.h"
#include "../../../../utility/cache/cache.h"
#include "../../../../utility/struct/offsets.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace thenwefuckin::world::skyshader
{
    namespace
    {
        struct style
        {
            const char* name;
            const char* bk;
            const char* dn;
            const char* ft;
            const char* lf;
            const char* rt;
            const char* up;
            float spin;
            float sun;
            float moon;
        };

        const style k_styles[] = {
            { "Off",              "", "", "", "", "", "", 0.0f, 0.0f, 0.0f },

            { "Aurora",           "http://www.roblox.com/asset/?id=16262356578", "http://www.roblox.com/asset/?id=16262358026", "http://www.roblox.com/asset/?id=16262360469", "http://www.roblox.com/asset/?id=16262362003", "http://www.roblox.com/asset/?id=16262363873", "http://www.roblox.com/asset/?id=16262366016", 3.0f, 0.0f, 0.0f },
            { "Nebula",           "rbxassetid://95020137072033", "rbxassetid://92862258103959", "rbxassetid://107665368823185", "rbxassetid://126542804346203", "rbxassetid://103716549795832", "rbxassetid://131036626982613", 1.5f, 0.0f, 0.0f },
            { "Galaxy",           "http://www.roblox.com/asset/?id=168387023", "http://www.roblox.com/asset/?id=168387089", "http://www.roblox.com/asset/?id=168387054", "http://www.roblox.com/asset/?id=168534432", "http://www.roblox.com/asset/?id=168387190", "http://www.roblox.com/asset/?id=168387135", 2.0f, 0.0f, 0.0f },
            { "Synthwave",        "rbxassetid://18164881924", "rbxassetid://18166113875", "rbxassetid://18164870251", "rbxassetid://18164877945", "rbxassetid://18164873920", "rbxassetid://18164890128", 1.0f, 9.0f, 0.0f },
            { "Sunset",           "http://www.roblox.com/asset/?id=151165214", "http://www.roblox.com/asset/?id=151165197", "http://www.roblox.com/asset/?id=151165224", "http://www.roblox.com/asset/?id=151165191", "http://www.roblox.com/asset/?id=151165206", "http://www.roblox.com/asset/?id=151165227", 0.6f, 7.0f, 0.0f },
            { "Tempest",          "http://www.roblox.com/asset/?id=4495864450", "http://www.roblox.com/asset/?id=4495864887", "http://www.roblox.com/asset/?id=4495865458", "http://www.roblox.com/asset/?id=4495866035", "http://www.roblox.com/asset/?id=4495866584", "http://www.roblox.com/asset/?id=4495867486", 4.0f, 0.0f, 0.0f },
            { "Inferno",          "http://www.roblox.com/asset/?id=4776124334", "http://www.roblox.com/asset/?id=4776125375", "http://www.roblox.com/asset/?id=4776131365", "http://www.roblox.com/asset/?id=4776128425", "http://www.roblox.com/asset/?id=4776133150", "http://www.roblox.com/asset/?id=4776130793", 2.5f, 12.0f, 0.0f },
            { "Frost",            "http://www.roblox.com/asset/?id=155657655", "http://www.roblox.com/asset/?id=155674246", "http://www.roblox.com/asset/?id=155657609", "http://www.roblox.com/asset/?id=155657671", "http://www.roblox.com/asset/?id=155657619", "http://www.roblox.com/asset/?id=155674931", 1.2f, 3.0f, 4.0f },
            { "Void",             "rbxassetid://15470149279", "rbxassetid://15470151245", "rbxassetid://15470153860", "rbxassetid://15470155938", "rbxassetid://15470158022", "rbxassetid://15470160563", 0.3f, 0.5f, 0.5f },
            { "Candy",            "rbxassetid://103994796436499", "rbxassetid://88135141884296", "rbxassetid://71705651078185", "rbxassetid://83560072752341", "rbxassetid://96879039628172", "rbxassetid://131043401069407", 1.8f, 4.0f, 3.0f },
        };
        constexpr int k_style_count = (int)(sizeof(k_styles) / sizeof(k_styles[0]));

        struct backup_t
        {
            uintptr_t sky = 0;
            std::string bk, dn, ft, lf, rt, up;
            float spin = 0.0f;
            float sun = 0.0f;
            float moon = 0.0f;
            bool captured = false;
        } s_backup{};

        uintptr_t s_sky = 0;
        int s_style = -1;
        float s_phase = 0.0f;
        float s_elapsed = 0.0f;
        int s_beats = 0;
        std::chrono::steady_clock::time_point s_last{};

        uintptr_t find_lighting()
        {
            if (!g_proc || !g_proc->connected() || !g_ctx.datamodel)
                return 0;
            node l = g_ctx.datamodel.find_child_of_class("Lighting");
            if (!l)
                l = g_ctx.datamodel.find_child("Lighting");
            return l ? l.addr : 0;
        }

        uintptr_t find_sky(uintptr_t lighting)
        {
            if (!lighting)
                return 0;
            const uintptr_t sky = g_proc->read<uintptr_t>(lighting + Offsets::Lighting::Sky);
            if (sky >= 0x10000 && node{ sky }.class_name() == "Sky")
                return sky;
            node c = node{ lighting }.find_child_of_class("Sky");
            return c ? c.addr : 0;
        }

        void capture(uintptr_t sky)
        {
            s_backup = {};
            s_backup.sky = sky;
            s_backup.bk = g_proc->read_game_string(sky + Offsets::Sky::SkyboxBk);
            s_backup.dn = g_proc->read_game_string(sky + Offsets::Sky::SkyboxDn);
            s_backup.ft = g_proc->read_game_string(sky + Offsets::Sky::SkyboxFt);
            s_backup.lf = g_proc->read_game_string(sky + Offsets::Sky::SkyboxLf);
            s_backup.rt = g_proc->read_game_string(sky + Offsets::Sky::SkyboxRt);
            s_backup.up = g_proc->read_game_string(sky + Offsets::Sky::SkyboxUp);
            s_backup.spin = g_proc->read<float>(sky + Offsets::Sky::SkyboxOrientation);
            s_backup.sun = g_proc->read<float>(sky + Offsets::Sky::SunAngularSize);
            s_backup.moon = g_proc->read<float>(sky + Offsets::Sky::MoonAngularSize);
            s_backup.captured = true;
        }

        void write_face(uintptr_t field, const std::string& url)
        {
            if (!field || url.empty())
                return;
            g_proc->write_game_string(field, url);
        }

        void apply(const style& s)
        {
            write_face(s_sky + Offsets::Sky::SkyboxBk, s.bk);
            write_face(s_sky + Offsets::Sky::SkyboxDn, s.dn);
            write_face(s_sky + Offsets::Sky::SkyboxFt, s.ft);
            write_face(s_sky + Offsets::Sky::SkyboxLf, s.lf);
            write_face(s_sky + Offsets::Sky::SkyboxRt, s.rt);
            write_face(s_sky + Offsets::Sky::SkyboxUp, s.up);
            if (s.sun > 0.0f)
                g_proc->write<float>(s_sky + Offsets::Sky::SunAngularSize, s.sun);
            if (s.moon > 0.0f)
                g_proc->write<float>(s_sky + Offsets::Sky::MoonAngularSize, s.moon);
            s_phase = 0.0f;
            env::force_lighting_dirty();
        }

        void restore()
        {
            if (!s_backup.captured || !s_backup.sky)
                return;
            write_face(s_backup.sky + Offsets::Sky::SkyboxBk, s_backup.bk);
            write_face(s_backup.sky + Offsets::Sky::SkyboxDn, s_backup.dn);
            write_face(s_backup.sky + Offsets::Sky::SkyboxFt, s_backup.ft);
            write_face(s_backup.sky + Offsets::Sky::SkyboxLf, s_backup.lf);
            write_face(s_backup.sky + Offsets::Sky::SkyboxRt, s_backup.rt);
            write_face(s_backup.sky + Offsets::Sky::SkyboxUp, s_backup.up);
            g_proc->write<float>(s_backup.sky + Offsets::Sky::SkyboxOrientation, s_backup.spin);
            g_proc->write<float>(s_backup.sky + Offsets::Sky::SunAngularSize, s_backup.sun);
            g_proc->write<float>(s_backup.sky + Offsets::Sky::MoonAngularSize, s_backup.moon);
            env::force_lighting_dirty();
        }

        float clampf(float v, float lo, float hi)
        {
            return v < lo ? lo : (v > hi ? hi : v);
        }
    }

    int style_count()
    {
        return k_style_count;
    }

    const char* const* style_names()
    {
        static std::vector<const char*> names;
        if (names.empty())
        {
            names.reserve((size_t)k_style_count);
            for (int i = 0; i < k_style_count; ++i)
                names.push_back(k_styles[i].name);
        }
        return names.data();
    }

    void tick(bool on)
    {
        if (!g_proc || !g_proc->connected())
            return;

        const auto now = std::chrono::steady_clock::now();
        float dt = 0.0f;
        if (s_last.time_since_epoch().count() != 0)
            dt = std::chrono::duration<float>(now - s_last).count();
        s_last = now;
        if (dt < 0.0f || dt > 0.5f)
            dt = 0.033f;
        s_elapsed += dt;

        if (!on)
        {
            if (s_style > 0)
                restore();
            s_style = -1;
            s_sky = 0;
            s_phase = 0.0f;
            return;
        }

        const uintptr_t lighting = find_lighting();
        if (!lighting)
            return;
        const uintptr_t sky = find_sky(lighting);
        if (!sky)
            return;
        if (sky != s_sky)
        {
            s_sky = sky;
            capture(sky);
            s_style = -1;
        }

        int sel = settings::world_skyshader_style;
        if (sel < 0 || sel >= k_style_count)
            sel = 0;
        if (sel != s_style)
        {
            if (sel == 0)
                restore();
            else
                apply(k_styles[sel]);
            s_style = sel;
        }
        if (sel == 0)
            return;

        const style& st = k_styles[sel];
        s_phase += dt * st.spin;
        if (s_phase > 360.0f)
            s_phase -= 360.0f;
        g_proc->write<float>(s_sky + Offsets::Sky::SkyboxOrientation,
            s_phase > 180.0f ? s_phase - 360.0f : s_phase);
        if (st.sun > 0.0f)
            g_proc->write<float>(s_sky + Offsets::Sky::SunAngularSize,
                clampf(st.sun * (1.0f + 0.10f * std::sin(s_elapsed * 0.8f)), 0.1f, 60.0f));
        if (++s_beats >= 8)
        {
            s_beats = 0;
            env::force_lighting_dirty();
        }
    }

    void reset()
    {
        s_style = -1;
        s_sky = 0;
        s_phase = 0.0f;
        s_elapsed = 0.0f;
        s_beats = 0;
        s_last = {};
        s_backup = {};
    }
}
