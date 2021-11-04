#include "TasUtils.hpp"

Vector TasUtils::GetVelocityAngles(const TasPlayerInfo *pi) {
	auto velocityAngles = pi->velocity;
	if (velocityAngles.Length() == 0) {
		return {0, 0, 0};
	}

	Math::VectorNormalize(velocityAngles);

	float yaw = atan2f(velocityAngles.y, velocityAngles.x);
	float pitch = atan2f(velocityAngles.z, sqrtf(velocityAngles.y * velocityAngles.y + velocityAngles.x * velocityAngles.x));

	return {RAD2DEG(yaw), RAD2DEG(pitch), 0};
}