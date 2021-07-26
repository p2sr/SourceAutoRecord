#include "ApertureTag.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ApertureTag::ApertureTag() {
	this->version = SourceGame_ApertureTag;
	Game::mapNames = {
		"gg_intro_wakeup",
		"gg_blue_only",
		"gg_blue_only_2",
		"gg_blue_only_3",
		"gg_blue_only_2_pt2",
		"gg_a1_intro4",
		"gg_blue_upplatform",
		"gg_red_only",
		"gg_red_surf",
		"gg_all_intro",
		"gg_all_rotating_wall",
		"gg_all_fizzler",
		"gg_all_intro_2",
		"gg_a2_column_blocker",
		"gg_all_puzzle2",
		"gg_all2_puzzle1",
		"gg_all_puzzle1",
		"gg_all2_escape",
		"gg_stage_reveal",
		"gg_stage_bridgebounce_2",
		"gg_stage_redfirst",
		"gg_stage_laserrelay",
		"gg_stage_beamscotty",
		"gg_stage_bridgebounce",
		"gg_stage_roofbounce",
		"gg_stage_pickbounce",
		"gg_stage_theend",
	};
}
void ApertureTag::LoadOffsets() {
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
const char *ApertureTag::Version() {
	return "Aperture Tag (7054)";
}
const char *ApertureTag::GameDir() {
	return "Aperture Tag";
}
