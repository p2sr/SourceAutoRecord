#include "AutoAimTool.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Features/EntityList.hpp"
#include "Features/Tas/TasParser.hpp"
#include "StrafeTool.hpp"

AutoAimTool autoAimTool[2] = {{0}, {1}};

std::shared_ptr<TasToolParams> AutoAimTool::ParseParams(std::vector<std::string> args) {
	bool usesEntitySelector = args.size() > 0 && args[0] == "ent";

	if (args.size() < 1 || args.size() > (usesEntitySelector ? 4 : 5)) {
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), args.size()));
	}

	if (args.size() == 1 && args[0] == "off") {
		return std::make_shared<AutoAimParams>();
	} 
	else if (!usesEntitySelector && args.size() < 3) {
		throw TasParserException(Utils::ssprintf("Bad argument for tool %s: %s", this->GetName(), args[0].c_str()));
	}

	int ticks;
	AngleToolsUtils::EasingType easingType;

	// ticks
	unsigned ticksPos = usesEntitySelector ? 2 : 3;
	try {
		ticks = args.size() >= ticksPos + 1 ? std::stoi(args[ticksPos]) : 1;
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad tick value for tool %s: %s", this->GetName(), args[ticksPos].c_str()));
	}

	// easing type
	unsigned typePos = usesEntitySelector ? 3 : 4;
	try {
		easingType = AngleToolsUtils::ParseEasingType(args.size() >= typePos + 1 ? args[typePos] : "linear");
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad interpolation value for tool %s: %s", this->GetName(), args[typePos].c_str()));
	}

	if (usesEntitySelector) {
		return std::make_shared<AutoAimParams>(args[1], ticks, easingType);
	} else {
		float x;
		float y;
		float z;

		try {
			x = std::stof(args[0]);
		} catch (...) {
			throw TasParserException(Utils::ssprintf("Bad x value for tool %s: %s", this->GetName(), args[0].c_str()));
		}

		try {
			y = std::stof(args[1]);
		} catch (...) {
			throw TasParserException(Utils::ssprintf("Bad y value for tool %s: %s", this->GetName(), args[1].c_str()));
		}

		try {
			z = std::stof(args[2]);
		} catch (...) {
			throw TasParserException(Utils::ssprintf("Bad z value for tool %s: %s", this->GetName(), args[2].c_str()));
		}

		return std::make_shared<AutoAimParams>(Vector{x, y, z}, ticks, easingType);
	}

	return nullptr;
}

void AutoAimTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &playerInfo) {
	if (!params.enabled) return;

	if (this->updated) {
		elapsedTicks = 0;
		this->updated = false;
	}

	void *player = server->GetPlayer(playerInfo.slot + 1);
	if (!player) return;

	Vector cam = playerInfo.position + server->GetViewOffset(player) + server->GetPortalLocal(player).m_vEyeOffset;

	// attempt to predict the position next frame. can't be worse than being always a tick behind.
	// TODO: FIGURE OUT WHAT THE FCK IS WRONG WITH THIS I JUST DONT GET IT?!?!?!
	//Vector newVel = autoStrafeTool.GetVelocityAfterMove(playerInfo, bulk.moveAnalog.y, bulk.moveAnalog.x);
	//cam += newVel * playerInfo.ticktime;

	Vector target;
	if (params.entity) {
		CEntInfo *entity = entityList->QuerySelector(params.ent_selector.c_str());
		
		if (entity != NULL) target = ((ServerEnt*)entity->m_pEntity)->abs_origin();
		else target = Vector{0, 0, 0};
	} else {
		target = params.point;
	}

	Vector forward = target - cam;

	float pitch = -atan2Old(forward.z, forward.Length2D());
	float yaw = atan2Old(forward.y, forward.x);

	pitch *= 180.0f / M_PI;
	yaw *= 180.0f / M_PI;

	bulk.viewAnalog = AngleToolsUtils::GetInterpolatedViewAnalog(
		QAngleToVector(playerInfo.angles), 
		Vector{pitch, yaw}, 
		params.easingTicks, 
		elapsedTicks, 
		params.easingType
	);

	if (sar_tas_debug.GetBool()) {
		console->Print("autoaim pitch:%.2f yaw:%.2f\n", pitch, yaw);
	}

	if (elapsedTicks < params.easingTicks) {
		++elapsedTicks;
	}

	//do not let autoaim affect current movement direction. this will allow autoaim to be more precise with its prediction
	//float yawRaw = DEG2RAD(yawDelta);
	//Vector oldMove = bulk.moveAnalog;
	//bulk.moveAnalog.x = cosf(yawRaw) * oldMove.x - sinf(yawRaw) * oldMove.y;
	//bulk.moveAnalog.y = sinf(yawRaw) * oldMove.x + cosf(yawRaw) * oldMove.y;
}
