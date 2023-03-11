#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct DecelParams : public TasToolParams {
	DecelParams()
		: TasToolParams() {}

	DecelParams(float targetVel)
		: TasToolParams(true)
		, targetVel(targetVel) {}

	float targetVel;
};


class DecelTool : public TasToolWithParams<DecelParams> {
public:
	DecelTool(int slot)
		: TasToolWithParams("decel", slot) {}
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
};
