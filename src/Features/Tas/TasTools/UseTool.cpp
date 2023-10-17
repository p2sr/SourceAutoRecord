#include "UseTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

UseTool tasUseTool[2] = { {0}, {1} };

void UseTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (this->updated) {
		hasPressedLastTick = false;
		this->updated = false;
	}

	if (params.enabled) {
		if (!hasPressedLastTick) {
			bulk.buttonStates[TasControllerInput::Use] = true;
			hasPressedLastTick = true;

			if (!params.spam) Reset();
		} else {
			bulk.buttonStates[TasControllerInput::Use] = false;
			hasPressedLastTick = false;
		}
	}
}

std::shared_ptr<TasToolParams> UseTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() > 1)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	if (vp.size() == 0) {
		return std::make_shared<UseToolParams>(true, false);
	}

	bool enabled = true;
	bool spam = false;

	if (vp[0] == "off" || vp[0] == "stop") {
		enabled = false;
	}
	else if (vp[0] == "spam") {
		spam = true;
	} else {
		throw TasParserException(Utils::ssprintf("Incorrect parameter for tool %s: %s", this->GetName(), vp[0].c_str()));
	}

	return std::make_shared<UseToolParams>(enabled, spam);
}
