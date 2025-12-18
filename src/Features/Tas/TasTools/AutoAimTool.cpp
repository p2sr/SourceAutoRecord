#include "AutoAimTool.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Features/EntityList.hpp"
#include "Features/Tas/TasParser.hpp"
#include "StrafeTool.hpp"

AutoAimTool autoAimTool[2] = {{0}, {1}};

std::shared_ptr<TasToolParams> AutoAimTool::ParseParams(std::vector<std::string> args) {
	bool usesEntitySelector = args.size() > 0 && args[0] == "ent";

	if (args.size() < (usesEntitySelector ? 2 : 1) || args.size() > (usesEntitySelector ? 4 : 5)) {
		throw TasParserArgumentCountException(this, args.size());
	}

	if (args.size() == 1 && args[0] == "off") {
		return std::make_shared<AutoAimParams>();
	} 
	else if (!usesEntitySelector && args.size() < 3) {
		throw TasParserArgumentCountException(this, args.size());
	}

	int ticks = 1;
	AngleToolsUtils::EasingType easingType = AngleToolsUtils::EasingType::Linear;

	// ticks
	unsigned ticksPos = usesEntitySelector ? 2 : 3;
	if (args.size() >= ticksPos + 1) {
		try {
			ticks = std::stoi(args[ticksPos]);
		} catch (...) {
			throw TasParserArgumentException(this, "ticks", args[ticksPos]);
		}
	}

	// easing type
	unsigned typePos = usesEntitySelector ? 3 : 4;
	if (args.size() >= typePos + 1) {
		try {
			easingType = AngleToolsUtils::ParseEasingType(args[typePos]);
		} catch (...) {
			throw TasParserArgumentException(this, "interpolation", args[typePos]);
		}
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
			throw TasParserArgumentException(this, "x value", args[0]);
		}

		try {
			y = std::stof(args[1]);
		} catch (...) {
			throw TasParserArgumentException(this, "y value", args[1]);
		}

		try {
			z = std::stof(args[2]);
		} catch (...) {
			throw TasParserArgumentException(this, "z value", args[2]);
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
