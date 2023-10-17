#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct LookToolParams : public TasToolParams {
	float pitchDelta = 0;
	float yawDelta = 0;
	int time = 0;
	LookToolParams()
		: TasToolParams() {
	}

	LookToolParams(float pitchDelta, float yawDelta, int time)
		: TasToolParams(true)
		, pitchDelta(pitchDelta)
		, yawDelta(yawDelta) 
		, time(time) {
	}
};

class LookTool : public TasToolWithParams<LookToolParams> {
private:
	int remainingTime;
public:
	LookTool(const char *name, int slot)
		: TasToolWithParams(name, slot){};
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
};

extern LookTool tasLookTool[2];
