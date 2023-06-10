#include "SetAngleTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "TasUtils.hpp"

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

	auto target = Vector{params.pitch, params.yaw};
	if (params.target == "ahead") {
		auto velAng = TasUtils::GetVelocityAngles(&playerInfo).x;
		if (playerInfo.velocity.Length2D() == 0.0f) velAng = playerInfo.angles.y;
		target = Vector{0, velAng};
	}

	bulk.viewAnalog = bulk.viewAnalog + AngleToolsUtils::GetInterpolatedViewAnalog(
		QAngleToVector(playerInfo.angles),
		target,
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
	if (vp.size() < 1 || vp.size() > 4) 
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	std::string target;
	float pitch;
	float yaw;
	int ticks;
	AngleToolsUtils::EasingType easingType;
	int i = 2;

	if (vp[0] == "ahead") {
		if (vp.size() > 3) throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));
		target = vp[0];
		i = 1;
	} else {
		if (vp.size() < 2) throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

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
	}

	// ticks
	try {
		ticks = vp.size() >= (size_t)(i + 1) ? std::stoi(vp[i]) : 1;
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad tick value for tool %s: %s", this->GetName(), vp[i].c_str()));
	}

	// easing type
	try {
		easingType = AngleToolsUtils::ParseEasingType(vp.size() >= (size_t)(i + 2) ? vp[i + 1] : "linear");
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad interpolation value for tool %s: %s", this->GetName(), vp[i + 1].c_str()));
	}

	return std::make_shared<SetAngleParams>(target, pitch, yaw, ticks, easingType);
}
