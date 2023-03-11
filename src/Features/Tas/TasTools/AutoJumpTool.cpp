#include "AutoJumpTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

AutoJumpTool autoJumpTool[2] = {{0}, {1}};

void AutoJumpTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (this->updated) {
		hasJumpedLastTick = false;
	}

	if (params.enabled) {
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

std::shared_ptr<TasToolParams> AutoJumpTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	bool arg = vp[0] == "on";

	return std::make_shared<AutoJumpToolParams>(arg);
}
