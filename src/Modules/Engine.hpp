#pragma once
#include "Command.hpp"
#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#if _WIN32
#	define IServerMessageHandler_VMT_Offset 8
#endif

class Engine : public Module {
public:
	Interface *engineClient = nullptr;
	Interface *cl = nullptr;
	Interface *s_GameEventManager = nullptr;
	Interface *eng = nullptr;
	Interface *debugoverlay = nullptr;
	Interface *s_ServerPlugin = nullptr;
	Interface *engineTool = nullptr;
	Interface *engineTrace = nullptr;
	Interface *g_VEngineServer = nullptr;

	using _ClientCmd = int(__rescall *)(void *thisptr, const char *szCmdString);
	using _ExecuteClientCmd = int(__rescall *)(void *thisptr, const char *szCmdString);
	using _GetLocalPlayer = int(__rescall *)(void *thisptr);
	using _GetViewAngles = int(__rescall *)(void *thisptr, QAngle &va);
	using _SetViewAngles = int(__rescall *)(void *thisptr, QAngle &va);
	using _GetMaxClients = int (*)();
	using _GetGameDirectory = char *(__cdecl *)();
	using _GetSaveDirName = char *(__cdecl *)();
	using _AddListener = bool(__rescall *)(void *thisptr, IGameEventListener2 *listener, const char *name, bool serverside);
	using _RemoveListener = bool(__rescall *)(void *thisptr, IGameEventListener2 *listener);
	using _Cbuf_AddText = void(__cdecl *)(int slot, const char *pText, int nTickDelay);
	using _AddText = void(__rescall *)(void *thisptr, const char *pText, int nTickDelay);
	using _ClientCommand = int (*)(void *thisptr, void *pEdict, const char *szFmt, ...);
	using _IsServerPaused = bool (*)(void *thisptr);
	using _ServerPause = bool (*)(void *thisptr, bool bPause, bool bForce);
	using _GetLocalClient = int (*)(int index);
	using _HostFrameTime = float (*)(void *thisptr);
	using _ClientTime = float (*)(void *thisptr);
	using _IsPaused = bool (*)(void *thisptr);
	using _TraceRay = void(__rescall *)(void *thisptr, const Ray_t &ray, unsigned int fMask, ITraceFilter *pTraceFilter, CGameTrace *pTrace);
	using _GetCount = int(__rescall *)(void *thisptr);
	using _Con_IsVisible = bool(__rescall *)(void *thisptr);
	using _GetLevelNameShort = const char *(__rescall *)(void *thisptr);
	using _GetScreenSize = int(__rescall *)(void *thisptr, int &width, int &height);
	using _GetActiveSplitScreenPlayerSlot = int (__rescall *)(void *thisptr);
	using _ScreenPosition = int(__rescall *)(void *thisptr, const Vector &point, Vector &screen);
	using _ConPrintEvent = int(__rescall *)(void *thisptr, IGameEvent *ev);
	using _PrecacheModel = int(__rescall *)(void *thisptr, const char *, bool);
	using _AddBoxOverlay = int(__rescall *)(void *thisptr, const Vector &origin, const Vector &mins, const Vector &MAX, QAngle const &orientation, int r, int g, int b, int a, float duration);
	using _AddSphereOverlay = int(__rescall *)(void *thisptr, const Vector &vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration);
	using _AddTriangleOverlay = int(__rescall *)(void *thisptr, const Vector &p1, const Vector &p2, const Vector &p3, int r, int g, int b, int a, bool noDepthTest, float duration);
	using _AddLineOverlay = int(__rescall *)(void *thisptr, const Vector &origin, const Vector &dest, int r, int g, int b, bool noDepthText, float duration);
	using _AddScreenTextOverlay = void(__rescall *)(void *thisptr, float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char *text);
	using _ClearAllOverlays = void(__rescall *)(void *thisptr);

	_GetScreenSize GetScreenSize = nullptr;
	_ClientCmd ClientCmd = nullptr;
	_ExecuteClientCmd ExecuteClientCmd = nullptr;
	_GetLocalPlayer GetLocalPlayer = nullptr;
	_GetViewAngles GetViewAngles = nullptr;
	_SetViewAngles SetViewAngles = nullptr;
	_GetMaxClients GetMaxClients = nullptr;
	_GetGameDirectory GetGameDirectory = nullptr;
	_GetSaveDirName GetSaveDirName = nullptr;
	_GetActiveSplitScreenPlayerSlot GetActiveSplitScreenPlayerSlot = nullptr;
	_AddListener AddListener = nullptr;
	_RemoveListener RemoveListener = nullptr;
	_Cbuf_AddText Cbuf_AddText = nullptr;
	_AddText AddText = nullptr;
	_ScreenPosition ScreenPosition = nullptr;
	_ConPrintEvent ConPrintEvent = nullptr;
	_ClientCommand ClientCommand = nullptr;
	_IsServerPaused IsServerPaused = nullptr;
	_ServerPause ServerPause = nullptr;
	_GetLocalClient GetLocalClient = nullptr;
	_HostFrameTime HostFrameTime = nullptr;
	_ClientTime ClientTime = nullptr;
	_PrecacheModel PrecacheModel = nullptr;
	_AddBoxOverlay AddBoxOverlay = nullptr;
	_AddSphereOverlay AddSphereOverlay = nullptr;
	_AddTriangleOverlay AddTriangleOverlay = nullptr;
	_AddLineOverlay AddLineOverlay = nullptr;
	_AddScreenTextOverlay AddScreenTextOverlay = nullptr;
	_ClearAllOverlays ClearAllOverlays = nullptr;
	_IsPaused IsPaused = nullptr;
	_TraceRay TraceRay = nullptr;
	_GetCount GetCount = nullptr;
	_Con_IsVisible Con_IsVisible = nullptr;
	_GetLevelNameShort GetLevelNameShort = nullptr;

