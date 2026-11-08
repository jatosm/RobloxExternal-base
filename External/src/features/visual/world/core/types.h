#pragma once
#include "../../../../utility/cache/cache.h"
#include "../../../../utility/cache/math.h"
#include "../../../../menu/overlay/settings.h"
#include <vector>
#include <string>
#include <queue>
#include <chrono>
namespace thenwefuckin::world
{
    struct Entry
    {
        std::string name;
        math::vec3 pos{};
        math::vec3 headPos{};
        math::vec3 rootPos{};
        float dist = 0.0f;
        int idx = -1;
        bool plant = false;
        bool bots = false;
        bool animal = false;
        bool tool = false;
        bool crate = false;
        bool keycard = false;
        float hp = 0.0f;
        float maxHp = 0.0f;
        bool hpValid = false;
        std::uintptr_t humanoid = 0;
        std::uintptr_t inst = 0;
        std::uintptr_t headPart = 0;
        std::uintptr_t rootPart = 0;
        std::uintptr_t headPrim = 0;
        std::uintptr_t rootPrim = 0;
        std::uintptr_t torsoPrim = 0;
        std::uintptr_t hpValue = 0;
        std::uintptr_t maxHpValue = 0;
        bool hpValueIsInt = false;
        bool maxHpValueIsInt = false;
        std::uintptr_t armLPrim = 0;
        std::uintptr_t armRPrim = 0;
        std::uintptr_t legLPrim = 0;
        std::uintptr_t legRPrim = 0;
        bool npcR6 = true;
        int zeroStreak = 0;
    };
    inline std::vector<Entry> g_entries;
    inline std::chrono::steady_clock::time_point g_lastScan{};
    inline std::chrono::steady_clock::time_point g_lastFast{};
    inline std::queue<std::uintptr_t> g_scanQ;
    inline std::vector<Entry> g_scanOut;
    inline bool g_scanning = false;
    inline std::uintptr_t g_scanWs = 0;
    inline math::vec3 g_scanLp{};
    inline std::size_t g_scanned = 0;
    inline std::uintptr_t g_scanLocalChar = 0;
    inline constexpr std::size_t kSliceNodes = 150;
    constexpr std::size_t kMaxScan = 12000;
    constexpr std::size_t kMaxQueue = 8000;
    constexpr float kMaxDist = 20000.0f;
    inline const char* kOreNames[3] = { "Stone_Node", "Phosphate Ore", "Metal_Node" };
    inline const char* kOreDisplay[3] = { "Stone", "Phosphate", "Metal" };
    inline const char* kPlantNames[7] = { "Wool Plant", "Blueberry Plant", "Raspberry Plant", "Lemon Plant", "Corn Plant", "Pumpkin Plant", "Tomato Plant" };
    inline const char* kBotsNames[4] = { "MilitaryHitbox", "ZombieHitbox", "BTR", "Heli" };
    inline const char* kBotsDisplay[4] = { "Military", "Zombie", "BTR", "Heli" };
    inline const char* kAnimalNames[3] = { "PREFAB_ANIMAL_DEER", "PREFAB_ANIMAL_WILDBOAR", "PREFAB_ANIMAL_WOLF" };
    inline const char* kAnimalDisplay[3] = { "Deer", "WildBoar", "Wolf" };
    inline const char* kToolNames[7] = { "Base Cabinet", "Small Storage Box", "Large Storage Box", "Anvil", "Furnace", "Storage Cabinet", "Sleeping Bag" };
    inline const char* kCrateNames[8] = { "Care Package", "Food Crate", "Wooden Crate", "Locked Wooden Crate", "Locked Steel Crate", "Locked Metal Crate", "Timed Crate", "Brutus Locker" };
    inline const char* kKeycardNames[5] = { "Yellow", "Purple", "Red", "Pink", "Black" };
}
