#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct AbsoluteMoveToolParams : public TasToolParams {
	float direction = 0;
	float strength = 1;
	AbsoluteMoveToolParams()
		: TasToolParams() {
	}

	AbsoluteMoveToolParams(float direction, float strength)
		: TasToolParams(true)
		, direction(direction)
		, strength(strength) {
	}
};

class AbsoluteMoveTool : public TasToolWithParams<AbsoluteMoveToolParams> {
private:
	AbsoluteMoveToolParams amParams;
public:
	AbsoluteMoveTool(const char *name, int slot)
		: TasToolWithParams(name, slot){};
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
};

extern AbsoluteMoveTool tasAbsoluteMoveTool[2];
