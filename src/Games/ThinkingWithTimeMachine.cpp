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
