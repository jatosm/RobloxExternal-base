#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "explorer.h"
#include "../../../menu/core/gui/menu.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/logger/logger.h"
#include "../../../utility/stb/stb_image.h"
#include "../../../utility/struct/offsets.h"
#include "assets/dex_icons.h"
#include <Windows.h>
#include <commdlg.h>
#include <winhttp.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#pragma comment(lib, "winhttp.lib")
namespace thenwefuckin::explorer
{
namespace
{
constexpr int kMaxTreeNodes = 1500;
constexpr int kMaxKidsShown = 150;
constexpr int kMaxKidsCached = 200;
constexpr int kMaxDepth = 10;
char s_search[64] = "";
int s_filter = 0;
uintptr_t s_selected = 0;
std::string s_status = "idle";
std::mutex s_statusMtx;
void set_status(const std::string& s)
{
    std::lock_guard<std::mutex> lk(s_statusMtx);
    s_status = s;
}
uintptr_t explorer_datamodel();
std::string get_status()
{
    std::lock_guard<std::mutex> lk(s_statusMtx);
    return s_status;
}
void exp_log(const std::string& msg)
{
    LOG("explorer", msg.c_str());
}
struct SnapNode
{
    std::string name;
    std::string cls;
    uintptr_t parent = 0;
    std::vector<uintptr_t> kids;
};
using SnapMap = std::unordered_map<uintptr_t, SnapNode>;
std::atomic<std::shared_ptr<const SnapMap>> s_snapAtomic;
std::mutex s_snapPubMtx;
std::atomic<bool> s_snapReady{ false };
std::atomic<bool> s_snapRunning{ false };
std::atomic<size_t> s_snapCount{ 0 };
uintptr_t s_snapDm = 0;
std::atomic<long long> s_dbgViewHit{ 0 };
std::atomic<long long> s_dbgViewMiss{ 0 };
std::atomic<long long> s_dbgLiveFallback{ 0 };
std::atomic<long long> s_dbgCacheHit{ 0 };
std::atomic<long long> s_dbgCacheMiss{ 0 };
std::chrono::steady_clock::time_point s_dbgScanStart{};
std::chrono::steady_clock::time_point s_dbgLastWaitLog{};
void start_snapshot(uintptr_t dm)
{
    if (!dm || s_snapRunning.exchange(true))
        return;
    s_snapDm = dm;
    s_snapReady.store(false);
    s_snapCount.store(0);
    s_dbgScanStart = std::chrono::steady_clock::now();
    std::thread([dm]() {
        auto t0 = std::chrono::steady_clock::now();
        {
            node rt{ dm };
            auto rk = rt.children();
            char b[160];
            std::snprintf(b, sizeof(b), "snapshot scan started dm=0x%llX rootKids=%zu",
                (unsigned long long)dm, rk.size());
            exp_log(b);
        }
        std::unordered_map<uintptr_t, SnapNode> tmp;
        tmp.reserve(100000);
        std::vector<uintptr_t> stack;
        stack.push_back(dm);
        size_t visited = 0;
        bool capped = false;
        while (!stack.empty() && tmp.size() < 150000 && visited < 200000)
        {
            uintptr_t addr = stack.back();
            stack.pop_back();
            ++visited;
            if ((visited & 1023) == 0)
            {
                if (std::chrono::steady_clock::now() - t0 > std::chrono::seconds(8))
                {
                    capped = true;
                    break;
                }
            }
            if (tmp.find(addr) != tmp.end())
                continue;
            node inst{ addr };
            std::string nm = inst.name();
            if (nm.empty())
                nm = "Unnamed";
            if (nm.size() > 120)
                nm.resize(120);
            std::string cl = inst.class_name();
            if (cl.size() > 64)
                cl.resize(64);
            auto kids = inst.children();
            SnapNode sn;
            sn.name = std::move(nm);
            sn.cls = std::move(cl);
            sn.kids.reserve(kids.size());
            for (auto& k : kids)
            {
                if (k.addr)
                    sn.kids.push_back(k.addr);
            }
            tmp[addr] = std::move(sn);
            auto& kk = tmp[addr].kids;
            for (auto it = kk.rbegin(); it != kk.rend(); ++it)
                stack.push_back(*it);
            if ((tmp.size() & 511) == 0)
                s_snapCount.store(tmp.size());
            if ((visited & 16383) == 0)
            {
                char pb[128];
                std::snprintf(pb, sizeof(pb), "snapshot scan progress visited=%zu stored=%zu",
                    visited, tmp.size());
                exp_log(pb);
            }
        }
        for (auto& p : tmp)
        {
            for (uintptr_t k : p.second.kids)
            {
                auto it = tmp.find(k);
                if (it != tmp.end() && !it->second.parent)
                    it->second.parent = p.first;
            }
        }
        s_snapCount.store(tmp.size());
        bool keep = (dm == s_snapDm);
        if (keep)
        {
            auto pub = std::make_shared<const SnapMap>(std::move(tmp));
            std::lock_guard<std::mutex> lk(s_snapPubMtx);
            s_snapAtomic.store(pub);
            s_snapReady.store(true);
        }
        s_snapRunning.store(false);
        if (keep)
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0).count();
            char b[160];
            if (capped)
                std::snprintf(b, sizeof(b), "snapshot: %zu inst partial (%lldms cap)",
                    s_snapCount.load(), (long long)ms);
            else
                std::snprintf(b, sizeof(b), "snapshot: %zu inst (%lldms)",
                    s_snapCount.load(), (long long)ms);
            exp_log(b);
            set_status(b);
            if (s_snapCount.load() == 0)
                exp_log("snapshot EMPTY: root unreadable, children read failing?");
        }
        else
        {
            char b[128];
            std::snprintf(b, sizeof(b), "snapshot discarded: dm changed mid-scan (was 0x%llX)",
                (unsigned long long)dm);
            exp_log(b);
        }
    }).detach();
}
void ensure_snapshot(uintptr_t dm)
{
    if (!dm)
        return;
    if (s_snapRunning.load())
    {
        auto now = std::chrono::steady_clock::now();
        if (now - s_dbgLastWaitLog > std::chrono::seconds(5))
        {
            s_dbgLastWaitLog = now;
            long long secs = 0;
            if (s_dbgScanStart.time_since_epoch().count() != 0)
                secs = std::chrono::duration_cast<std::chrono::seconds>(now - s_dbgScanStart).count();
            char b[160];
            std::snprintf(b, sizeof(b), "snapshot: scan still running after %llds (scanned=%zu) dm=0x%llX",
                secs, s_snapCount.load(), (unsigned long long)dm);
            exp_log(b);
            if (secs > 30)
                exp_log("snapshot STUCK: worker deadlocked or killed, reopen panel to retry");
        }
        return;
    }
    if (s_snapReady.load() && dm == s_snapDm)
        return;
    if (dm != s_snapDm)
    {
        s_snapDm = 0;
        s_snapReady.store(false);
    }
    start_snapshot(dm);
}
struct CachedNode
{
    std::string name;
    std::string cls;
    std::vector<uintptr_t> kids;
    bool hasKids = false;
    std::chrono::steady_clock::time_point ts{};
};
std::unordered_map<uintptr_t, CachedNode> s_cache;
std::mutex s_cacheMtx;
CachedNode get_cached(uintptr_t addr)
{
    auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lk(s_cacheMtx);
        auto it = s_cache.find(addr);
        if (it != s_cache.end() &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.ts).count() < 1500)
        {
            s_dbgCacheHit.fetch_add(1);
            return it->second;
        }
    }
    s_dbgCacheMiss.fetch_add(1);
    CachedNode ec;
    node inst{ addr };
    ec.name = inst.name();
    if (ec.name.empty())
        ec.name = "Unnamed";
    ec.cls = inst.class_name();
    auto cl = inst.children();
    ec.kids.reserve(cl.size() > kMaxKidsCached ? kMaxKidsCached : cl.size());
    for (size_t i = 0; i < cl.size() && i < (size_t)kMaxKidsCached; ++i)
        if (cl[i].addr)
            ec.kids.push_back(cl[i].addr);
    ec.hasKids = !ec.kids.empty();
    ec.ts = now;
    {
        std::lock_guard<std::mutex> lk(s_cacheMtx);
        s_cache[addr] = ec;
        if (s_cache.size() > 60000)
            s_cache.clear();
    }
    return ec;
}
std::string instance_path(uintptr_t addr)
{
    if (!addr)
        return "";
    std::vector<std::string> parts;
    uintptr_t cur = addr;
    for (int i = 0; i < 32 && cur; ++i)
    {
        node n{ cur };
        std::string nm = n.name();
        if (nm.empty())
            nm = "Unnamed";
        parts.push_back(nm);
        node p = n.parent();
        if (!p.addr || p.addr == cur)
            break;
        if (p.name() == "Game")
            break;
        cur = p.addr;
    }
    std::string out;
    for (auto it = parts.rbegin(); it != parts.rend(); ++it)
    {
        if (!out.empty())
            out += ".";
        out += *it;
    }
    return out;
}
bool contains_ci(const std::string& hay, const std::string& needleLower)
{
    if (needleLower.empty())
        return true;
    if (hay.size() < needleLower.size())
        return false;
    const size_t n = needleLower.size(), h = hay.size();
    for (size_t i = 0; i <= h - n; ++i)
    {
        bool ok = true;
        for (size_t j = 0; j < n; ++j)
        {
            char c = hay[i + j];
            if (c >= 'A' && c <= 'Z')
                c += 32;
            if (c != needleLower[j]) { ok = false; break; }
        }
        if (ok)
            return true;
    }
    return false;
}
bool match_filter(const std::string& cls, int filter)
{
    switch (filter)
    {
    case 1: return cls.find("Script") != std::string::npos;
    case 2: return cls.find("Part") != std::string::npos || cls == "MeshPart";
    case 3: return cls == "Folder";
    case 4: return cls == "RemoteEvent" || cls == "RemoteFunction" ||
                   cls == "BindableEvent" || cls == "BindableFunction";
    default: return true;
    }
}
struct SearchResult
{
    uintptr_t addr = 0;
    std::string name;
    std::string cls;
    std::string path;
    std::string top;
    std::string sub;
};
struct FileNode
{
    std::string name;
    std::string cls;
    uintptr_t parent = 0;
    std::vector<uintptr_t> kids;
};
std::unordered_map<uintptr_t, FileNode> s_fileCache;
std::mutex s_fileMtx;
uintptr_t s_fileRoot = 0;
std::string s_fileName;
size_t s_fileCount = 0;
std::atomic<bool> s_fileLoading{ false };
bool s_useFile = false;
struct NodeView
{
    std::string name;
    std::string cls;
    std::vector<uintptr_t> kids;
    bool hasKids = false;
    bool ok = false;
};
bool view_node(uintptr_t addr, NodeView& out)
{
    out = NodeView{};
    if (!addr)
        return false;
    if (s_useFile && s_fileRoot)
    {
        std::lock_guard<std::mutex> lk(s_fileMtx);
        auto it = s_fileCache.find(addr);
        if (it == s_fileCache.end())
            return false;
        out.name = it->second.name;
        out.cls = it->second.cls;
        out.kids = it->second.kids;
        out.hasKids = !out.kids.empty();
        out.ok = true;
        return true;
    }
    auto snap = s_snapAtomic.load();
    if (snap && s_snapReady.load())
    {
        auto it = snap->find(addr);
        if (it != snap->end())
        {
            out.name = it->second.name;
            out.cls = it->second.cls;
            out.kids = it->second.kids;
            out.hasKids = !out.kids.empty();
            out.ok = true;
            s_dbgViewHit.fetch_add(1);
            return true;
        }
        s_dbgViewMiss.fetch_add(1);
    }
    else
    {
        s_dbgLiveFallback.fetch_add(1);
    }
    CachedNode ec = get_cached(addr);
    out.name = ec.name;
    out.cls = ec.cls;
    out.kids = ec.kids;
    out.hasKids = ec.hasKids;
    out.ok = true;
    return true;
}
std::string view_parent_name(uintptr_t addr)
{
    if (s_useFile && s_fileRoot)
    {
        std::lock_guard<std::mutex> lk(s_fileMtx);
        auto it = s_fileCache.find(addr);
        if (it == s_fileCache.end() || !it->second.parent)
            return "None";
        auto p = s_fileCache.find(it->second.parent);
        if (p == s_fileCache.end() || p->second.name.empty())
            return "None";
        return p->second.name;
    }
    auto snap = s_snapAtomic.load();
    if (snap && s_snapReady.load())
    {
        auto it = snap->find(addr);
        if (it != snap->end() && it->second.parent)
        {
            auto p = snap->find(it->second.parent);
            if (p != snap->end() && !p->second.name.empty())
                return p->second.name;
        }
        else if (it != snap->end())
            return "None";
    }
    std::string pn = node{ addr }.parent().name();
    return pn.empty() ? "None" : pn;
}
std::string view_path(uintptr_t addr)
{
    if (s_useFile && s_fileRoot)
    {
        std::vector<std::string> parts;
        uintptr_t cur = addr;
        std::lock_guard<std::mutex> lk(s_fileMtx);
        for (int i = 0; i < 32 && cur; ++i)
        {
            auto it = s_fileCache.find(cur);
            if (it == s_fileCache.end())
                break;
            parts.push_back(it->second.name.empty() ? "Unnamed" : it->second.name);
            cur = it->second.parent;
        }
        std::string out;
        for (auto it = parts.rbegin(); it != parts.rend(); ++it)
        {
            if (!out.empty())
                out += ".";
            out += *it;
        }
        return out;
    }
    auto snap = s_snapAtomic.load();
    if (snap && s_snapReady.load())
    {
        std::vector<std::string> parts;
        uintptr_t cur = addr;
        for (int i = 0; i < 32 && cur; ++i)
        {
            auto it = snap->find(cur);
            if (it == snap->end())
                break;
            parts.push_back(it->second.name.empty() ? "Unnamed" : it->second.name);
            cur = it->second.parent;
        }
        if (!parts.empty())
        {
            std::string out;
            for (auto it = parts.rbegin(); it != parts.rend(); ++it)
            {
                if (!out.empty())
                    out += ".";
                out += *it;
            }
            return out;
        }
    }
    return instance_path(addr);
}
std::vector<SearchResult> s_results;
std::mutex s_resultsMtx;
std::atomic<int> s_resultsVer{ 0 };
std::string s_lastQuery;
int s_lastFilter = -1;
std::string s_prevQLower;
int s_prevFilter = -1;
uintptr_t s_prevRoot = 0;
std::shared_ptr<const SnapMap> s_prevSnap;
std::vector<uintptr_t> s_prevAddrs;
std::atomic<bool> s_searchRunning{ false };
ULONGLONG s_lastTrigger = 0;
std::string s_pendingQuery;
int s_pendingFilter = 0;
std::mutex s_pendingMtx;
bool s_hasPending = false;
void run_search_scan(const std::string& query, int filter)
{
    auto t0 = std::chrono::steady_clock::now();
    {
        char b[128];
        std::snprintf(b, sizeof(b), "search started '%s' filter=%d",
            query.c_str(), filter);
        exp_log(b);
    }
    std::string q = query;
    for (char& c : q)
        c = (char)tolower((unsigned char)c);
    std::vector<SearchResult> out;
    out.reserve(300);
    uintptr_t root = explorer_datamodel();
    if (!root)
    {
        std::lock_guard<std::mutex> lk(s_resultsMtx);
        s_results.clear();
        s_prevAddrs.clear();
        s_prevQLower.clear();
        s_prevSnap.reset();
        s_resultsVer.fetch_add(1);
        exp_log("search skipped: no datamodel");
        return;
    }
    auto snap = s_snapAtomic.load();
    bool haveSnap = (snap && !snap->empty() && s_snapReady.load());
    bool incremental = haveSnap && !s_prevQLower.empty() && filter == s_prevFilter && root == s_prevRoot &&
        s_prevSnap == snap && !s_prevAddrs.empty() && q.rfind(s_prevQLower, 0) == 0;
    if (incremental)
    {
        for (uintptr_t addr : s_prevAddrs)
        {
            if (out.size() >= 2000)
                break;
            auto it = snap->find(addr);
            if (it == snap->end())
                continue;
            const std::string& nm = it->second.name;
            const std::string& cl = it->second.cls;
            bool m = true;
            if (!q.empty())
                m = contains_ci(nm, q) || contains_ci(cl, q);
            if (m && filter != 0)
                m = match_filter(cl, filter);
            if (!m)
                continue;
            SearchResult r;
            r.addr = addr;
            r.name = nm;
            r.cls = cl;
            out.push_back(std::move(r));
        }
    }
    else if (haveSnap)
    {
        for (auto& kv : *snap)
        {
            if (out.size() >= 2000)
                break;
            if (kv.first == root)
                continue;
            const std::string& nm = kv.second.name;
            const std::string& cl = kv.second.cls;
            bool m = true;
            if (!q.empty())
                m = contains_ci(nm, q) || contains_ci(cl, q);
            if (m && filter != 0)
                m = match_filter(cl, filter);
            if (!m)
                continue;
            SearchResult r;
            r.addr = kv.first;
            r.name = nm;
            r.cls = cl;
            out.push_back(std::move(r));
        }
        s_prevQLower = q;
        s_prevFilter = filter;
        s_prevRoot = root;
        s_prevSnap = snap;
        s_prevAddrs.clear();
        s_prevAddrs.reserve(out.size());
        for (auto& r : out)
            s_prevAddrs.push_back(r.addr);
    }
    else
    {
        std::vector<uintptr_t> stack;
        stack.push_back(root);
        size_t visited = 0;
        while (!stack.empty() && out.size() < 2000 && visited < 25000)
        {
            uintptr_t addr = stack.back();
            stack.pop_back();
            ++visited;
            if ((visited & 1023) == 0)
            {
                if (std::chrono::steady_clock::now() - t0 > std::chrono::milliseconds(600))
                    break;
            }
            NodeView ev;
            if (!view_node(addr, ev))
                continue;
            if (addr != root)
            {
                bool m = true;
                if (!q.empty())
                    m = contains_ci(ev.name, q) || contains_ci(ev.cls, q);
                if (m && filter != 0)
                    m = match_filter(ev.cls, filter);
                if (m)
                {
                    SearchResult r;
                    r.addr = addr;
                    r.name = ev.name;
                    r.cls = ev.cls;
                    out.push_back(std::move(r));
                }
            }
            for (auto it = ev.kids.rbegin(); it != ev.kids.rend(); ++it)
                stack.push_back(*it);
        }
        s_prevQLower = q;
        s_prevFilter = filter;
        s_prevRoot = root;
        s_prevSnap.reset();
        s_prevAddrs.clear();
        s_prevAddrs.reserve(out.size());
        for (auto& r : out)
            s_prevAddrs.push_back(r.addr);
    }
    size_t pathed = out.size() > 400 ? 400 : out.size();
    for (size_t i = 0; i < pathed; ++i)
        out[i].path = view_path(out[i].addr);
    {
        std::lock_guard<std::mutex> lk(s_resultsMtx);
        s_results = std::move(out);
        s_resultsVer.fetch_add(1);
        char b[128];
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();
        std::snprintf(b, sizeof(b), "search done: %zu results (%lld ms)",
            s_results.size(), (long long)ms);
        exp_log(b);
    }
}
void trigger_search(const std::string& query, int filter)
{
    {
        std::lock_guard<std::mutex> lk(s_pendingMtx);
        s_pendingQuery = query;
        s_pendingFilter = filter;
        s_hasPending = true;
    }
    if (s_searchRunning.exchange(true))
        return;
    s_lastQuery = query;
    s_lastFilter = filter;
    std::thread([]() {
        for (;;)
        {
            std::string query;
            int filter = 0;
            {
                std::lock_guard<std::mutex> lk(s_pendingMtx);
                if (!s_hasPending)
                    break;
                query = s_pendingQuery;
                filter = s_pendingFilter;
                s_hasPending = false;
            }
            run_search_scan(query, filter);
        }
        s_searchRunning = false;
    }).detach();
}
std::atomic<bool> s_dumping{ false };
void load_dump_file(const std::string& path)
{
    if (s_fileLoading.exchange(true))
        return;
    set_status("parsing dump...");
    std::thread([path]() {
        exp_log(("parsing " + path).c_str());
        std::unordered_map<uintptr_t, FileNode> tmp;
        tmp.reserve(100000);
        uintptr_t root = 0;
        size_t count = 0;
        FILE* f = nullptr;
        fopen_s(&f, path.c_str(), "r");
        if (f)
        {
            char line[512];
            std::vector<std::pair<uintptr_t, int>> stack;
            while (fgets(line, sizeof(line), f))
            {
                if (tmp.size() >= 300000)
                    break;
                const char* p = line;
                int sp = 0;
                while (*p == ' ') { ++sp; ++p; }
                if (*p != '-')
                    continue;
                int depth = sp / 2;
                ++p;
                while (*p == ' ') ++p;
                if (*p != '[')
                    continue;
                const char* cc = strchr(p + 1, ']');
                if (!cc)
                    continue;
                const char* ar = strstr(cc, "(0x");
                if (!ar)
                    continue;
                unsigned long long av = strtoull(ar + 3, nullptr, 16);
                if (!av)
                    continue;
                uintptr_t addr = (uintptr_t)av;
                std::string cls(p + 1, (size_t)(cc - p - 1));
                const char* n0 = cc + 1;
                while (*n0 == ' ') ++n0;
                std::string nm(n0, (size_t)(ar - n0));
                while (!nm.empty() && (nm.back() == ' ' || nm.back() == '\t' || nm.back() == '\r' || nm.back() == '\n'))
                    nm.pop_back();
                if (nm.size() > 120)
                    nm.resize(120);
                if (cls.size() > 64)
                    cls.resize(64);
                if (nm.empty())
                    nm = "Unnamed";
                while (!stack.empty() && stack.back().second >= depth)
                    stack.pop_back();
                uintptr_t par = stack.empty() ? 0 : stack.back().first;
                if (!root)
                    root = addr;
                FileNode fn;
                fn.name = nm;
                fn.cls = cls;
                fn.parent = par;
                tmp[addr] = std::move(fn);
                if (par)
                    tmp[par].kids.push_back(addr);
                stack.push_back({ addr, depth });
                ++count;
            }
            fclose(f);
        }
        std::string base = path;
        auto sl = base.find_last_of("\\/");
        if (sl != std::string::npos)
            base = base.substr(sl + 1);
        {
            std::lock_guard<std::mutex> lk(s_fileMtx);
            s_fileCache.swap(tmp);
            s_fileRoot = root;
            s_fileName = base;
            s_fileCount = count;
        }
        s_fileLoading.store(false);
        char b[160];
        std::snprintf(b, sizeof(b), "dump ready: %zu instances (%s)", count, base.c_str());
        exp_log(b);
        set_status(b);
    }).detach();
}
bool pick_dump_file(std::string& out)
{
    wchar_t buf[MAX_PATH]{};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Dump files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn))
        return false;
    char nb[MAX_PATH * 2]{};
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, nb, (int)sizeof(nb), nullptr, nullptr);
    out = nb;
    return true;
}
std::string dump_dir()
{
    char* up = nullptr;
    size_t len = 0;
    _dupenv_s(&up, &len, "USERPROFILE");
    std::string base = up ? std::string(up) : std::string("C:\\Users\\Public");
    if (up)
        free(up);
    return base + "\\Desktop\\thenwefuckin dumps\\";
}
void dump_all()
{
    if (s_dumping.exchange(true))
        return;
    set_status("dumping...");
    std::thread([]() {
        exp_log("dumping instances...");
        std::string dir = dump_dir();
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        std::time_t t = std::time(nullptr);
        std::tm tm{};
        localtime_s(&tm, &t);
        char stamp[32];
        std::snprintf(stamp, sizeof(stamp), "dump_%04d%02d%02d_%02d%02d%02d.txt",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        std::string path = dir + stamp;
        FILE* f = nullptr;
        fopen_s(&f, path.c_str(), "w");
        if (!f)
        {
            s_dumping.store(false);
            set_status("dump failed");
            return;
        }
        uintptr_t root = explorer_datamodel();
        node rn{ root };
        std::string rname = root ? rn.name() : "?";
        std::string rcls = root ? rn.class_name() : "?";
        std::fprintf(f, "=== INSTANCE DUMP ===\nRoot Address: 0x%llX\nRoot Name: %s\nRoot Class: %s\n\n",
            (unsigned long long)root, rname.c_str(), rcls.c_str());
        struct Item { uintptr_t addr; int depth; };
        std::vector<Item> stack;
        if (root)
            stack.push_back({ root, 0 });
        size_t count = 0;
        const size_t kCap = 300000;
        char line[512];
        while (!stack.empty() && count < kCap)
        {
            Item cur = stack.back();
            stack.pop_back();
            ++count;
            node inst{ cur.addr };
            std::string nm = inst.name();
            std::string cl = inst.class_name();
            if (nm.size() > 120)
                nm.resize(120);
            if (cl.size() > 64)
                cl.resize(64);
            std::string pad((size_t)(cur.depth > 12 ? 12 : cur.depth) * 2, ' ');
            std::snprintf(line, sizeof(line), "%s- [%s] %s (0x%llX)\n", pad.c_str(),
                cl.c_str(), nm.c_str(), (unsigned long long)cur.addr);
            std::fputs(line, f);
            auto kids = inst.children();
            for (auto it = kids.rbegin(); it != kids.rend(); ++it)
                if (it->addr && count + stack.size() < kCap)
                    stack.push_back({ it->addr, cur.depth + 1 });
        }
        std::fprintf(f, "\nTotal: %zu instances\n", count);
        std::fclose(f);
        char b[256];
        std::snprintf(b, sizeof(b), "dumped %zu -> %s", count, path.c_str());
        exp_log(b);
        char sb[64];
        std::snprintf(sb, sizeof(sb), "dumped %zu instances", count);
        set_status(sb);
        s_dumping.store(false);
    }).detach();
}
std::atomic<bool> s_imgDumping{ false };
std::atomic<int> s_imgDone{ 0 };
std::atomic<int> s_imgTotal{ 0 };
std::string read_img_str(uintptr_t addr, uintptr_t off)
{
    if (!addr || !g_proc || !g_proc->connected())
        return {};
    const uintptr_t cand[] = { off, off + 8, off - 8, off + 0x10, off - 0x10 };
    for (uintptr_t o : cand)
    {
        std::string s = g_proc->read_game_string(addr + o);
        if (!s.empty() && s.find("rbxassetid://") != std::string::npos)
            return s;
        uintptr_t ptr = g_proc->read<uintptr_t>(addr + o);
        if (ptr)
        {
            s = g_proc->read_game_string(ptr);
            if (!s.empty() && s.find("rbxassetid://") != std::string::npos)
                return s;
        }
    }
    return {};
}
std::string img_http_get(const std::wstring& host, const std::wstring& path)
{
    std::string out;
    HINTERNET hs = WinHttpOpen(L"thenwefuckin/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!hs)
        return out;
    HINTERNET hc = WinHttpConnect(hs, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hc) { WinHttpCloseHandle(hs); return out; }
    HINTERNET hr = WinHttpOpenRequest(hc, L"GET", path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hr) { WinHttpCloseHandle(hc); WinHttpCloseHandle(hs); return out; }
    bool ok = WinHttpSendRequest(hr, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
        && WinHttpReceiveResponse(hr, nullptr);
    std::vector<char> buf;
    if (ok)
    {
        for (;;)
        {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(hr, &avail) || !avail)
                break;
            size_t old = buf.size();
            buf.resize(old + avail);
            DWORD rd = 0;
            if (!WinHttpReadData(hr, buf.data() + old, avail, &rd) || !rd)
            {
                buf.resize(old);
                break;
            }
            buf.resize(old + rd);
        }
    }
    out.assign(buf.begin(), buf.end());
    WinHttpCloseHandle(hr);
    WinHttpCloseHandle(hc);
    WinHttpCloseHandle(hs);
    return out;
}
std::string img_asset_url(const std::string& json)
{
    auto p = json.find("imageUrl");
    if (p == std::string::npos)
        return {};
    auto q = json.find("http", p);
    if (q == std::string::npos)
        return {};
    auto e = json.find('"', q);
    std::string url = json.substr(q, e == std::string::npos ? e : e - q);
    size_t pos = url.find("\\u0026");
    while (pos != std::string::npos)
    {
        url.replace(pos, 6, "&");
        pos = url.find("\\u0026", pos + 1);
    }
    return url;
}
void dump_images()
{
    if (s_imgDumping.exchange(true))
        return;
    set_status("scanning images...");
    std::thread([]() {
        exp_log("dumping images...");
        std::string dir = dump_dir() + "images\\";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        struct Found { std::string id; std::string name; };
        std::vector<Found> items;
        std::unordered_map<std::string, bool> seen;
        uintptr_t root = explorer_datamodel();
        std::vector<uintptr_t> stack;
        if (root)
            stack.push_back(root);
        size_t vis = 0;
        const size_t kCap = 300000;
        while (!stack.empty() && vis < kCap && items.size() < 5000)
        {
            uintptr_t addr = stack.back();
            stack.pop_back();
            ++vis;
            node inst{ addr };
            std::string cls = inst.class_name();
            std::string asset;
            if (cls == "ImageLabel" || cls == "ImageButton" || cls == "Decal" || cls == "Texture")
                asset = read_img_str(addr, (cls == "Decal" || cls == "Texture") ? 0x1E0 : 0x988);
            if (!asset.empty())
            {
                std::string id = asset;
                if (id.rfind("rbxassetid://", 0) == 0)
                    id = id.substr(13);
                auto at = id.find_first_not_of("0123456789");
                if (at != std::string::npos)
                    id = id.substr(0, at);
                if (!id.empty() && !seen[id])
                {
                    seen[id] = true;
                    std::string nm = inst.name();
                    if (nm.empty())
                        nm = cls;
                    items.push_back({ id, nm });
                }
            }
            auto kids = inst.children();
            for (auto it = kids.rbegin(); it != kids.rend(); ++it)
                if (it->addr && vis + stack.size() < kCap)
                    stack.push_back(it->addr);
        }
        s_imgTotal.store((int)items.size());
        s_imgDone.store(0);
        FILE* idx = nullptr;
        fopen_s(&idx, (dir + "images_index.txt").c_str(), "w");
        int done = 0, fail = 0;
        std::unordered_map<std::string, int> nameCount;
        for (auto& it : items)
        {
            std::wstring q = L"/v1/assets?assetIds=" + std::wstring(it.id.begin(), it.id.end()) + L"&size=420x420&format=Png";
            std::string json = img_http_get(L"thumbnails.roblox.com", q);
            std::string url = img_asset_url(json);
            if (url.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                json = img_http_get(L"thumbnails.roblox.com", q);
                url = img_asset_url(json);
            }
            if (url.empty())
            {
                ++fail;
                if (idx)
                    fprintf(idx, "FAIL %s %s no-url\n", it.id.c_str(), it.name.c_str());
                s_imgDone.store(done + fail);
                continue;
            }
            std::string tmp = url;
            if (tmp.rfind("https://", 0) == 0)
                tmp = tmp.substr(8);
            else if (tmp.rfind("http://", 0) == 0)
                tmp = tmp.substr(7);
            std::string host, path;
            auto sl = tmp.find('/');
            if (sl != std::string::npos) { host = tmp.substr(0, sl); path = tmp.substr(sl); }
            else { host = tmp; path = "/"; }
            std::string data = img_http_get(std::wstring(host.begin(), host.end()),
                std::wstring(path.begin(), path.end()));
            if (data.size() < 100)
            {
                ++fail;
                if (idx)
                    fprintf(idx, "FAIL %s %s no-data\n", it.id.c_str(), it.name.c_str());
                s_imgDone.store(done + fail);
                continue;
            }
            std::string safe;
            for (char c : it.name)
            {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-')
                    safe.push_back(c);
                else if (c == ' ')
                    safe.push_back('_');
                if (safe.size() >= 48)
                    break;
            }
            if (safe.empty())
                safe = "image";
            int n = nameCount[safe + "_" + it.id]++;
            std::string fname = safe + "_" + it.id + (n ? "_" + std::to_string(n) : "") + ".png";
            FILE* f = nullptr;
            fopen_s(&f, (dir + fname).c_str(), "wb");
            if (f)
            {
                fwrite(data.data(), 1, data.size(), f);
                fclose(f);
                ++done;
                if (idx)
                    fprintf(idx, "OK %s %s %s\n", it.id.c_str(), it.name.c_str(), fname.c_str());
            }
            else
            {
                ++fail;
                if (idx)
                    fprintf(idx, "FAIL %s %s save-fail\n", it.id.c_str(), it.name.c_str());
            }
            s_imgDone.store(done + fail);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        if (idx)
            fclose(idx);
        char b[128];
        std::snprintf(b, sizeof(b), "images done=%d failed=%d", done, fail);
        exp_log(b);
        set_status(b);
        s_imgDumping.store(false);
    }).detach();
}
int s_rendered = 0;
bool s_needOpenLog = true;
uintptr_t s_lastRootDbg = 0;
uintptr_t s_indepDm = 0;
std::chrono::steady_clock::time_point s_indepTs{};
uintptr_t s_lastLiveDm = 0;
std::chrono::steady_clock::time_point s_lastPoll{};
std::unordered_map<uintptr_t, std::string> s_knownPlayers;
uintptr_t explorer_datamodel()
{
    if (s_useFile && s_fileRoot)
        return s_fileRoot;
    auto now = std::chrono::steady_clock::now();
    if (s_indepDm && now - s_indepTs < std::chrono::milliseconds(500))
        return s_indepDm;
    s_indepTs = now;
    s_indepDm = 0;
    if (g_ctx.ready && g_ctx.datamodel.addr)
        s_indepDm = g_ctx.datamodel.addr;
    else if (g_proc && g_proc->connected())
    {
        std::uintptr_t base = (std::uintptr_t)g_proc->base();
        if (base)
        {
            std::uint64_t fake = g_proc->read<std::uint64_t>(base + Offsets::FakeDataModel::Pointer);
            if (fake)
                s_indepDm = g_proc->read<std::uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
        }
    }
    return s_indepDm;
}
void poll_game_events(uintptr_t dm)
{
    auto now = std::chrono::steady_clock::now();
    if (s_useFile && s_fileRoot)
        return;
    if (dm && dm != s_lastLiveDm)
    {
        uintptr_t old = s_lastLiveDm;
        s_lastLiveDm = dm;
        if (old != 0)
        {
            exp_log("new game detected (datamodel changed)");
            set_status("new game detected");
        }
        {
            std::lock_guard<std::mutex> lk(s_cacheMtx);
            s_cache.clear();
        }
        s_snapDm = 0;
        s_snapReady.store(false);
        s_knownPlayers.clear();
        s_selected = 0;
        s_lastQuery.clear();
        s_lastFilter = -1;
        s_prevQLower.clear();
        s_prevRoot = 0;
        s_prevAddrs.clear();
        {
            std::lock_guard<std::mutex> lk(s_resultsMtx);
            s_results.clear();
            s_resultsVer.fetch_add(1);
        }
    }
    if (!dm)
        return;
    static std::chrono::steady_clock::time_point s_lastPlayersPoll{};
    if (now - s_lastPlayersPoll < std::chrono::seconds(1))
        return;
    s_lastPlayersPoll = now;
    s_lastPoll = now;
    node players = node{ dm }.find_child_of_class("Players");
    if (!players.addr)
        players = node{ dm }.find_child("Players");
    if (!players.addr)
        players = node{ dm }.find_child("PlayerService");
    if (!players.addr)
        return;
    std::unordered_map<uintptr_t, std::string> cur;
    for (auto& k : players.children())
    {
        if (!k.addr)
            continue;
        std::string nm = k.name();
        if (nm.empty())
            nm = "Unnamed";
        cur[k.addr] = nm;
    }
    bool first = s_knownPlayers.empty() && !cur.empty();
    if (!first)
    {
        for (auto& p : cur)
        {
            if (s_knownPlayers.find(p.first) == s_knownPlayers.end())
            {
                char b[160];
                std::snprintf(b, sizeof(b), "player joined: %s", p.second.c_str());
                exp_log(b);
            }
        }
        for (auto& p : s_knownPlayers)
        {
            if (cur.find(p.first) == cur.end())
            {
                char b[160];
                std::snprintf(b, sizeof(b), "player left: %s", p.second.c_str());
                exp_log(b);
            }
        }
    }
    s_knownPlayers.swap(cur);
}
struct IconTex
{
    ID3D11ShaderResourceView* tex = nullptr;
    int w = 0, h = 0;
};
std::unordered_map<std::string, IconTex> g_iconCache;
std::mutex g_iconMutex;
bool g_texturesLoaded = false;
bool load_tex_from_mem(const unsigned char* data, unsigned int len, const std::string& name)
{
    ID3D11Device* dev = ::Overlay::GetDeviceStatic();
    if (!dev || !data || !len)
        return false;
    int w = 0, h = 0, ch = 0;
    unsigned char* px = stbi_load_from_memory(data, (int)len, &w, &h, &ch, 4);
    if (!px || w <= 0 || h <= 0)
    {
        if (px)
            stbi_image_free(px);
        return false;
    }
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = (UINT)w;
    desc.Height = (UINT)h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem = px;
    sub.SysMemPitch = (UINT)(w * 4);
    ID3D11Texture2D* t = nullptr;
    HRESULT hr = dev->CreateTexture2D(&desc, &sub, &t);
    if (FAILED(hr) || !t)
    {
        stbi_image_free(px);
        return false;
    }
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
    sd.Format = desc.Format;
    sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    sd.Texture2D.MipLevels = 1;
    hr = dev->CreateShaderResourceView(t, &sd, &srv);
    t->Release();
    stbi_image_free(px);
    if (FAILED(hr) || !srv)
        return false;
    std::lock_guard<std::mutex> lk(g_iconMutex);
    g_iconCache[name] = IconTex{ srv, w, h };
    return true;
}
#define LOAD_ICON(name) load_tex_from_mem(name##_png, name##_png_len, #name)
void load_all_icons()
{
    if (g_texturesLoaded)
        return;
    if (!::Overlay::GetDeviceStatic())
        return;
#include "assets/dex_icons_loader.inc"
    g_texturesLoaded = true;
    char b[64];
    std::snprintf(b, sizeof(b), "loaded %zu class icons", g_iconCache.size());
    exp_log(b);
}
IconTex* get_icon(const std::string& cls)
{
    std::lock_guard<std::mutex> lk(g_iconMutex);
    auto it = g_iconCache.find(cls);
    if (it != g_iconCache.end())
        return &it->second;
    auto fb = g_iconCache.find("Folder");
    if (fb != g_iconCache.end())
        return &fb->second;
    return nullptr;
}
void draw_node(uintptr_t addr, int depth)
{
    if (!addr || s_rendered > kMaxTreeNodes || depth > kMaxDepth)
        return;
    NodeView ec;
    if (!view_node(addr, ec))
        return;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                               ImGuiTreeNodeFlags_SpanAvailWidth;
    if (addr == s_selected)
        flags |= ImGuiTreeNodeFlags_Selected;
    if (!ec.hasKids)
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s [%s]", ec.name.c_str(), ec.cls.c_str());
    bool open = ImGui::TreeNodeEx((void*)(intptr_t)addr, flags, "%s", "");
    if (ImGui::IsItemClicked())
        s_selected = addr;
    IconTex* icon = get_icon(ec.cls);
    const float iconSize = 16.0f;
    if (icon && icon->tex)
    {
        ImGui::SameLine(0, 0.0f);
        ImGui::Image((ImTextureID)icon->tex, ImVec2(iconSize, iconSize));
        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x);
    }
    else
    {
        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x);
    }
    ImGui::TextUnformatted(buf);
    ++s_rendered;
    if (open && ec.hasKids)
    {
        size_t total = ec.kids.size();
        size_t shown = total > (size_t)kMaxKidsShown ? (size_t)kMaxKidsShown : total;
        for (size_t i = 0; i < shown; ++i)
            draw_node(ec.kids[i], depth + 1);
        if (total > shown)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 24.0f);
            ImGui::TextDisabled("... %zu more (use search)", total - shown);
        }
        ImGui::TreePop();
    }
}
std::string trunc_fit(ImFont* f, float fs, const std::string& s, float maxW)
{
    if (s.empty())
        return s;
    ImVec2 full = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, s.c_str());
    if (full.x <= maxW)
        return s;
    std::string out = s;
    while (out.size() > 4)
    {
        out.erase(out.size() - 4, 3);
        out += "...";
        ImVec2 w = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, out.c_str());
        if (w.x <= maxW)
            break;
    }
    return out;
}
}
void on_close()
{
    std::lock_guard<std::mutex> lk(s_resultsMtx);
    s_results.clear();
    s_resultsVer.fetch_add(1);
    s_lastQuery.clear();
    s_lastFilter = -1;
    s_prevQLower.clear();
    s_prevRoot = 0;
    s_prevSnap.reset();
    s_prevAddrs.clear();
    {
        std::lock_guard<std::mutex> lk(s_pendingMtx);
        s_hasPending = false;
        s_pendingQuery.clear();
    }
    set_status("idle");
    s_needOpenLog = true;
    s_lastRootDbg = 0;
    s_indepDm = 0;
    s_useFile = false;
    s_knownPlayers.clear();
}
void render(ImDrawList* dl, ImVec2 pos, ImVec2 mx)
{
    using namespace Menu;
    if (s_needOpenLog)
    {
        s_needOpenLog = false;
        exp_log("panel opened");
    }
    uintptr_t rootDbg = explorer_datamodel();
    poll_game_events(rootDbg);
    {
        static std::chrono::steady_clock::time_point lastSum{};
        auto nowSum = std::chrono::steady_clock::now();
        if (lastSum.time_since_epoch().count() == 0)
            lastSum = nowSum;
        if (nowSum - lastSum > std::chrono::seconds(5))
        {
            lastSum = nowSum;
            size_t snapN = 0;
            {
                auto sumSnap = s_snapAtomic.load();
                if (sumSnap)
                    snapN = sumSnap->size();
            }
            char b[256];
            std::snprintf(b, sizeof(b),
                "cache summary: snapHit=%lld snapMiss=%lld liveFallback=%lld ttlHit=%lld ttlMiss=%lld snapReady=%d snapN=%zu dm=0x%llX",
                s_dbgViewHit.load(), s_dbgViewMiss.load(), s_dbgLiveFallback.load(),
                s_dbgCacheHit.load(), s_dbgCacheMiss.load(),
                s_snapReady.load() ? 1 : 0, snapN, (unsigned long long)rootDbg);
            exp_log(b);
        }
    }
    if (!s_useFile && rootDbg)
        ensure_snapshot(rootDbg);
    if (rootDbg != s_lastRootDbg)
    {
        s_lastRootDbg = rootDbg;
        if (rootDbg)
        {
            char b[96];
            std::snprintf(b, sizeof(b), "datamodel resolved: 0x%llX (%d children)",
                (unsigned long long)rootDbg, (int)node{ rootDbg }.children().size());
            exp_log(b);
            set_status(" Datamodel OK");
        }
        else
        {
            exp_log("datamodel lost / not ready");
        }
    }
    ImFont* f = g_fontSmall ? g_fontSmall : ImGui::GetFont();
    const float fs = 13.0f;
    const ImU32 col_outline = IM_COL32(0, 0, 0, 255);
    const float left = pos.x + 14.0f;
    const float right = mx.x - 14.0f;
    float y = pos.y + 24.0f;
    DrawOutlinedText(dl, f, 12.0f, ImVec2(left, y), TextDimU32(), col_outline, "search");
    DrawOutlinedText(dl, f, 12.0f, ImVec2(left + 308.0f, y), TextDimU32(), col_outline, "filter");
    {
        std::string st = get_status();
        float avail = right - (left + 308.0f + 150.0f + 12.0f);
        if (avail > 30.0f && !st.empty())
        {
            ImVec2 ssz = f->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, st.c_str());
            if (ssz.x > avail)
            {
                size_t keep = (size_t)((float)st.size() * avail / ssz.x);
                if (keep > 3) st = st.substr(0, keep - 3) + "...";
                else st.clear();
                ssz = f->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, st.c_str());
            }
            if (!st.empty())
                DrawOutlinedText(dl, f, 12.0f, ImVec2(right - ssz.x, y), TextDimU32(), col_outline, st.c_str());
        }
    }
    y += 13.0f;
    DrawTextInput("##exp_search", dl, ImVec2(left, y), 300.0f, s_search, (int)sizeof(s_search),
        nullptr, "Search instances...", f, fs);
    static const char* const kFilterItems[] = { "All", "Scripts", "Parts", "Folders", "Remotes" };
    DrawDropdownSingle("##exp_filter", dl, ImVec2(left + 308.0f, y), 150.0f, &s_filter,
        kFilterItems, 5, nullptr, f, fs);
    y += 28.0f;
    const float gy0 = y;
    const float gy1 = mx.y - 10.0f;
    const float leftW = 392.0f;
    ImVec2 bmin(left, gy0), bmax(left + leftW, gy1);
    ImVec2 smin(left + leftW + 8.0f, gy0), smax(right, gy1);
    DrawGroupBox(dl, bmin, bmax, "browser", f, fs, GroupU32(), BorderU32(), col_outline);
    DrawGroupBox(dl, smin, smax, "selection", f, fs, GroupU32(), BorderU32(), col_outline);
    const ImVec2 treeMin(bmin.x + 8.0f, bmin.y + 20.0f);
    const ImVec2 treeMax(bmax.x - 8.0f, bmax.y - 8.0f);
    if (!g_texturesLoaded)
        load_all_icons();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GroupU32());
    ImGui::PushStyleColor(ImGuiCol_Header, CtrlBgU32());
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, CtrlHovU32());
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, AccentDimU32());
    ImGui::PushFont(f);
    ImGui::SetCursorScreenPos(treeMin);
    ImGui::BeginChild("##exp_tree", ImVec2(treeMax.x - treeMin.x, treeMax.y - treeMin.y),
        false, ImGuiWindowFlags_NoScrollbar);
    {
        bool searching = (s_search[0] != '\0' || s_filter != 0);
        if (searching)
        {
            ULONGLONG now = GetTickCount64();
            std::string q(s_search);
            if ((q != s_lastQuery || s_filter != s_lastFilter) && now - s_lastTrigger > 250)
            {
                s_lastTrigger = now;
                trigger_search(q, s_filter);
            }
            static std::vector<SearchResult> copy;
            static int seenVer = -1;
            static float seenW = -1.0f;
            int curVer = s_resultsVer.load();
            float rowWNow = ImGui::GetContentRegionAvail().x;
            if (rowWNow < 50.0f)
                rowWNow = 50.0f;
            if (curVer != seenVer || (seenW > 0.0f && fabsf(rowWNow - seenW) > 2.0f))
            {
                {
                    std::lock_guard<std::mutex> lk(s_resultsMtx);
                    copy = s_results;
                }
                seenVer = curVer;
                seenW = rowWNow;
                for (auto& r : copy)
                {
                    char top[256];
                    snprintf(top, sizeof(top), "%s [%s]", r.name.c_str(), r.cls.c_str());
                    r.top = top;
                    r.sub = trunc_fit(f, fs, r.path, rowWNow - 16.0f - 14.0f);
                }
            }
            if (s_searchRunning && copy.empty())
                ImGui::TextDisabled("Searching...");
            else if (copy.empty())
                ImGui::TextDisabled("No matches.");
            else
            {
                ImGui::TextDisabled("%zu results", copy.size());
                size_t shown = copy.size() > 400 ? 400 : copy.size();
                for (size_t k = 0; k < shown; ++k)
                {
                    auto& it = copy[k];
                    ImGui::PushID((int)k);
                    const float ric = 16.0f;
                    const float itemH = ImGui::GetFontSize() + 18.0f;
                    ImVec2 cur = ImGui::GetCursorScreenPos();
                    float rowW = ImGui::GetContentRegionAvail().x;
                    if (rowW < 50.0f)
                        rowW = 50.0f;
                    bool clicked = ImGui::Selectable("##res", s_selected == it.addr, 0,
                        ImVec2(rowW, itemH));
                    if (clicked)
                        s_selected = it.addr;
                    ImDrawList* rdl = ImGui::GetWindowDrawList();
                    IconTex* ricon = get_icon(it.cls);
                    if (ricon && ricon->tex)
                        rdl->AddImage((ImTextureID)ricon->tex, cur + ImVec2(4.0f, 4.0f),
                            cur + ImVec2(4.0f + ric, 4.0f + ric));
                    rdl->AddText(cur + ImVec2(ric + 10.0f, 2.0f), TextU32(), it.top.c_str());
                    rdl->AddText(cur + ImVec2(ric + 10.0f, ImGui::GetFontSize() + 4.0f),
                        TextDimU32(), it.sub.c_str());
                    ImGui::PopID();
                }
                if (copy.size() > shown)
                    ImGui::TextDisabled("... %zu more", copy.size() - shown);
            }
        }
        else if (rootDbg)
        {
            if (!s_useFile && !s_snapReady.load())
            {
                char sb[64];
                std::snprintf(sb, sizeof(sb), "scanning %zu...",
                    s_snapCount.load());
                ImGui::TextDisabled(sb);
            }
            else
            {
                s_rendered = 0;
                draw_node(rootDbg, 0);
            }
        }
        else if (!g_proc || !g_proc->connected())
        {
            ImGui::TextDisabled("Game not attached.");
        }
        else
        {
            ImGui::TextDisabled("Resolving datamodel...");
        }
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::PopStyleColor(4);
    float ry = smin.y + 20.0f;
    const float rx = smin.x + 8.0f;
    const float rw = (smax.x - 8.0f) - rx;
    auto row = [&](const char* k, const std::string& v, bool dimVal = false) {
        DrawOutlinedText(dl, f, 12.0f, ImVec2(rx, ry), TextDimU32(), col_outline, k);
        ry += 13.0f;
        std::string t = trunc_fit(f, fs, v.empty() ? "-" : v, rw);
        DrawOutlinedText(dl, f, fs, ImVec2(rx, ry),
            dimVal ? TextDimU32() : TextU32(), col_outline, t.c_str());
        ry += 16.0f;
    };
    bool offline = (s_useFile && s_fileRoot);
    node sel{ s_selected };
    NodeView sv;
    bool hasSel = false;
    if (s_selected != 0)
    {
        if (offline)
            hasSel = view_node(s_selected, sv);
        else if (g_proc && g_proc->connected())
            hasSel = view_node(s_selected, sv);
    }
    std::string selCls = hasSel ? sv.cls : "";
    std::string selName = hasSel ? sv.name : "";
    if (hasSel)
    {
        char addrBuf[32], kidBuf[16];
        snprintf(addrBuf, sizeof(addrBuf), "0x%llX", (unsigned long long)s_selected);
        snprintf(kidBuf, sizeof(kidBuf), "%d", (int)sv.kids.size());
        row("path", view_path(s_selected));
        row("name", selName);
        row("class", selCls);
        row("parent", view_parent_name(s_selected));
        row("address", addrBuf);
        row("children", kidBuf);
        ry -= 6.0f;
        const uintptr_t va = s_selected + Offsets::Value::Value;
        if (!offline && selCls == "NumberValue")
        {
            static uintptr_t lastNum = 0;
            static float numF = 0.0f, numLo = -1.0f, numHi = 1.0f;
            double cur = g_proc->read<double>(va);
            if (lastNum != s_selected)
            {
                lastNum = s_selected;
                numF = (float)cur;
                float span = (float)fabs(cur) * 2.0f;
                if (span < 1.0f)
                    span = 1.0f;
                numLo = (cur < 0.0 ? (float)cur : 0.0f) - span;
                numHi = (cur > 0.0 ? (float)cur : 0.0f) + span;
            }
            DrawOutlinedText(dl, f, 12.0f, ImVec2(rx, ry), TextDimU32(), col_outline, "value");
            ry += 13.0f;
            float before = numF;
            DrawSliderFloat("##exp_num", dl, ImVec2(rx, ry), rw, &numF, numLo, numHi,
                nullptr, "%.4f", f, fs);
            if (numF != before)
                g_proc->write<double>(va, (double)numF);
            ry += 20.0f;
        }
        else if (!offline && selCls == "IntValue")
        {
            static uintptr_t lastInt = 0;
            static float intF = 0.0f;
            int cur = g_proc->read<int>(va);
            if (lastInt != s_selected)
            {
                lastInt = s_selected;
                intF = (float)cur;
            }
            float lo = intF - 50.0f, hi = intF + 50.0f;
            if (hi - lo < 20.0f) { lo -= 10.0f; hi += 10.0f; }
            DrawOutlinedText(dl, f, 12.0f, ImVec2(rx, ry), TextDimU32(), col_outline, "value");
            ry += 13.0f;
            float before = intF;
            DrawSliderFloat("##exp_int", dl, ImVec2(rx, ry), rw, &intF, lo, hi,
                nullptr, "%.0f", f, fs);
            if (intF != before)
                g_proc->write<int>(va, (int)intF);
            ry += 20.0f;
        }
        else if (!offline && selCls == "BoolValue")
        {
            bool b = g_proc->read<uint8_t>(va) != 0;
            DrawCheckbox(dl, ImVec2(rx, ry), &b, "value", f, fs);
            g_proc->write<uint8_t>(va, b ? 1 : 0);
            ry += 20.0f;
        }
        ry += 4.0f;
    }
    else
    {
        row("path", "-", true);
        row("name", "-", true);
        row("class", "-", true);
        row("parent", "-", true);
        row("address", "-", true);
        row("children", "-", true);
        ry -= 6.0f;
        DrawOutlinedText(dl, f, 12.0f, ImVec2(rx, ry), TextDimU32(), col_outline,
            "select an instance");
        ry += 20.0f;
    }
    const float bh = 20.0f, gap = 6.0f;
    const float half = (rw - gap) * 0.5f;
    auto copy_btn = [&](const char* txt, float bx, float bw, const std::string& v) {
        if (DrawButton(dl, ImVec2(bx, ry), bw, bh, txt, f, fs) && !v.empty())
            ImGui::SetClipboardText(v.c_str());
    };
    if (hasSel)
    {
        char addrBuf[32];
        snprintf(addrBuf, sizeof(addrBuf), "0x%llX", (unsigned long long)s_selected);
        std::string selPath = view_path(s_selected);
        copy_btn("copy name", rx, half, selName);
        copy_btn("copy class", rx + half + gap, half, selCls);
        ry += bh + gap;
        copy_btn("copy path", rx, half, selPath);
        copy_btn("copy address", rx + half + gap, half, addrBuf);
        ry += bh + gap;
    }
    else
    {
        copy_btn("copy name", rx, half, "");
        copy_btn("copy class", rx + half + gap, half, "");
        ry += bh + gap;
        copy_btn("copy path", rx, half, "");
        copy_btn("copy address", rx + half + gap, half, "");
        ry += bh + gap;
    }
    if (DrawButton(dl, ImVec2(rx, ry), rw, bh,
            s_dumping.load() ? "dumping..." : "dump", f, fs) && !s_dumping.load())
    {
        if (offline)
            set_status("dump needs live game");
        else
            dump_all();
    }
    ry += bh + gap;
    {
        char lbl[48];
        if (s_imgDumping.load() && s_imgTotal.load() > 0)
            snprintf(lbl, sizeof(lbl), "images %d/%d", s_imgDone.load(), s_imgTotal.load());
        else if (s_imgDumping.load())
            snprintf(lbl, sizeof(lbl), "images...");
        else
            snprintf(lbl, sizeof(lbl), "images");
        if (DrawButton(dl, ImVec2(rx, ry), rw, bh, lbl, f, fs) && !s_imgDumping.load())
        {
            if (offline)
                set_status("images need live game");
            else
                dump_images();
        }
        ry += bh + gap;
    }
    bool wantRefresh = DrawButton(dl, ImVec2(rx, ry), half, bh,
        s_snapRunning.load() ? "scanning..." : "refresh", f, fs);
    bool wantLoad = DrawButton(dl, ImVec2(rx + half + gap, ry), half, bh,
        s_fileLoading.load() ? "loading..." : "load dump", f, fs);
    ry += bh + gap;
    if (wantRefresh)
    {
        if (offline)
            set_status("refresh needs live game");
        else
        {
            s_snapReady.store(false);
            s_snapDm = 0;
            {
                std::lock_guard<std::mutex> lk(s_snapPubMtx);
                s_snapAtomic.store(std::shared_ptr<const SnapMap>{});
            }
            set_status("rescanning...");
        }
    }
    if (wantLoad && !s_fileLoading.load())
    {
        std::string picked;
        if (pick_dump_file(picked))
            load_dump_file(picked);
        else
            set_status("pick cancelled");
    }
    if (!s_fileRoot)
        s_useFile = false;
    DrawCheckbox(dl, ImVec2(rx, ry), &s_useFile, "use dump", f, 12.0f);
    ry += 20.0f;
    {
        std::string src = "src: live game";
        if (s_fileRoot)
        {
            char nb[96];
            std::snprintf(nb, sizeof(nb), "src: %s (%zu)", s_fileName.c_str(), s_fileCount);
            src = offline ? nb : std::string("src: live game | ") + nb + " ready";
        }
        else if (offline)
            src = "src: live game";
        std::string t = trunc_fit(f, 11.0f, src, rw);
        DrawOutlinedText(dl, f, 11.0f, ImVec2(rx, ry), TextDimU32(), col_outline, t.c_str());
        ry += 15.0f;
    }
    if (ry + 28.0f <= smax.y - 6.0f)
    {
        size_t cacheN = 0;
        {
            std::lock_guard<std::mutex> lk(s_cacheMtx);
            cacheN = s_cache.size();
        }
        char d1[80], d2[80];
        std::snprintf(d1, sizeof(d1), "dm 0x%llX players %zu",
            (unsigned long long)s_lastRootDbg, s_knownPlayers.size());
        std::snprintf(d2, sizeof(d2), "cache %zu shown %d",
            cacheN, s_rendered);
        DrawOutlinedText(dl, f, 11.0f, ImVec2(rx, ry), TextDimU32(), col_outline, d1);
        DrawOutlinedText(dl, f, 11.0f, ImVec2(rx, ry + 13.0f), TextDimU32(), col_outline, d2);
    }
}
}
