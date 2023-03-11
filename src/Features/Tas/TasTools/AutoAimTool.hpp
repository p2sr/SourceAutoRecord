#pragma once

#include "../TasTool.hpp"
#include "Utils/SDK/Math.hpp"
#include "Features/Tas/TasTools/AngleToolsUtils.hpp"

struct AutoAimParams : public TasToolParams {
	AutoAimParams()
		: TasToolParams(false) {}

	AutoAimParams(Vector point, int easingTicks, AngleToolsUtils::EasingType easingType)
		: TasToolParams(true)
		, entity(false)
		, point(point)
		, easingTicks(easingTicks)
		, easingType(easingType) {}

	AutoAimParams(std::string selector, int easingTicks, AngleToolsUtils::EasingType easingType)
		: TasToolParams(true)
		, entity(true)
		, ent_selector(selector)
		, easingTicks(easingTicks)
		, easingType(easingType) {}

	bool entity;
	std::string ent_selector;
	Vector point;
	int easingTicks;
	AngleToolsUtils::EasingType easingType;
};

class AutoAimTool : public TasToolWithParams<AutoAimParams> {
private:
	int elapsedTicks;
public:
	AutoAimTool(int slot)
		: TasToolWithParams("autoaim", slot)
		, elapsedTicks(0) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};
