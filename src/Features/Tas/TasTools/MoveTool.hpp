#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

struct MoveToolParams : public TasToolParams {
	float forwardMove = 0;
	float sideMove = 0;
	// `<N>deg`: the direction is computed in Apply, where the script version is known
	bool movingByDegrees = false;
	float degrees = 0;
	float scale = 1;
	bool scaleAssigned = false;
	MoveToolParams()
		: TasToolParams() {
	}

	MoveToolParams(float forwardMove, float sideMove)
		: TasToolParams(true)
		, forwardMove(forwardMove)
		, sideMove(sideMove) {
	}

	MoveToolParams(float degrees, float scale, bool scaleAssigned)
		: TasToolParams(true)
		, movingByDegrees(true)
		, degrees(degrees)
		, scale(scale)
		, scaleAssigned(scaleAssigned) {
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
