#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct AbsoluteMoveToolParams : public TasToolParams {
	float direction = 0;
	AbsoluteMoveToolParams()
		: TasToolParams() {
	}

	AbsoluteMoveToolParams(float direction)
		: TasToolParams(true)
		, direction(direction) {
	}
};

class AbsoluteMoveTool : public TasTool {
public:
	AbsoluteMoveTool(const char *name)
		: TasTool(name){};
	virtual AbsoluteMoveTool *GetTool();
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual void Reset();
};

extern AbsoluteMoveTool tasAbsoluteMoveTool;
