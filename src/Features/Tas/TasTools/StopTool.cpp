#include "StopTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

StopTool stopTool[2] = {{0}, {1}};

void StopTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (params.enabled) {
		for (TasTool *tool : TasTool::GetList(pInfo.slot)) {

			bool toolJustRequested = false;
			for (auto toolCmd : bulk.toolCmds) {
				if (tool == toolCmd.tool) {
					toolJustRequested = true;
					break;
				}
			}

			if (!toolJustRequested) {
				tool->Reset();
			}
		}
		params.enabled = false;
	}
}

std::shared_ptr<TasToolParams> StopTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() > 0)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	return std::make_shared<StopToolParams>(true);
}
