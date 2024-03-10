#include "Client.hpp"

#include "Command.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/FovChanger.hpp"
#include "Features/GroundFramesCounter.hpp"
#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/ScrollSpeed.hpp"
#include "Features/Hud/StrafeHud.hpp"
#include "Features/Hud/StrafeQuality.hpp"
#include "Features/NetMessage.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/PlayerTrace.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Sync.hpp"
#include "Features/Stitcher.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Game.hpp"
#include "Hook.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <deque>

Variable cl_showpos;
Variable cl_sidespeed;
Variable cl_backspeed;
Variable cl_forwardspeed;
Variable in_forceuser;
Variable crosshairVariable;
Variable cl_fov;
Variable prevent_crouch_jump;
Variable r_portaltestents;
Variable r_portalsopenall;
Variable r_drawviewmodel;

Variable sar_disable_coop_score_hud("sar_disable_coop_score_hud", "0", "Disables the coop score HUD which appears in demo playback.\n");
Variable sar_disable_save_status_hud("sar_disable_save_status_hud", "0", "Disables the saving/saved HUD which appears when you make a save.\n");

Variable sar_patch_small_angle_decay("sar_patch_small_angle_decay", "0", "Patches small angle decay (not minor decay).\n");
Variable sar_patch_major_angle_decay("sar_patch_major_angle_decay", "0", "Patches major pitch angle decay. Requires cheats.\n");
#ifdef _WIN32
Variable sar_patch_minor_angle_decay("sar_patch_minor_angle_decay", "0", "Patches minor pitch angle decay present on Windows version of the game.\n");
#endif

REDECL(Client::LevelInitPreEntity);
REDECL(Client::CreateMove);
REDECL(Client::CreateMove2);
REDECL(Client::GetName);
REDECL(Client::ShouldDraw_BasicInfo);
REDECL(Client::ShouldDraw_SaveStatus);
REDECL(Client::MsgFunc_SayText2);
REDECL(Client::GetTextColorForClient);
REDECL(Client::DecodeUserCmdFromBuffer);
REDECL(Client::CInput_CreateMove);
REDECL(Client::GetButtonBits);
REDECL(Client::ApplyMouse);
REDECL(Client::SteamControllerMove);
REDECL(Client::playvideo_end_level_transition_callback);
REDECL(Client::OverrideView);
REDECL(Client::ProcessMovement);
REDECL(Client::DrawTranslucentRenderables);
REDECL(Client::DrawOpaqueRenderables);
REDECL(Client::CalcViewModelLag);
REDECL(Client::AddShadowToReceiver);
#ifdef _WIN32
REDECL(Client::ApplyMouse_Mid);
REDECL(Client::ApplyMouse_Mid_Continue);
#endif


CMDECL(Client::GetAbsOrigin, Vector, m_vecAbsOrigin);
CMDECL(Client::GetAbsAngles, QAngle, m_angAbsRotation);
CMDECL(Client::GetLocalVelocity, Vector, m_vecVelocity);
CMDECL(Client::GetViewOffset, Vector, m_vecViewOffset);
CMDECL(Client::GetPortalLocal, CPortalPlayerLocalData, m_PortalLocal);
CMDECL(Client::GetPlayerState, CPlayerState, pl);

DECL_CVAR_CALLBACK(cl_fov) {
	if (engine->demoplayer->IsPlaying())
		fovChanger->Force();
}

