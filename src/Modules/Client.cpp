#include "Client.hpp"

#include "Command.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Features/AutoSubmit.hpp"
#include "Features/Camera.hpp"
#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/FovChanger.hpp"
#include "Features/GroundFramesCounter.hpp"
#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/RhythmGame.hpp"
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

#define LEADERBOARD_MESSAGE_TYPE "cmboard"

Variable cl_showpos;
Variable cl_sidespeed;
Variable cl_backspeed;
Variable cl_forwardspeed;
Variable hidehud;
Variable in_forceuser;
Variable crosshairVariable;
Variable cl_fov;
Variable cl_viewmodelfov;
Variable prevent_crouch_jump;
Variable r_PortalTestEnts;
Variable r_portalsopenall;
Variable r_drawviewmodel;

// some of these are actually commands but that's fine, dw about it :)
Variable soundfade;
Variable leaderboard_open;
Variable gameui_activate;
Variable gameui_allowescape;
Variable gameui_preventescape;
Variable setpause;
Variable snd_ducktovolume;
Variable say;

Variable cl_cmdrate;
Variable cl_updaterate;
Variable cl_chat_active;
Variable ss_pipsplit;
Variable ss_pipscale;
Variable ss_verticalsplit;

Variable sar_disable_coop_score_hud("sar_disable_coop_score_hud", "0", "Disables the coop score HUD which appears in demo playback.\n");
Variable sar_disable_save_status_hud("sar_disable_save_status_hud", "0", "Disables the saving/saved HUD which appears when you make a save.\n");

Variable sar_patch_small_angle_decay("sar_patch_small_angle_decay", "0", "Patches small angle decay (not minor decay).\n");
Variable sar_patch_major_angle_decay("sar_patch_major_angle_decay", "0", "Patches major pitch angle decay. Requires cheats.\n");

// need to do this to include in docs lol
Variable sar_patch_minor_angle_decay("sar_patch_minor_angle_decay", "0", "Patches minor pitch angle decay present on Windows version of the game.\n"
#ifndef _WIN32
                                     ,
                                     FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY
#endif
);

Variable sar_unlocked_chapters("sar_unlocked_chapters", "-1", "Max unlocked chapter.\n");

Variable sar_portalcolor_enable("sar_portalcolor_enable", "0", "Enable custom portal colors.\n");
Variable sar_portalcolor_sp_1("sar_portalcolor_sp_1", "64 160 255", "Portal color for Chell's left portal. r_portal_fastpath 0 required.\n");
Variable sar_portalcolor_sp_2("sar_portalcolor_sp_2", "255 160 32", "Portal color for Chell's right portal. r_portal_fastpath 0 required.\n");
Variable sar_portalcolor_mp1_1("sar_portalcolor_mp1_1", "31 127 210", "Portal color for Atlas (blue)'s left portal.\n");
Variable sar_portalcolor_mp1_2("sar_portalcolor_mp1_2", "19 0 210",   "Portal color for Atlas (blue)'s right portal.\n");
Variable sar_portalcolor_mp2_1("sar_portalcolor_mp2_1", "255 179 31", "Portal color for P-Body (orange)'s left portal.\n");
Variable sar_portalcolor_mp2_2("sar_portalcolor_mp2_2", "57 2 2",     "Portal color for P-Body (orange)'s right portal.\n");
Variable sar_portalcolor_rainbow("sar_portalcolor_rainbow", "0", "Rainbow portals!\n");

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
REDECL(Client::openleaderboard_callback);
REDECL(Client::closeleaderboard_callback);
REDECL(Client::OverrideView);
REDECL(Client::ProcessMovement);
REDECL(Client::DrawTranslucentRenderables);
REDECL(Client::DrawOpaqueRenderables);
REDECL(Client::CalcViewModelLag);
REDECL(Client::AddShadowToReceiver);
REDECL(Client::StartSearching);
REDECL(Client::GetLeaderboard);
REDECL(Client::PurgeAndDeleteElements);
REDECL(Client::IsQuerying);
REDECL(Client::SetPanelStats);
REDECL(Client::OnCommand);
#ifdef _WIN32
REDECL(Client::ApplyMouse_Mid);
REDECL(Client::ApplyMouse_Mid_Continue);
#endif
REDECL(Client::DrawPortal);
REDECL(Client::GetChapterProgress);


CMDECL(Client::GetAbsOrigin, Vector, m_vecAbsOrigin);
CMDECL(Client::GetAbsAngles, QAngle, m_angAbsRotation);
CMDECL(Client::GetLocalVelocity, Vector, m_vecVelocity);
CMDECL(Client::GetViewOffset, Vector, m_vecViewOffset);
CMDECL(Client::GetPortalLocal, CPortalPlayerLocalData, m_PortalLocal);
CMDECL(Client::GetPlayerState, CPlayerState, pl);

DECL_CVAR_CALLBACK(cl_fov) {
	fovChanger->Force();
}

DECL_CVAR_CALLBACK(cl_viewmodelfov) {
	fovChanger->Force();
}

