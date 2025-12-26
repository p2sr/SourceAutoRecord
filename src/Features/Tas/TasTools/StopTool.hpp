#pragma once
#include "../TasTool.hpp"
#include <cstdint>

struct StopToolParams : public TasToolParams {
	uint32_t typesToDisableMask;

	StopToolParams()
		: TasToolParams() {
	}
	StopToolParams(bool enabled, uint32_t typesToDisableMask)
		: TasToolParams(enabled)
		, typesToDisableMask(typesToDisableMask) {
	}
};

class StopTool : public TasToolWithParams<StopToolParams> {
public:
	StopTool(int slot)
		: TasToolWithParams("stop", POST_PROCESSING, META, slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};

extern StopTool stopTool[2];
