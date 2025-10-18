#include "TasScript.hpp"

#include <ranges>

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

// Return the first different tick between this and other
// Return -1 if no difference, and -2 if the difference is not in the 
int TasScript::FirstDifference(TasScript* other) {
	if (
		this->name != other->name
		|| this->path != other->path
		|| this->header != other->header
	) {
		return -2;
	}

	auto thislen = this->framebulks.size();
	auto otherlen = other->framebulks.size();

	for (unsigned i = 0; i < std::min(thislen, otherlen); i++) {
		auto thisbulk = this->framebulks[i];
		auto otherbulk = other->framebulks[i];
		if (thisbulk != otherbulk) {
			return std::min(thisbulk.tick, otherbulk.tick);
		}
	}

	// If they are different lengths, and all the ones in common are
	// identical, return the first one after the ones in common
	if (thislen < otherlen) {
		return other->framebulks[thislen].tick;
	} else if (thislen > otherlen) {
		return this->framebulks[otherlen].tick;
	}

	return -1;
}
