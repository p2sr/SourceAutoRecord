#include "AngleToolsUtils.hpp"

#include "Features/Tas/TasPlayer.hpp"

namespace AngleToolsUtils {
	float Ease(int slot, float t, EasingType easingType) {
		if (t == 0.0f || t == 1.0f) return t;

		switch (easingType) {
		case Sine:
			return (tasCosf(M_PI * t) - 1.0f) / -2.0f;
		case Cubic:
			return t < 0.5f ? 4.0f * t * t * t : 1.0f - tasPowf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
		case Exponential:
			return t < 0.5f ? tasPowf(2.0f, 20.0f * t - 10.0f) / 2 : (2 - tasPowf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
		default:
			return t;
		}
	}

	EasingType ParseEasingType(std::string typeStr) {
		if (typeStr.length() == 0 || typeStr == "linear") {
			return Linear;
		} else if (typeStr == "sine" || typeStr == "sin") {
			return Sine;
		} else if (typeStr == "cubic") {
			return Cubic;
		} else if (typeStr == "exponential" || typeStr == "exp") {
			return Exponential;
		} else {
			throw 0;
		}
	}

	Vector GetInterpolatedViewAnalog(int slot, Vector currentAngles, Vector targetAngles, int easingTicks, int elapsedTicks, EasingType easingType) {
		Vector requiredDelta = currentAngles - targetAngles;

		while (requiredDelta.y < 0.0f) requiredDelta.y += 360.0f;
		if (requiredDelta.y > 180.0f) requiredDelta.y -= 360.0f;

		float pitchDelta, yawDelta;

		if (easingType == Linear || elapsedTicks >= easingTicks - 1) {
			int remaining = std::max(easingTicks - elapsedTicks, 1);

			pitchDelta = requiredDelta.x / remaining;
			yawDelta = requiredDelta.y / remaining;
		} else {
			float realFactorNow = elapsedTicks / (float)easingTicks;
			float realFactorNext = (elapsedTicks + 1) / (float)easingTicks;
			float easeFactorNow = Ease(slot, realFactorNow, easingType);
			float easeFactorNext = Ease(slot, realFactorNext, easingType);

			// how much of the remaining angle distance we should move
			float partFactor = (easeFactorNext - easeFactorNow) / (1.0f - easeFactorNow);

			pitchDelta = requiredDelta.x * partFactor;
			yawDelta = requiredDelta.y * partFactor;
		}

		return Vector{yawDelta, pitchDelta};
	}
}
