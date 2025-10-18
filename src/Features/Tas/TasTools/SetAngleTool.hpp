#pragma once

#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Features/Tas/TasTools/AngleToolsUtils.hpp"

struct SetAngleParams : public TasToolParams {
	SetAngleParams()
		: TasToolParams() {}

	SetAngleParams(std::string target, float pitch, float yaw, int easingTicks, AngleToolsUtils::EasingType easingType)
		: TasToolParams(true)
		, target(target)
		, pitch(pitch)
		, yaw(yaw) 
		, easingTicks(easingTicks) 
		, easingType(easingType) {}

	std::string target;
	float pitch;
	float yaw;

	int easingTicks;
	AngleToolsUtils::EasingType easingType;

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<SetAngleParams *>(&rhs_base);
		return TasToolParams::operator==(rhs) 
			&& std::tie(this->target, this->pitch, this->yaw, this->easingTicks, this->easingType) 
			== std::tie(rhs.target, rhs.pitch, rhs.yaw, rhs.easingTicks, rhs.easingType);
	}
};

class SetAngleTool : public TasToolWithParams<SetAngleParams> {
private:
	int elapsedTicks;
public:
	SetAngleTool(int slot)
		: TasToolWithParams("setang", POST_PROCESSING, slot)
		, elapsedTicks(0) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};
