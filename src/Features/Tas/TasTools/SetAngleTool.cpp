#include "SetAngleTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"

SetAngleTool setAngleTool[2] = {{0}, {1}};

void SetAngleTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &playerInfo) {
	if (!params.enabled) {
		return;
	}

	if (this->updated) {
		elapsedTicks = 0;
		this->updated = false;
	}

	if (elapsedTicks >= params.easingTicks) {
		params.enabled = false;
		return;
	}

	bulk.viewAnalog = bulk.viewAnalog + AngleToolsUtils::GetInterpolatedViewAnalog(
		QAngleToVector(playerInfo.angles),
		Vector{params.pitch, params.yaw},
		params.easingTicks,
		elapsedTicks,
		params.easingType
	);

	if (sar_tas_debug.GetBool()) {
		console->Print("setang %.3f %.3f\n", bulk.viewAnalog.x, bulk.viewAnalog.y);
	}

	++elapsedTicks;
}

std::shared_ptr<TasToolParams> SetAngleTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() < 2 || vp.size() > 4) 
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	float pitch;
	float yaw = atof(vp[1].c_str());
	int ticks = vp.size() == 3 ? atoi(vp[2].c_str()) : 1;
	AngleToolsUtils::EasingType easingType;

	// pitch
	try {
		pitch = std::stof(vp[0]);
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad pitch value for tool %s: %s", this->GetName(), vp[0].c_str()));
	}

	// yaw
	try {
		yaw = std::stof(vp[1]);
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad yaw value for tool %s: %s", this->GetName(), vp[1].c_str()));
	}

	// ticks
	try {
		ticks = vp.size() >= 3 ? std::stoi(vp[2]) : 1;
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad tick value for tool %s: %s", this->GetName(), vp[2].c_str()));
	}

	// easing type
	try{
		easingType = AngleToolsUtils::ParseEasingType(vp.size() >= 4 ? vp[3] : "linear");
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad interpolation value for tool %s: %s", this->GetName(), vp[3].c_str()));
	}

	return std::make_shared<SetAngleParams>(pitch, yaw, ticks, easingType);
}
