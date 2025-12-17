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
		throw TasParserArgumentCountException(this, vp.size());

	std::string target;
	float pitch;
	float yaw;
	int ticks;
	AngleToolsUtils::EasingType easingType;
	int i = 2;

	if (vp[0] == "ahead") {
		if (vp.size() > 3) throw TasParserArgumentCountException(this, vp.size());
		target = vp[0];
		i = 1;
	} else {
		if (vp.size() < 2) throw TasParserArgumentCountException(this, vp.size());

		// pitch
		try {
			pitch = std::stof(vp[0]);
		} catch (...) {
			throw TasParserArgumentException(this, "pitch", vp[0]);
		}

		// yaw
		try {
			yaw = std::stof(vp[1]);
		} catch (...) {
			throw TasParserArgumentException(this, "yaw", vp[1]);
		}
	}

	// ticks
	try {
		ticks = vp.size() >= (size_t)(i + 1) ? std::stoi(vp[i]) : 1;
	} catch (...) {
		throw TasParserArgumentException(this, "tick", vp[i]);
	}

	// easing type
	try {
		easingType = AngleToolsUtils::ParseEasingType(vp.size() >= (size_t)(i + 2) ? vp[i + 1] : "linear");
	} catch (...) {
		throw TasParserArgumentException(this, "interpolation", vp[i + 1]);
	}

	return std::make_shared<SetAngleParams>(target, pitch, yaw, ticks, easingType);
}
