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
};

class ShootTool : public TasToolWithParams<ShootToolParams> {
public:
	ShootTool(int slot)
		: TasToolWithParams("shoot", POST_PROCESSING, BUTTONS, slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};

extern ShootTool tasShootTool[2];
