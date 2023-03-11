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

class AutoJumpTool : public TasToolWithParams<AutoJumpToolParams> {
public:
	AutoJumpTool(int slot)
		: TasToolWithParams("autojump", slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);

private:
	bool hasJumpedLastTick = false;
};

extern AutoJumpTool autoJumpTool[2];
