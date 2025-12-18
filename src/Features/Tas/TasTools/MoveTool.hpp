#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct MoveToolParams : public TasToolParams {
	float forwardMove = 0;
	float sideMove = 0;
	MoveToolParams()
		: TasToolParams() {
	}

	MoveToolParams(float forwardMove, float sideMove)
		: TasToolParams(true)
		, forwardMove(forwardMove)
		, sideMove(sideMove) {
	}
};

class MoveTool : public TasToolWithParams<MoveToolParams> {
public:
	MoveTool(int slot)
		: TasToolWithParams("move", PRE_PROCESSING, MOVEMENT, slot) {};
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
};

extern MoveTool tasMoveTool[2];
