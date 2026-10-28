#include "tracking.h"
#include <cmath>
namespace thenwefuckin::aim::tracking
{
    namespace
    {
        constexpr size_t kMaxSamples = 14;
        constexpr long long kWindowMs = 450;
        constexpr size_t kMaxTargets = 48;
        std::unordered_map<uintptr_t, std::deque<Sample>>& store()
        {
            static std::unordered_map<uintptr_t, std::deque<Sample>> s;
            return s;
        }
    }
    void push(uintptr_t key, float x, float y, float z)
    {
        if (!key)
            return;
        auto now = std::chrono::steady_clock::now();
        auto& s = store();
        auto& dq = s[key];
        dq.push_back(Sample{ x, y, z, now });
        while (dq.size() > kMaxSamples)
            dq.pop_front();
        if (s.size() > kMaxTargets)
        {
            for (auto it = s.begin(); it != s.end();)
            {
                if (it->first != key && !it->second.empty()
                    && std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.back().t).count() > 2000)
                    it = s.erase(it);
                else
                    ++it;
            }
        }
    }
    void clear(uintptr_t key)
    {
        store().erase(key);
    }
    void clear_all()
    {
        store().clear();
    }
    bool sample(uintptr_t key, Velocity& out)
    {
        auto it = store().find(key);
        if (it == store().end())
            return false;
        auto& dq = it->second;
        auto now = std::chrono::steady_clock::now();
        while (!dq.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(now - dq.front().t).count() > kWindowMs)
            dq.pop_front();
        size_t n = dq.size();
        if (n < 3)
            return false;
        double st = 0.0, stt = 0.0;
        double sx = 0.0, sy = 0.0, sz = 0.0;
        double stx = 0.0, sty = 0.0, stz = 0.0;
        bool first = true;
        double base = 0.0;
        for (auto& s : dq)
        {
            double t = std::chrono::duration<double>(s.t - dq.front().t).count();
            if (first) { base = t; first = false; }
            double rt = t - base;
            st += rt; stt += rt * rt;
            sx += s.x; sy += s.y; sz += s.z;
            stx += rt * s.x; sty += rt * s.y; stz += rt * s.z;
        }
        double denom = (double)n * stt - st * st;
        if (fabs(denom) < 1e-6)
            return false;
        out.vx = (float)(((double)n * stx - st * sx) / denom);
        out.vy = (float)(((double)n * sty - st * sy) / denom);
        out.vz = (float)(((double)n * stz - st * sz) / denom);
        float hSpeed = sqrtf(out.vx * out.vx + out.vz * out.vz);
        if (hSpeed < 2.0f && fabsf(out.vy) < 2.0f)
        {
            out.vx = 0.0f;
            out.vy = 0.0f;
            out.vz = 0.0f;
        }
        float yMin = dq.front().y, yMax = dq.front().y;
        for (auto& s : dq)
        {
            if (s.y < yMin) yMin = s.y;
            if (s.y > yMax) yMax = s.y;
        }
        out.airborne = (fabsf(out.vy) > 6.0f) || ((yMax - yMin) > 3.5f);
        out.damp = 1.0f;
        if (n >= 5)
        {
            size_t half = n / 2;
            double ax = 0.0, az = 0.0, bx = 0.0, bz = 0.0;
            size_t ca = 0, cb = 0;
            for (size_t i = 1; i < n; ++i)
            {
                double dt = std::chrono::duration<double>(dq[i].t - dq[i - 1].t).count();
                if (dt < 1e-4)
                    continue;
                float vx = (dq[i].x - dq[i - 1].x) / (float)dt;
                float vz = (dq[i].z - dq[i - 1].z) / (float)dt;
                if (i <= half) { ax += vx; az += vz; ++ca; }
                else { bx += vx; bz += vz; ++cb; }
            }
            if (ca && cb)
            {
                double al = sqrt(ax * ax + az * az);
                double bl = sqrt(bx * bx + bz * bz);
                if (al > 1.0 && bl > 1.0)
                {
                    double cosA = (ax * bx + az * bz) / (al * bl);
                    if (cosA < 1.0)
                    {
                        double d = 0.45 + 0.55 * (cosA * 0.5 + 0.5);
                        if (d < 0.45) d = 0.45;
                        if (d > 1.0) d = 1.0;
                        out.damp = (float)d;
                    }
                }
            }
        }
        return true;
    }
}
