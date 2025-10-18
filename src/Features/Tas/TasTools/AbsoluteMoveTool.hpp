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

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<AbsoluteMoveToolParams *>(&rhs_base);
		return TasToolParams::operator==(rhs) 
			&& std::tie(this->direction, this->strength) 
			== std::tie(rhs.direction, rhs.strength);
	}

};

class AbsoluteMoveTool : public TasToolWithParams<AbsoluteMoveToolParams> {
private:
	AbsoluteMoveToolParams amParams;
public:
	AbsoluteMoveTool(int slot) : TasToolWithParams("absmov", POST_PROCESSING, slot) {};
	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};

extern AbsoluteMoveTool tasAbsoluteMoveTool[2];
