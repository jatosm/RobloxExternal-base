#include "geo.h"
#include "../core/scan.h"
#include "../core/draw.h"
#include "../../../../menu/core/gui/menu.h"
#include "../../../../menu/overlay/settings.h"
#include "../../../../utility/struct/world2screen.h"
#include "../../../../utility/struct/offsets.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <queue>
#include <vector>
namespace thenwefuckin::worldgeo
{
    namespace
    {
        std::vector<Box> g_boxes;
        std::vector<Box> g_next;
        std::queue<std::uintptr_t> g_q;
        std::uintptr_t g_ws = 0;
        std::size_t g_scanned = 0;
        bool g_scanning = false;
        bool g_terrainFound = false;
        std::chrono::steady_clock::time_point g_lastFull{};
        constexpr std::size_t kSliceNodes = 500;
        constexpr std::size_t kMaxScan = 250000;
        constexpr std::size_t kMaxQueue = 150000;
        constexpr std::size_t kMaxBoxes = 60000;
        bool want_parse()
        {
            bool show = settings::world_geo_enabled;
            if (show && settings::world_geo_key != 0)
                show = Menu::IsMenuKeyActive("##geo_key", settings::world_geo_key, settings::world_geo_key_mode);
            return show || settings::aim_visible_check;
        }
        bool want_show()
        {
            bool show = settings::world_geo_enabled;
            if (show && settings::world_geo_key != 0)
                show = Menu::IsMenuKeyActive("##geo_key", settings::world_geo_key, settings::world_geo_key_mode);
            return show;
        }
        bool is_part_class(const std::string& cls)
        {
            return cls == "Part" || cls == "MeshPart" || cls == "WedgePart" ||
                cls == "CornerWedgePart" || cls == "TrussPart" || cls == "SpawnLocation" ||
                cls == "Seat" || cls == "VehicleSeat" || cls == "UnionOperation" ||
                cls == "PartOperation" || cls == "FlagStand" || cls == "SkateboardPlatform";
        }
        bool finite3(const math::vec3& v)
        {
            return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
        }
        void full_parse()
        {
            if (!g_ctx.workspace || !g_proc || !g_proc->connected())
            {
                g_scanning = false;
                return;
            }
            if (!g_scanning || g_ctx.workspace.addr != g_ws)
            {
                while (!g_q.empty())
                    g_q.pop();
                g_next.clear();
                g_next.reserve(4096);
                g_q.push(g_ctx.workspace.addr);
                g_ws = g_ctx.workspace.addr;
                g_scanned = 0;
                g_scanning = true;
                g_terrainFound = false;
            }
            unsigned long long t0 = GetTickCount64();
            std::size_t n = 0;
            while (!g_q.empty() && g_scanned < kMaxScan && g_next.size() < kMaxBoxes)
            {
                if (n >= kSliceNodes)
                    break;
                if (n > 0 && GetTickCount64() - t0 >= 12)
                    break;
                node cur{ g_q.front() };
                g_q.pop();
                ++g_scanned;
                ++n;
                if (!cur)
                    continue;
                std::string cls = cur.class_name();
                if (cls == "Terrain")
                {
                    g_terrainFound = true;
                    continue;
                }
                if (is_part_class(cls))
                {
                    uintptr_t prim = g_proc->read<uintptr_t>(cur.addr + Offsets::BasePart::Primitive);
                    if (prim)
                    {
                        uint8_t flags = Offsets::Primitive::PrimitiveFlags ? g_proc->read<uint8_t>(prim + Offsets::Primitive::PrimitiveFlags) : 0xFF;
                        math::frame f{};
                        if (g_proc->read_buf(prim + Offsets::Primitive::Rotation, &f, sizeof(f)))
                        {
                            math::vec3 sz = g_proc->read<math::vec3>(prim + Offsets::Primitive::Size);
                            if (finite3(f.pos) && finite3(sz) &&
                                sz.x > 0.0f && sz.y > 0.0f && sz.z > 0.0f &&
                                sz.x < 2000.0f && sz.y < 2000.0f && sz.z < 2000.0f)
                            {
                                math::vec3 ax0{ f.rot[0], f.rot[3], f.rot[6] };
                                math::vec3 ax1{ f.rot[1], f.rot[4], f.rot[7] };
                                math::vec3 ax2{ f.rot[2], f.rot[5], f.rot[8] };
                                float l0 = ax0.length(), l1 = ax1.length(), l2 = ax2.length();
                                if (l0 > 0.001f && l1 > 0.001f && l2 > 0.001f && finite3(ax0) && finite3(ax1) && finite3(ax2))
                                {
                                    Box b{};
                                    b.c = f.pos;
                                    b.ax[0] = ax0 / l0;
                                    b.ax[1] = ax1 / l1;
                                    b.ax[2] = ax2 / l2;
                                    b.h[0] = sz.x * 0.5f;
                                    b.h[1] = sz.y * 0.5f;
                                    b.h[2] = sz.z * 0.5f;
                                    b.solid = (flags & Offsets::PrimitiveFlags::CanCollide) != 0;
                                    b.prim = prim;
                                    float longest = sz.x;
                                    if (sz.y > longest) longest = sz.y;
                                    if (sz.z > longest) longest = sz.z;
                                    float shortest = sz.x;
                                    if (sz.y < shortest) shortest = sz.y;
                                    if (sz.z < shortest) shortest = sz.z;
                                    if (cls == "Part" && Offsets::BasePart::Shape)
                                    {
                                        int shape = g_proc->read<int>(cur.addr + Offsets::BasePart::Shape);
                                        if (shape == 0)
                                            b.radius = longest * 0.5f;
                                    }
                                    else if (cls == "MeshPart" && longest > 0.0f && (longest - shortest) / longest < 0.05f)
                                    {
                                        std::string nm = cur.name();
                                        for (auto& ch : nm) ch = (char)tolower((unsigned char)ch);
                                        if (nm.find("ball") != std::string::npos ||
                                            nm.find("sphere") != std::string::npos ||
                                            nm.find("orb") != std::string::npos)
                                            b.radius = longest * 0.5f;
                                    }
                                    g_next.push_back(b);
                                }
                            }
                        }
                    }
                }
                if (cls == "Workspace" || cls == "Folder" || cls == "Model")
                {
                    for (auto& k : cur.children())
                    {
                        if (g_q.size() > kMaxQueue)
                            break;
                        g_q.push(k.addr);
                    }
                }
            }
            if (!g_q.empty() && g_scanned < kMaxScan && g_next.size() < kMaxBoxes)
                return;
            g_boxes.swap(g_next);
            g_scanning = false;
            g_lastFull = std::chrono::steady_clock::now();
        }
        bool seg_hits_box(const math::vec3& from, const math::vec3& dir, float len, const Box& b)
        {
            math::vec3 p{ from.x - b.c.x, from.y - b.c.y, from.z - b.c.z };
            float t0 = 0.0f;
            float t1 = len;
            for (int i = 0; i < 3; ++i)
            {
                float e = b.ax[i].x * p.x + b.ax[i].y * p.y + b.ax[i].z * p.z;
                float f = b.ax[i].x * dir.x + b.ax[i].y * dir.y + b.ax[i].z * dir.z;
                float h = b.h[i];
                if (fabsf(f) < 1e-9f)
                {
                    if (e < -h - 1e-4f || e > h + 1e-4f)
                        return false;
                    continue;
                }
                float ta = (-h - e) / f;
                float tb = (h - e) / f;
                if (ta > tb) { float t = ta; ta = tb; tb = t; }
                if (ta > t0) t0 = ta;
                if (tb < t1) t1 = tb;
                if (t0 > t1)
                    return false;
            }
            return t0 < len - 1.0f && t1 > 1.5f;
        }
    }
    void clear()
    {
        g_boxes.clear();
        g_next.clear();
        while (!g_q.empty())
            g_q.pop();
        g_ws = 0;
        g_scanned = 0;
        g_scanning = false;
        g_terrainFound = false;
        g_lastFull = std::chrono::steady_clock::time_point{};
    }
    const std::vector<Box>& boxes()
    {
        return g_boxes;
    }
    bool terrain_found()
    {
        return g_terrainFound;
    }
    bool visible(const math::vec3& from, const math::vec3& to)
    {
        math::vec3 d{ to.x - from.x, to.y - from.y, to.z - from.z };
        float len = d.length();
        if (len < 1e-6f)
            return true;
        if (!finite3(from) || !finite3(to))
            return true;
        math::vec3 dir = d / len;
        for (auto& b : g_boxes)
        {
            if (!b.solid || !b.prim)
                continue;
            math::vec3 to_c{ b.c.x - from.x, b.c.y - from.y, b.c.z - from.z };
            float along = to_c.x * dir.x + to_c.y * dir.y + to_c.z * dir.z;
            if (along < -2.0f || along > len + 2.0f)
                continue;
            float cl = along < 0.0f ? 0.0f : (along > len ? len : along);
            float cx = from.x + dir.x * cl;
            float cy = from.y + dir.y * cl;
            float cz = from.z + dir.z * cl;
            float dx = b.c.x - cx, dy = b.c.y - cy, dz = b.c.z - cz;
            float bound = sqrtf(b.h[0] * b.h[0] + b.h[1] * b.h[1] + b.h[2] * b.h[2]) + 0.5f;
            if (dx * dx + dy * dy + dz * dz > bound * bound)
                continue;
            math::vec3 live_c = g_proc->read<math::vec3>(b.prim + Offsets::Primitive::Position);
            if (live_c.x == 0.0f && live_c.y == 0.0f && live_c.z == 0.0f)
                continue;
            if (!finite3(live_c))
                continue;
            Box live = b;
            live.c = live_c;
            if (seg_hits_box(from, dir, len, live))
                return false;
        }
        return true;
    }
    void tick(ImDrawList* dl, int w, int h)
    {
        if (!dl)
            return;
        if (!g_proc || !g_proc->connected() || !g_ctx.workspace)
            return;
        if (!want_parse())
            return;
        if (g_scanning)
        {
            full_parse();
        }
        else if (g_lastFull == std::chrono::steady_clock::time_point{} ||
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - g_lastFull).count() >= 2500)
        {
            full_parse();
        }
        if (!want_show() || !g_ctx.camera)
            return;

