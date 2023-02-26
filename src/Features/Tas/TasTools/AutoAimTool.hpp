#pragma once

#include "../TasTool.hpp"

class AutoAimTool : public TasTool {
private:
	int elapsedTicks;
public:
	AutoAimTool(int slot)
		: TasTool("autoaim", slot)
		, elapsedTicks(0) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual void Reset();
};
