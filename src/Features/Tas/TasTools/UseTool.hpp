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

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<UseToolParams *>(&rhs_base);
		return TasToolParams::operator==(rhs) 
			&& std::tie(this->spam) 
			== std::tie(rhs.spam);
	}
};

class UseTool : public TasToolWithParams<UseToolParams> {
public:
	UseTool(int slot)
		: TasToolWithParams("use", PRE_PROCESSING, slot) {}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);

private:
	bool hasPressedLastTick = false;
};

extern UseTool tasUseTool[2];
