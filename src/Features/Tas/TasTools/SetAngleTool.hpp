#pragma once

#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Features/Tas/TasTools/AngleToolsUtils.hpp"

struct SetAngleParams : public TasToolParams {
	SetAngleParams()
		: TasToolParams() {}

	SetAngleParams(float pitch, float yaw, int easingTicks, AngleToolsUtils::EasingType easingType)
		: TasToolParams(true)
		, pitch(pitch)
		, yaw(yaw) 
		, easingTicks(easingTicks) 
		, easingType(easingType) {}

	float pitch;
	float yaw;

	int easingTicks;
	AngleToolsUtils::EasingType easingType;
};

class SetAngleTool : public TasToolWithParams<SetAngleParams> {
private:
	int elapsedTicks;
public:
	SetAngleTool(int slot)
		: TasToolWithParams("setang", slot)
		, elapsedTicks(0) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
};
