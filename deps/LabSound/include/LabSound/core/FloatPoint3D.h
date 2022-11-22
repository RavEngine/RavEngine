#ifndef FLOAT_POINT_3D_H
#define FLOAT_POINT_3D_H

#include <float.h>
#include <math.h>

namespace lab
{
struct float3
{
    float x {0.f}, y {0.f}, z {0.f};
    float3(float x, float y, float z) : x(x), y(y), z(z) {};
    float3() = default;
    float & operator[](int i) { return (&x)[i]; }
    const float & operator[](int i) const { return (&x)[i]; }
};

inline bool operator==(const float3 & a, const float3 & b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline bool operator!=(const float3 & a, const float3 & b)
{
    return !(a == b);
}

inline float3 operator+(const float3 & a, const float3 & b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline float3 operator-(const float3 & v)
{
    return {-v.x, -v.y, -v.z};
}

inline float3 operator-(const float3 & a, const float3 & b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline float3 operator*(const float3 & v, float s)
{
    return {v.x * s, v.y * s, v.z * s};
}

inline float3 operator*(float s, const float3 & v)
{
    return v * s;
}

inline float3 operator/(const float3 & v, float s)
{
    return v * (1.0f / s);
}

inline float3 operator+=(float3 & a, const float3 & b)
{
    return a = a + b;
}

inline float3 operator-=(float3 & a, const float3 & b)
{
    return a = a - b;
}

inline float3 operator*=(float3 & v, const float & s)
{
    return v = v * s;
}

inline float3 operator/=(float3 & v, const float & s)
{
    return v = v / s;
}

inline float dot(const float3 & a, const float3 & b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float3 cross(const float3 & a, const float3 & b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline float magnitude(const float3 & v)
{
    return sqrtf(dot(v, v));
}

inline float3 normalize(const float3 & v)
{
    return v / magnitude(v);
}

inline bool is_zero(const float3 & v)
{
    return fabsf(v.x) < FLT_EPSILON && fabsf(v.y) < FLT_EPSILON && fabsf(v.z) < FLT_EPSILON;
}

typedef float3 FloatPoint3D;

}

#endif
