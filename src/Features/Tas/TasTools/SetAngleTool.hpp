#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Features/Tas/TasTools/AngleToolsUtils.hpp"

using namespace AngleToolsUtils;

struct SetAngleParams : public TasToolParams {
	SetAngleParams()
		: TasToolParams() {}

	SetAngleParams(float pitch, float yaw, int easingTicks, EasingType easingType)
		: TasToolParams(true)
		, pitch(pitch)
		, yaw(yaw) 
		, easingTicks(easingTicks) 
		, easingType(easingType) 
		, elapsedTicks(0) {}

	float pitch;
	float yaw;

	int easingTicks;
	EasingType easingType;
	int elapsedTicks;
};

class SetAngleTool : public TasToolWithParams<SetAngleParams> {
public:
	SetAngleTool(int slot)
		: TasToolWithParams("setang", slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
};
