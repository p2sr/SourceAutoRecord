#include "ThinkingWithTimeMachine.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ThinkingWithTimeMachine::ThinkingWithTimeMachine() {
	this->version = SourceGame_ThinkingWithTimeMachine;
}
void ThinkingWithTimeMachine::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

#ifdef _WIN32
	// client.dll
	m_pCommands = 228;  // CInput::DecodeUserCmdFromBuffer
#else
	// client.so
	m_pCommands = 228;  // CInput::DecodeUserCmdFromBuffer

#define OFFSET_DEFAULT(name, win, linux)
#define OFFSET_EMPTY(name)
#define OFFSET_LINMOD(name, off) name = off;
#include "OffsetsData.hpp"
#endif
}
const char *ThinkingWithTimeMachine::Version() {
	return "Thinking with Time Machine (5723)";
}
const char *ThinkingWithTimeMachine::GameDir() {
	return "Thinking with Time Machine";
}
