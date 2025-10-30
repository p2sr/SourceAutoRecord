#include "JumpTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

JumpTool autoJumpTool[2] = {{0, true}, {1, true}};
JumpTool jumpTool[2] = {{0, false}, {1, false}};

void JumpTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (this->updated) {
		hasJumpedLastTick = false;
	}

	if (params.enabled) {
		bool shouldJump;

		if (this->automatic) {
			shouldJump = ShouldJump(pInfo);
		} else {
			shouldJump = true;
			params.enabled = false;
		}

		SetJumpInput(bulk, shouldJump);
	} else {
		hasJumpedLastTick = false;
	}
}

bool JumpTool::WillJump(const TasPlayerInfo &pInfo) {
	return params.enabled && ShouldJump(pInfo);
}

bool JumpTool::ShouldJump(const TasPlayerInfo &pInfo) {
	return pInfo.grounded && !pInfo.ducked && !hasJumpedLastTick;
}

void JumpTool::SetJumpInput(TasFramebulk &bulk, bool jump) {
	bulk.buttonStates[TasControllerInput::Jump] = jump;
	if (params.ducked) {
		bulk.buttonStates[TasControllerInput::Crouch] = jump;
	}
	hasJumpedLastTick = jump;
}

std::shared_ptr<TasToolParams> JumpTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() == 0 && !this->automatic) {
		// allow 0 parameters for non-automated jump tool
		return std::make_shared<JumpToolsParams>(true, false);
	}

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

	return std::make_shared<JumpToolsParams>(enabled, ducked);
}
