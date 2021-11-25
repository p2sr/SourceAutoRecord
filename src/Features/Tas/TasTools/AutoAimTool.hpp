#pragma once

#include "../TasTool.hpp"

class AutoAimTool : public TasTool {
public:
	AutoAimTool(int slot)
		: TasTool("autoaim", slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual void Reset();
};
