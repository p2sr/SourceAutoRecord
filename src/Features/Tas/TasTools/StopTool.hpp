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
		: TasToolWithParams("stop", POST_PROCESSING, slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};

extern StopTool stopTool[2];