ClientEnt *Client::GetPlayer(int index) {
	if ((index & Offsets::ENT_ENTRY_MASK) == Offsets::ENT_ENTRY_MASK) {
		return nullptr;
	}
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
	if (!this->ShouldDraw) return false;
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
	if (!client->ChatPrintf) return;
	g_nameColorOverrides.push_back(col);
	client->ChatPrintf(client->g_HudChat->ThisPtr(), 0, 0, "%c%s", TextColor::PLAYERNAME, str);
}

void Client::MultiColorChat(const std::vector<std::pair<Color, std::string>> &components) {
	if (!client->ChatPrintf) return;
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

void Client::ShowLocator(Vector position, Vector normal, Color color) {
	Vector colorAsVector = {(float)color.r, (float)color.g, (float)color.b};
	QAngle angles;

	Vector pseudoup = fabsf(normal.z) < 0.999f ? Vector{0, 0, 1} : Vector{1, 0, 0};

	Math::VectorAngles(normal, pseudoup, &angles);
	angles.x += 90.0f;

	PrecacheParticleSystem("command_target_ping");
	DispatchParticleEffect("command_target_ping", position, colorAsVector, angles, nullptr, 0, nullptr);
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

static Vector player_size_standing;
static Vector player_size_ducked;
Vector Client::GetPlayerSize(bool ducked) {
	if (ducked && player_size_ducked.x != 0) {
		return player_size_ducked;
	}
	if (!ducked && player_size_standing.x != 0) {
		return player_size_standing;
	}
	auto player = client->GetPlayer(1);
	if (!player) return {32, 32, 72};
	if (ducked) {
		return player_size_ducked = player->field<Vector>("m_DuckHullMax") - player->field<Vector>("m_DuckHullMin");
	} else {
		return player_size_standing = player->field<Vector>("m_StandHullMax") - player->field<Vector>("m_StandHullMin");
	}
}

void Client::ClFrameStageNotify(int stage) {
	this->FrameStageNotify(this->g_ClientDLL->ThisPtr(), stage);
}

void Client::OpenChat() {
	if (!this->StartMessageMode) return;
	this->StartMessageMode(this->g_HudChat->ThisPtr(), 1);  // MM_SAY
}

// CHLClient::LevelInitPreEntity
DETOUR(Client::LevelInitPreEntity, const char *levelName) {
	client->lastLevelName = std::string(levelName);
	return Client::LevelInitPreEntity(thisptr, levelName);
}

// ClientModeShared::CreateMove
DETOUR(Client::CreateMove, float flInputSampleTime, CUserCmd *cmd) {
	int slot = engine->IsOrange() ? 1 : 0;
	if (!in_forceuser.isReference || (in_forceuser.isReference && !in_forceuser.GetBool())) {
		inputHud.SetInputInfo(engine->IsCoop() ? slot : 0, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});
	}

	if (sv_cheats.GetBool() && engine->hoststate->m_activeGame) {
		camera->OverrideMovement(cmd);
		Stitcher::OverrideMovement(cmd);
	}

	if (GhostEntity::GetFollowTarget()) {
		cmd->buttons = 0;
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
	}

	if (sar_strafesync.GetBool()) {
		synchro->UpdateSync(slot, cmd);
	}

	strafeQualityHud->OnUserCmd(slot, *cmd);

	if (cmd->buttons & IN_ATTACK) {
		g_bluePortalAngles[slot] = engine->GetAngles(slot);
	}

	if (cmd->buttons & IN_ATTACK2) {
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

	strafeQualityHud->OnUserCmd(1, *cmd);

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
	if (sar_disable_challenge_stats_hud.GetInt() == -1) return "";
	return Client::GetName(thisptr);
}

static bool g_leaderboardOpen = false;
static bool g_leaderboardWillClose = false;
DETOUR_COMMAND(Client::openleaderboard) {
	Client::openleaderboard_callback(args);

	if (args.ArgC() == 2 && !strcmp(args[1], "4") && client->GetChallengeStatus() == CMStatus::CHALLENGE) {
		g_leaderboardOpen = true;
		auto ticks = 6;
		if (sar_disable_challenge_stats_hud.GetInt() > 1) ticks = sar_disable_challenge_stats_hud.GetInt();
		Scheduler::InHostTicks(ticks, []() {
			if (sar.game->Is(SourceGame_Portal2) && sar_disable_challenge_stats_hud.GetInt() > 0 && (!engine->IsCoop() || engine->IsOrange() || g_leaderboardWillClose)) {
				g_leaderboardWillClose = false;
				engine->ExecuteCommand("-leaderboard");
			}
		});
	}
}

Memory::Patch *g_drawPortalPatch;
Memory::Patch *g_drawPortalGhostPatch;
// C_Prop_Portal::DrawPortal
extern Hook g_DrawPortalHook;
DETOUR(Client::DrawPortal, void *pRenderContext) {
	if (sar_portalcolor_enable.GetBool() &&
		!(!strcmp(sar_portalcolor_sp_1.GetString(), sar_portalcolor_sp_1.ThisPtr()->m_pszDefaultValue) &&
		  !strcmp(sar_portalcolor_sp_2.GetString(), sar_portalcolor_sp_2.ThisPtr()->m_pszDefaultValue))) {
		g_drawPortalPatch->Execute();
	} else {
		g_drawPortalPatch->Restore();
	}
	g_DrawPortalHook.Disable();
	auto ret = Client::DrawPortal(thisptr, pRenderContext);
	g_DrawPortalHook.Enable();
	return ret;
}
Hook g_DrawPortalHook(&Client::DrawPortal_Hook);

static void (*g_DrawPortalGhost)(void *pRenderContext);

// C_Prop_Portal::DrawPortalGhostLocations
extern Hook g_DrawPortalGhostHook;
static void DrawPortalGhost_Hook(void *pRenderContext) {
	if (sar_portalcolor_enable.GetBool() &&
		!(!strcmp(sar_portalcolor_sp_1.GetString(), sar_portalcolor_sp_1.ThisPtr()->m_pszDefaultValue) &&
		  !strcmp(sar_portalcolor_sp_2.GetString(), sar_portalcolor_sp_2.ThisPtr()->m_pszDefaultValue))) {
		g_drawPortalGhostPatch->Execute();
	} else {
		g_drawPortalGhostPatch->Restore();
	}
	g_DrawPortalGhostHook.Disable();
	g_DrawPortalGhost(pRenderContext);
	g_DrawPortalGhostHook.Enable();
	return;
}
Hook g_DrawPortalGhostHook(&DrawPortalGhost_Hook);


static SourceColor (*UTIL_Portal_Color)(int iPortal, int iTeamNumber);
extern Hook UTIL_Portal_Color_Hook;
static SourceColor UTIL_Portal_Color_Detour(int iPortal, int iTeamNumber) {
	UTIL_Portal_Color_Hook.Disable();
	SourceColor ret = UTIL_Portal_Color(iPortal, iTeamNumber);
	UTIL_Portal_Color_Hook.Enable();

	if (sar_portalcolor_enable.GetBool()) {
		std::optional<Color> modify;
		// Yes, blue and orange are swapped. Mhm.
		// Also 1 is unused.
		if (sar_portalcolor_rainbow.GetBool()) {
			int host, server, client;
			engine->GetTicks(host, server, client);
			modify = {Utils::HSVToRGB((host + (iTeamNumber == 2 ? 180 : 0) + (iPortal == 1 ? 0 : 30)) % 360, 100, iPortal == 1 ? 100 : 50)};
		} else if (iTeamNumber == 0) {
			modify = Utils::GetColor(iPortal == 1 ? sar_portalcolor_sp_1.GetString() : sar_portalcolor_sp_2.GetString());
		} else if (iTeamNumber == 2) {
			modify = Utils::GetColor(iPortal == 1 ? sar_portalcolor_mp2_1.GetString() : sar_portalcolor_mp2_2.GetString());
		} else if (iTeamNumber == 3) {
			modify = Utils::GetColor(iPortal == 1 ? sar_portalcolor_mp1_1.GetString() : sar_portalcolor_mp1_2.GetString());
		}
		if (modify.has_value()) ret = SourceColor(modify.value().r, modify.value().g, modify.value().b);
	}

	return ret;
}
Hook UTIL_Portal_Color_Hook(&UTIL_Portal_Color_Detour);

Color SARUTIL_Portal_Color(int iPortal, int iTeamNumber) {
	SourceColor col = UTIL_Portal_Color(iPortal, iTeamNumber);
	return Color(col.r(), col.g(), col.b());
}

static SourceColor (*UTIL_Portal_Color_Particles)(int iPortal, int iTeamNumber);
extern Hook UTIL_Portal_Color_Particles_Hook;
static SourceColor UTIL_Portal_Color_Particles_Detour(int iPortal, int iTeamNumber) {
	UTIL_Portal_Color_Particles_Hook.Disable();
	SourceColor ret = UTIL_Portal_Color_Particles(iPortal, iTeamNumber);
	UTIL_Portal_Color_Particles_Hook.Enable();

	if (sar_portalcolor_enable.GetBool()) {
		if (iTeamNumber == 0) {
			// HACK: If we're at the default SP color, just let the function use its hardcoded return path.
			// Otherwise, draw the particles according to portal colors
			if (iPortal == 1 && !strcmp(sar_portalcolor_sp_1.GetString(), sar_portalcolor_sp_1.ThisPtr()->m_pszDefaultValue)) return ret;
			if (iPortal == 2 && !strcmp(sar_portalcolor_sp_2.GetString(), sar_portalcolor_sp_2.ThisPtr()->m_pszDefaultValue)) return ret;
		}
		return UTIL_Portal_Color(iPortal, iTeamNumber);
	}

	return ret;
}
Hook UTIL_Portal_Color_Particles_Hook(&UTIL_Portal_Color_Particles_Detour);

ON_INIT {
	NetMessage::RegisterHandler(LEADERBOARD_MESSAGE_TYPE, +[](const void *data, size_t size) {
		// TODO: Investigate why this sometimes doesn't work - AMJ 2024-04-25
		if (sar_disable_challenge_stats_hud_partner.GetBool()) {
			g_leaderboardWillClose = true;
			engine->ExecuteCommand("-leaderboard");
		} });
}

DETOUR_COMMAND(Client::closeleaderboard) {
	// this is actually "unpause"
	if (g_coop_pausable != -1) {
		Variable("sv_pausable").SetValue(g_coop_pausable ? "1" : "0");
		g_coop_pausable = -1;
	}

	Client::closeleaderboard_callback(args);

	if (g_leaderboardOpen) {
		g_leaderboardOpen = false;
		NetMessage::SendMsg(LEADERBOARD_MESSAGE_TYPE, 0, 0);
	}
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
	} else if (sar.game->Is(SourceGame_PortalReloaded | SourceGame_Portal2_2011)) {
		// Reloaded and 2011 Portal 2 use the legacy format where it's just one string
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
		if (!sar_netmessage_debug.GetBool()) return 0;
	}

	msg = pre;

	return Client::MsgFunc_SayText2(thisptr, msg);
}

// MSVC bug workaround - see Engine::GetLightAtPoint for explanation
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

	strafeQualityHud->OnUserCmd(nSlot, *cmd);
	void *player = client->GetPlayer(nSlot + 1);
	if (player && !sar.game->Is(SourceGame_INFRA)) {
		bool grounded = CE(player)->ground_entity();
		strafeHud.SetData(nSlot, player, cmd, false);
		Event::Trigger<Event::PROCESS_MOVEMENT>({nSlot, false, player, nullptr, cmd, grounded});  // There isn't really one, just pretend it's here lol
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

	auto view = ViewSetupCreate(m_View);

	camera->OverrideView(view);
	Stitcher::OverrideView(view);
	GhostEntity::FollowPov(view);
	engine->demoplayer->OverrideView(view);

	ViewSetupCopy(view, m_View);
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

			Event::Trigger<Event::PROCESS_MOVEMENT>({slot, false, player, move, nullptr, grounded});
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

extern Hook g_StartSearchingHook;
DETOUR(Client::StartSearching) {
	struct CPortalLeaderboard {
		char m_szMapName[128];
	};

	AutoSubmit::Search(((CPortalLeaderboard *)thisptr)->m_szMapName);

	return 0;
}
Hook g_StartSearchingHook(&Client::StartSearching_Hook);

static std::vector<std::string> g_registeredLbs;

extern Hook g_GetLeaderboardHook;
DETOUR_T(void *, Client::GetLeaderboard, const char *a2) {
	// the game only updates the boards once when it is initialized, we want to update them each time it gets clicked, but we still want the boards to get constructed
	g_GetLeaderboardHook.Disable();
	auto ret = Client::GetLeaderboard(thisptr, a2);
	g_GetLeaderboardHook.Enable();

	if (!std::count(g_registeredLbs.begin(), g_registeredLbs.end(), a2)) {
		g_registeredLbs.push_back(a2);
	} else if (ret) {
		Client::StartSearching(ret);
	}

	return ret;
}
Hook g_GetLeaderboardHook(&Client::GetLeaderboard_Hook);

extern Hook g_PurgeAndDeleteElementsHook;
DETOUR(Client::PurgeAndDeleteElements) {
	g_registeredLbs.clear();

	g_PurgeAndDeleteElementsHook.Disable();
	auto ret = Client::PurgeAndDeleteElements(thisptr);
	g_PurgeAndDeleteElementsHook.Enable();
	return ret;
}
Hook g_PurgeAndDeleteElementsHook(&Client::PurgeAndDeleteElements_Hook);

extern Hook g_IsQueryingHook;
DETOUR(Client::IsQuerying) {
	return AutoSubmit::IsQuerying();
}
Hook g_IsQueryingHook(&Client::IsQuerying_Hook);

extern Hook g_SetPanelStatsHook;
DETOUR(Client::SetPanelStats) {
	void *m_pLeaderboard = *(void **)((uintptr_t)thisptr + Offsets::m_pLeaderboard);
	void *m_pStatList = *(void **)((uintptr_t)thisptr + Offsets::m_pStatList);
	int m_nStatHeight = *(int *)((uintptr_t)thisptr + Offsets::m_nStatHeight);

	const auto &times = AutoSubmit::GetTimes();

	for (size_t i = 0; i < times.size(); ++i) {
		const auto &time = times[i];

		PortalLeaderboardItem_t data;
		data.m_xuid = atoll(time["userData"]["profileNumber"].string_value().c_str());
		data.m_iScore = atoi(time["scoreData"]["score"].string_value().c_str());
		strncpy(data.m_szName, time["userData"]["boardname"].string_value().c_str(), sizeof(data.m_szName));

		client->AddAvatarPanelItem(m_pLeaderboard, m_pStatList, &data, data.m_iScore, 1, -1, i, m_nStatHeight, -1, 0);
	}

	return 0;
}
Hook g_SetPanelStatsHook(&Client::SetPanelStats_Hook);

extern Hook g_OnCommandHook;
DETOUR(Client::OnCommand, const char *a2) {
	if (!strcmp(a2, "Leaderboard_Time")) {
		return 0;
	}

	g_OnCommandHook.Disable();
	auto ret = Client::OnCommand(thisptr, a2);
	g_OnCommandHook.Enable();
	return ret;
}
Hook g_OnCommandHook(&Client::OnCommand_Hook);

extern Hook g_GetChapterProgressHook;
DETOUR(Client::GetChapterProgress) {
	if (sar_unlocked_chapters.GetInt() > -1)
		return sar_unlocked_chapters.GetInt() + 1;

	g_GetChapterProgressHook.Disable();
	auto ret = Client::GetChapterProgress(thisptr);
	g_GetChapterProgressHook.Enable();
	return ret;
}
Hook g_GetChapterProgressHook(&Client::GetChapterProgress_Hook);

bool Client::Init() {
	bool readJmp = false;
#ifdef _WIN32
	if (sar.game->Is(SourceGame_BeginnersGuide | SourceGame_StanleyParable)) {
		readJmp = true;
	}
#endif
	this->g_ClientDLL = Interface::Create(this->Name(), "VClient016");
	this->s_EntityList = Interface::Create(this->Name(), "VClientEntityList003", false);
	this->g_GameMovement = Interface::Create(this->Name(), "GameMovement001");

	if (this->g_GameMovement) {
		this->g_GameMovement->Hook(Client::ProcessMovement_Hook, Client::ProcessMovement, Offsets::ProcessMovement);
	}

	if (this->g_ClientDLL) {
		this->GetAllClasses = this->g_ClientDLL->Original<_GetAllClasses>(Offsets::GetAllClasses, readJmp);
		this->FrameStageNotify = this->g_ClientDLL->Original<_FrameStageNotify>(Offsets::GetAllClasses + 27);

		Client::DispatchParticleEffect = (Client::_DispatchParticleEffect)Memory::Scan(this->Name(), Offsets::DispatchParticleEffect);
		Client::PrecacheParticleSystem = (Client::_PrecacheParticleSystem)Memory::Scan(this->Name(), Offsets::PrecacheParticleSystem);

		this->g_ClientDLL->Hook(Client::LevelInitPreEntity_Hook, Client::LevelInitPreEntity, Offsets::LevelInitPreEntity);

		using _GetHud = void *(__cdecl *)(int unk);
		using _FindElement = void *(__rescall *)(void *thisptr, const char *pName);
		_GetHud GetHud = nullptr;
		_FindElement FindElement = nullptr;

		Command leaderboard("+leaderboard");
		if (!!leaderboard) {
			auto cc_leaderboard_enable = (uintptr_t)leaderboard.ThisPtr()->m_pCommandCallback;
			if (readJmp) {
				// this game has an extra layer of indirection
				// for whatever reason... (JMP LAB)
				cc_leaderboard_enable = Memory::Read<uintptr_t>(cc_leaderboard_enable + 1);
			}
			GetHud = Memory::Read<_GetHud>(cc_leaderboard_enable + Offsets::GetHud);
			FindElement = Memory::Read<_FindElement>(cc_leaderboard_enable + Offsets::FindElement);
		} else if (Offsets::GetHudSig && Offsets::FindElementSig) {
			GetHud = Memory::Scan<_GetHud>(this->Name(), Offsets::GetHudSig);
			FindElement = Memory::Scan<_FindElement>(this->Name(), Offsets::FindElementSig);
		}

		if (GetHud && FindElement) {
			auto CHUDChallengeStats = FindElement(GetHud(-1), "CHUDChallengeStats");
			if (this->g_HUDChallengeStats = Interface::Create(CHUDChallengeStats)) {
				this->g_HUDChallengeStats->Hook(Client::GetName_Hook, Client::GetName, Offsets::GetName);
			} else {
				console->DevWarning("Failed to hook CHUDChallengeStats\n");
			}

			auto CHUDQuickInfo = FindElement(GetHud(-1), "CHUDQuickInfo");
			if (this->g_HUDQuickInfo = Interface::Create(CHUDQuickInfo)) {
				this->ShouldDraw = this->g_HUDQuickInfo->Original<_ShouldDraw>(Offsets::ShouldDraw);
			} else {
				console->DevWarning("Failed to hook CHUDQuickInfo\n");
			}

			auto CHudChat = FindElement(GetHud(-1), "CHudChat");
			if (this->g_HudChat = Interface::Create(CHudChat)) {
				this->ChatPrintf = g_HudChat->Original<_ChatPrintf>(Offsets::ChatPrintf);
				this->StartMessageMode = g_HudChat->Original<_StartMessageMode>(Offsets::ChatPrintf + 1);
				if (sar.game->Is(SourceGame_Portal2)) {
					this->g_HudChat->Hook(Client::MsgFunc_SayText2_Hook, Client::MsgFunc_SayText2, Offsets::MsgFunc_SayText2);
					this->g_HudChat->Hook(Client::GetTextColorForClient_Hook, Client::GetTextColorForClient, Offsets::GetTextColorForClient);
				} else if (sar.game->Is(SourceGame_PortalReloaded | SourceGame_Portal2_2011)) {
					// This hooks SayText, not SayText2, but the function signature is compatible
					this->g_HudChat->Hook(Client::MsgFunc_SayText2_Hook, Client::MsgFunc_SayText2, Offsets::MsgFunc_SayText);
				}
			} else {
				console->DevWarning("Failed to hook CHudChat\n");
			}

			auto CHudMultiplayerBasicInfo = FindElement(GetHud(-1), "CHudMultiplayerBasicInfo");
			if (this->g_HudMultiplayerBasicInfo = Interface::Create(CHudMultiplayerBasicInfo)) {
				this->g_HudMultiplayerBasicInfo->Hook(Client::ShouldDraw_BasicInfo_Hook, Client::ShouldDraw_BasicInfo, Offsets::ShouldDraw);
			} else {
				console->DevWarning("Failed to hook CHudMultiplayerBasicInfo\n");
			}

			auto CHudSaveStatus = FindElement(GetHud(-1), "CHudSaveStatus");
			if (this->g_HudSaveStatus = Interface::Create(CHudSaveStatus)) {
				this->g_HudSaveStatus->Hook(Client::ShouldDraw_SaveStatus_Hook, Client::ShouldDraw_SaveStatus, Offsets::ShouldDraw);
			} else {
				console->DevWarning("Failed to hook CHudSaveStatus\n");
			}
		} else {
			console->DevWarning("Failed to hook GetHud and FindElement\n");
		}

		this->IN_ActivateMouse = this->g_ClientDLL->Original<_IN_ActivateMouse>(Offsets::IN_ActivateMouse, readJmp);
		this->IN_DeactivateMouse = this->g_ClientDLL->Original<_IN_DeactivateMouse>(Offsets::IN_DeactivateMouse, readJmp);
		void *g_InputAddr = Memory::DerefDeref<void *>((uintptr_t)IN_ActivateMouse + Offsets::g_Input);

		if (g_Input = Interface::Create(g_InputAddr)) {
			g_Input->Hook(Client::DecodeUserCmdFromBuffer_Hook, Client::DecodeUserCmdFromBuffer, Offsets::DecodeUserCmdFromBuffer);
			g_Input->Hook(Client::GetButtonBits_Hook, Client::GetButtonBits, Offsets::GetButtonBits);
			g_Input->Hook(Client::SteamControllerMove_Hook, Client::SteamControllerMove, Offsets::SteamControllerMove);
			g_Input->Hook(Client::ApplyMouse_Hook, Client::ApplyMouse, Offsets::ApplyMouse);

			if (Offsets::JoyStickApplyMovement) {
				auto JoyStickApplyMovement = g_Input->Original(Offsets::JoyStickApplyMovement, readJmp);
				Memory::Read(JoyStickApplyMovement + Offsets::KeyDown, &this->KeyDown);
				Memory::Read(JoyStickApplyMovement + Offsets::KeyUp, &this->KeyUp);
			}
			if (Offsets::in_jump) {
				auto GetButtonBits = g_Input->Original(Offsets::GetButtonBits, readJmp);
				Memory::Deref(GetButtonBits + Offsets::in_jump, &this->in_jump);
			}

#ifdef _WIN32
			auto ApplyMouse_Mid_addr = (uintptr_t)(Client::ApplyMouse) + Offsets::ApplyMouse_Mid;
			g_ApplyMouseMidHook.SetFunc(ApplyMouse_Mid_addr, false);
			Client::ApplyMouse_Mid_Continue = ApplyMouse_Mid_addr + 0x5;
#endif
			MatrixBuildRotationAboutAxis = (decltype(MatrixBuildRotationAboutAxis))Memory::Scan(client->Name(), Offsets::MatrixBuildRotationAboutAxis);
			MatrixBuildRotationAboutAxisHook.SetFunc(MatrixBuildRotationAboutAxis, false);  // only during ApplyMouse

			Client::DrawPortal = (decltype(Client::DrawPortal))Memory::Scan(client->Name(), Offsets::DrawPortal);
			g_DrawPortalGhost = (decltype(g_DrawPortalGhost))Memory::Scan(client->Name(), Offsets::DrawPortalGhost);
			if (Client::DrawPortal && g_DrawPortalGhost) {
				auto drawPortalSpBranch = Memory::Scan(client->Name(), Offsets::DrawPortalSpBranch);
				auto drawPortalGhostSpBranch = Memory::Scan(client->Name(), Offsets::DrawPortalGhostSpBranch);

				g_DrawPortalHook.SetFunc(Client::DrawPortal);
				g_DrawPortalGhostHook.SetFunc(g_DrawPortalGhost);

				g_drawPortalPatch = new Memory::Patch();
				g_drawPortalGhostPatch = new Memory::Patch();

				unsigned char drawPortalGhostByte = 0x80;
#ifndef _WIN32
				unsigned char drawPortalBytes[5];

				*(int32_t *)(drawPortalBytes + 1) = *(int32_t *)(drawPortalSpBranch + 2) + Offsets::DrawPortalSpBranchOff;
				drawPortalBytes[0] = 0x81;

				g_drawPortalPatch->Execute(drawPortalSpBranch + 1, drawPortalBytes, 5);
				g_drawPortalGhostPatch->Execute(drawPortalGhostSpBranch + 1, &drawPortalGhostByte, 1);

#else
				unsigned char drawPortalBytes[2];

				drawPortalBytes[0] = 0xEB;
				drawPortalBytes[1] = Offsets::DrawPortalSpBranchOff;

				g_drawPortalPatch->Execute(drawPortalSpBranch, drawPortalBytes, 2);
				g_drawPortalGhostPatch->Execute(drawPortalGhostSpBranch + 1, &drawPortalGhostByte, 1);
#endif
			}

			in_forceuser = Variable("in_forceuser");
			if (!!in_forceuser && this->g_Input) {
				this->g_Input->Hook(CInput_CreateMove_Hook, CInput_CreateMove, Offsets::GetButtonBits + 1);
			}

			Command::Hook("playvideo_end_level_transition", Client::playvideo_end_level_transition_callback_hook, Client::playvideo_end_level_transition_callback);
			Command::Hook("+leaderboard", Client::openleaderboard_callback_hook, Client::openleaderboard_callback);
			Command::Hook("unpause", Client::closeleaderboard_callback_hook, Client::closeleaderboard_callback);
		} else {
			console->DevWarning("Failed to hook input\n");
		}

		auto HudProcessInput = this->g_ClientDLL->Original(Offsets::HudProcessInput, readJmp);
		auto GetClientMode = Memory::Read<uintptr_t>(HudProcessInput + Offsets::GetClientMode);
		if (readJmp) GetClientMode = Memory::Read<uintptr_t>(GetClientMode + 1);
		auto g_pClientMode = Memory::Deref<uintptr_t>(GetClientMode + Offsets::g_pClientMode);
		auto clientMode = Memory::Deref<void *>(g_pClientMode);
		auto clientMode2 = Memory::Deref<void *>(g_pClientMode + sizeof(void *));

		if (this->g_pClientMode = Interface::Create(clientMode)) {
			this->g_pClientMode->Hook(Client::CreateMove_Hook, Client::CreateMove, Offsets::CreateMove);
			this->g_pClientMode->Hook(Client::OverrideView_Hook, Client::OverrideView, Offsets::OverrideView);
		} else {
			console->DevWarning("Failed to hook client mode\n");
		}

		if (this->g_pClientMode2 = Interface::Create(clientMode2)) {
			this->g_pClientMode2->Hook(Client::CreateMove2_Hook, Client::CreateMove2, Offsets::CreateMove);
		} else {
			console->DevWarning("Failed to hook client mode 2\n");
		}
	}

	if (this->s_EntityList) {
		this->GetClientEntity = this->s_EntityList->Original<_GetClientEntity>(Offsets::GetClientEntity, readJmp);
	}

	Client::DrawTranslucentRenderables = (decltype(Client::DrawTranslucentRenderables))Memory::Scan(client->Name(), Offsets::DrawTranslucentRenderables);
	Client::DrawOpaqueRenderables = (decltype(Client::DrawOpaqueRenderables))Memory::Scan(client->Name(), Offsets::DrawOpaqueRenderables);
	g_DrawTranslucentRenderablesHook.SetFunc(Client::DrawTranslucentRenderables);
	g_DrawOpaqueRenderablesHook.SetFunc(Client::DrawOpaqueRenderables);

	if (sar.game->Is(SourceGame_Portal2 | SourceGame_ApertureTag)) {
		MsgPreSkipToNextLevel = (decltype(MsgPreSkipToNextLevel))Memory::Scan(client->Name(), Offsets::MsgPreSkipToNextLevel);
	}

	if (sar.game->Is(SourceGame_Portal2)) {
		Client::CalcViewModelLag = (decltype(Client::CalcViewModelLag))Memory::Scan(client->Name(), Offsets::CalcViewModelLag);
	}

	g_CalcViewModelLagHook.SetFunc(Client::CalcViewModelLag);

	if (sar.game->Is(SourceGame_Portal2 | SourceGame_PortalStoriesMel | SourceGame_PortalReloaded)) {
		Client::AddShadowToReceiver = (decltype(Client::AddShadowToReceiver))Memory::Scan(client->Name(), Offsets::AddShadowToReceiver);
	}

	g_AddShadowToReceiverHook.SetFunc(Client::AddShadowToReceiver);

	UTIL_Portal_Color = (decltype(UTIL_Portal_Color))Memory::Scan(client->Name(), Offsets::UTIL_Portal_Color);
	UTIL_Portal_Color_Particles = (decltype(UTIL_Portal_Color_Particles))Memory::Scan(client->Name(), Offsets::UTIL_Portal_Color_Particles);
	UTIL_Portal_Color_Hook.SetFunc(UTIL_Portal_Color);
	UTIL_Portal_Color_Particles_Hook.SetFunc(UTIL_Portal_Color_Particles);

	// Get at gamerules
	auto mouse_menu = Command("+mouse_menu");
	if (mouse_menu.ThisPtr()) {
		uintptr_t cbk = (uintptr_t)mouse_menu.ThisPtr()->m_pCommandCallback;
		if (readJmp) {
			cbk = Memory::Read<uintptr_t>(cbk + 1);
		}
		if (sar.game->Is(SourceGame_Portal2_2011)) {
			cbk = Memory::Read<uintptr_t>(cbk + 3);
		}
		// OpenRadialMenuCommand is inlined on Windows
#ifndef _WIN32
		cbk = Memory::Read(cbk + Offsets::OpenRadialMenuCommand);
#endif
		this->gamerules = Memory::Deref<void **>(cbk + Offsets::gamerules);
	}

	if (sar.game->Is(SourceGame_PortalStoriesMel)) {
		auto GetNumChapters = Memory::Scan(this->Name(), Offsets::GetNumChapters);
		if (GetNumChapters) {
			this->nNumSPChapters = Memory::Deref<int *>(GetNumChapters + Offsets::nNumSPChapters);
			this->g_ChapterContextNames = Memory::Deref<ChapterContextData_t *>(GetNumChapters + Offsets::g_ChapterContextNames);
			this->nNumMPChapters = Memory::Deref<int *>(GetNumChapters + Offsets::nNumMPChapters);
			this->g_ChapterMPContextNames = Memory::Deref<ChapterContextData_t *>(GetNumChapters + Offsets::g_ChapterMPContextNames);
		}

		auto CPortalLeaderboardPanel_OnThink = Memory::Scan(this->Name(), Offsets::CPortalLeaderboardPanel_OnThink);
		if (CPortalLeaderboardPanel_OnThink) {
			Client::GetLeaderboard = Memory::Read<decltype(Client::GetLeaderboard)>(CPortalLeaderboardPanel_OnThink + Offsets::GetLeaderboard);
			Client::IsQuerying = Memory::Read<decltype(Client::IsQuerying)>(CPortalLeaderboardPanel_OnThink + Offsets::IsQuerying);
			Client::SetPanelStats = Memory::Read<decltype(Client::SetPanelStats)>(CPortalLeaderboardPanel_OnThink + Offsets::SetPanelStats);
			Client::StartSearching = Memory::Read<decltype(Client::StartSearching)>((uintptr_t)GetLeaderboard + Offsets::StartSearching);
			Client::AddAvatarPanelItem = Memory::Read<decltype(Client::AddAvatarPanelItem)>((uintptr_t)SetPanelStats + Offsets::AddAvatarPanelItem);

			auto OnEvent = Memory::Scan(this->Name(), Offsets::OnEvent);
			Client::PurgeAndDeleteElements = Memory::Read<decltype(Client::PurgeAndDeleteElements)>(OnEvent + Offsets::PurgeAndDeleteElements);
			Client::OnCommand = (decltype(Client::OnCommand))Memory::Scan(this->Name(), Offsets::OnCommand);
		}

		g_GetLeaderboardHook.SetFunc(Client::GetLeaderboard);
		g_IsQueryingHook.SetFunc(Client::IsQuerying);
		g_SetPanelStatsHook.SetFunc(Client::SetPanelStats);
		g_StartSearchingHook.SetFunc(Client::StartSearching);
		g_PurgeAndDeleteElementsHook.SetFunc(Client::PurgeAndDeleteElements);
		g_OnCommandHook.SetFunc(Client::OnCommand);
	}

	Client::GetChapterProgress = (decltype(Client::GetChapterProgress))Memory::Scan(this->Name(), Offsets::GetChapterProgress);
	g_GetChapterProgressHook.SetFunc(Client::GetChapterProgress);

	cl_showpos = Variable("cl_showpos");
	cl_sidespeed = Variable("cl_sidespeed");
	cl_forwardspeed = Variable("cl_forwardspeed");
	cl_backspeed = Variable("cl_backspeed");
	hidehud = Variable("hidehud");
	prevent_crouch_jump = Variable("prevent_crouch_jump");
	crosshairVariable = Variable("crosshair");
	r_PortalTestEnts = Variable("r_PortalTestEnts");
	r_portalsopenall = Variable("r_portalsopenall");
	r_drawviewmodel = Variable("r_drawviewmodel");

	soundfade = Variable("soundfade");
	leaderboard_open = Variable("leaderboard_open");
	gameui_activate = Variable("gameui_activate");
	gameui_allowescape = Variable("gameui_allowescape");
	gameui_preventescape = Variable("gameui_preventescape");
	setpause = Variable("setpause");
	snd_ducktovolume = Variable("snd_ducktovolume");
	say = Variable("say");
	cl_cmdrate = Variable("cl_cmdrate");
	cl_updaterate = Variable("cl_updaterate");
	cl_chat_active = Variable("cl_chat_active");
	ss_pipsplit = Variable("ss_pipsplit");
	ss_pipscale = Variable("ss_pipscale");
	ss_verticalsplit = Variable("ss_verticalsplit");
	cl_viewmodelfov = Variable("cl_viewmodelfov");

	CVAR_HOOK_AND_CALLBACK(cl_fov);
	CVAR_HOOK_AND_CALLBACK(cl_viewmodelfov);

	return this->hasLoaded = this->g_ClientDLL && this->s_EntityList;
}
void Client::Shutdown() {
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
	Command::Unhook("+leaderboard", Client::openleaderboard_callback);
	Command::Unhook("unpause", Client::closeleaderboard_callback);
	g_drawPortalPatch->Restore();
	g_drawPortalGhostPatch->Restore();
	SAFE_DELETE(g_drawPortalPatch);
	SAFE_DELETE(g_drawPortalGhostPatch);
}

Client *client;