	EngineDemoPlayer *demoplayer = nullptr;
	EngineDemoRecorder *demorecorder = nullptr;

	int *tickcount = nullptr;
	double *net_time = nullptr;
	float *interval_per_tick = nullptr;
	char *m_szLevelName = nullptr;
	bool *m_bLoadgame = nullptr;
	CHostState *hoststate = nullptr;
	void *s_CommandBuffer = nullptr;
	bool *m_bWaitEnabled = nullptr;
	bool *m_bWaitEnabled2 = nullptr;
	int lastTick = 0;

	bool overlayActivated = false;
	bool hasRecorded = false;
	bool hasPaused = false;
	bool isPausing = false;
	int pauseTick;
	bool hasWaited = false;
	bool startedTransitionFadeout = false;
	bool forcedPrimaryFullscreen = false;
	bool shouldPauseForSync = false;

public:
	void ExecuteCommand(const char *cmd, bool immediately = false);
	int GetTick();
	float ToTime(int tick);
	int GetLocalPlayerIndex();
	edict_t *PEntityOfEntIndex(int iEntIndex);
	QAngle GetAngles(int nSlot);
	void SetAngles(int nSlot, QAngle va);
	void SendToCommandBuffer(const char *text, int delay);
	int PointToScreen(const Vector &point, Vector &screen);
	void SafeUnload(const char *postCommand = nullptr);
	float GetHostFrameTime();
	float GetClientTime();
	bool isRunning();
	bool IsGamePaused();
	int GetMapIndex(const std::string map);
	std::string GetCurrentMapName();
	bool IsCoop();
	bool IsOrange();
	void RecordDemoData(void *data, size_t len);
	bool Trace(Vector &pos, QAngle &angle, float distMax, CTraceFilterSimple &filter, CGameTrace &tr);
	bool TraceFromCamera(float distMax, CGameTrace &tr);
	bool ConsoleVisible();
	void GetTicks(int &host, int &server, int &client);

	// CClientState::Disconnect
	DECL_DETOUR(Disconnect, bool bShowMainMenu);

	// CClientState::SetSignonState
	DECL_DETOUR(SetSignonState, int state, int count, void *unk);

	// CEngine::Frame
	DECL_DETOUR(Frame);

	// CModelLoader
	DECL_DETOUR(PurgeUnusedModels);

	// CSteam3Client::OnGameOverlayActivated
	DECL_DETOUR_B(OnGameOverlayActivated, GameOverlayActivated_t *pGameOverlayActivated);

	DECL_DETOUR_COMMAND(plugin_load);
	DECL_DETOUR_COMMAND(plugin_unload);
	DECL_DETOUR_COMMAND(exit);
	DECL_DETOUR_COMMAND(quit);
	DECL_DETOUR_COMMAND(help);
	DECL_DETOUR_COMMAND(gameui_activate);
	DECL_DETOUR_COMMAND(playvideo_end_level_transition);
	DECL_DETOUR_COMMAND(stop_transition_videos_fadeout);
	DECL_DETOUR_COMMAND(unpause);
	DECL_DETOUR_COMMAND(load);
	DECL_DETOUR_COMMAND(give);

	DECL_DETOUR(ReadCustomData, int *callbackIndex, char **data);
	DECL_DETOUR_T(const char *, ReadConsoleCommand);

#ifdef _WIN32
	// CDemoSmootherPanel::ParseSmoothingInfo
	static uintptr_t ParseSmoothingInfo_Skip;
	static uintptr_t ParseSmoothingInfo_Default;
	static uintptr_t ParseSmoothingInfo_Continue;
	DECL_DETOUR_MID_MH(ParseSmoothingInfo_Mid);

	Memory::Patch *demoSmootherPatch = nullptr;
#endif

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("engine"); }

private:
	uintptr_t readCustomDataInjectAddr;
	uintptr_t readConsoleCommandInjectAddr;
};

extern Engine *engine;

extern Variable host_framerate;
extern Variable net_showmsg;
extern Variable sv_portal_players;
extern Variable fps_max;
extern Variable mat_norendering;

extern Variable sar_record_at;
extern Variable sar_record_at_demo_name;
extern Variable sar_record_at_increment;

extern Variable sar_pause_at;
extern Variable sar_pause_for;

extern Variable sar_tick_debug;

#define TIME_TO_TICKS(dt) ((int)(0.5f + (float)(dt) / *engine->interval_per_tick))
#define GET_SLOT() engine->GetLocalPlayerIndex() - 1
#define IGNORE_DEMO_PLAYER()          \
	if (engine->demoplayer->IsPlaying()) \
		return;
#define NOW() std::chrono::high_resolution_clock::now()
#define NOW_STEADY() std::chrono::steady_clock::now()

#define GET_ACTIVE_SPLITSCREEN_SLOT() engine->GetActiveSplitScreenPlayerSlot(nullptr)
