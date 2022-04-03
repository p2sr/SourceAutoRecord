#include "CheckTool.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Modules/Console.hpp"

#include <cmath>

#define DEFAULT_POS_EPSILON 0.5
#define DEFAULT_ANG_EPSILON 0.2

Variable sar_tas_check_max_replays("sar_tas_check_max_replays", "15", 0, "Maximum replays for the 'check' TAS tool until it gives up.\n");
Variable sar_tas_check_disable("sar_tas_check_disable", "0", "Globally disable the 'check' TAS tool.\n");

CheckTool tasCheckTool[2] = {{0}, {1}};

int g_tas_check_replays = 0;

static void maybeReplay() {
	int replays = g_tas_check_replays;
	if (replays < sar_tas_check_max_replays.GetInt()) {
		console->Print("Replaying TAS (attempt %d)\n", replays);
		tasPlayer->Replay();
		// Replaying the TAS will have reset g_tas_check_replays
		g_tas_check_replays = replays + 1;
	} else {
		console->Print("TAS failed after %d replays - giving up\n", replays);
		tasPlayer->Stop(true);
	}
}

void CheckTool::Apply(TasFramebulk &fb, const TasPlayerInfo &info) {
	auto ttParams = std::static_pointer_cast<CheckToolParams>(params);

	if (!ttParams->enabled) return;

	if (sar_tas_check_disable.GetBool()) {
		params->enabled = false;
		return;
	}

	if (ttParams->pos) {
		Vector delta = info.position - *ttParams->pos;
		if (delta.SquaredLength() > ttParams->posepsilon*ttParams->posepsilon) {
			console->Print("Position was %.2f units away from target!\n", delta.Length());
			maybeReplay();
		}
	}

	if (ttParams->ang) {
		float pitchDelta = info.angles.x - ttParams->ang->x;
		float yawDelta = info.angles.y - ttParams->ang->y;
		float distSquared = pitchDelta*pitchDelta + yawDelta*yawDelta;
		if (distSquared > ttParams->angepsilon*ttParams->angepsilon) {
			console->Print("Angle was %.2f degrees away from target!\n", sqrtf(distSquared));
			maybeReplay();
		}
	}

	params->enabled = false;
}

static Vector parsePos(const std::vector<std::string> &args, size_t idx) {
	if (args.size() - idx < 3) {
		throw TasParserException("Bad position for check tool");
	}
	try {
		float x = std::stof(args[idx + 0]);
		float y = std::stof(args[idx + 1]);
		float z = std::stof(args[idx + 2]);
		return {x,y,z};
	} catch (...) {
		throw TasParserException("Bad position for check tool");
	}
}

static QAngle parseAng(const std::vector<std::string> &args, size_t idx) {
	if (args.size() - idx < 2) {
		throw TasParserException("Bad angle for check tool");
	}
	try {
		float p = std::stof(args[idx + 0]);
		float y = std::stof(args[idx + 1]);
		return {p,y,0};
	} catch (...) {
		throw TasParserException("Bad angle for check tool");
	}
}

std::shared_ptr<TasToolParams> CheckTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() < 1) {
		throw TasParserException("Expected at least 1 argument for check tool");
	}

	std::optional<Vector> pos = {};
	std::optional<QAngle> ang = {};
	std::optional<float> posepsilon = {};
	std::optional<float> angepsilon = {};

	size_t i = 0;

	while (i < vp.size()) {
		if (vp[i] == "pos") {
			if (pos) {
				throw TasParserException("Duplicate position given to check tool");
			}
			pos = parsePos(vp, i+1);
			i += 4;
		} else if (vp[i] == "ang") {
			if (ang) {
				throw TasParserException("Duplicate angle given to check tool");
			}
			ang = parseAng(vp, i+1);
			i += 3;
		} else if (vp[i] == "posepsilon") {
			if (posepsilon) {
				throw TasParserException("Duplicate position epsilon given to check tool");
			}
			try {
				posepsilon = std::stof(vp[i+1]);
			} catch (...) {
				throw TasParserException("Bad position epsilon for check tool");
			}
			i += 2;
		} else if (vp[i] == "angepsilon") {
			if (angepsilon) {
				throw TasParserException("Duplicate angle epsilon given to check tool");
			}
			try {
				angepsilon = std::stof(vp[i+1]);
			} catch (...) {
				throw TasParserException("Bad angle epsilon for check tool");
			}
			i += 2;
		} else {
			throw TasParserException(Utils::ssprintf("Bad argument for check tool: \"%s\"", vp[i].c_str()));
		}
	}

	return std::make_shared<CheckToolParams>(pos, ang, posepsilon ? *posepsilon : DEFAULT_POS_EPSILON, angepsilon ? *angepsilon : DEFAULT_ANG_EPSILON);
}

void CheckTool::Reset() {
	params = std::make_shared<CheckToolParams>();
}
