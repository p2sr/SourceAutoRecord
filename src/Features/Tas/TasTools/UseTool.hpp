#pragma once
#include "../TasTool.hpp"

struct UseToolParams : public TasToolParams {
	bool spam;

	UseToolParams()
		: TasToolParams() {
	}
	UseToolParams(bool enabled, bool spam)
		: TasToolParams(enabled)
		, spam(spam) {
	}
};

class UseTool : public TasToolWithParams<UseToolParams> {
public:
	UseTool(int slot) : TasToolWithParams("use", slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);

private:
	bool hasPressedLastTick = false;
};

extern UseTool tasUseTool[2];
