#pragma once
#include "../TasTool.hpp"

struct CommandToolParams : public TasToolParams {
	CommandToolParams()
		: TasToolParams() {
	}
	CommandToolParams(bool enabled, std::string command)
		: TasToolParams(enabled)
		, command(command) {
	}

	std::string command;

	inline bool operator==(TasToolParams& rhs_base) {
		auto rhs = *dynamic_cast<CommandToolParams *>(&rhs_base);
		return TasToolParams::operator==(rhs)
			&& std::tie(this->command) 
			== std::tie(rhs.command);
	}
};

class CommandTool : public TasToolWithParams<CommandToolParams> {
public:
	CommandTool(int slot)
		: TasToolWithParams("cmd", PRE_PROCESSING, slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
	virtual std::shared_ptr<TasTool> makeCopy();
	virtual void fromCopy(std::shared_ptr<TasTool>);
};

extern CommandTool commandTool[2];
