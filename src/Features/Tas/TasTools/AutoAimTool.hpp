#pragma once

#include "../TasTool.hpp"
#include "Utils/SDK/Math.hpp"
#include "Features/Tas/TasTools/AngleToolsUtils.hpp"

using namespace AngleToolsUtils;

struct AutoAimParams : public TasToolParams {
	AutoAimParams()
		: TasToolParams(false) {}

	AutoAimParams(Vector point, int easingTicks, EasingType easingType)
		: TasToolParams(true)
		, entity(false)
		, point(point)
		, easingTicks(easingTicks)
		, easingType(easingType)
		, elapsedTicks(0) {}

	AutoAimParams(std::string selector, int easingTicks, EasingType easingType)
		: TasToolParams(true)
		, entity(true)
		, ent_selector(selector)
		, easingTicks(easingTicks)
		, easingType(easingType)
		, elapsedTicks(0) {}

	bool entity;
	std::string ent_selector;
	Vector point;
	int easingTicks;
	EasingType easingType;
	int elapsedTicks;
};

class AutoAimTool : public TasToolWithParams<AutoAimParams> {
public:
	AutoAimTool(int slot)
		: TasToolWithParams("autoaim", slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};
