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
};

class CommandTool : public TasToolWithParams<CommandToolParams> {
public:
	CommandTool(int slot)
		: TasToolWithParams("cmd", PRE_PROCESSING, COMMANDS, slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);
};

extern CommandTool commandTool[2];
