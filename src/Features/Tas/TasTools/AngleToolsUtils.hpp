#pragma once
#include <Utils/Math.hpp>

namespace AngleToolsUtils {
	enum EasingType {
		Linear,
		Sine,
		Cubic,
		Exponential,
	};

	float Ease(int slot, float t, EasingType easingType);
	EasingType ParseEasingType(std::string typeStr);
	Vector GetInterpolatedViewAnalog(int slot, Vector currentAngles, Vector targetAngles, int easingTicks, int elapsedTicks, EasingType easingType);
}
