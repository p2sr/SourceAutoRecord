#pragma once
#include <Utils/Math.hpp>

namespace AngleToolsUtils {
	enum EasingType {
		Linear,
		Sine,
		Cubic,
		Exponential,
	};

	float Ease(float t, EasingType easingType);
	EasingType ParseEasingType(std::string typeStr);
	Vector GetInterpolatedViewAnalog(Vector currentAngles, Vector targetAngles, int easingTicks, int elapsedTicks, EasingType easingType);
}
