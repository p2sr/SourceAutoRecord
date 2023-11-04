#include "DuckTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

DuckTool duckTool[2] = {{0}, {1}};

void DuckTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (!params.enabled) {
		return;
	}
	
	if (this->updated) {
		elapsedTicks = 0;
		this->updated = false;
	}

	if (elapsedTicks >= params.time) {
		params.enabled = false;
		return;
	}

	bulk.buttonStates[TasControllerInput::Crouch] = true;

	elapsedTicks++;
}

std::shared_ptr<TasToolParams> DuckTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	bool enabled = true;
	int time = INT32_MAX;

	if (vp[0] == "on") {
		enabled = true;
	} else if (vp[0] == "off") {
		enabled = false;
		time = 0;
	} else{
		try {
			time = std::stoi(vp[0]);
		} catch (...) {
			throw TasParserException(Utils::ssprintf("Incorrect parameter for tool %s: %s", this->GetName(), vp[0].c_str()));
		}
	}

	return std::make_shared<DuckToolParams>(enabled, time);
}
