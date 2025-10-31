#include "CommandTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

CommandTool commandTool[2] = {{0}, {1}};

void CommandTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (params.enabled) {
		bulk.commands.push_back(params.command);

		FOR_TAS_SCRIPT_VERSIONS_UNTIL(8) {
			// Commands in frame bulk are called by FetchInput
			// but before version 9 they were added in post processing, so they need to be executed here.
			engine->ExecuteCommand(params.command.c_str(), true);
		}

		params.enabled = false;
	}
}

std::shared_ptr<TasToolParams> CommandTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() == 0)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	std::string command;

	for (const std::string &str : vp) {
		if (!command.empty()) {
			command += " ";
		}
		command += str;
	}

	return std::make_shared<CommandToolParams>(true, command);
}
