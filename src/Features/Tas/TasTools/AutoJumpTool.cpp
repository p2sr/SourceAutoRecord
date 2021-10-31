#include "AutoJumpTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"

AutoJumpTool autoJumpTool("autojump");

void AutoJumpTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	auto ttParams = std::static_pointer_cast<AutoJumpToolParams>(this->params);
	if (ttParams->enabled) {
		if (pInfo.grounded && !pInfo.ducked && !hasJumpedLastTick) {
			bulk.buttonStates[TasControllerInput::Jump] = true;
			hasJumpedLastTick = true;
		} else {
			bulk.buttonStates[TasControllerInput::Jump] = false;
			hasJumpedLastTick = false;
		}
	} else {
		hasJumpedLastTick = false;
	}
}

AutoJumpTool *AutoJumpTool::GetTool() {
	return &autoJumpTool;
}

std::shared_ptr<TasToolParams> AutoJumpTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	bool arg = vp[0] == "on";

	return std::make_shared<AutoJumpToolParams>(arg);
}

void AutoJumpTool::Reset() {
	this->params = std::make_shared<AutoJumpToolParams>();
	hasJumpedLastTick = false;
}
