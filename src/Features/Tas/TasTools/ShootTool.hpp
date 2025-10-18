#pragma once
#include "../TasTool.hpp"
#include "../TasController.hpp"

struct ShootToolParams : public TasToolParams {

	TasControllerInput shotPortal;
	bool spam;
	bool hold;

	ShootToolParams()
		: TasToolParams() {
	}
	ShootToolParams(bool enabled, TasControllerInput shotPortal, bool spam, bool hold)
		: TasToolParams(enabled)
		, shotPortal(shotPortal)
		, spam(spam)
		, hold(hold) {
	}

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<ShootToolParams *>(&rhs_base);
		return TasToolParams::operator==(rhs) 
			&& std::tie(this->shotPortal, this->spam, this->hold) 
			== std::tie(rhs.shotPortal, rhs.spam, rhs.hold);
	}
};

class ShootTool : public TasToolWithParams<ShootToolParams> {
public:
	ShootTool(int slot)
		: TasToolWithParams("shoot", PRE_PROCESSING, slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};

extern ShootTool tasShootTool[2];
