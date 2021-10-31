#include "SetAngleTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"

struct SetAngleParams : public TasToolParams {
	SetAngleParams()
		: TasToolParams() {}

	SetAngleParams(int ticks, float pitch, float yaw)
		: TasToolParams(true)
		, ticks(ticks)
		, elapsed(0)
		, pitch(pitch)
		, yaw(yaw) {}

	int ticks;
	int elapsed;
	float pitch;
	float yaw;
};

SetAngleTool setAngleTool;

SetAngleTool *SetAngleTool::GetTool() {
	return &setAngleTool;
}

void SetAngleTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &playerInfo) {
	auto params = std::static_pointer_cast<SetAngleParams>(this->params);

	if (!params->enabled) {
		return;
	}

	if (params->elapsed >= params->ticks) {
		params->enabled = false;
		return;
	}

	int remaining = params->ticks - params->elapsed;

	Vector requiredDelta = QAngleToVector(playerInfo.angles) - Vector{params->pitch, params->yaw};

	while (requiredDelta.y < 0.0f) requiredDelta.y += 360.0f;
	if (requiredDelta.y > 180.0f) requiredDelta.y -= 360.0f;

	float pitchDelta = requiredDelta.x / remaining;
	float yawDelta = requiredDelta.y / remaining;

	bulk.viewAnalog = bulk.viewAnalog - Vector{-yawDelta, -pitchDelta};

	if (sar_tas_debug.GetBool()) {
		console->Print("setang %.3f %.3f\n", bulk.viewAnalog.x, bulk.viewAnalog.y);
	}

	++params->elapsed;
}

std::shared_ptr<TasToolParams> SetAngleTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 2 && vp.size() != 3) 
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	float pitch;
	float yaw = atof(vp[1].c_str());
	int ticks = vp.size() == 3 ? atoi(vp[2].c_str()) : 1;

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
		ticks = vp.size() == 3 ? std::stoi(vp[2]) : 1;
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad tick value for tool %s: %s", this->GetName(), vp[2].c_str()));
	}

	return std::make_shared<SetAngleParams>(ticks, pitch, yaw);
}

void SetAngleTool::Reset() {
	params = std::make_shared<SetAngleParams>();
}
