#include "predict.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include <cmath>
#include <algorithm>
#include <Windows.h>
namespace thenwefuckin::aim::predict
{
    const std::unordered_map<std::string, FallenWeaponBallistics> FALLEN_WEAPONS = {
        {"Military M4A1",        {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Bruno's M4A1",         {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged AK47",        {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged AK74U",       {1800.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged AK74u",       {1800.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Military PKM",         {2400.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Military MP7",         {1900.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged SMG",         {1800.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged Skorpion",    {1600.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Military Barrett",     {2500.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Military M39",         {2400.f, 0.52f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged Sniper",      {2400.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged M14",         {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged Pipe Rifle",  {1700.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Military USP",         {1500.f, 0.60f, 17.1f, 10.4f, 8.4f}},
        {"Salvaged Python",      {1800.f, 0.60f, 17.1f, 10.4f, 8.4f}},
        {"Salvaged P250",        {1400.f, 0.60f, 17.1f, 10.4f, 8.4f}},
        {"Military AA12",        { 600.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged Pump Action", { 650.f, 0.60f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged Break Action",{ 550.f, 0.60f, 17.1f, 10.4f, 8.4f}},
        {"Salvaged Shotgun",     { 400.f, 0.60f, 17.1f, 10.4f, 8.4f}},
        {"Nail Gun",             { 165.f, 0.18f, 18.0f, 11.0f, 8.8f}},
        {"Crossbow",             { 420.f, 0.17f, 17.1f, 10.4f, 8.4f}},
        {"Wooden Bow",           { 280.f, 0.17f, 18.0f, 11.0f, 8.8f}},
        {"Salvaged RPG",               {100.f, 0.12f, 16.2f, 9.9f, 7.9f}},
        {"Pumpkin Launcher",           { 80.f, 0.16f, 16.2f, 9.9f, 7.9f}},
        {"Military Grenade Launcher",  { 85.f, 0.15f, 16.2f, 9.9f, 7.9f}},
        {"Salvaged Grenade Launcher",  { 85.f, 0.15f, 16.2f, 9.9f, 7.9f}},
    };
    const std::vector<std::string> FALLEN_WEAPON_LIST = {
        "Auto Detect",
        "Military M4A1",
        "Bruno's M4A1",
        "Salvaged AK47",
        "Salvaged AK74U",
        "Salvaged AK74u",
        "Military PKM",
        "Military MP7",
        "Salvaged SMG",
        "Salvaged Skorpion",
        "Military Barrett",
        "Military M39",
        "Salvaged Sniper",
        "Salvaged M14",
        "Salvaged Pipe Rifle",
        "Military USP",
        "Salvaged Python",
        "Salvaged P250",
        "Military AA12",
        "Salvaged Pump Action",
        "Salvaged Break Action",
        "Salvaged Shotgun",
        "Nail Gun",
        "Crossbow",
        "Wooden Bow",
        "Salvaged RPG",
        "Pumpkin Launcher",
        "Military Grenade Launcher",
        "Salvaged Grenade Launcher",
        "Custom / Manual"
    };
    const char* const FALLEN_WEAPON_ITEMS[] = {
        "Auto Detect",
        "Military M4A1",
        "Bruno's M4A1",
        "Salvaged AK47",
        "Salvaged AK74U",
        "Salvaged AK74u",
        "Military PKM",
        "Military MP7",
        "Salvaged SMG",
        "Salvaged Skorpion",
        "Military Barrett",
        "Military M39",
        "Salvaged Sniper",
        "Salvaged M14",
        "Salvaged Pipe Rifle",
        "Military USP",
        "Salvaged Python",
        "Salvaged P250",
        "Military AA12",
        "Salvaged Pump Action",
        "Salvaged Break Action",
        "Salvaged Shotgun",
        "Nail Gun",
        "Crossbow",
        "Wooden Bow",
        "Salvaged RPG",
        "Pumpkin Launcher",
        "Military Grenade Launcher",
        "Salvaged Grenade Launcher",
        "Custom / Manual"
    };
    std::string detected_weapon_name = "None";
    std::string fallen_cached_wpn;
    float fallen_cached_base_bv = 0.0f;
    unsigned long fallen_last_wpn_check = 0;
    float fallen_get_base_bv(const std::string& weapon)
    {
        auto it = FALLEN_WEAPONS.find(weapon);
        return (it != FALLEN_WEAPONS.end()) ? it->second.bv : 0.f;
    }
    float fallen_get_base_grav(const std::string& weapon)
    {
        auto it = FALLEN_WEAPONS.find(weapon);
        return (it != FALLEN_WEAPONS.end()) ? it->second.grav : 0.f;
    }
    const FallenWeaponBallistics* fallen_get_weapon_data(const std::string& weapon)
    {
        auto it = FALLEN_WEAPONS.find(weapon);
        return (it != FALLEN_WEAPONS.end()) ? &it->second : nullptr;
    }
    float fallen_get_bullet_velocity(const std::string& weapon)
    {
        if (settings::aim_fallen_bv > 0.f)
            return settings::aim_fallen_bv;
        return fallen_get_base_bv(weapon);
    }
    std::string fallen_detect_weapon(uintptr_t character)
    {
        if (!character || !g_proc || !g_proc->connected())
            return "";
        node ch{ character };
        for (auto& child : ch.children())
        {
            if (!child)
                continue;
            if (child.class_name() != "Model")
                continue;
            node handle = child.find_child("Handle");
            if (!handle)
                handle = child.find_child("Main");
            if (!handle)
                continue;
            std::string name = child.name();
            if (name == "HolsterModel")
                continue;
            return name;
        }
        return "";
    }
    std::string fallen_get_local_weapon()
    {
        if (!g_ctx.local_player)
            return "";
        node ch = g_ctx.get_character(g_ctx.local_player);
        if (!ch)
            return "";
        return fallen_detect_weapon(ch.addr);
    }
    void fallen_update_weapon_auto()
    {
        unsigned long now = GetTickCount();
        if (now - fallen_last_wpn_check < 500)
            return;
        fallen_last_wpn_check = now;
        if (settings::aim_weapon == 0)
        {
            std::string new_wpn = fallen_get_local_weapon();
            if (new_wpn.empty())
                return;
            float new_base = fallen_get_base_bv(new_wpn);
            if (new_wpn != fallen_cached_wpn)
            {
                settings::aim_fallen_bv = new_base;
                settings::aim_fallen_grav = fallen_get_base_grav(new_wpn);
                detected_weapon_name = new_wpn;
            }
            fallen_cached_wpn = new_wpn;
            fallen_cached_base_bv = new_base;
        }
        else if (settings::aim_weapon > 0 && settings::aim_weapon < FALLEN_WEAPON_COUNT - 1)
        {
            static int lastSel = -1;
            if (settings::aim_weapon != lastSel)
            {
                lastSel = settings::aim_weapon;
                std::string selected_wpn = FALLEN_WEAPON_LIST[(size_t)settings::aim_weapon];
                settings::aim_fallen_bv = fallen_get_base_bv(selected_wpn);
                settings::aim_fallen_grav = fallen_get_base_grav(selected_wpn);
                detected_weapon_name = selected_wpn;
            }
        }
        else
        {
            detected_weapon_name = "Custom / Manual";
        }
    }
    void fallen_predict(math::vec3& target_pos, const math::vec3& cam_pos,
        const math::vec3& target_vel, float bullet_vel,
        const math::vec3& local_vel, float grav_mult)
    {
        if (bullet_vel <= 0.f)
            return;
        float gm = (grav_mult >= 0.f) ? grav_mult : settings::aim_fallen_grav;
        float dist = (target_pos - cam_pos).length();
        if (dist < 1.f)
            return;
        float ox = target_pos.x, oy = target_pos.y, oz = target_pos.z;
        for (int i = 0; i < 3; i++)
        {
            float dx = target_pos.x - cam_pos.x;
            float dy = target_pos.y - cam_pos.y;
            float dz = target_pos.z - cam_pos.z;
            float inv_dist = (dist > 0.1f) ? 1.f / dist : 0.f;
            float dir_x = dx * inv_dist, dir_y = dy * inv_dist, dir_z = dz * inv_dist;
            float vel_proj = local_vel.x * dir_x + local_vel.y * dir_y + local_vel.z * dir_z;
            float eff_bv = bullet_vel + vel_proj;
            if (eff_bv < 10.f)
                eff_bv = 10.f;
            float ft = dist / eff_bv;
            float px = ox + target_vel.x * ft;
            float pz = oz + target_vel.z * ft;
            float py = oy + target_vel.y * ft + 0.5f * FALLEN_GRAVITY * gm * ft * ft;
            dx = px - cam_pos.x;
            dy = py - cam_pos.y;
            dz = pz - cam_pos.z;
            dist = sqrtf(dx * dx + dy * dy + dz * dz);
            target_pos.x = px;
            target_pos.y = py;
            target_pos.z = pz;
        }
    }
}
