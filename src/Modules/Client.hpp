#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#include <cstdint>

enum class CMStatus {
	CHALLENGE,
	WRONG_WARP,
	NONE,
};

class Client : public Module {
private:
	Interface *g_ClientDLL = nullptr;
	Interface *g_pClientMode = nullptr;
	Interface *g_pClientMode2 = nullptr;
	Interface *g_HUDChallengeStats = nullptr;
	Interface *g_HUDQuickInfo = nullptr;
	Interface *s_EntityList = nullptr;
	Interface *g_Input = nullptr;
	Interface *g_HudChat = nullptr;
	Interface *g_HudMultiplayerBasicInfo = nullptr;
	Interface *g_HudSaveStatus = nullptr;
	Interface *g_GameMovement = nullptr;

public:
	DECL_M(GetAbsOrigin, Vector);
	DECL_M(GetAbsAngles, QAngle);
	DECL_M(GetLocalVelocity, Vector);
	DECL_M(GetViewOffset, Vector);
	DECL_M(GetPortalLocal, CPortalPlayerLocalData);
	DECL_M(GetPlayerState, CPlayerState);

	using _GetClientEntity = ClientEnt *(__rescall *)(void *thisptr, int entnum);
	using _KeyDown = int(__cdecl *)(void *b, const char *c);
	using _KeyUp = int(__cdecl *)(void *b, const char *c);
	using _GetAllClasses = ClientClass *(*)();
	using _FrameStageNotify = void(__rescall *)(void *thisptr, int stage);
	using _ShouldDraw = bool(__rescall *)(void *thisptr);
	using _ChatPrintf = void (*)(void *thisptr, int iPlayerIndex, int iFilter, const char *fmt, ...);
	using _StartMessageMode = void(__rescall *)(void *thisptr, int type);
	using _IN_ActivateMouse = void (*)(void *thisptr);
	using _IN_DeactivateMouse = void (*)(void *thisptr);
	using _AddAvatarPanelItem = void(__cdecl *)(void *pLeaderboard, void *pStatLists, const PortalLeaderboardItem_t *pData, int nScore, int nType, int nPlayerType, int nAvatarIndex, int nHeight, int nSlot, bool bHUDElement);
	using _PrecacheParticleSystem = int(__cdecl *)(const char *pszParticleName);
	using _DispatchParticleEffect = void (__cdecl *)(const char *pszParticleName, Vector vecOrigin, Vector vecStart, QAngle vecAngles, void *pEntity, int nSplitScreenPlayerSlot, void *filter);


	_GetClientEntity GetClientEntity = nullptr;
	_KeyDown KeyDown = nullptr;
	_KeyUp KeyUp = nullptr;
	_GetAllClasses GetAllClasses = nullptr;
	_FrameStageNotify FrameStageNotify = nullptr;
	_ShouldDraw ShouldDraw = nullptr;
	_ChatPrintf ChatPrintf = nullptr;
	_StartMessageMode StartMessageMode = nullptr;
	_IN_ActivateMouse IN_ActivateMouse = nullptr;
	_IN_DeactivateMouse IN_DeactivateMouse = nullptr;
	_AddAvatarPanelItem AddAvatarPanelItem = nullptr;
	_DispatchParticleEffect DispatchParticleEffect = nullptr;
	_PrecacheParticleSystem PrecacheParticleSystem = nullptr;

	ChapterContextData_t *g_ChapterContextNames;
	ChapterContextData_t *g_ChapterMPContextNames;

	void *in_jump;

	int *nNumSPChapters;
	int *nNumMPChapters;

	std::string lastLevelName;
	void **gamerules;

public:
	ClientEnt *GetPlayer(int index);
	void CalcButtonBits(int nSlot, int &bits, int in_button, int in_ignore, kbutton_t *button, bool reset);
	bool ShouldDrawCrosshair();
	void Chat(Color col, const char *str);
	void MultiColorChat(const std::vector<std::pair<Color, std::string>> &components);
	void ShowLocator(Vector position, Vector normal, Color color);
	void SetMouseActivated(bool state);
	CMStatus GetChallengeStatus();
	int GetSplitScreenPlayerSlot(void *entity);
	static Vector GetPlayerSize(bool ducked);
	void ClFrameStageNotify(int stage);
	void OpenChat();

public:
	// CBaseModPanel::GetChapterProgress
	DECL_DETOUR(GetChapterProgress);

	// BaseModUI::CPortalLeaderboardPanel::OnCommand
	DECL_DETOUR(OnCommand, const char *a2);

	// BaseModUI::CPortalLeaderboardPanel::SetPanelStats
	DECL_DETOUR(SetPanelStats);

	// CPortalLeaderboard::IsQuerying
	DECL_DETOUR(IsQuerying);

	// CUtlStringMap<CPortalLeaderboard *>::PurgeAndDeleteElements
	DECL_DETOUR(PurgeAndDeleteElements);

