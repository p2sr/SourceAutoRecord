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

	// Transferred from old Portal2 - should be removed if and when game
	// upgrades to new-style PIC engine
	GetClientStateFunction = 11;         // CEngineClient::ClientCmd
	GetLocalClient = 92;                 // CEngineClient::SetViewAngles
	HostState_OnClientConnected = 1523;  // CClientState::SetSignonState
	FireEventIntern = 36;                // CGameEventManager::FireEventClientSide
	ConPrintEvent = 254;                 // CGameEventManager::FireEventIntern
	AutoCompletionFunc = 37;             // listdemo_CompletionFunc
	Key_SetBinding = 60;                 // unbind
	VideoMode_Create = 104;              // CEngineAPI::Init
	AirMove_Offset1 = 14;                // CPortalGameMovement::~CPortalGameMovement
	UTIL_PlayerByIndex = 61;             // CServerGameDLL::Think
	GetClientMode = 11;                  // CHLClient::HudProcessInput
	GetHud = 104;                        // cc_leaderboard_enable
	FindElement = 120;                   // cc_leaderboard_enable
	KeyDown = 295;                       // CInput::JoyStickApplyMovement
	KeyUp = 341;                         // CInput::JoyStickApplyMovement
	StartDrawing = 692;                  // CMatSystemSurface::PaintTraverseEx
	FinishDrawing = 627;                 // CMatSystemSurface::PaintTraverseEx
}
const char *ThinkingWithTimeMachine::Version() {
	return "Thinking with Time Machine (5723)";
}
const char *ThinkingWithTimeMachine::GameDir() {
	return "Thinking with Time Machine";
}
