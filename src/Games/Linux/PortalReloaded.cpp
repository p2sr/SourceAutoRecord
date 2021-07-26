#include "PortalReloaded.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalReloaded::PortalReloaded() {
	this->version = SourceGame_PortalReloaded;
	Game::mapNames = {
		"sp_a1_pr_map_001",
		"sp_a1_pr_map_002",
		"sp_a1_pr_map_003",
		"sp_a1_pr_map_004",
		"sp_a1_pr_map_005",
		"sp_a1_pr_map_006",
		"sp_a1_pr_map_007",
		"sp_a1_pr_map_008",
		"sp_a1_pr_map_009",
		"sp_a1_pr_map_010",
		"sp_a1_pr_map_011",
		"sp_a1_pr_map_012",
	};
}
void PortalReloaded::LoadOffsets() {
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
const char *PortalReloaded::Version() {
	return "Portal Reloaded";
}
const char *PortalReloaded::ModDir() {
	return "portalreloaded";
}
