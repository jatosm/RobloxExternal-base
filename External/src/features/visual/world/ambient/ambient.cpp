#include "ambient.h"
#include "../env/env.h"
namespace thenwefuckin::world::ambient
{
    void tick(bool on)
    {
        if (on)
        {
            env::vec3 ac{ settings::ambient_color[0], settings::ambient_color[1], settings::ambient_color[2] };
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::Ambient, ac);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::OutdoorAmbient, ac);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::ColorShift_Top, ac);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::ColorShift_Bottom, ac);
            if (!env::s_clockActive)
            {
                g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::GradientTop, ac);
                g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::GradientBottom, ac);
            }
            env::s_ambientActive = true;
        }
        else if (env::s_ambientActive)
        {
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::Ambient, env::s_backup.ambient);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::OutdoorAmbient, env::s_backup.outdoor_ambient);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::ColorShift_Top, env::s_backup.color_shift_top);
            g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::ColorShift_Bottom, env::s_backup.color_shift_bottom);
            if (!env::s_clockActive)
            {
                g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::GradientTop, env::s_backup.gradient_top);
                g_proc->write<env::vec3>(env::s_lightingAddress + Offsets::Lighting::GradientBottom, env::s_backup.gradient_bottom);
            }
            env::s_ambientActive = false;
        }
    }
}
