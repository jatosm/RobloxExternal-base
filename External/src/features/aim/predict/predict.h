#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "../../../utility/cache/math.h"
namespace thenwefuckin::aim::predict
{
    struct FallenWeaponBallistics
    {
        float bv;
        float grav;
        float spd_sprint;
        float spd_walk;
        float spd_ads;
    };
    extern const std::unordered_map<std::string, FallenWeaponBallistics> FALLEN_WEAPONS;
    extern const std::vector<std::string> FALLEN_WEAPON_LIST;
    extern const char* const FALLEN_WEAPON_ITEMS[];
    constexpr int FALLEN_WEAPON_COUNT = 30;
    constexpr float FALLEN_GRAVITY = 196.2f;
    extern std::string detected_weapon_name;
    float fallen_get_base_bv(const std::string& weapon);
    float fallen_get_base_grav(const std::string& weapon);
    const FallenWeaponBallistics* fallen_get_weapon_data(const std::string& weapon);
    float fallen_get_bullet_velocity(const std::string& weapon);
    std::string fallen_detect_weapon(uintptr_t character);
    std::string fallen_get_local_weapon();
    void fallen_update_weapon_auto();
    void fallen_predict(math::vec3& target_pos, const math::vec3& cam_pos,
        const math::vec3& target_vel, float bullet_vel,
        const math::vec3& local_vel = {}, float grav_mult = -1.0f);
}
