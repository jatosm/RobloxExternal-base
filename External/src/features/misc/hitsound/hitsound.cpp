#include "hitsound.h"
#include "../../aim/core/aim.h"
#include "../../../utility/cache/cache.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/core/gui/menu.h"
#include <Windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#pragma comment(lib, "winmm.lib")
namespace thenwefuckin::misc::hitsound
{
    static std::vector<std::string> g_sounds;
    static bool g_initialized = false;
    static void create_default_wav(const std::string& path, float freq)
    {
        uint32_t sampleRate = 44100;
        float duration = 0.12f;
        uint32_t numSamples = static_cast<uint32_t>(sampleRate * duration);
        uint32_t dataSize = numSamples * 2;
        uint32_t fileSize = 36 + dataSize;
        std::ofstream f(path, std::ios::binary);
        if (!f.is_open()) return;
        f.write("RIFF", 4);
        f.write(reinterpret_cast<const char*>(&fileSize), 4);
        f.write("WAVE", 4);
        f.write("fmt ", 4);
        uint32_t sub1 = 16;
        f.write(reinterpret_cast<const char*>(&sub1), 4);
        uint16_t audioFormat = 1;
        f.write(reinterpret_cast<const char*>(&audioFormat), 2);
        uint16_t numChannels = 1;
        f.write(reinterpret_cast<const char*>(&numChannels), 2);
        f.write(reinterpret_cast<const char*>(&sampleRate), 4);
        uint32_t byteRate = sampleRate * 2;
        f.write(reinterpret_cast<const char*>(&byteRate), 4);
        uint16_t blockAlign = 2;
        f.write(reinterpret_cast<const char*>(&blockAlign), 2);
        uint16_t bits = 16;
        f.write(reinterpret_cast<const char*>(&bits), 2);
        f.write("data", 4);
        f.write(reinterpret_cast<const char*>(&dataSize), 4);
        for (uint32_t i = 0; i < numSamples; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(sampleRate);
            float env = expf(-t * 30.0f);
            float wave = sinf(2.0f * 3.14159265f * freq * t);
            int16_t sample = static_cast<int16_t>(wave * env * 28000.0f);
            f.write(reinterpret_cast<const char*>(&sample), 2);
        }
    }
    void refresh_sounds()
    {
        g_sounds.clear();
        std::error_code ec;
        if (!std::filesystem::exists("sounds", ec))
        {
            std::filesystem::create_directories("sounds", ec);
        }
        bool hasFiles = false;
        for (const auto& entry : std::filesystem::directory_iterator("sounds", ec))
        {
            if (entry.is_regular_file())
            {
                auto ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".wav")
                {
                    hasFiles = true;
                    break;
                }
            }
        }
        if (!hasFiles)
        {
            create_default_wav("sounds/ding.wav", 1200.0f);
            create_default_wav("sounds/bell.wav", 850.0f);
        }
        for (const auto& entry : std::filesystem::directory_iterator("sounds", ec))
        {
            if (entry.is_regular_file())
            {
                auto ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".wav")
                {
                    g_sounds.push_back(entry.path().filename().string());
                }
            }
        }
        std::sort(g_sounds.begin(), g_sounds.end());
    }
    std::vector<std::string> get_sound_list()
    {
        if (!g_initialized || g_sounds.empty())
        {
            refresh_sounds();
            g_initialized = true;
        }
        return g_sounds;
    }
    void play()
    {
        auto list = get_sound_list();
        if (list.empty())
            return;
        int idx = settings::hitsound_selected;
        if (idx < 0 || idx >= static_cast<int>(list.size()))
            idx = 0;
        std::string fullPath = "sounds/" + list[idx];
        DWORD vol16 = static_cast<DWORD>(std::clamp(settings::hitsound_volume / 100.0f, 0.0f, 1.0f) * 65535.0f);
        DWORD vol = (vol16 & 0xFFFF) | ((vol16 & 0xFFFF) << 16);
        waveOutSetVolume(0, vol);
        PlaySoundA(fullPath.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
    void open_folder()
    {
        std::error_code ec;
        if (!std::filesystem::exists("sounds", ec))
        {
            std::filesystem::create_directories("sounds", ec);
        }
        char full[MAX_PATH]{};
        GetFullPathNameA("sounds", MAX_PATH, full, NULL);
        ShellExecuteA(NULL, "open", full, NULL, NULL, SW_SHOWNORMAL);
    }
    static bool aimed_at(const node& character)
    {
        if (!character || !g_ctx.camera)
            return false;
        node head = character.find_child("Head");
        if (!head)
            return false;
        uintptr_t prim = head.primitive();
        if (!prim)
            return false;
        math::vec3 hp = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
        if (hp.x == 0.0f && hp.y == 0.0f && hp.z == 0.0f)
            return false;
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        if (sw <= 0 || sh <= 0)
            return false;
        math::vec2 sp{};
        if (!projection::to_screen(hp, g_ctx.view, (float)sw, (float)sh, sp))
            return false;
        float dx = sp.x - (float)sw * 0.5f;
        float dy = sp.y - (float)sh * 0.5f;
        float d = sqrtf(dx * dx + dy * dy);
        return d <= settings::aim_fov + 50.0f;
    }
    void run()
    {
        if (!settings::hitsound_enabled)
            return;
        static bool lastShootDown = false;
        bool shootDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (settings::aim_autoshoot_key != 0 && Menu::IsKeybindDown(settings::aim_autoshoot_key))
            shootDown = true;
        bool justShot = shootDown && !lastShootDown;
        lastShootDown = shootDown;
        if (settings::hitsound_mode == 1)
        {
            if (justShot)
                play();
            return;
        }
        if (!g_ctx.ready || !g_proc || !g_proc->connected())
            return;
        {
            static auto lastScan = std::chrono::steady_clock::now() - std::chrono::seconds(1);
            auto now0 = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now0 - lastScan).count() < 10)
                return;
            lastScan = now0;
        }
        static std::unordered_map<uintptr_t, float> prevHp;
        static auto lastShotTime = std::chrono::steady_clock::now();
        if (shootDown)
            lastShotTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        bool shootingRecently = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShotTime).count() < 350;
        auto list = g_ctx.player_list();
        bool damageDetected = false;
        bool aimInUse = thenwefuckin::aim::debug_in_use();
        for (auto& entry : list)
        {
            if (entry.addr == g_ctx.local_player.addr)
            {
                prevHp.erase(entry.addr);
                continue;
            }
            node character{ g_proc->read<uintptr_t>(entry.addr + Offsets::Player::ModelInstance) };
            if (!character)
                continue;
            float hp = 0.0f;
            node humanoid = find_live_humanoid(character, &hp, nullptr);
            if (!humanoid)
                continue;
            auto it = prevHp.find(entry.addr);
            if (it != prevHp.end() && it->second > 0.1f && hp == 0.0f)
            {
                hp = g_proc->read<float>(humanoid.addr + Offsets::Humanoid::Health);
            }
            if (it != prevHp.end())
            {
                if (it->second > 0.1f && hp < it->second - 0.5f)
                {
                    if (shootingRecently)
                    {
                        bool mine = thenwefuckin::aim::debug_recent_player(entry.addr);
                        if (!mine && !aimInUse)
                            mine = aimed_at(character);
                        if (mine)
                            damageDetected = true;
                    }
                }
            }
            prevHp[entry.addr] = hp;
        }
        if (damageDetected)
        {
            play();
        }
        for (auto it = prevHp.begin(); it != prevHp.end();)
        {
            bool found = false;
            for (auto& p : list)
            {
                if (p.addr == it->first)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                it = prevHp.erase(it);
            else
                ++it;
        }
    }
}
