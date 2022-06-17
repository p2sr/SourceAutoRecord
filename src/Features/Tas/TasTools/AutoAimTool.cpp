#include "AutoAimTool.hpp"

#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "StrafeTool.hpp"

AutoAimTool autoAimTool[2] = {{0}, {1}};

struct AutoAimParams : public TasToolParams {
	AutoAimParams()
		: TasToolParams(false) {}

	AutoAimParams(Vector point, int ticks)
		: TasToolParams(true)
		, entity(false)
		, point(point)
		, ticks(ticks)
		, elapsed(0) {}

	AutoAimParams(std::string name, int ticks)
		: TasToolParams(true)
		, entity(true)
		, ent_name(name)
		, ticks(ticks)
		, elapsed(0) {}

	bool entity;
	std::string ent_name;
	Vector point;
	int ticks;
	int elapsed;
};

std::shared_ptr<TasToolParams> AutoAimTool::ParseParams(std::vector<std::string> args) {
	if (args.size() != 1 && args.size() != 2 && args.size() != 3 && args.size() != 4)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), args.size()));
	
	if (args.size() == 1)
		if (args[0] == "off")
			return std::make_shared<AutoAimParams>();
		else 
			throw TasParserException(Utils::ssprintf("Bad argument for tool %s: %s", this->GetName(), args[0].c_str()));

	if ((args.size() == 2 || args.size() == 3) && args[0] == "ent") {
		int ticks;

		try {
			ticks = args.size() == 3 ? std::stoi(args[2]) : 1;
		} catch (...) {
			throw TasParserException(Utils::ssprintf("Bad tick value for tool %s: %s", this->GetName(), args[2].c_str()));
		}

		return std::make_shared<AutoAimParams>(args[1], ticks);
	}

	if (args.size() == 2) {
		throw TasParserException(Utils::ssprintf("Bad argument for tool %s: %s", this->GetName(), args[0].c_str()));
	}

	if (args.size() == 3 || args.size() == 4) {
		float x;
		float y;
		float z;
		int ticks;

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

		try {
			ticks = args.size() == 4 ? std::stoi(args[3]) : 1;
		} catch (...) {
			throw TasParserException(Utils::ssprintf("Bad tick value for tool %s: %s", this->GetName(), args[3].c_str()));
		}

		return std::make_shared<AutoAimParams>(Vector{x, y, z}, ticks);
	}

	return nullptr;
}

void AutoAimTool::Reset() {
	this->params = std::make_shared<AutoAimParams>();
}

void AutoAimTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &playerInfo) {
	auto params = std::static_pointer_cast<AutoAimParams>(this->params);
	if (!params->enabled) return;

	int remaining = 1; // If there are no lerp ticks left, pretend we're on the last tick, so that we jump all the way to the final angle
	if (params->elapsed < params->ticks) {
		remaining = params->ticks - params->elapsed;
		++params->elapsed;
	}

	void *player = server->GetPlayer(playerInfo.slot + 1);
	if (!player) return;

	Vector cam = playerInfo.position + server->GetViewOffset(player) + server->GetPortalLocal(player).m_vEyeOffset;

	// attempt to predict the position next frame. can't be worse than being always a tick behind.
	// TODO: FIGURE OUT WHAT THE FCK IS WRONG WITH THIS I JUST DONT GET IT?!?!?!
	//Vector newVel = autoStrafeTool.GetVelocityAfterMove(playerInfo, bulk.moveAnalog.y, bulk.moveAnalog.x);
	//cam += newVel * playerInfo.ticktime;

	Vector target;
	if (params->entity) {
		void *entity = nullptr;

		for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
			void *ent = server->m_EntPtrArray[i].m_pEntity;
			if (!ent) continue;

			const char *name = server->GetEntityName(ent);
			if (!name) continue;
			if (strcmp(name, params->ent_name.c_str())) continue;

			entity = ent;
			break;
		}

		if (entity) target = server->GetAbsOrigin(entity);
		else target = Vector{0, 0, 0};
	} else {
		target = params->point;
	}

	Vector forward = target - cam;

	float pitch = -atan2Old(forward.z, forward.Length2D());
	float yaw = atan2Old(forward.y, forward.x);

	pitch *= 180.0f / M_PI;
	yaw *= 180.0f / M_PI;

	Vector requiredDelta = QAngleToVector(playerInfo.angles) - Vector{pitch, yaw};

	while (requiredDelta.y < 0.0f) requiredDelta.y += 360.0f;
	if (requiredDelta.y > 180.0f) requiredDelta.y -= 360.0f;

	float pitchDelta = requiredDelta.x / remaining;
	float yawDelta = requiredDelta.y / remaining;

	if (sar_tas_debug.GetBool()) {
		console->Print("autoaim pitch:%.2f yaw:%.2f\n", pitch, yaw);
	}

	bulk.viewAnalog = Vector{yawDelta, pitchDelta};

	//do not let autoaim affect current movement direction. this will allow autoaim to be more precise with its prediction
	//float yawRaw = DEG2RAD(yawDelta);
	//Vector oldMove = bulk.moveAnalog;
	//bulk.moveAnalog.x = cosf(yawRaw) * oldMove.x - sinf(yawRaw) * oldMove.y;
	//bulk.moveAnalog.y = sinf(yawRaw) * oldMove.x + cosf(yawRaw) * oldMove.y;
}
