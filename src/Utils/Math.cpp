#include "Math.hpp"

#include <cmath>
#include <float.h>

#include "SDK.hpp"

float Math::AngleNormalize(float angle)
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
float Math::VectorNormalize(Vector& vec)
{
    auto radius = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    auto iradius = 1.f / (radius + FLT_EPSILON);

    vec.x *= iradius;
    vec.y *= iradius;
    vec.z *= iradius;

    return radius;
}
void Math::AngleVectors(const QAngle& angles, Vector* forward)
{
    float sp, sy, cp, cy;

    Math::SinCos(DEG2RAD(angles.y), &sy, &cy);
    Math::SinCos(DEG2RAD(angles.x), &sp, &cp);

    forward->x = cp * cy;
    forward->y = cp * sy;
    forward->z = -sp;
}
void Math::AngleVectors(const QAngle& angles, Vector* forward, Vector* right, Vector* up)
{
    float sr, sp, sy, cr, cp, cy;

    Math::SinCos(DEG2RAD(angles.x), &sy, &cy);
    Math::SinCos(DEG2RAD(angles.y), &sp, &cp);
    Math::SinCos(DEG2RAD(angles.z), &sr, &cr);

    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if (right) {
        right->x = -1 * sr * sp * cy + -1 * cr * -sy;
        right->y = -1 * sr * sp * sy + -1 * cr * cy;
        right->z = -1 * sr * cp;
    }

    if (up) {
        up->x = cr * sp * cy + -sr * -sy;
        up->y = cr * sp * sy + -sr * cy;
        up->z = cr * cp;
    }
}
