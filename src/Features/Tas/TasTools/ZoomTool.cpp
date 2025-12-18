#include "ZoomTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

ZoomTool zoomTool[2] = {{0}, {1}};

void ZoomTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (params.enabled) {
		params.enabled = false;

		void *player = server->GetPlayer(pInfo.slot + 1);

		if (player == nullptr || (int)player == -1) return;

		bool isZoomedIn = SE(player)->field<CBaseHandle>("m_hZoomOwner");

		if (
			(params.zoomType == ZoomType::In && !isZoomedIn) ||
			(params.zoomType == ZoomType::Out && isZoomedIn) ||
			params.zoomType == ZoomType::Toggle
		) {
			bulk.buttonStates[TasControllerInput::Zoom] = true;
		}
	}
}

std::shared_ptr<TasToolParams> ZoomTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() != 1)
		throw TasParserArgumentCountException(this, vp.size());

	ZoomType type;

	if (vp[0] == "in") {
		type = ZoomType::In;
	} 
	else if (vp[0] == "out") {
		type = ZoomType::Out;
	} 
	else if (vp[0] == "toggle") {
		type = ZoomType::Toggle;
	} else {
		throw TasParserArgumentException(this, vp[0]);
	}

	return std::make_shared<ZoomToolParams>(true, type);
}
