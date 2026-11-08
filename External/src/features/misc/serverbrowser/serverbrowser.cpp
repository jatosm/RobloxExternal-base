#include "serverbrowser.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/global/globals.h"
#include "../../../utility/struct/offsets.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <thread>
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")

namespace thenwefuckin::serverbrowser
{
    namespace
    {
        std::mutex g_mtx;
        state_t g_state;
        std::vector<server_entry> g_all;
        bool g_inflight = false;

        void skip_ws(const std::string& s, size_t& i)
        {
            while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t'))
                ++i;
        }

        bool find_key(const std::string& s, const char* key, size_t& val_pos)
        {
            const std::string needle = std::string("\"") + key + "\"";
            const size_t p = s.find(needle);
            if (p == std::string::npos)
                return false;
            size_t i = p + needle.size();
            skip_ws(s, i);
            if (i >= s.size() || s[i] != ':')
                return false;
            ++i;
            skip_ws(s, i);
            val_pos = i;
            return true;
        }

        bool get_string(const std::string& s, const char* key, std::string& out)
        {
            size_t i = 0;
            if (!find_key(s, key, i) || i >= s.size() || s[i] != '"')
                return false;
            ++i;
            out.clear();
            while (i < s.size() && s[i] != '"')
            {
                if (s[i] == '\\' && i + 1 < s.size())
                    ++i;
                out += s[i++];
            }
            return true;
        }

        bool get_number(const std::string& s, const char* key, double& out)
        {
            size_t i = 0;
            if (!find_key(s, key, i))
                return false;
            const size_t start = i;
            while (i < s.size())
            {
                const char c = s[i];
                if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E'))
                    break;
                ++i;
            }
            if (i == start)
                return false;
            try
            {
                out = std::stod(s.substr(start, i - start));
            }
            catch (...)
            {
                return false;
            }
            return true;
        }

        std::vector<std::string> array_objects(const std::string& s, const char* key)
        {
            std::vector<std::string> out;
            size_t i = 0;
            if (!find_key(s, key, i) || i >= s.size() || s[i] != '[')
                return out;
            ++i;
            int depth = 0;
            bool in_str = false;
            std::string cur;
            for (; i < s.size(); ++i)
            {
                const char c = s[i];
                if (in_str)
                {
                    cur += c;
                    if (c == '\\' && i + 1 < s.size())
                    {
                        cur += s[++i];
                        continue;
                    }
                    if (c == '"')
                        in_str = false;
                    continue;
                }
                if (c == '"')
                {
                    in_str = true;
                    cur += c;
                    continue;
                }
                if (c == '{')
                {
                    ++depth;
                    cur += c;
                    continue;
                }
                if (c == '}')
                {
                    --depth;
                    cur += c;
                    if (depth == 0)
                    {
                        out.push_back(cur);
                        cur.clear();
                    }
                    continue;
                }
                if (depth > 0)
                    cur += c;
            }
            return out;
        }

