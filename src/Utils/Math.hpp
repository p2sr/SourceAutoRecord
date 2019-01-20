#pragma once
#include "SDK.hpp"

#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

namespace Math {

void SinCos(float radians, float* sine, float* cosine);
float AngleNormalize(float angle);
float VectorNormalize(Vector& vec);
void VectorAdd(const Vector& a, const Vector& b, Vector& c);
void AngleVectors(const QAngle& angles, Vector* forward);
void AngleVectors(const QAngle& angles, Vector* forward, Vector* right, Vector* up);
void VectorScale(Vector const& src, float b, Vector& dst);
void VectorCopy(const Vector& src, Vector& dst);
}

inline void Math::SinCos(float radians, float* sine, float* cosine)
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
inline void Math::VectorAdd(const Vector& a, const Vector& b, Vector& c)
{
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
}
inline void Math::VectorScale(Vector const& src, float b, Vector& dst)
{
    dst.x = src.x * b;
    dst.y = src.y * b;
    dst.z = src.z * b;
}
inline void Math::VectorCopy(const Vector& src, Vector& dst)
{
    dst.x = src.x;
    dst.y = src.y;
    dst.z = src.z;
}
