#include "LookTool.hpp"
#include "AbsoluteMoveTool.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <climits>

LookTool tasLookTool[2] = { {0}, {1} };

void LookTool::Apply(TasFramebulk &fb, const TasPlayerInfo &playerInfo) {
	if (!params.enabled)
		return;

	if (this->updated) {
		this->remainingTime = params.time;
		this->updated = false;
	}

	if (--this->remainingTime < 0) {
		Reset();
	}

	fb.viewAnalog.y = -params.pitchDelta;
	fb.viewAnalog.x = -params.yawDelta;

	if (sar_tas_debug.GetBool()) {
		console->Print("look %.3f %.3f\n", params.pitchDelta, params.yawDelta);
	}
}

std::shared_ptr<TasToolParams> LookTool::ParseParams(std::vector<std::string> vp) {

	if (vp.size() == 0) {
		throw TasParserArgumentCountException(this, vp.size());
	}

	float pitchDelta = 0.0f;
	float yawDelta = 0.0f;
	float degrees = 1.5f;
	int tickCount = INT_MAX;

	bool timeAssigned = false;
	int degreesCount = 0;
	int directionsCount = 0;

	for (std::string param : vp) {
		if (param == "off" || param == "stop") {
			return std::make_shared<LookToolParams>();
		}

		else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "deg") {
			float newDegrees = TasParser::toFloat(param.substr(0, param.size() - 3));
			degreesCount++;
			if (degreesCount == 2) {
				pitchDelta = degrees;
				yawDelta = newDegrees;
			}
			degrees = newDegrees;
		}

		else if (param == "up") {
			pitchDelta -= 1.0f;
			directionsCount++;
		} else if (param == "down") {
			pitchDelta += 1.0f;
			directionsCount++;
		} else if (param == "left") {
			yawDelta += 1.0f;
			directionsCount++;
		} else if (param == "right") {
			yawDelta -= 1.0f;
			directionsCount++;
		}

		// try to just parse a number as time if it's not any known parameter
		else {
			if (timeAssigned) {
				throw TasParserArgumentException(this, param);
			}
			tickCount = TasParser::toFloat(param);
			timeAssigned = true;
		}
	}

	if (directionsCount > 0 && degreesCount > 1) {
		throw TasParserException(Utils::ssprintf("Too many degrees value with direction name for tool %s", this->GetName()));
	}
	if (timeAssigned) {
		if (degreesCount == 0) {
			degrees = 90.0f;
		}
		degrees /= tickCount;
	}

	return std::make_shared<LookToolParams>(pitchDelta * degrees, yawDelta * degrees, tickCount);
}
