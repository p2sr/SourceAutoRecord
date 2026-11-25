#include "TasUtils.hpp"

Vector TasUtils::GetVelocityAngles(const TasPlayerInfo *pi) {
	auto velocityAngles = pi->velocity;
	if (velocityAngles.Length() == 0) {
		return {0, 0, 0};
	}

	Math::VectorNormalize(velocityAngles);

	int slot = pi->slot;
	float yaw = tasAtan2f(velocityAngles.y, velocityAngles.x);
	float pitch = tasAtan2f(velocityAngles.z, sqrtf(velocityAngles.y * velocityAngles.y + velocityAngles.x * velocityAngles.x));

	return {RAD2DEG(yaw), RAD2DEG(pitch), 0};
}