        math::view_matrix view = thenwefuckin::visual::wcache::viewValid
            ? thenwefuckin::visual::wcache::view
            : math::view_matrix{};
        if (!thenwefuckin::visual::wcache::viewValid && !world::fetch_view(view))
            return;
        math::vec3 camPos = g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        if (camPos.x == 0.0f && camPos.y == 0.0f && camPos.z == 0.0f)
            return;
        math::vec3 camRight{ 1.0f, 0.0f, 0.0f };
        {
            math::frame cf = g_proc->read<math::frame>(g_ctx.camera.addr + Offsets::Camera::Rotation);
            math::vec3 r{ cf.rot[0], cf.rot[3], cf.rot[6] };
            if (r.length() > 0.000001f)
                camRight = r.normalized();
        }
        float fw = static_cast<float>(w);
        float fh = static_cast<float>(h);
        const ImU32 col = IM_COL32(160, 160, 160, 255);
        std::size_t drawn = 0;
        constexpr std::size_t kMaxDraw = 400;
        constexpr float kMaxDrawDist = 600.0f;
        for (auto& b : g_boxes)
        {
            if (drawn >= kMaxDraw)
                break;
            math::vec3 dc{ b.c.x - camPos.x, b.c.y - camPos.y, b.c.z - camPos.z };
            float br = sqrtf(b.h[0] * b.h[0] + b.h[1] * b.h[1] + b.h[2] * b.h[2]);
            if (dc.length() - br > kMaxDrawDist)
                continue;
            if (b.radius > 0.0f)
            {
                math::vec2 cc{};
                if (!projection::to_screen(b.c, view, fw, fh, cc))
                    continue;
                if (cc.x < -12.0f || cc.x > fw + 12.0f || cc.y < -12.0f || cc.y > fh + 12.0f)
                    continue;
                math::vec3 ep{
                    b.c.x + camRight.x * b.radius,
                    b.c.y + camRight.y * b.radius,
                    b.c.z + camRight.z * b.radius
                };
                math::vec2 ep2d{};
                if (!projection::to_screen(ep, view, fw, fh, ep2d))
                    continue;
                float rr = (ep2d - cc).length();
                if (rr < 2.0f || rr > 4000.0f)
                    continue;
                dl->AddCircle(ImVec2(cc.x, cc.y), rr, col, 40);
                ++drawn;
                continue;
            }
            math::vec3 corners[8];
            int ci = 0;
            for (int sx = -1; sx <= 1; sx += 2)
                for (int sy = -1; sy <= 1; sy += 2)
                    for (int sz = -1; sz <= 1; sz += 2)
                    {
                        if (ci >= 8) break;
                        corners[ci] = math::vec3{
                            b.c.x + b.ax[0].x * (sx * b.h[0]) + b.ax[1].x * (sy * b.h[1]) + b.ax[2].x * (sz * b.h[2]),
                            b.c.y + b.ax[0].y * (sx * b.h[0]) + b.ax[1].y * (sy * b.h[1]) + b.ax[2].y * (sz * b.h[2]),
                            b.c.z + b.ax[0].z * (sx * b.h[0]) + b.ax[1].z * (sy * b.h[1]) + b.ax[2].z * (sz * b.h[2])
                        };
                        ++ci;
                    }
            math::vec2 sp[8];
            bool ok[8]{};
            int okCount = 0;
            for (int k = 0; k < 8; ++k)
            {
                if (projection::to_screen(corners[k], view, fw, fh, sp[k]))
                {
                    ok[k] = true;
                    ++okCount;
                }
            }
            if (okCount < 2)
                continue;
            constexpr int edges[12][2] = {
                {0,1},{0,2},{1,3},{2,3},
                {4,5},{4,6},{5,7},{6,7},
                {0,4},{1,5},{2,6},{3,7}
            };
            for (auto& e : edges)
            {
                if (!ok[e[0]] || !ok[e[1]])
                    continue;
                dl->AddLine(ImVec2(sp[e[0]].x, sp[e[0]].y), ImVec2(sp[e[1]].x, sp[e[1]].y), col, 1.0f);
            }
            ++drawn;
        }
    }
}
