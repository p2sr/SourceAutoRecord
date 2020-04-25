#pragma once
#include "Module.hpp"

#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"

#include "Command.hpp"
#include "Interface.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#if _WIN32
#define IServerMessageHandler_VMT_Offset 8
#endif

class Engine : public Module {
public:
    Interface* engineClient = nullptr;
    Interface* cl = nullptr;
    Interface* s_GameEventManager = nullptr;
    Interface* eng = nullptr;
    Interface* debugoverlay = nullptr;
    Interface* s_ServerPlugin = nullptr;
    Interface* engineTool = nullptr;

    using _ClientCmd = int(__rescall*)(void* thisptr, const char* szCmdString);
    using _GetLocalPlayer = int(__rescall*)(void* thisptr);
    using _GetViewAngles = int(__rescall*)(void* thisptr, QAngle& va);
    using _SetViewAngles = int(__rescall*)(void* thisptr, QAngle& va);
    using _GetMaxClients = int (*)();
    using _GetGameDirectory = char*(__cdecl*)();
    using _AddListener = bool(__rescall*)(void* thisptr, IGameEventListener2* listener, const char* name, bool serverside);
    using _RemoveListener = bool(__rescall*)(void* thisptr, IGameEventListener2* listener);
    using _Cbuf_AddText = void(__cdecl*)(int slot, const char* pText, int nTickDelay);
    using _AddText = void(__rescall*)(void* thisptr, const char* pText, int nTickDelay);
    using _ClientCommand = int (*)(void* thisptr, void* pEdict, const char* szFmt, ...);
    using _GetLocalClient = int (*)(int index);
    using _HostFrameTime = float (*)(void* thisptr);
    using _ClientTime = float (*)(void* thisptr);
#ifdef _WIN32
    using _GetScreenSize = int(__stdcall*)(int& width, int& height);
    using _GetActiveSplitScreenPlayerSlot = int (*)();
    using _ScreenPosition = int(__stdcall*)(const Vector& point, Vector& screen);
    using _ConPrintEvent = int(__stdcall*)(IGameEvent* ev);
#else
    using _GetScreenSize = int(__cdecl*)(void* thisptr, int& width, int& height);
    using _GetActiveSplitScreenPlayerSlot = int (*)(void* thisptr);
    using _ScreenPosition = int(__stdcall*)(void* thisptr, const Vector& point, Vector& screen);
    using _ConPrintEvent = int(__cdecl*)(void* thisptr, IGameEvent* ev);
#endif

    _GetScreenSize GetScreenSize = nullptr;
    _ClientCmd ClientCmd = nullptr;
    _GetLocalPlayer GetLocalPlayer = nullptr;
    _GetViewAngles GetViewAngles = nullptr;
    _SetViewAngles SetViewAngles = nullptr;
    _GetMaxClients GetMaxClients = nullptr;
    _GetGameDirectory GetGameDirectory = nullptr;
    _GetActiveSplitScreenPlayerSlot GetActiveSplitScreenPlayerSlot = nullptr;
    _AddListener AddListener = nullptr;
    _RemoveListener RemoveListener = nullptr;
    _Cbuf_AddText Cbuf_AddText = nullptr;
    _AddText AddText = nullptr;
    _ScreenPosition ScreenPosition = nullptr;
    _ConPrintEvent ConPrintEvent = nullptr;
    _ClientCommand ClientCommand = nullptr;
    _GetLocalClient GetLocalClient = nullptr;
    _HostFrameTime HostFrameTime = nullptr;
    _ClientTime ClientTime = nullptr;

    EngineDemoPlayer* demoplayer = nullptr;
    EngineDemoRecorder* demorecorder = nullptr;

    int* tickcount = nullptr;
    double* net_time = nullptr;
    float* interval_per_tick = nullptr;
    char* m_szLevelName = nullptr;
    bool* m_bLoadgame = nullptr;
    CHostState* hoststate = nullptr;
    void* s_CommandBuffer = nullptr;
    bool* m_bWaitEnabled = nullptr;
    bool* m_bWaitEnabled2 = nullptr;

    bool overlayActivated = false;

public:
    void ExecuteCommand(const char* cmd);
    int GetTick();
    float ToTime(int tick);
    int GetLocalPlayerIndex();
    edict_t* PEntityOfEntIndex(int iEntIndex);
    QAngle GetAngles(int nSlot);
    void SetAngles(int nSlot, QAngle va);
    void SendToCommandBuffer(const char* text, int delay);
    int PointToScreen(const Vector& point, Vector& screen);
    void SafeUnload(const char* postCommand = nullptr);
    float GetHostFrameTime();
    float GetClientTime();

    // CClientState::Disconnect
    DECL_DETOUR(Disconnect, bool bShowMainMenu);
#ifdef _WIN32
    DECL_DETOUR(Disconnect2, int unk1, int unk2, int unk3);
    DECL_DETOUR_COMMAND(connect);
#else
    DECL_DETOUR(Disconnect2, int unk, bool bShowMainMenu);
#endif

    // CClientState::SetSignonState
    DECL_DETOUR(SetSignonState, int state, int count, void* unk);
    DECL_DETOUR(SetSignonState2, int state, int count);

    // CEngine::Frame
    DECL_DETOUR(Frame);

    // CSteam3Client::OnGameOverlayActivated
    DECL_DETOUR_B(OnGameOverlayActivated, GameOverlayActivated_t* pGameOverlayActivated);

    DECL_DETOUR_COMMAND(plugin_load);
    DECL_DETOUR_COMMAND(plugin_unload);
    DECL_DETOUR_COMMAND(exit);
    DECL_DETOUR_COMMAND(quit);
    DECL_DETOUR_COMMAND(help);
    DECL_DETOUR_COMMAND(gameui_activate);

#ifdef _WIN32
    using _ReadCustomData = int(__fastcall*)(void* thisptr, int edx, void* unk1, void* unk2);
    static _ReadCustomData ReadCustomData;

    // CDemoSmootherPanel::ParseSmoothingInfo
    static uintptr_t ParseSmoothingInfo_Skip;
    static uintptr_t ParseSmoothingInfo_Default;
    static uintptr_t ParseSmoothingInfo_Continue;
    DECL_DETOUR_MID_MH(ParseSmoothingInfo_Mid);

    Memory::Patch* demoSmootherPatch = nullptr;
#endif

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};

extern Engine* engine;

extern Variable host_framerate;
extern Variable net_showmsg;

#define TIME_TO_TICKS(dt) ((int)(0.5f + (float)(dt) / *engine->interval_per_tick))
#define GET_SLOT() engine->GetLocalPlayerIndex() - 1
#define IGNORE_DEMO_PLAYER() if (engine->demoplayer->IsPlaying()) return;

#ifdef _WIN32
#define GET_ACTIVE_SPLITSCREEN_SLOT() engine->GetActiveSplitScreenPlayerSlot()
#else
#define GET_ACTIVE_SPLITSCREEN_SLOT() engine->GetActiveSplitScreenPlayerSlot(nullptr)
#endif
