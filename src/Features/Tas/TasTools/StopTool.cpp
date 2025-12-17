#include "StopTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

StopTool stopTool[2] = {{0}, {1}};

void StopTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (params.enabled) {
		for (TasTool *tool : TasTool::GetList(pInfo.slot)) {

			if ((tool->GetBulkType() & params.typesToDisableMask) == 0) {
				continue;
			}

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
	uint32_t typesToDisableMask = TasToolBulkType::NONE;

	if (vp.size() == 0) {
		typesToDisableMask = TasToolBulkType::ALL_TYPES_MASK;
	}

	for (const auto &arg : vp) {
		if (arg == "movement" || arg == "moving" || arg == "move") {
			typesToDisableMask |= TasToolBulkType::MOVEMENT;
		} else if (arg == "viewangles" || arg == "angles" || arg == "ang" || arg == "looking" || arg == "look") {
			typesToDisableMask |= TasToolBulkType::VIEWANGLES;
		} else if (arg == "buttons" || arg == "inputs" || arg == "pressing" || arg == "press") {
			typesToDisableMask |= TasToolBulkType::BUTTONS;
		} else if (arg == "commands" || arg == "cmd") {
			typesToDisableMask |= TasToolBulkType::COMMANDS;
		} else if (arg == "all" || arg == "everything") {
			typesToDisableMask = TasToolBulkType::ALL_TYPES_MASK;
		} else {
			throw TasParserArgumentException(this, arg);
		}
	}

	return std::make_shared<StopToolParams>(true, typesToDisableMask);
}
