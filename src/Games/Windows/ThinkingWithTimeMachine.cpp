#include "ThinkingWithTimeMachine.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ThinkingWithTimeMachine::ThinkingWithTimeMachine() {
	this->version = SourceGame_ThinkingWithTimeMachine;
}
void ThinkingWithTimeMachine::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

	// client.dll

	m_pCommands = 228;  // CInput::DecodeUserCmdFromBuffer
}
const char *ThinkingWithTimeMachine::Version() {
	return "Thinking with Time Machine (5723)";
}
const char *ThinkingWithTimeMachine::GameDir() {
	return "Thinking with Time Machine";
}
