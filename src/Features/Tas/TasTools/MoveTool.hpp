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

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<MoveToolParams *>(&rhs_base);
		return TasToolParams::operator==(rhs) 
			&& std::tie(this->forwardMove, this->sideMove) 
			== std::tie(rhs.forwardMove, rhs.sideMove);
	}
};

class MoveTool : public TasToolWithParams<MoveToolParams> {
public:
	MoveTool(int slot)
		: TasToolWithParams("move", PRE_PROCESSING, slot) {};
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};

extern MoveTool tasMoveTool[2];
