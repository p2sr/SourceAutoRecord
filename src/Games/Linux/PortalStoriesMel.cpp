#include "PortalStoriesMel.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalStoriesMel::PortalStoriesMel() {
	this->version = SourceGame_PortalStoriesMel;
	Game::mapNames = {
		"sp_a1_tramride",
		"sp_a1_mel_intro",
		"sp_a1_lift",
		"sp_a1_garden",
		"sp_a2_garden_de",
		"sp_a2_underbounce",
		"sp_a2_once_upon",
		"sp_a2_past_power",
		"sp_a2_ramp",
		"sp_a2_firestorm",
		"sp_a3_junkyard",
		"sp_a3_concepts",
		"sp_a3_paint_fling",
		"sp_a3_faith_plate",
		"sp_a3_transition",
		"sp_a4_overgrown",
		"sp_a4_tb_over_goo",
		"sp_a4_two_of_a_kind",
		"sp_a4_destroyed",
		"sp_a4_factory",
		"sp_a4_core_access",
		"sp_a4_finale"};
}
void PortalStoriesMel::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

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
const char *PortalStoriesMel::Version() {
	return "Portal Stories: Mel (5723)";
}
const char *PortalStoriesMel::ModDir() {
	return "portal_stories";
}
