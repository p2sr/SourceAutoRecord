#include "AbsoluteMoveTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "MoveTool.hpp"

AbsoluteMoveTool tasAbsoluteMoveTool[2] = {
	{ "absmov", 0 },
	{ "absmov", 1 },
};

void AbsoluteMoveTool::Apply(TasFramebulk &fb, const TasPlayerInfo &playerInfo) {
	if (!params.enabled)
		return;

	if (tasMoveTool[slot].GetCurrentParams().enabled) {
		tasMoveTool[slot].Reset();
	}

	auto angles = playerInfo.angles;
	angles.y -= fb.viewAnalog.x;
	angles.x -= fb.viewAnalog.y;

	float forward_coef;
	if (fabsf(angles.x) >= 30.0f && !playerInfo.willBeGrounded) {
		forward_coef = cosOld(DEG2RAD(angles.x));
	} else {
		forward_coef = 1.0f;
	}

	float desired = params.direction;
	float rad = DEG2RAD(desired - angles.y);

	float x = -sinf(rad);
	float y = cosf(rad) / forward_coef;

	x *= params.strength;
	y *= params.strength;

	if (y > 1.0f) {
		// We can't actually move this fast. Scale the movement down so 'y'
		// is within the allowed range
		x /= y;
		y = 1.0f;
	}

	fb.moveAnalog.x = x;
	fb.moveAnalog.y = y;

	if (sar_tas_debug.GetBool()) {
		console->Print("absmov %.3f %.3f\n", x, y);
	}
}

std::shared_ptr<TasToolParams> AbsoluteMoveTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1 && vp.size() != 2)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	if (vp[0] == "off")
		return std::make_shared<AbsoluteMoveToolParams>();
	
	float angle;
	try {
		angle = std::stof(vp[0]);
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad direction for tool %s: %s", this->GetName(), vp[0].c_str()));
	}
	
	float strength;
	try {
		strength = (vp.size()==2) ? std::stof(vp[1]) : 1;
		if (strength > 1) strength = 1;
		if (strength < 0) strength = 0;
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad strength for tool %s: %s", this->GetName(), vp[0].c_str()));
	}

	return std::make_shared<AbsoluteMoveToolParams>(angle, strength);
}
