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
			if (params.ducked) {
				bulk.buttonStates[TasControllerInput::Crouch] = true;
			}
			hasJumpedLastTick = true;
		} else {
			bulk.buttonStates[TasControllerInput::Jump] = false;
			if (params.ducked) {
				bulk.buttonStates[TasControllerInput::Crouch] = false;
			}
			hasJumpedLastTick = false;
		}
	} else {
		hasJumpedLastTick = false;
	}
}

std::shared_ptr<TasToolParams> AutoJumpTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	bool ducked = false;
	bool enabled = false;

	if (vp[0] == "on" || vp[0] == "unducked" || vp[0] == "unduck") {
		enabled = true;
	} else if (vp[0] == "ducked" || vp[0] == "duck") {
		enabled = true;
		ducked = true;
	} else if (vp[0] != "off") {
		throw TasParserException(Utils::ssprintf("Bad parameter for tool %s: %s", this->GetName(), vp[0].c_str()));
	}

	return std::make_shared<AutoJumpToolParams>(enabled, ducked);
}
