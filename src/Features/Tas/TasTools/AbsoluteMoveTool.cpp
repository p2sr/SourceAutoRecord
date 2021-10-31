#include "AbsoluteMoveTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"

AbsoluteMoveTool tasAbsoluteMoveTool("absmov");

AbsoluteMoveTool *AbsoluteMoveTool::GetTool() {
	return &tasAbsoluteMoveTool;
}

void AbsoluteMoveTool::Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo) {
	auto ttParams = std::static_pointer_cast<AbsoluteMoveToolParams>(params);

	if (!ttParams->enabled)
		return;

	auto nSlot = GET_SLOT();

	float angle = pInfo.angles.y - fb.viewAnalog.x - 90.0;
	float desired = ttParams->direction;

	float delta = desired - angle;
	auto R = DEG2RAD(delta);
	auto X = cosf(R);
	auto Y = sinf(R);

	fb.moveAnalog.x = X;
	fb.moveAnalog.y = Y;

	if (sar_tas_debug.GetBool()) {
		console->Print("absmov %.3f %.3f\n", X, Y);
	}
}

std::shared_ptr<TasToolParams> AbsoluteMoveTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	if (vp[0] == "off")
		return std::make_shared<AbsoluteMoveToolParams>();
	
	float angle;
	try {
		angle = std::stof(vp[0]);
	} catch (...) {
		throw TasParserException(Utils::ssprintf("Bad parameter for tool %s: %s", this->GetName(), vp[0].c_str()));
	}

	return std::make_shared<AbsoluteMoveToolParams>(angle);
}

void AbsoluteMoveTool::Reset() {
	params = std::make_shared<AbsoluteMoveToolParams>();
}
