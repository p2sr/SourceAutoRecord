#include "MoveTool.hpp"

#include "AbsoluteMoveTool.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"

MoveTool tasMoveTool[2] = { {0}, {1} };

const std::map<std::string, Vector> directionLookup = {
	{"forward", Vector(0, 1)},
	{"back", Vector(0, -1)},
	{"left", Vector(-1, 0)},
	{"right", Vector(1, 0)},
	{"forwardleft", Vector(-0.7071, 0.7071)},
	{"forwardright", Vector(0.7071, 0.7071)},
	{"backwardleft", Vector(-0.7071, -0.7071)},
	{"backwardright", Vector(0.7071, -0.7071)},
};

void MoveTool::Apply(TasFramebulk &fb, const TasPlayerInfo &playerInfo) {
	if (!params.enabled)
		return;

	if (tasAbsoluteMoveTool[slot].GetCurrentParams().enabled) {
		tasAbsoluteMoveTool[slot].Reset();
	}

	float forwardMove = params.forwardMove;
	float sideMove = params.sideMove;

	if (params.movingByDegrees) {
		forwardMove = tasCosf(params.degrees);
		sideMove = tasSinf(params.degrees);

		if (params.scaleAssigned) {
			auto length = sqrtf((forwardMove * forwardMove) + (sideMove * sideMove));
			forwardMove /= length;
			sideMove /= length;
		}

		forwardMove *= params.scale;
		sideMove *= params.scale;
	}

	fb.moveAnalog.x = sideMove;
	fb.moveAnalog.y = forwardMove;

	if (sar_tas_debug.GetBool()) {
		console->Print("move %.3f %.3f\n", forwardMove, sideMove);
	}
}

std::shared_ptr<TasToolParams> MoveTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() == 0) {
		throw TasParserArgumentCountException(this, vp.size());
	}

	float forwardMove = 0.0f;
	float sideMove = 0.0f;
	float degrees = 0.0f;
	float scale = 1.0f;

	int numberCount = 0;
	int directionsCount = 0;
	bool movingByDegrees = false;
	bool scaleAssigned = false;

	for (std::string param : vp) {
		if (param == "off" || param == "stop") {
			return std::make_shared<MoveToolParams>();
		}

		else if (param.size() > 3 && param.substr(param.size() - 3, 3) == "deg") {
			if (movingByDegrees) {
				throw TasParserException(Utils::ssprintf("Repeated degrees parameter for tool %s: %s", this->GetName(), param.c_str()));
			}
			movingByDegrees = true;
			degrees = TasParser::toFloat(param.substr(0, param.size() - 3));
		}

		else if (param == "forward") {
			forwardMove += 1.0f;
			directionsCount++;
		} else if (param == "back" || param == "backward") {
			forwardMove -= 1.0f;
			directionsCount++;
		} else if (param == "left") {
			sideMove -= 1.0f;
			directionsCount++;
		} else if (param == "right") {
			sideMove += 1.0f;
			directionsCount++;
		}

		// try to just parse a number as number if it's not any known parameter
		else {
			auto number = TasParser::toFloat(param);
			numberCount++;
			if (!movingByDegrees && directionsCount == 0) {
				if (numberCount == 1) {
					sideMove = number;
				} else if (numberCount == 2) {
					forwardMove = number;
				} else if (numberCount > 2) {
					throw TasParserArgumentCountException(this, vp.size());
				}
			} else {
				if (numberCount == 1) {
					scale = number;
					scaleAssigned = true;
				} else {
					throw TasParserArgumentCountException(this, vp.size());
				}
			}
		}
	}

	if (directionsCount > 0 && movingByDegrees) {
		throw TasParserException(Utils::ssprintf("Too many direction identifiers for tool %s", this->GetName()));
	}

	if (movingByDegrees) {
		return std::make_shared<MoveToolParams>(degrees, scale, scaleAssigned);
	}

	if (directionsCount > 0 || scaleAssigned) {
		auto length = sqrtf((forwardMove * forwardMove) + (sideMove * sideMove));
		forwardMove /= length;
		sideMove /= length;
	}

	return std::make_shared<MoveToolParams>(forwardMove * scale, sideMove * scale);
}
