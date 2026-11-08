#include "avatars.h"
#include <Windows.h>
#include <wininet.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <cstdio>
#pragma comment(lib, "wininet.lib")
namespace Menu
{
    ID3D11ShaderResourceView* LoadIconTexture(const unsigned char* png, int len);
}
namespace thenwefuckin::visual::players::avatars
{
    namespace
    {
        std::mutex g_mtx;
        std::condition_variable g_cv;
        std::unordered_map<long long, ID3D11ShaderResourceView*> g_cache;
        std::deque<long long> g_order;
        std::unordered_set<long long> g_pending;
        std::unordered_map<long long, long long> g_failedAt;
        bool g_workerStarted = false;
        constexpr size_t kMaxCache = 128;
        constexpr long long kRetryMs = 30000;
        long long now_ms()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }
        bool http_get(const std::string& url, std::string& out)
        {
            out.clear();
            HINTERNET s = InternetOpenA("thenwefuckin", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
            if (!s)
                return false;
            DWORD to = 8000;
            InternetSetOptionA(s, INTERNET_OPTION_CONNECT_TIMEOUT, &to, sizeof(to));
            InternetSetOptionA(s, INTERNET_OPTION_SEND_TIMEOUT, &to, sizeof(to));
            InternetSetOptionA(s, INTERNET_OPTION_RECEIVE_TIMEOUT, &to, sizeof(to));
            HINTERNET h = InternetOpenUrlA(s, url.c_str(), NULL, 0,
                INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE |
                INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0);
            if (!h)
            {
                InternetCloseHandle(s);
                return false;
            }
            char chunk[8192];
            DWORD got = 0;
            bool ok = false;
            while (InternetReadFile(h, chunk, sizeof(chunk), &got) && got > 0)
            {
                out.append(chunk, got);
                ok = true;
                if (out.size() > 4 * 1024 * 1024)
                    break;
            }
            InternetCloseHandle(h);
            InternetCloseHandle(s);
            return ok && !out.empty();
        }
        void parse_pairs(const std::string& json, std::vector<std::pair<long long, std::string>>& out)
        {
            size_t i = 0;
            while (i < json.size())
            {
                size_t t = json.find("\"targetId\"", i);
                if (t == std::string::npos)
                    break;
                t = json.find(':', t);
                if (t == std::string::npos)
                    break;
                ++t;
                while (t < json.size() && (json[t] == ' ' || json[t] == '\t'))
                    ++t;
                long long id = 0;
                bool neg = false;
                if (t < json.size() && json[t] == '-')
                {
                    neg = true;
                    ++t;
                }
                bool any = false;
                while (t < json.size() && json[t] >= '0' && json[t] <= '9')
                {
                    id = id * 10 + (json[t] - '0');
                    ++t;
                    any = true;
                }
                if (!any)
                {
                    i = t;
                    continue;
                }
                if (neg)
                    id = -id;
                size_t u = json.find("\"imageUrl\"", t);
                if (u == std::string::npos)
                    break;
                u = json.find('"', u + 11);
                if (u == std::string::npos)
                    break;
                ++u;
                std::string url;
                while (u < json.size() && json[u] != '"')
                {
                    if (json[u] == '\\' && u + 1 < json.size())
                    {
                        ++u;
                        url += json[u];
                        ++u;
                        continue;
                    }
                    url += json[u];
                    ++u;
                }
                if (!url.empty())
                    out.emplace_back(id, url);
                i = u;
            }
        }
        void store_texture(long long userId, const std::string& png)
        {
            if (png.empty() || userId <= 0)
                return;
            ID3D11ShaderResourceView* srv = Menu::LoadIconTexture(
                reinterpret_cast<const unsigned char*>(png.data()), (int)png.size());
            if (!srv)
                return;
            std::lock_guard<std::mutex> lk(g_mtx);
            auto it = g_cache.find(userId);
            if (it != g_cache.end())
            {
                if (it->second)
                    it->second->Release();
                it->second = srv;
                return;
            }
            if (g_cache.size() >= kMaxCache && !g_order.empty())
            {
                long long old = g_order.front();
                g_order.pop_front();
                auto oit = g_cache.find(old);
                if (oit != g_cache.end())
                {
                    if (oit->second)
                        oit->second->Release();
                    g_cache.erase(oit);
                }
            }
            g_cache[userId] = srv;
            g_order.push_back(userId);
        }
        void fetch_batch(const std::vector<long long>& ids)
        {
            if (ids.empty())
                return;
            std::string url = "https://thumbnails.roblox.com/v1/users/avatar-headshot?userIds=";
            for (size_t i = 0; i < ids.size(); ++i)
            {
                if (i)
                    url += ",";
                char b[32];
                snprintf(b, sizeof(b), "%lld", ids[i]);
                url += b;
            }
            url += "&size=48x48&format=Png&isCircular=false";
            std::string json;
            if (!http_get(url, json))
                return;
            std::vector<std::pair<long long, std::string>> pairs;
            parse_pairs(json, pairs);
            for (auto& pr : pairs)
            {
                std::string png;
                if (http_get(pr.second, png))
                    store_texture(pr.first, png);
                else
                {
                    std::lock_guard<std::mutex> lk(g_mtx);
                    g_failedAt[pr.first] = now_ms();
                }
            }
        }
        void worker()
        {
            for (;;)
            {
                std::vector<long long> batch;
                {
                    std::unique_lock<std::mutex> lk(g_mtx);
                    g_cv.wait_for(lk, std::chrono::milliseconds(100), [] {
                        return !g_pending.empty();
                    });
                    for (auto it = g_pending.begin(); it != g_pending.end() && batch.size() < 10;)
                    {
                        long long id = *it;
                        it = g_pending.erase(it);
                        if (id <= 0)
                            continue;
                        if (g_cache.find(id) != g_cache.end())
                            continue;
                        auto fit = g_failedAt.find(id);
                        if (fit != g_failedAt.end() && now_ms() - fit->second < kRetryMs)
                            continue;
                        batch.push_back(id);
                    }
                }
                if (!batch.empty())
                    fetch_batch(batch);
            }
        }
        void ensure_worker()
        {
            std::lock_guard<std::mutex> lk(g_mtx);
            if (g_workerStarted)
                return;
            g_workerStarted = true;
            for (int i = 0; i < 6; ++i)
                std::thread(worker).detach();
        }
    }
    ID3D11ShaderResourceView* avatar_for(long long userId)
    {
        if (userId <= 0)
            return nullptr;
        ensure_worker();
        std::lock_guard<std::mutex> lk(g_mtx);
        auto it = g_cache.find(userId);
        if (it != g_cache.end())
            return it->second;
        if (g_pending.insert(userId).second)
            g_cv.notify_one();
        return nullptr;
    }
    void clear()
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        for (auto& kv : g_cache)
        {
            if (kv.second)
                kv.second->Release();
        }
        g_cache.clear();
        g_order.clear();
        g_pending.clear();
        g_failedAt.clear();
    }
}
