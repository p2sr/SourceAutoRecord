#include "Client.hpp"

#include "Command.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/FovChanger.hpp"
#include "Features/GroundFramesCounter.hpp"
#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/ScrollSpeed.hpp"
#include "Features/Hud/StrafeQuality.hpp"
#include "Features/Imitator.hpp"
#include "Features/NetMessage.hpp"
#include "Features/OffsetFinder.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/ReplaySystem/ReplayRecorder.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Sync.hpp"
#include "Features/Stitcher.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#include <cstdarg>
#include <cstdint>
#include <cstring>

Variable cl_showpos;
Variable cl_sidespeed;
Variable cl_backspeed;
Variable cl_forwardspeed;
Variable in_forceuser;
Variable crosshairVariable;
Variable cl_fov;
Variable prevent_crouch_jump;
Variable r_portaltestents;

Variable sar_disable_coop_score_hud("sar_disable_coop_score_hud", "0", "Disables the coop score HUD which appears in demo playback.\n");
Variable sar_disable_save_status_hud("sar_disable_save_status_hud", "0", "Disables the saving/saved HUD which appears when you make a save.\n");

REDECL(Client::LevelInitPreEntity);
REDECL(Client::CreateMove);
REDECL(Client::CreateMove2);
REDECL(Client::GetName);
REDECL(Client::ShouldDraw_BasicInfo);
REDECL(Client::ShouldDraw_SaveStatus);
REDECL(Client::MsgFunc_SayText2);
REDECL(Client::DecodeUserCmdFromBuffer);
REDECL(Client::CInput_CreateMove);
REDECL(Client::GetButtonBits);
REDECL(Client::SteamControllerMove);
REDECL(Client::playvideo_end_level_transition_callback);
REDECL(Client::OverrideView);
REDECL(Client::ProcessMovement);

MDECL(Client::GetAbsOrigin, Vector, C_m_vecAbsOrigin);
MDECL(Client::GetAbsAngles, QAngle, C_m_angAbsRotation);
MDECL(Client::GetLocalVelocity, Vector, C_m_vecVelocity);
MDECL(Client::GetViewOffset, Vector, C_m_vecViewOffset);

DECL_CVAR_CALLBACK(cl_fov) {
	if (engine->demoplayer->IsPlaying())
		fovChanger->Force();
}

void *Client::GetPlayer(int index) {
	return this->GetClientEntity(this->s_EntityList->ThisPtr(), index);
}
void Client::CalcButtonBits(int nSlot, int &bits, int in_button, int in_ignore, kbutton_t *button, bool reset) {
	auto pButtonState = &button->m_PerUser[nSlot];
	if (pButtonState->state & 3) {
		bits |= in_button;
	}

	int clearmask = ~2;
	if (in_ignore & in_button) {
		clearmask = ~3;
	}

	if (reset) {
		pButtonState->state &= clearmask;
	}
}

bool Client::ShouldDrawCrosshair() {
	if (!crosshairVariable.GetBool()) {
		crosshairVariable.SetValue(1);
		auto value = this->ShouldDraw(this->g_HUDQuickInfo->ThisPtr());
		crosshairVariable.SetValue(0);
		return value;
	}

	return this->ShouldDraw(this->g_HUDQuickInfo->ThisPtr());
}

void Client::Chat(TextColor color, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	char data[1024];
	vsnprintf(data, sizeof(data), fmt, argptr);
	va_end(argptr);
	client->ChatPrintf(client->g_HudChat->ThisPtr(), 0, 0, "%c%s", color, data);
}

void Client::QueueChat(TextColor color, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	char data[1024];
	vsnprintf(data, sizeof data, fmt, argptr);
	va_end(argptr);
	this->chatQueue.push_back(std::pair(color, std::string(data)));
}

void Client::FlushChatQueue() {
	for (auto &s : this->chatQueue) {
		this->Chat(s.first, "%s", s.second.c_str());
	}
	this->chatQueue.clear();
}

void Client::SetMouseActivated(bool state) {
	if (state) {
		this->IN_ActivateMouse(g_Input->ThisPtr());
	} else {
		this->IN_DeactivateMouse(g_Input->ThisPtr());
	}
}