        std::string http_get(const std::wstring& host, const std::wstring& path)
        {
            std::string out;
            HINTERNET session = WinHttpOpen(
                L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!session)
                return out;
            HINTERNET connect = WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
            if (!connect)
            {
                WinHttpCloseHandle(session);
                return out;
            }
            HINTERNET request = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            if (!request)
            {
                WinHttpCloseHandle(connect);
                WinHttpCloseHandle(session);
                return out;
            }
            DWORD timeout = 10000;
            WinHttpSetOption(request, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
            WinHttpSetOption(request, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
            LPCWSTR headers = L"Accept: application/json\r\n";
            WinHttpAddRequestHeaders(request, headers, static_cast<DWORD>(-1),
                WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
            if (WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                WinHttpReceiveResponse(request, nullptr))
            {
                char buffer[8192];
                DWORD bytes_read = 0;
                while (WinHttpReadData(request, buffer, sizeof(buffer) - 1, &bytes_read) && bytes_read > 0)
                    out.append(buffer, bytes_read);
            }
            WinHttpCloseHandle(request);
            WinHttpCloseHandle(connect);
            WinHttpCloseHandle(session);
            return out;
        }

        void set_clipboard(const std::string& text)
        {
            if (!OpenClipboard(nullptr))
                return;
            EmptyClipboard();
            HGLOBAL glob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
            if (glob)
            {
                void* dst = GlobalLock(glob);
                if (dst)
                    memcpy(dst, text.c_str(), text.size() + 1);
                GlobalUnlock(glob);
                SetClipboardData(CF_TEXT, glob);
            }
            CloseClipboard();
        }

        void apply_filter_locked()
        {
            std::string keep;
            if (g_state.selected >= 0 && g_state.selected < static_cast<int>(g_state.servers.size()))
                keep = g_state.servers[g_state.selected].id;

            std::string query = g_state.filter;
            std::transform(query.begin(), query.end(), query.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            std::vector<server_entry> out;
            for (const auto& s : g_all)
            {
                if (g_state.hide_full && s.max_players > 0 && s.playing >= s.max_players)
                    continue;
                if (!query.empty())
                {
                    std::string id = s.id;
                    std::transform(id.begin(), id.end(), id.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    if (id.find(query) == std::string::npos)
                        continue;
                }
                out.push_back(s);
            }

            switch (g_state.sort_mode)
            {
            case 1:
                std::sort(out.begin(), out.end(), [](const server_entry& a, const server_entry& b) {
                    if (a.playing != b.playing)
                        return a.playing > b.playing;
                    return a.ping < b.ping;
                });
                break;
            case 2:
                std::sort(out.begin(), out.end(), [](const server_entry& a, const server_entry& b) {
                    if (a.ping != b.ping)
                        return a.ping < b.ping;
                    return a.playing < b.playing;
                });
                break;
            case 3:
                std::sort(out.begin(), out.end(), [](const server_entry& a, const server_entry& b) {
                    return a.fps > b.fps;
                });
                break;
            default:
                std::sort(out.begin(), out.end(), [](const server_entry& a, const server_entry& b) {
                    if (a.playing != b.playing)
                        return a.playing < b.playing;
                    return a.ping < b.ping;
                });
                break;
            }

            g_state.servers = std::move(out);
            g_state.selected = -1;
            if (!keep.empty())
            {
                for (int i = 0; i < static_cast<int>(g_state.servers.size()); ++i)
                {
                    if (g_state.servers[i].id == keep)
                    {
                        g_state.selected = i;
                        break;
                    }
                }
            }
        }

        void worker(uint64_t place, std::string cursor)
        {
            const std::wstring host = L"games.roblox.com";
            auto servers_path = [&cursor](uint64_t id) {
                std::wstring path = L"/v1/games/" + std::to_wstring(id) + L"/servers/Public?limit=100";
                if (!cursor.empty())
                    path += L"&cursor=" + std::wstring(cursor.begin(), cursor.end());
                return path;
            };

            std::string body = http_get(host, servers_path(place));
            uint64_t resolved = 0;

            if (body.find("The place is invalid") != std::string::npos || body.find("\"data\":[]") != std::string::npos)
            {
                const std::string uni = http_get(host, L"/v1/games?universeIds=" + std::to_wstring(place));
                const std::vector<std::string> uni_data = array_objects(uni, "data");
                if (!uni_data.empty())
                {
                    double root = 0.0;
                    if (get_number(uni_data[0], "rootPlaceId", root) && root > 0.0)
                    {
                        resolved = static_cast<uint64_t>(root);
                        body = http_get(host, servers_path(resolved));
                    }
                }
            }

            std::vector<server_entry> list;
            std::string next;
            std::string err;

            if (body.empty())
            {
                err = "request failed (network/timeout)";
            }
            else
            {
                const std::vector<std::string> objects = array_objects(body, "data");
                for (const auto& obj : objects)
                {
                    server_entry e;
                    if (!get_string(obj, "id", e.id) || e.id.empty())
                        continue;
                    double v = 0.0;
                    if (get_number(obj, "maxPlayers", v))
                        e.max_players = static_cast<int>(v);
                    if (get_number(obj, "playing", v))
                        e.playing = static_cast<int>(v);
                    if (get_number(obj, "ping", v))
                        e.ping = static_cast<int>(v);
                    if (get_number(obj, "fps", v))
                        e.fps = static_cast<float>(v);
                    list.push_back(e);
                }
                get_string(body, "nextPageCursor", next);
                if (objects.empty() && body.find("\"errors\"") != std::string::npos)
                {
                    const std::vector<std::string> errors = array_objects(body, "errors");
                    if (!errors.empty())
                        get_string(errors[0], "message", err);
                    if (err.empty())
                        err = "api error";
                }
            }

            std::lock_guard<std::mutex> lock(g_mtx);
            g_inflight = false;
            g_state.loading = false;
            if (!err.empty())
            {
                g_state.status = err;
                return;
            }
            if (resolved)
                g_state.resolved_place = resolved;
            g_all = std::move(list);
            g_state.next_cursor = next;
            apply_filter_locked();
            g_state.status = "found " + std::to_string(g_all.size()) + " servers";
        }
    }

    std::mutex& mtx()
    {
        return g_mtx;
    }

    state_t& state()
    {
        return g_state;
    }

    void refresh_current_game()
    {
        uint64_t place = 0;
        std::string job;
        if (g_proc && g_proc->connected() && g_ctx.datamodel.addr)
        {
            const uintptr_t dm = static_cast<uintptr_t>(g_ctx.datamodel.addr);
            place = g_proc->read<uint64_t>(dm + Offsets::DataModel::PlaceId);
            job = g_proc->read_game_string(dm + Offsets::DataModel::JobId);
            if (job.size() < 32 || job.find('-') == std::string::npos)
            {
                job.clear();
                for (uintptr_t off = 0x100; off <= 0x140; off += 8)
                {
                    const std::string candidate = g_proc->read_game_string(dm + off);
                    if (candidate.size() >= 32 && candidate.find('-') != std::string::npos)
                    {
                        job = candidate;
                        break;
                    }
                }
            }
        }

        std::lock_guard<std::mutex> lock(g_mtx);
        g_state.active_place = place;
        g_state.active_job = job;
        if (place)
        {
            snprintf(g_state.place_id, sizeof(g_state.place_id), "%llu", static_cast<unsigned long long>(place));
            g_state.status = job.empty() ? "place loaded" : "place + job loaded";
        }
        else
        {
            g_state.status = "no game attached";
        }
    }

    uint64_t target_place_id()
    {
        char buf[sizeof(g_state.place_id)] = {};
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            memcpy(buf, g_state.place_id, sizeof(buf));
        }
        std::string digits;
        for (const char* p = buf; *p; ++p)
        {
            if (std::isdigit(static_cast<unsigned char>(*p)))
                digits += *p;
        }
        if (!digits.empty())
        {
            try
            {
                return std::stoull(digits);
            }
            catch (...)
            {
            }
        }
        std::lock_guard<std::mutex> lock(g_mtx);
        return g_state.active_place;
    }

    void fetch(uint64_t place_id, const std::string& cursor)
    {
        if (!place_id)
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            g_state.status = "enter a valid place id";
            return;
        }
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            if (g_inflight)
                return;
            g_inflight = true;
            g_state.loading = true;
            g_state.status = "fetching place " + std::to_string(place_id) + "...";
            g_state.resolved_place = 0;
        }
        std::thread(worker, place_id, cursor).detach();
    }

    void apply_filter()
    {
        std::lock_guard<std::mutex> lock(g_mtx);
        apply_filter_locked();
    }

    void join(uint64_t place_id, const std::string& job_id)
    {
        if (!place_id || job_id.empty())
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            g_state.status = "nothing selected";
            return;
        }
        const std::string uri = "roblox://experiences/start?placeId=" + std::to_string(place_id) +
            "&gameInstanceId=" + job_id;
        const HINSTANCE result = ShellExecuteA(nullptr, "open", uri.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        std::lock_guard<std::mutex> lock(g_mtx);
        g_state.status = (reinterpret_cast<INT_PTR>(result) > 32) ? "launching roblox..." : "failed to open roblox://";
    }

    void copy_job_id(const std::string& job_id)
    {
        set_clipboard(job_id);
        std::lock_guard<std::mutex> lock(g_mtx);
        g_state.status = "job id copied";
    }

    void copy_teleport_script(uint64_t place_id, const std::string& job_id)
    {
        const std::string script =
            "game:GetService(\"TeleportService\"):TeleportToPlaceInstance(" +
            std::to_string(place_id) + ", \"" + job_id + "\", game.Players.LocalPlayer)";
        set_clipboard(script);
        std::lock_guard<std::mutex> lock(g_mtx);
        g_state.status = "teleport code copied";
    }

    void next_page()
    {
        const uint64_t place = target_place_id();
        if (!place)
            return;
        std::string cursor;
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            if (g_state.next_cursor.empty() || g_inflight)
                return;
            cursor = g_state.next_cursor;
            g_state.cursors.push_back(cursor);
            g_state.page = static_cast<int>(g_state.cursors.size()) + 1;
        }
        fetch(place, cursor);
    }

    void prev_page()
    {
        const uint64_t place = target_place_id();
        if (!place)
            return;
        std::string cursor;
        {
            std::lock_guard<std::mutex> lock(g_mtx);
            if (g_state.page <= 1 || g_state.cursors.empty() || g_inflight)
                return;
            g_state.cursors.pop_back();
            cursor = g_state.cursors.empty() ? std::string() : g_state.cursors.back();
            g_state.page = static_cast<int>(g_state.cursors.size()) + 1;
        }
        fetch(place, cursor);
    }
}
