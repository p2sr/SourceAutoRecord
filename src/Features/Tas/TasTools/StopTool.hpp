#pragma once
#include "../TasTool.hpp"

struct StopToolParams : public TasToolParams {
	StopToolParams()
		: TasToolParams() {
	}
	StopToolParams(bool enabled)
		: TasToolParams(enabled) {
	}
};

class StopTool : public TasToolWithParams<StopToolParams> {
public:
	StopTool(int slot)
		: TasToolWithParams("stop", slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};

extern StopTool stopTool[2];
