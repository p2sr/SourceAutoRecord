#include "TasScript.hpp"

std::string TasFramebulk::ToString() {
	std::string output = "[" + std::to_string(tick) + "] mov: (" + std::to_string(moveAnalog.x) + " " + std::to_string(moveAnalog.y) + "), ang:" + std::to_string(viewAnalog.x) + " " + std::to_string(viewAnalog.y) + "), btns:";
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		output += (buttonStates[i]) ? "1" : "0";
	}
	output += ", cmds: ";
	for (std::string command : commands) {
		output += command + ";";
	}
	output += ", tools:";
	for (TasToolCommand toolCmd : toolCmds) {
		output += " {" + std::string(toolCmd.tool->GetName()) + "}";
	}
	return output;
}

void TasScript::ClearGeneratedContent() {
	this->processedFramebulks.clear();
	this->userCmdDebugs.clear();
	this->playerInfoDebugs.clear();
}
