#include "DecelTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "TasUtils.hpp"
#include "StrafeTool.hpp"


DecelTool decelTool[2] = {{0}, {1}};

void DecelTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &playerInfo) {
	if (!params.enabled) {
		return;
	}

	float targetVel = params.targetVel;
	float playerVel = autoStrafeTool[this->slot].GetGroundFrictionVelocity(playerInfo).Length2D(); 

	// cant decelerate by accelerating lmfao
	if (targetVel >= playerVel) {
		params.enabled = false;
		return;
	}

	// move the opposite side of your current velocity
	float moveAngle = DEG2RAD(TasUtils::GetVelocityAngles(&playerInfo).x + 180 - playerInfo.angles.y + bulk.viewAnalog.x);
	bulk.moveAnalog.x = -sinf(moveAngle);
	bulk.moveAnalog.y = cosf(moveAngle);

	// make sure to move only the necessary amount
	float maxAccel = autoStrafeTool[this->slot].GetMaxAccel(playerInfo, autoStrafeTool[this->slot].CreateWishDir(playerInfo,bulk.moveAnalog.y, bulk.moveAnalog.x));
	if (playerVel - targetVel < maxAccel) {
		bulk.moveAnalog = bulk.moveAnalog.Normalize() * ((playerVel - targetVel) / maxAccel);
		// dont need to decelerate next tick
		params.enabled = false;
	}


	if (sar_tas_debug.GetBool()) {
		console->Print("decel %.3fdeg %.3fups\n", RAD2DEG(moveAngle), bulk.moveAnalog.Length());
	}
}

std::shared_ptr<TasToolParams> DecelTool::ParseParams(std::vector<std::string> args) {
	if (args.size() != 1) {
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), args.size()));
	}
	if (args[0] == "off") {
		return std::make_shared<TasToolParams>(false);
	}

	float targetVel;

	try {
		targetVel = atof(args[0].c_str());
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad target velocity for tool %s: %s", this->GetName(), args[0].c_str()));
	}

	return std::make_shared<DecelParams>(targetVel);
}
