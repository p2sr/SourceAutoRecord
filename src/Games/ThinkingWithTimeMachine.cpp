#include "ThinkingWithTimeMachine.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ThinkingWithTimeMachine::ThinkingWithTimeMachine() {
	this->version = SourceGame_ThinkingWithTimeMachine;
	Game::maps = {
		{"tm_intro_01",          "Intro"},
		{"tm_training_01b",      "Training"},
		{"tm_map_01b",           "Bridge"},
		{"tm_scene_map-update2", "Robots"},
		{"tm_map_02b",           "Lasers"},
		{"tm_map_03b",           "Three Cubes"},
		{"tm_map_04a",           "Laser Over Goo"},
		{"tm_map_05a-update2",   "Ceiling Buttons"},
		{"tm_map_06a",           "Catapults"},
		{"tm_map_07",            "Fizzlers"},
		{"tm_map_08",            "Buttons"},
		{"tm_map_final",         "Finale"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}
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
#include "Offsets/Default.hpp"
#endif

#ifndef _WIN32
    tickcount = 73; // CClientState::ProcessTick
    interval_per_tick = 81; // CClientState::ProcessTick
    HostState_OnClientConnected = 735; // CClientState::SetSignonState
    demoplayer = 93; // CClientState::Disconnect
    demorecorder = 106; // CClientState::Disconnect
	m_szLevelName = 72; // CEngineTool::GetCurrentMap
	GetClientMode = 12; // CHLClient::HudProcessInput
	StartDrawing = 193; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 590; // CMatSystemSurface::PaintTraverseEx
    GetLocalClient = 85; // CEngineClient::SetViewAngles
	net_time = 28; // CDemoRecorder::GetRecordingTick
	PerUserInput_tSize = 344; // CInput::DecodeUserCmdFromBuffer
	VideoMode_Create = 103; // CEngineAPI::Init
	snd_linear_count = 33; // SND_RecordBuffer
	snd_p = 72; // SND_RecordBuffer
	snd_vol = 80; // SND_RecordBuffer
#endif
}
const char *ThinkingWithTimeMachine::Version() {
	return "Thinking with Time Machine (5723)";
}
const char *ThinkingWithTimeMachine::GameDir() {
	return "Thinking with Time Machine";
}