CMStatus Client::GetChallengeStatus() {
	if (engine->IsOrange()) {
		return sv_bonus_challenge.GetBool() ? CMStatus::CHALLENGE : CMStatus::NONE;
	}

	auto player = client->GetPlayer(1);
	if (!player) {
		return CMStatus::NONE;
	}

	int bonusChallenge = *(int *)((uintptr_t)player + Offsets::m_iBonusChallenge);

	if (bonusChallenge) {
		return CMStatus::CHALLENGE;
	} else if (sv_bonus_challenge.GetBool()) {
		return CMStatus::WRONG_WARP;
	} else {
		return CMStatus::NONE;
	}
}

int Client::GetSplitScreenPlayerSlot(void *entity) {
	for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
		if (client->GetPlayer(i + 1) == entity) {
			return i;
		}
	}
	return 0;
}

// CHLClient::LevelInitPreEntity
DETOUR(Client::LevelInitPreEntity, const char *levelName) {
	client->lastLevelName = std::string(levelName);
	return Client::LevelInitPreEntity(thisptr, levelName);
}

// ClientModeShared::CreateMove
DETOUR(Client::CreateMove, float flInputSampleTime, CUserCmd *cmd) {
	if (cmd->command_number) {
		if (replayPlayer1->IsPlaying()) {
			replayPlayer1->Play(replayProvider->GetCurrentReplay(), cmd);
		} else if (replayRecorder1->IsRecording()) {
			replayRecorder1->Record(replayProvider->GetCurrentReplay(), cmd);
		}
	}

	if (sar_mimic.isRegistered && sar_mimic.GetBool()) {
		imitator->Save(cmd);
	}

	if (!in_forceuser.isReference || (in_forceuser.isReference && !in_forceuser.GetBool())) {
		if (engine->IsCoop() && engine->IsOrange())
			inputHud.SetInputInfo(1, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
		else
			inputHud.SetInputInfo(0, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
	}

	if (sv_cheats.GetBool() && engine->hoststate->m_activeGame) {
		camera->OverrideMovement(cmd);
	}

	if (engine->hoststate->m_activeGame) {
		Stitcher::OverrideMovement(cmd);
	}

	if (sar_strafesync.GetBool()) {
		synchro->UpdateSync(engine->IsOrange() ? 1 : 0, cmd);
	}

	strafeQuality.OnUserCmd(engine->IsOrange() ? 1 : 0, *cmd);

	return Client::CreateMove(thisptr, flInputSampleTime, cmd);
}
DETOUR(Client::CreateMove2, float flInputSampleTime, CUserCmd *cmd) {
	if (cmd->command_number) {
		if (replayPlayer2->IsPlaying()) {
			replayPlayer2->Play(replayProvider->GetCurrentReplay(), cmd);
		} else if (replayRecorder2->IsRecording()) {
			replayRecorder2->Record(replayProvider->GetCurrentReplay(), cmd);
		}
	}

	if (sar_mimic.GetBool() && (!sv_bonus_challenge.GetBool() || sv_cheats.GetBool())) {
		imitator->Modify(cmd);
	}

	if (in_forceuser.GetBool()) {
		inputHud.SetInputInfo(1, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
	}

	if (sar_strafesync.GetBool()) {
		synchro->UpdateSync(1, cmd);
	}

	strafeQuality.OnUserCmd(1, *cmd);

	return Client::CreateMove2(thisptr, flInputSampleTime, cmd);
}

// CHud::GetName
DETOUR_T(const char *, Client::GetName) {
	// Never allow CHud::FindElement to find this HUD
	if (sar_disable_challenge_stats_hud.GetBool())
		return "";

	return Client::GetName(thisptr);
}

// CHudMultiplayerBasicInfo::ShouldDraw
DETOUR_T(bool, Client::ShouldDraw_BasicInfo) {
	if (sar_disable_coop_score_hud.GetBool()) {
		return false;
	}

	return Client::ShouldDraw_BasicInfo(thisptr);
}

// CHudSaveStatus::ShouldDraw
DETOUR_T(bool, Client::ShouldDraw_SaveStatus) {
	if (sar_disable_save_status_hud.GetBool()) {
		return false;
	}

	return Client::ShouldDraw_SaveStatus(thisptr);
}

// CHudChat::MsgFunc_SayText2
DETOUR(Client::MsgFunc_SayText2, bf_read &msg) {
	// copy old state in case we need to recover it
	bf_read pre = msg;

	// skip client id
	msg.ReadUnsigned(8);

	std::string str = "";
	while (true) {
		char c = (char)(uint8_t)msg.ReadUnsigned(8);
		if (!c) break;
		str += c;
	}

	if (NetMessage::ChatData(str)) {
		// skip the other crap, just in case it matters
		msg.ReadUnsigned(8);
		return 0;
	}

	msg = pre;

	return Client::MsgFunc_SayText2(thisptr, msg);
}

// CInput::DecodeUserCmdFromBuffer
DETOUR(Client::DecodeUserCmdFromBuffer, int nSlot, int buf, signed int sequence_number) {
	auto result = Client::DecodeUserCmdFromBuffer(thisptr, nSlot, buf, sequence_number);

	auto m_pCommands = *reinterpret_cast<uintptr_t *>((uintptr_t)thisptr + nSlot * Offsets::PerUserInput_tSize + Offsets::m_pCommands);
	auto cmd = reinterpret_cast<CUserCmd *>(m_pCommands + Offsets::CUserCmdSize * (sequence_number % Offsets::MULTIPLAYER_BACKUP));

	Vector cmdMove = {cmd->sidemove, cmd->forwardmove, cmd->upmove};
	if (nSlot == 0) {
		// A bit weird - for some reason, when playing back Orange
		// demos, nSlot is 0 even though the player's actual slot
		// (including in HUD stuff) is 1. This works as a workaround
		inputHud.SetInputInfo(0, cmd->buttons, cmdMove);
		inputHud.SetInputInfo(1, cmd->buttons, cmdMove);
	} else if (nSlot == 1) {
		inputHud.SetInputInfo(1, cmd->buttons, cmdMove);
	}

	if (sar_strafesync.GetBool()) {
		synchro->UpdateSync(nSlot, cmd);
	}

	strafeQuality.OnUserCmd(nSlot, *cmd);
	void *player = client->GetPlayer(nSlot + 1);
	if (player) {
		unsigned int groundHandle = *(unsigned int *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
		bool grounded = groundHandle != 0xFFFFFFFF;
		groundFramesCounter->HandleMovementFrame(nSlot, grounded);
		strafeQuality.OnMovement(nSlot, grounded);
		Event::Trigger<Event::PROCESS_MOVEMENT>({ nSlot, false }); // There isn't really one, just pretend it's here lol
	}

	return result;
}

// CInput::CreateMove
DETOUR(Client::CInput_CreateMove, int sequence_number, float input_sample_frametime, bool active) {
	auto result = Client::CInput_CreateMove(thisptr, sequence_number, input_sample_frametime, active);

	return result;
}

// CInput::GetButtonBits
DETOUR(Client::GetButtonBits, bool bResetState) {
	auto bits = Client::GetButtonBits(thisptr, bResetState);

	return bits;
}

// CInput::SteamControllerMove
DETOUR(Client::SteamControllerMove, int nSlot, float flFrametime, CUserCmd *cmd) {
	auto result = Client::SteamControllerMove(thisptr, nSlot, flFrametime, cmd);

	tasController->ControllerMove(nSlot, flFrametime, cmd);

	return result;
}

DETOUR_COMMAND(Client::playvideo_end_level_transition) {
	console->DevMsg("%s\n", args.m_pArgSBuffer);
	//session->Ended();

	return Client::playvideo_end_level_transition_callback(args);
}

DETOUR(Client::OverrideView, CViewSetup *m_View) {
	camera->OverrideView(m_View);
	Stitcher::OverrideView(m_View);
	return Client::OverrideView(thisptr, m_View);
}

DETOUR(Client::ProcessMovement, void *player, CMoveData *move) {
	// This should only be run if prediction is occurring, i.e. if we
	// are orange, but check anyway
	if (engine->IsOrange() && session->isRunning) {
		// The client does prediction very often (twice per frame?) so
		// we have to do some weird stuff
		static int lastTick;

		int tick = session->GetTick();

		if (tick != lastTick) {
			unsigned int groundHandle = *(unsigned int *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
			bool grounded = groundHandle != 0xFFFFFFFF;
			int slot = client->GetSplitScreenPlayerSlot(player);
			groundFramesCounter->HandleMovementFrame(slot, grounded);
			strafeQuality.OnMovement(slot, grounded);
			if (move->m_nButtons & IN_JUMP) scrollSpeedHud.OnJump(slot);
			Event::Trigger<Event::PROCESS_MOVEMENT>({ slot, false });
			lastTick = tick;
		}
	}

	return Client::ProcessMovement(thisptr, player, move);
}

bool Client::Init() {
	bool readJmp = false;

	this->g_ClientDLL = Interface::Create(this->Name(), "VClient016");
	this->s_EntityList = Interface::Create(this->Name(), "VClientEntityList003", false);
	this->g_GameMovement = Interface::Create(this->Name(), "GameMovement001");

	if (this->g_GameMovement) {
		this->g_GameMovement->Hook(Client::ProcessMovement_Hook, Client::ProcessMovement, Offsets::ProcessMovement);
	}

	if (this->g_ClientDLL) {
		this->GetAllClasses = this->g_ClientDLL->Original<_GetAllClasses>(Offsets::GetAllClasses, readJmp);

		this->g_ClientDLL->Hook(Client::LevelInitPreEntity_Hook, Client::LevelInitPreEntity, Offsets::LevelInitPreEntity);

		auto leaderboard = Command("+leaderboard");
		if (!!leaderboard) {
			using _GetHud = void *(__cdecl *)(int unk);
			using _FindElement = void *(__rescall *)(void *thisptr, const char *pName);

			auto cc_leaderboard_enable = (uintptr_t)leaderboard.ThisPtr()->m_pCommandCallback;
			auto GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
			auto FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);
			auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");

			if (this->g_HUDChallengeStats = Interface::Create(CHUDChallengeStats)) {
				this->g_HUDChallengeStats->Hook(Client::GetName_Hook, Client::GetName, Offsets::GetName);
			}

			auto CHUDQuickInfo = FindElement(GetHud(-1), "CHUDQuickInfo");

			if (this->g_HUDQuickInfo = Interface::Create(CHUDQuickInfo)) {
				this->ShouldDraw = this->g_HUDQuickInfo->Original<_ShouldDraw>(Offsets::ShouldDraw, readJmp);
			}

			auto CHudChat = FindElement(GetHud(-1), "CHudChat");
			if (this->g_HudChat = Interface::Create(CHudChat)) {
				this->ChatPrintf = g_HudChat->Original<_ChatPrintf>(Offsets::ChatPrintf);
				this->g_HudChat->Hook(Client::MsgFunc_SayText2_Hook, Client::MsgFunc_SayText2, Offsets::MsgFunc_SayText2);
			}

			auto CHudMultiplayerBasicInfo = FindElement(GetHud(-1), "CHudMultiplayerBasicInfo");
			if (this->g_HudMultiplayerBasicInfo = Interface::Create(CHudMultiplayerBasicInfo)) {
				this->g_HudMultiplayerBasicInfo->Hook(Client::ShouldDraw_BasicInfo_Hook, Client::ShouldDraw_BasicInfo, Offsets::ShouldDraw);
			}

			auto CHudSaveStatus = FindElement(GetHud(-1), "CHudSaveStatus");
			if (this->g_HudSaveStatus = Interface::Create(CHudSaveStatus)) {
				this->g_HudSaveStatus->Hook(Client::ShouldDraw_SaveStatus_Hook, Client::ShouldDraw_SaveStatus, Offsets::ShouldDraw);
			}
		}

		this->IN_ActivateMouse = this->g_ClientDLL->Original<_IN_ActivateMouse>(Offsets::IN_ActivateMouse, readJmp);
		this->IN_DeactivateMouse = this->g_ClientDLL->Original<_IN_DeactivateMouse>(Offsets::IN_DeactivateMouse, readJmp);

		auto IN_ActivateMouse = this->g_ClientDLL->Original(Offsets::IN_ActivateMouse, readJmp);
		void *g_InputAddr;
#ifndef _WIN32
		if (sar.game->Is(SourceGame_EIPRelPIC)) {
			g_InputAddr = *(void **)(IN_ActivateMouse + 5 + *(uint32_t *)(IN_ActivateMouse + 6) + *(uint32_t *)(IN_ActivateMouse + 12));
		} else
#endif
			g_InputAddr = Memory::DerefDeref<void *>(IN_ActivateMouse + Offsets::g_Input);

		if (g_Input = Interface::Create(g_InputAddr)) {
			g_Input->Hook(Client::DecodeUserCmdFromBuffer_Hook, Client::DecodeUserCmdFromBuffer, Offsets::DecodeUserCmdFromBuffer);
			g_Input->Hook(Client::GetButtonBits_Hook, Client::GetButtonBits, Offsets::GetButtonBits);
			g_Input->Hook(Client::SteamControllerMove_Hook, Client::SteamControllerMove, Offsets::SteamControllerMove);

			auto JoyStickApplyMovement = g_Input->Original(Offsets::JoyStickApplyMovement, readJmp);
			Memory::Read(JoyStickApplyMovement + Offsets::KeyDown, &this->KeyDown);
			Memory::Read(JoyStickApplyMovement + Offsets::KeyUp, &this->KeyUp);

			in_forceuser = Variable("in_forceuser");
			if (!!in_forceuser && this->g_Input) {
				this->g_Input->Hook(CInput_CreateMove_Hook, CInput_CreateMove, Offsets::GetButtonBits + 1);
			}

			Command::Hook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback_hook, Client::playvideo_end_level_transition_callback);
		}

		auto HudProcessInput = this->g_ClientDLL->Original(Offsets::HudProcessInput, readJmp);
		auto GetClientMode = Memory::Read<uintptr_t>(HudProcessInput + Offsets::GetClientMode);
		uintptr_t g_pClientMode;
#ifndef _WIN32
		if (sar.game->Is(SourceGame_EIPRelPIC)) {
			g_pClientMode = GetClientMode + 6 + *(uint32_t *)(GetClientMode + 8) + *(uint32_t *)(GetClientMode + 35);
		} else
#endif
			g_pClientMode = Memory::Deref<uintptr_t>(GetClientMode + Offsets::g_pClientMode);
		void *clientMode = Memory::Deref<void *>(g_pClientMode);
		void *clientMode2 = Memory::Deref<void *>(g_pClientMode + sizeof(void *));

		if (this->g_pClientMode = Interface::Create(clientMode)) {
			this->g_pClientMode->Hook(Client::CreateMove_Hook, Client::CreateMove, Offsets::CreateMove);
			this->g_pClientMode->Hook(Client::OverrideView_Hook, Client::OverrideView, Offsets::OverrideView);
		}

		if (this->g_pClientMode2 = Interface::Create(clientMode2)) {
			this->g_pClientMode2->Hook(Client::CreateMove2_Hook, Client::CreateMove2, Offsets::CreateMove);
		}
	}

	Variable("r_PortalTestEnts").RemoveFlag(FCVAR_CHEAT);

	if (this->s_EntityList) {
		this->GetClientEntity = this->s_EntityList->Original<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
	}

	offsetFinder->ClientSide("CBasePlayer", "m_vecVelocity[0]", &Offsets::C_m_vecVelocity);
	offsetFinder->ClientSide("CBasePlayer", "m_vecViewOffset[0]", &Offsets::C_m_vecViewOffset);
	offsetFinder->ClientSide("CBasePlayer", "m_hGroundEntity", &Offsets::C_m_hGroundEntity);
	offsetFinder->ClientSide("CBasePlayer", "m_iBonusChallenge", &Offsets::m_iBonusChallenge);
	offsetFinder->ClientSide("CPortal_Player", "m_StatsThisLevel", &Offsets::C_m_StatsThisLevel);

	cl_showpos = Variable("cl_showpos");
	cl_sidespeed = Variable("cl_sidespeed");
	cl_forwardspeed = Variable("cl_forwardspeed");
	cl_backspeed = Variable("cl_backspeed");
	prevent_crouch_jump = Variable("prevent_crouch_jump");
	crosshairVariable = Variable("crosshair");
	r_portaltestents = Variable("r_portaltestents");

	// Useful for fixing rendering bugs
	r_portaltestents.RemoveFlag(FCVAR_CHEAT);

	CVAR_HOOK_AND_CALLBACK(cl_fov);

	return this->hasLoaded = this->g_ClientDLL && this->s_EntityList;
}
void Client::Shutdown() {
	r_portaltestents.AddFlag(FCVAR_CHEAT);
	Interface::Delete(this->g_ClientDLL);
	Interface::Delete(this->g_pClientMode);
	Interface::Delete(this->g_pClientMode2);
	Interface::Delete(this->g_HUDChallengeStats);
	Interface::Delete(this->s_EntityList);
	Interface::Delete(this->g_Input);
	Interface::Delete(this->g_HUDQuickInfo);
	Interface::Delete(this->g_HudChat);
	Interface::Delete(this->g_HudMultiplayerBasicInfo);
	Interface::Delete(this->g_HudSaveStatus);
	Interface::Delete(this->g_GameMovement);
	Command::Unhook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback);
}

Client *client;