ClientEnt *Client::GetPlayer(int index) {
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

static std::deque<Color> g_nameColorOverrides;

void Client::Chat(Color col, const char *str) {
	g_nameColorOverrides.push_back(col);
	client->ChatPrintf(client->g_HudChat->ThisPtr(), 0, 0, "%c%s", TextColor::PLAYERNAME, str);
}

void Client::MultiColorChat(const std::vector<std::pair<Color, std::string>> &components) {
	// this sucks, but because c varargs are stupid, we have to construct a *format* string containing what we want (escaping any % signs)
	std::string fmt = "";

	for (auto &comp : components) {
		g_nameColorOverrides.push_back(comp.first);
		fmt += (char)TextColor::PLAYERNAME;
		for (char c : comp.second) {
			if (c == '%') fmt += '%';
			fmt += c;
		}
	}

	client->ChatPrintf(client->g_HudChat->ThisPtr(), 0, 0, fmt.c_str());
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

	int bonusChallenge = player->field<int>("m_iBonusChallenge");

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

void Client::ClFrameStageNotify(int stage) {
	this->FrameStageNotify(this->g_ClientDLL->ThisPtr(), stage);
}

void Client::OpenChat() {
	this->StartMessageMode(this->g_HudChat->ThisPtr(), 1);  // MM_SAY
}

// CHLClient::LevelInitPreEntity
DETOUR(Client::LevelInitPreEntity, const char *levelName) {
	client->lastLevelName = std::string(levelName);
	return Client::LevelInitPreEntity(thisptr, levelName);
}

// ClientModeShared::CreateMove
DETOUR(Client::CreateMove, float flInputSampleTime, CUserCmd *cmd) {
	if (!in_forceuser.isReference || (in_forceuser.isReference && !in_forceuser.GetBool())) {
		if (engine->IsCoop() && engine->IsOrange())
			inputHud.SetInputInfo(1, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
		else
			inputHud.SetInputInfo(0, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
	}

	if (sv_cheats.GetBool() && engine->hoststate->m_activeGame) {
		camera->OverrideMovement(cmd);
	}

	if (GhostEntity::GetFollowTarget()) {
		cmd->buttons = 0;
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
	}

	if (engine->hoststate->m_activeGame) {
		Stitcher::OverrideMovement(cmd);
	}

	if (sar_strafesync.GetBool()) {
		synchro->UpdateSync(engine->IsOrange() ? 1 : 0, cmd);
	}

	strafeQuality.OnUserCmd(engine->IsOrange() ? 1 : 0, *cmd);

	if (cmd->buttons & IN_ATTACK) {
		int slot = engine->IsOrange() ? 1 : 0;
		g_bluePortalAngles[slot] = engine->GetAngles(slot);
	}

	if (cmd->buttons & IN_ATTACK2) {
		int slot = engine->IsOrange() ? 1 : 0;
		g_orangePortalAngles[slot] = engine->GetAngles(slot);
	}

	return Client::CreateMove(thisptr, flInputSampleTime, cmd);
}
DETOUR(Client::CreateMove2, float flInputSampleTime, CUserCmd *cmd) {
	if (in_forceuser.GetBool()) {
		inputHud.SetInputInfo(1, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
	}

	if (sar_strafesync.GetBool()) {
		synchro->UpdateSync(1, cmd);
	}

	strafeQuality.OnUserCmd(1, *cmd);

	if (cmd->buttons & IN_ATTACK) {
		g_bluePortalAngles[1] = engine->GetAngles(1);
	}

	if (cmd->buttons & IN_ATTACK2) {
		g_orangePortalAngles[1] = engine->GetAngles(1);
	}

	return Client::CreateMove2(thisptr, flInputSampleTime, cmd);
}

// CHud::GetName
DETOUR_T(const char *, Client::GetName) {
	if (sar_disable_challenge_stats_hud.GetBool()) return "";
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
	if (sar.game->Is(SourceGame_Portal2)) {
		while (true) {
			char c = (char)(uint8_t)msg.ReadUnsigned(8);
			if (!c) break;
			str += c;
		}

		if (str != "\x01Portal2_Coloured_Chat_Format") {
			console->Print("Your partner is on an old version of Portal 2! Tell them to update.\n");
			msg = pre;
			return Client::MsgFunc_SayText2(thisptr, msg);
		}

		// Skip player name
		while (true) {
			char c = (char)(uint8_t)msg.ReadUnsigned(8);
			if (!c) break;
		}
	} else if (sar.game->Is(SourceGame_PortalReloaded)) {
		// Reloaded uses the legacy format where it's just one string
		while (true) {
			char c = (char)(uint8_t)msg.ReadUnsigned(8);
			if (!c) break;
			str += c;
			// People can put colons in names and chats, stupid.
			// Too bad!
			if (Utils::EndsWith(str, ": ")) break;
		}
	}

	// Read actual message
	str = "";
	while (true) {
		char c = (char)(uint8_t)msg.ReadUnsigned(8);
		if (!c) break;
		str += c;
	}
	// remove any newlines (reloaded / old chat format has one trailing)
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

	if (NetMessage::ChatData(str)) {
		// skip the other crap, just in case it matters
		msg.ReadUnsigned(8);
		return 0;
	}

	msg = pre;

	return Client::MsgFunc_SayText2(thisptr, msg);
}

// MSVC bug workaround - see Engine::GetColorAtPoint for explanation
#ifdef _WIN32
DETOUR_T(void *, Client::GetTextColorForClient, Color *col_out, TextColor color, int client_idx) {
#else
DETOUR_T(Color, Client::GetTextColorForClient, TextColor color, int client_idx) {
#endif
	Color ret;
	if (!g_nameColorOverrides.empty() && color == TextColor::PLAYERNAME) {
		ret = g_nameColorOverrides.front();
		g_nameColorOverrides.pop_front();
	} else {
#ifdef _WIN32
		Client::GetTextColorForClient(thisptr, &ret, color, client_idx);
#else
		ret = Client::GetTextColorForClient(thisptr, color, client_idx);
#endif
	}
#ifdef _WIN32
	*col_out = ret;
	return col_out;
#else
	return ret;
#endif
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
		bool grounded = CE(player)->ground_entity();
		groundFramesCounter->HandleMovementFrame(nSlot, grounded);
		strafeQuality.OnMovement(nSlot, grounded);
		strafeHud.SetData(nSlot, player, cmd, false);
		Event::Trigger<Event::PROCESS_MOVEMENT>({nSlot, false});  // There isn't really one, just pretend it's here lol
	}

	if (cmd->buttons & IN_ATTACK) {
		g_bluePortalAngles[nSlot] = engine->GetAngles(nSlot);
	}

	if (cmd->buttons & IN_ATTACK2) {
		g_orangePortalAngles[nSlot] = engine->GetAngles(nSlot);
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

#ifdef _WIN32
extern Hook g_ApplyMouseMidHook;
DETOUR_MID_MH(Client::ApplyMouse_Mid) {
	static float input;
	static float result;
	_asm {
		fstp dword ptr[input]
	}

	result = acosf(input);

	_asm {
		fld dword ptr[result]
		jmp Client::ApplyMouse_Mid_Continue
	}
}
Hook g_ApplyMouseMidHook(&Client::ApplyMouse_Mid_Hook);
#endif

static void (*MatrixBuildRotationAboutAxis)(Vector &, float, matrix3x4_t &);
extern Hook MatrixBuildRotationAboutAxisHook;
static void MatrixBuildRotationAboutAxis_Detour(Vector *vAxisOfRot, float angleDegrees, matrix3x4_t *dst) {
	float radians;
	float xSquared;
	float ySquared;
	float zSquared;
	float fSin;
	float fCos;

	// this is the actual patch
	// some decay will happen initially when you get major decay
	if (fabsf(vAxisOfRot->z - 1.0f) < 0.000001f) {
		vAxisOfRot->z = 1.0f;
	}

	radians = angleDegrees * (M_PI / 180.0);
	#ifdef _WIN32
		fSin = sin(radians);
		fCos = cos(radians);
	#else
		sincosf(radians, &fSin, &fCos);
	#endif

	xSquared = vAxisOfRot->x * vAxisOfRot->x;
	ySquared = vAxisOfRot->y * vAxisOfRot->y;
	zSquared = vAxisOfRot->z * vAxisOfRot->z;

	dst->m_flMatVal[0][0] = xSquared + (1 - xSquared) * fCos;
	dst->m_flMatVal[1][0] = vAxisOfRot->x * vAxisOfRot->y * (1 - fCos) + vAxisOfRot->z * fSin;
	dst->m_flMatVal[2][0] = vAxisOfRot->z * vAxisOfRot->x * (1 - fCos) - vAxisOfRot->y * fSin;

	dst->m_flMatVal[0][1] = vAxisOfRot->x * vAxisOfRot->y * (1 - fCos) - vAxisOfRot->z * fSin;
	dst->m_flMatVal[1][1] = ySquared + (1 - ySquared) * fCos;
	dst->m_flMatVal[2][1] = vAxisOfRot->y * vAxisOfRot->z * (1 - fCos) + vAxisOfRot->x * fSin;

	dst->m_flMatVal[0][2] = vAxisOfRot->z * vAxisOfRot->x * (1 - fCos) + vAxisOfRot->y * fSin;
	dst->m_flMatVal[1][2] = vAxisOfRot->y * vAxisOfRot->z * (1 - fCos) - vAxisOfRot->x * fSin;
	dst->m_flMatVal[2][2] = zSquared + (1 - zSquared) * fCos;

	dst->m_flMatVal[0][3] = 0;
	dst->m_flMatVal[1][3] = 0;
	dst->m_flMatVal[2][3] = 0;
}
Hook MatrixBuildRotationAboutAxisHook(&MatrixBuildRotationAboutAxis_Detour);

// C_Paint_Input::ApplyMouse
DETOUR(Client::ApplyMouse, int nSlot, QAngle &viewangles, CUserCmd *cmd, float mouse_x, float mouse_y) {
	auto lastViewAngles = viewangles;

#ifdef _WIN32
	if (sar_patch_minor_angle_decay.GetBool()) g_ApplyMouseMidHook.Enable();
#endif
	if (sar_patch_major_angle_decay.GetBool() && sv_cheats.GetBool()) MatrixBuildRotationAboutAxisHook.Enable();
	auto result = Client::ApplyMouse(thisptr, nSlot, viewangles, cmd, mouse_x, mouse_y);
	if (sar_patch_major_angle_decay.GetBool() && sv_cheats.GetBool()) MatrixBuildRotationAboutAxisHook.Disable();
#ifdef _WIN32
	if (sar_patch_minor_angle_decay.GetBool()) g_ApplyMouseMidHook.Disable();
#endif

	Vector delta = {
		viewangles.x - lastViewAngles.x,
		viewangles.y - lastViewAngles.y,
		viewangles.z - lastViewAngles.z,
	};

	auto upDelta = 1.0f;
	auto player = client->GetPlayer(nSlot + 1);
	if (player) {
		upDelta = fabsf(client->GetPortalLocal(player).m_up.z - 1);
	}

	if (sar_patch_small_angle_decay.GetBool()) {
		// yaw decay
		if (mouse_x == 0.0f && delta.y != 0.0f) viewangles.y = lastViewAngles.y;
		if ((upDelta == 0.0f || (fabsf(viewangles.x) < 45.0f))
#ifdef _WIN32
			&& fabsf(viewangles.x) > 15.0f
#endif
			&& (mouse_y == 0.0f && delta.x != 0.0f))
			viewangles.x = lastViewAngles.x;
	}

	return result;
}

// CInput::SteamControllerMove
DETOUR(Client::SteamControllerMove, int nSlot, float flFrametime, CUserCmd *cmd) {
	auto result = Client::SteamControllerMove(thisptr, nSlot, flFrametime, cmd);

	tasControllers[nSlot]->ControllerMove(nSlot, flFrametime, cmd);

	return result;
}

DETOUR_COMMAND(Client::playvideo_end_level_transition) {
	console->DevMsg("%s\n", args.m_pArgSBuffer);
	//	session->Ended();

	return Client::playvideo_end_level_transition_callback(args);
}

DETOUR_T(void, Client::OverrideView, CViewSetup *m_View) {
	Client::OverrideView(thisptr, m_View);

	camera->OverrideView(m_View);
	Stitcher::OverrideView(m_View);
	GhostEntity::FollowPov(m_View);
	engine->demoplayer->OverrideView(m_View);
}


DETOUR(Client::ProcessMovement, void *player, CMoveData *move) {
	// This should only be run if prediction is occurring, i.e. if we
	// are orange, but check anyway

	int slot = -1;

	if (engine->IsOrange() && session->isRunning) {
		// The client does prediction very often (twice per frame?) so
		// we have to do some weird stuff
		static int lastTick;

		int tick = session->GetTick();

		if (tick != lastTick) {
			bool grounded = CE(player)->ground_entity();
			slot = client->GetSplitScreenPlayerSlot(player);
			groundFramesCounter->HandleMovementFrame(slot, grounded);
			strafeQuality.OnMovement(slot, grounded);
			if (move->m_nButtons & IN_JUMP) scrollSpeedHud.OnJump(slot);
			Event::Trigger<Event::PROCESS_MOVEMENT>({slot, false});
			lastTick = tick;
		}
	}

	auto result = Client::ProcessMovement(thisptr, player, move);

	playerTrace->TweakLatestEyeOffsetForPortalShot(move, slot, true);

	return result;
}

CON_COMMAND(sar_chat, "sar_chat - open the chat HUD\n") {
	if (g_chatType == 0) {
		g_wasChatType = 0;
		g_chatType = 1;
		client->OpenChat();
	}
}

extern Hook g_DrawTranslucentRenderablesHook;
DETOUR(Client::DrawTranslucentRenderables, bool inSkybox, bool shadowDepth) {
	g_DrawTranslucentRenderablesHook.Disable();
	auto ret = Client::DrawTranslucentRenderables(thisptr, inSkybox, shadowDepth);
	g_DrawTranslucentRenderablesHook.Enable();
	OverlayRender::drawTranslucents(thisptr);
	return ret;
}
Hook g_DrawTranslucentRenderablesHook(&Client::DrawTranslucentRenderables_Hook);

extern Hook g_DrawOpaqueRenderablesHook;
DETOUR(Client::DrawOpaqueRenderables, void *renderCtx, int renderPath, void *deferClippedOpaqueRenderablesOut) {
	g_DrawOpaqueRenderablesHook.Disable();
	auto ret = Client::DrawOpaqueRenderables(thisptr, renderCtx, renderPath, deferClippedOpaqueRenderablesOut);
	g_DrawOpaqueRenderablesHook.Enable();
	OverlayRender::drawOpaques(thisptr);
	return ret;
}
Hook g_DrawOpaqueRenderablesHook(&Client::DrawOpaqueRenderables_Hook);

extern Hook g_CalcViewModelLagHook;
DETOUR_T(void, Client::CalcViewModelLag, Vector &origin, QAngle &angles, QAngle &original_angles) {
	if (sar_disable_weapon_sway.GetBool()) {
		return;
	}

	g_CalcViewModelLagHook.Disable();
	Client::CalcViewModelLag(thisptr, origin, angles, original_angles);
	g_CalcViewModelLagHook.Enable();
}
Hook g_CalcViewModelLagHook(&Client::CalcViewModelLag_Hook);

extern Hook g_AddShadowToReceiverHook;
DETOUR_T(void, Client::AddShadowToReceiver, unsigned short handle, void *pRenderable, int type) {
	if (sar_disable_viewmodel_shadows.GetBool()) {
		// IClientRenderable::GetModel()
		using _GetModel = model_t *(__rescall *)(void *);
		model_t *model = Memory::VMT<_GetModel>(pRenderable, Offsets::GetModel)(pRenderable);

		if (!strcmp(model->szPathName, "models/weapons/v_portalgun.mdl"))
			return;
	}

	g_AddShadowToReceiverHook.Disable();
	Client::AddShadowToReceiver(thisptr, handle, pRenderable, type);
	g_AddShadowToReceiverHook.Enable();
}
Hook g_AddShadowToReceiverHook(&Client::AddShadowToReceiver_Hook);

static void (*MsgPreSkipToNextLevel)();

CON_COMMAND(sar_workshop_skip, "sar_workshop_skip - Skips to the next level in workshop\n") {
	if (!sv_cheats.GetBool()) {
		return console->Print("This command requires sv_cheats\n");
	}
	if (strncmp("workshop/", engine->GetCurrentMapName().c_str(), 9)) {
		return console->Print("This command only works in workshop maps.\n");
	}
	// open the console so that it works on binds lmao (the menu has to be visible probably)
	// debug with -vguimessages launch option
	engine->ExecuteCommand("showconsole");
	MsgPreSkipToNextLevel();
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
		this->FrameStageNotify = this->g_ClientDLL->Original<_FrameStageNotify>(Offsets::GetAllClasses + 27);

		this->g_ClientDLL->Hook(Client::LevelInitPreEntity_Hook, Client::LevelInitPreEntity, Offsets::LevelInitPreEntity);

		Command leaderboard("+leaderboard");
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
				this->StartMessageMode = g_HudChat->Original<_StartMessageMode>(Offsets::ChatPrintf + 1);
				if (sar.game->Is(SourceGame_Portal2)) {
					this->g_HudChat->Hook(Client::MsgFunc_SayText2_Hook, Client::MsgFunc_SayText2, Offsets::MsgFunc_SayText2);
					this->g_HudChat->Hook(Client::GetTextColorForClient_Hook, Client::GetTextColorForClient, Offsets::GetTextColorForClient);
				} else if (sar.game->Is(SourceGame_PortalReloaded)) {
					this->g_HudChat->Hook(Client::MsgFunc_SayText2_Hook, Client::MsgFunc_SayText2, Offsets::MsgFunc_SayTextReloaded);
				}
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
		void *g_InputAddr = Memory::DerefDeref<void *>(IN_ActivateMouse + Offsets::g_Input);

		if (g_Input = Interface::Create(g_InputAddr)) {
			g_Input->Hook(Client::DecodeUserCmdFromBuffer_Hook, Client::DecodeUserCmdFromBuffer, Offsets::DecodeUserCmdFromBuffer);
			g_Input->Hook(Client::GetButtonBits_Hook, Client::GetButtonBits, Offsets::GetButtonBits);
			g_Input->Hook(Client::SteamControllerMove_Hook, Client::SteamControllerMove, Offsets::SteamControllerMove);
			g_Input->Hook(Client::ApplyMouse_Hook, Client::ApplyMouse, Offsets::ApplyMouse);

			#ifdef _WIN32
				auto ApplyMouse_Mid_addr = (uintptr_t)(Client::ApplyMouse) + 0x3E1;
				g_ApplyMouseMidHook.SetFunc(ApplyMouse_Mid_addr);
				g_ApplyMouseMidHook.Disable();
				Client::ApplyMouse_Mid_Continue = ApplyMouse_Mid_addr + 0x5;
				MatrixBuildRotationAboutAxis = (decltype(MatrixBuildRotationAboutAxis))Memory::Scan(client->Name(), "55 8B EC 51 F3 0F 10 45 ? 0F 5A C0 F2 0F 59 05 ? ? ? ? 66 0F 5A C0 F3 0F 11 45 ? E8 ? ? ? ? F3 0F 11 45 ? F3 0F 10 45 ? E8 ? ? ? ? 8B 45 ? F3 0F 10 08");
			#else
				MatrixBuildRotationAboutAxis = (decltype(MatrixBuildRotationAboutAxis))Memory::Scan(client->Name(), "56 66 0F EF C0 53 83 EC 14 8B 5C 24 ? 8D 44 24");
			#endif

			MatrixBuildRotationAboutAxisHook.SetFunc(MatrixBuildRotationAboutAxis);
			MatrixBuildRotationAboutAxisHook.Disable(); // only during ApplyMouse

			in_forceuser = Variable("in_forceuser");
			if (!!in_forceuser && this->g_Input) {
				this->g_Input->Hook(CInput_CreateMove_Hook, CInput_CreateMove, Offsets::GetButtonBits + 1);
			}

			Command::Hook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback_hook, Client::playvideo_end_level_transition_callback);
		}

		auto HudProcessInput = this->g_ClientDLL->Original(Offsets::HudProcessInput, readJmp);
		auto GetClientMode = Memory::Read<uintptr_t>(HudProcessInput + Offsets::GetClientMode);
		uintptr_t g_pClientMode = Memory::Deref<uintptr_t>(GetClientMode + Offsets::g_pClientMode);
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

#ifdef _WIN32
	Client::DrawTranslucentRenderables = (decltype(Client::DrawTranslucentRenderables))Memory::Scan(client->Name(), "55 8B EC 81 EC 80 00 00 00 53 56 8B F1 8B 0D ? ? ? ? 8B 01 8B 90 C4 01 00 00 57 89 75 F0 FF D2 8B F8");
	Client::DrawOpaqueRenderables = (decltype(Client::DrawOpaqueRenderables))Memory::Scan(client->Name(), "55 8B EC 83 EC 54 83 7D 0C 00 A1 ? ? ? ? 53 56 0F 9F 45 EC 83 78 30 00 57 8B F1 0F 84 BA 03 00 00");
#else
	if (sar.game->Is(SourceGame_EIPRelPIC)) {
		Client::DrawTranslucentRenderables = (decltype(Client::DrawTranslucentRenderables))Memory::Scan(client->Name(), "55 89 E5 57 56 53 81 EC B8 00 00 00 8B 45 10 8B 5D 0C 89 85 60 FF FF FF 88 45 A7 A1 ? ? ? ?");
		Client::DrawOpaqueRenderables = (decltype(Client::DrawOpaqueRenderables))Memory::Scan(client->Name(), "55 89 E5 57 56 53 83 EC 7C A1 ? ? ? ? 8B 5D 08 89 45 90 85 C0 0F 85 34 04 00 00 A1 ? ? ? ? 8B 40 30 85 C0");
	} else {
		Client::DrawTranslucentRenderables = (decltype(Client::DrawTranslucentRenderables))Memory::Scan(client->Name(), "55 89 E5 57 56 53 81 EC DC 00 00 00 8B 45 08 8B 5D 0C 89 C7 89 45 84 8B 45 10 89 85 4C FF FF FF");
		Client::DrawOpaqueRenderables = (decltype(Client::DrawOpaqueRenderables))Memory::Scan(client->Name(), "55 89 E5 57 56 53 81 EC 8C 00 00 00 8B 45 0C 8B 5D 08 89 45 8C 8B 45 14 89 45 90 65 A1 14 00 00 00");
	}
#endif

	g_DrawTranslucentRenderablesHook.SetFunc(Client::DrawTranslucentRenderables);
	g_DrawOpaqueRenderablesHook.SetFunc(Client::DrawOpaqueRenderables);

#ifdef _WIN32
	MsgPreSkipToNextLevel = (decltype(MsgPreSkipToNextLevel))Memory::Scan(client->Name(), "57 8B F9 E8 ? ? ? ? 8B C8 E8 ? ? ? ? 0B C2");
#else
	MsgPreSkipToNextLevel = (decltype(MsgPreSkipToNextLevel))Memory::Scan(client->Name(), "53 83 EC 08 E8 ? ? ? ? 83 EC 0C 50 E8 ? ? ? ? 83 C4 10 09 C2");
#endif

	if (sar.game->Is(SourceGame_Portal2)) {
#ifdef _WIN32
		Client::CalcViewModelLag = (decltype(Client::CalcViewModelLag))Memory::Scan(client->Name(), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 1C 56 6A 00 6A 00 8D 45 F4 8B F1 8B 4B 0C 50 51 E8 ? ? ? ?");
#else
		Client::CalcViewModelLag = (decltype(Client::CalcViewModelLag))Memory::Scan(client->Name(), "56 53 83 EC 24 8B 74 24 30 8B 5C 24 34 6A 00 6A 00 8D 44 24 1C 50 FF 74 24 44 E8 ? ? ? ? A1 ? ? ? ? 83 C4 10 66 0F EF C9");
#endif
	}

	g_CalcViewModelLagHook.SetFunc(Client::CalcViewModelLag);

	if (sar.game->Is(SourceGame_Portal2 | SourceGame_PortalStoriesMel | SourceGame_PortalReloaded)) {
#ifdef _WIN32
		Client::AddShadowToReceiver = (decltype(Client::AddShadowToReceiver))Memory::Scan(client->Name(), "55 8B EC 51 53 56 57 0F B7 7D 08");
#else
		if (sar.game->Is(SourceGame_Portal2)) {
			Client::AddShadowToReceiver = (decltype(Client::AddShadowToReceiver))Memory::Scan(client->Name(), "55 89 E5 57 56 53 83 EC 44 8B 45 0C 8B 5D 08 8B 55 14 8B 75 10");
		} else {
			Client::AddShadowToReceiver = (decltype(Client::AddShadowToReceiver))Memory::Scan(client->Name(), "55 89 E5 57 56 53 83 EC ? 8B 45 ? 8B 4D ? 8B 7D ? 89 45 ? 0F B7 C0");
		}
#endif
	}

	g_AddShadowToReceiverHook.SetFunc(Client::AddShadowToReceiver);

	// Get at gamerules
	{
		uintptr_t cbk = (uintptr_t)Command("+mouse_menu").ThisPtr()->m_pCommandCallback;
#ifdef _WIN32
		// OpenRadialMenuCommand is inlined
		this->gamerules = *(void ***)(cbk + 2);
#else
		if (sar.game->Is(SourceGame_EIPRelPIC)) {
			cbk = (uintptr_t)Memory::Read(cbk + 9);  // openradialmenu -> OpenRadialMenuCommand
			this->gamerules = *(void ***)(cbk + 1);
		} else {
			cbk = (uintptr_t)Memory::Read(cbk + 12);  // openradialmenu -> OpenRadialMenuCommand
			this->gamerules = *(void ***)(cbk + 9);
		}
#endif
	}

	cl_showpos = Variable("cl_showpos");
	cl_sidespeed = Variable("cl_sidespeed");
	cl_forwardspeed = Variable("cl_forwardspeed");
	cl_backspeed = Variable("cl_backspeed");
	prevent_crouch_jump = Variable("prevent_crouch_jump");
	crosshairVariable = Variable("crosshair");
	r_portaltestents = Variable("r_portaltestents");
	r_portalsopenall = Variable("r_portalsopenall");
	r_drawviewmodel = Variable("r_drawviewmodel");

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
