#pragma once

#include "../TasTool.hpp"

class AutoAimTool : public TasTool {
public:
	AutoAimTool()
		: TasTool("autoaim") {}

	virtual AutoAimTool *GetTool();
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual void Reset();
};
