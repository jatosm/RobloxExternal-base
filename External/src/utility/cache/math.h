#pragma once
#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstdio>
namespace thenwefuckin::math
{
    struct vec2
    {
        float x = 0.f, y = 0.f;
        vec2() = default;
        vec2(float _x, float _y) : x(_x), y(_y) {}
        vec2 operator+(const vec2& o) const { return { x + o.x, y + o.y }; }
        vec2 operator-(const vec2& o) const { return { x - o.x, y - o.y }; }
        vec2 operator*(float s) const { return { x * s, y * s }; }
        float length() const { return sqrtf(x * x + y * y); }
        float dist_to(const vec2& o) const { return (*this - o).length(); }
    };
    struct vec3
    {
        float x = 0.f, y = 0.f, z = 0.f;
        vec3() = default;
        vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
        vec3 operator+(const vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
        vec3 operator-(const vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
        vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
        vec3 operator/(float s) const { return { x / s, y / s, z / s }; }
        vec3& operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
        float dot(const vec3& o) const { return x * o.x + y * o.y + z * o.z; }
        float length() const { return sqrtf(x * x + y * y + z * z); }
        float dist_to(const vec3& o) const { return (*this - o).length(); }
        vec3 normalized() const
        {
            float l = length();
            return l > 0.0001f ? (*this / l) : vec3{};
        }
    };
    struct vec4
    {
        float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
        vec4() = default;
        vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    };
    struct view_matrix
    {
        float m[16]{};
        vec4 transform(const vec3& p) const
        {
            return {
                p.x * m[0] + p.y * m[1] + p.z * m[2] + m[3],
                p.x * m[4] + p.y * m[5] + p.z * m[6] + m[7],
                p.x * m[8] + p.y * m[9] + p.z * m[10] + m[11],
                p.x * m[12] + p.y * m[13] + p.z * m[14] + m[15]
            };
        }
    };
    struct frame
    {
        float rot[9]{};
        vec3 pos{};
        vec3 right() const { return { rot[0], rot[3], rot[6] }; }
        vec3 up() const { return { rot[1], rot[4], rot[7] }; }
        vec3 look() const { return { rot[2], rot[5], rot[8] }; }
    };
    struct rgb
    {
        float r = 0.f, g = 0.f, b = 0.f;
        rgb() = default;
        rgb(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
        static rgb from_255(float _r, float _g, float _b)
        {
            return { _r / 255.f, _g / 255.f, _b / 255.f };
        }
    };
    inline float clamp_f(float v, float lo, float hi)
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }
}
