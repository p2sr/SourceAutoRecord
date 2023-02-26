#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"


class SetAngleTool : public TasTool {
private:
	int elapsedTicks;
public:
	SetAngleTool(int slot)
		: TasTool("setang", slot)
		, elapsedTicks(0) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual void Reset();
};
