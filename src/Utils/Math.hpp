#pragma once
#include <cmath>
#include <float.h>

#include "SDK.hpp"

#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

namespace Math {

inline void SinCos(float radians, float* sine, float* cosine)
{
#ifdef _WIN32
    _asm {
        fld DWORD PTR[radians]
        fsincos

        mov edx, DWORD PTR[cosine]
        mov eax, DWORD PTR[sine]

        fstp DWORD PTR[edx]
        fstp DWORD PTR[eax]
    }
#else
    register double __cosr, __sinr;
    __asm("fsincos"
          : "=t"(__cosr), "=u"(__sinr)
          : "0"(radians));

    *sine = __sinr;
    *cosine = __cosr;
#endif
}

inline void AngleVectors(const QAngle& angles, Vector* forward)
{
    float sp, sy, cp, cy;

    SinCos(DEG2RAD(angles.y), &sy, &cy);
    SinCos(DEG2RAD(angles.x), &sp, &cp);

    forward->x = cp * cy;
    forward->y = cp * sy;
    forward->z = -sp;
}

inline float AngleNormalize(float angle)
{
    angle = fmodf(angle, 360.0f);
    if (angle > 180) {
        angle -= 360;
    }
    if (angle < -180) {
        angle += 360;
    }
    return angle;
}

inline float VectorNormalize(Vector& vec)
{
    auto radius = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    auto iradius = 1.f / (radius + FLT_EPSILON);

    vec.x *= iradius;
    vec.y *= iradius;
    vec.z *= iradius;

    return radius;
}

inline void VectorAdd(const Vector& a, const Vector& b, Vector& c)
{
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
}

inline void AngleVectors(const QAngle& angles, Vector* forward, Vector* right, Vector* up)
{
    float sr, sp, sy, cr, cp, cy;

    SinCos(DEG2RAD(angles.x), &sy, &cy);
    SinCos(DEG2RAD(angles.y), &sp, &cp);
    SinCos(DEG2RAD(angles.z), &sr, &cr);

    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if (right) {
        right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
        right->y = (-1 * sr * sp * sy + -1 * cr * cy);
        right->z = -1 * sr * cp;
    }

    if (up) {
        up->x = (cr * sp * cy + -sr * -sy);
        up->y = (cr * sp * sy + -sr * cy);
        up->z = cr * cp;
    }
}

inline void VectorScale(Vector const& src, float b, Vector& dst)
{
    dst.x = src.x * b;
    dst.y = src.y * b;
    dst.z = src.z * b;
}

inline void VectorCopy(const Vector& src, Vector& dst)
{
    dst.x = src.x;
    dst.y = src.y;
    dst.z = src.z;
}
}