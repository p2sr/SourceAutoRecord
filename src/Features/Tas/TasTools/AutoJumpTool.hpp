#pragma once
#include "../TasTool.hpp"

struct AutoJumpToolParams : public TasToolParams {
	AutoJumpToolParams()
		: TasToolParams() {
	}
	AutoJumpToolParams(bool enabled)
		: TasToolParams(enabled) {
	}
};

class AutoJumpTool : public TasTool {
public:
	AutoJumpTool(const char *name)
		: TasTool(name) {
	}

	virtual AutoJumpTool *GetTool();
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual void Reset();

private:
	bool hasJumpedLastTick = false;
};

extern AutoJumpTool autoJumpTool;