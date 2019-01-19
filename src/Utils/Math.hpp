#pragma once
#include "SDK.hpp"

#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

namespace Math {

void SinCos(float radians, float* sine, float* cosine);

void AngleVectors(const QAngle& angles, Vector* forward);
float AngleNormalize(float angle);

float VectorNormalize(Vector& vec);
void VectorAdd(const Vector& a, const Vector& b, Vector& c);
void AngleVectors(const QAngle& angles, Vector* forward, Vector* right, Vector* up);
void VectorScale(Vector const& src, float b, Vector& dst);
void VectorCopy(const Vector& src, Vector& dst);
}