	// CPortalLeaderboardManager::GetLeaderboard
	DECL_DETOUR_T(void *, GetLeaderboard, const char *a2);

	// CPortalLeaderboard::StartSearching
	DECL_DETOUR(StartSearching);

	// CClientShadowMgr::AddShadowToReceiver
	DECL_DETOUR_T(void, AddShadowToReceiver, unsigned short handle, void *pRenderable, int type);

	// CBaseViewModel::CalcViewModelLag
	DECL_DETOUR_T(void, CalcViewModelLag, Vector &origin, QAngle &angles, QAngle &original_angles);

	// CGameMovement::ProcessMovement
	DECL_DETOUR(ProcessMovement, void *player, CMoveData *move);

	// CRendering3dView::DrawOpaqueRenderables
	DECL_DETOUR(DrawOpaqueRenderables, void *renderCtx, int renderPath, void *deferClippedOpaqueRenderablesOut);

	// CRendering3dView::DrawTranslucentRenderables
	DECL_DETOUR(DrawTranslucentRenderables, bool inSkybox, bool shadowDepth);

	// CHLClient::LevelInitPreEntity
	DECL_DETOUR(LevelInitPreEntity, const char *levelName);

	// ClientModeShared::CreateMove
	DECL_DETOUR(CreateMove, float flInputSampleTime, CUserCmd *cmd);
	DECL_DETOUR(CreateMove2, float flInputSampleTime, CUserCmd *cmd);

	// CHud::GetName
	DECL_DETOUR_T(const char *, GetName);

	// CHudMultiplayerBasicInfo::ShouldDraw
	DECL_DETOUR_T(bool, ShouldDraw_BasicInfo);

	// CHudSaveStatus::ShouldDraw
	DECL_DETOUR_T(bool, ShouldDraw_SaveStatus);

	// CHudChat::MsgFunc_SayText2
	DECL_DETOUR(MsgFunc_SayText2, bf_read &msg);

	// CHudChat::GetTextColorForClient
#ifdef _WIN32
	DECL_DETOUR_T(void *, GetTextColorForClient, Color *col_out, TextColor color, int client_idx);
#else
	DECL_DETOUR_T(Color, GetTextColorForClient, TextColor color, int client_idx);
#endif

	// CInput::_DecodeUserCmdFromBuffer
	DECL_DETOUR(DecodeUserCmdFromBuffer, int nSlot, int buf, signed int sequence_number);

	// CInput::CreateMove
	DECL_DETOUR(CInput_CreateMove, int sequence_number, float input_sample_frametime, bool active);

	// CInput::GetButtonBits
	DECL_DETOUR(GetButtonBits, bool bResetState);

	// C_Paint_Input::ApplyMouse
	DECL_DETOUR(ApplyMouse, int nSlot, QAngle &viewangles, CUserCmd *cmd, float mouse_x, float mouse_y);

	// CInput::SteamControllerMove
	DECL_DETOUR(SteamControllerMove, int nSlot, float flFrametime, CUserCmd *cmd);  //	is it slot though? :thinking:

	// ClientModeShared::OverrideView
	DECL_DETOUR_T(void, OverrideView, CViewSetup *m_View);

	DECL_DETOUR_COMMAND(playvideo_end_level_transition);
	DECL_DETOUR_COMMAND(openleaderboard);
	DECL_DETOUR_COMMAND(closeleaderboard);

#ifdef _WIN32
	// C_Paint_Input::ApplyMouse
	static uintptr_t ApplyMouse_Mid_Continue;
	DECL_DETOUR_MID_MH(ApplyMouse_Mid);
#endif

	// C_Prop_Portal::DrawPortal
	DECL_DETOUR(DrawPortal, void *pRenderContext);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override {
		return MODULE("client");
	}
};

extern Client *client;

extern Variable cl_showpos;
extern Variable cl_sidespeed;
extern Variable cl_backspeed;
extern Variable cl_forwardspeed;
extern Variable hidehud;
extern Variable in_forceuser;
extern Variable cl_fov;
extern Variable prevent_crouch_jump;
extern Variable r_PortalTestEnts;
extern Variable r_portalsopenall;
extern Variable r_drawviewmodel;
extern Variable crosshairVariable;

extern Variable soundfade;
extern Variable leaderboard_open;
extern Variable gameui_activate;
extern Variable gameui_allowescape;
extern Variable gameui_preventescape;
extern Variable setpause;
extern Variable snd_ducktovolume;
extern Variable say;

extern Variable cl_cmdrate;
extern Variable cl_updaterate;
extern Variable cl_chat_active;
extern Variable ss_pipsplit;
extern Variable ss_pipscale;
extern Variable ss_verticalsplit;
extern Variable cl_viewmodelfov;

extern Command sar_workshop_skip;

extern Color SARUTIL_Portal_Color(int iPortal, int iTeamNumber);
