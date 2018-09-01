#pragma once
#include "Module.hpp"

#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"

#include "Command.hpp"
#include "Interface.hpp"
#include "Utils.hpp"

#if _WIN32
#define IServerMessageHandler_VMT_Offset 8
#endif

class Engine : public Module {
public:
    Interface* engineClient;
    Interface* cl;
    Interface* s_GameEventManager;
    Interface* eng;
    Interface* debugoverlay;

    using _GetScreenSize = int(__stdcall*)(int& width, int& height);
    using _ClientCmd = int(__func*)(void* thisptr, const char* szCmdString);
    using _GetLocalPlayer = int(__func*)(void* thisptr);
    using _GetViewAngles = int(__func*)(void* thisptr, QAngle& va);
    using _SetViewAngles = int(__func*)(void* thisptr, QAngle& va);
    using _GetMaxClients = int (*)();
    using _GetGameDirectory = char*(__cdecl*)();
    using _AddListener = bool(__func*)(void* thisptr, IGameEventListener2* listener, const char* name, bool serverside);
    using _RemoveListener = bool(__func*)(void* thisptr, IGameEventListener2* listener);
    using _Cbuf_AddText = void(__cdecl*)(int slot, const char* pText, int nTickDelay);
    using _AddText = void(__func*)(void* thisptr, const char* pText, int nTickDelay);
#ifdef _WIN32
    using _GetActiveSplitScreenPlayerSlot = int (*)();
    using _ScreenPosition = int(__stdcall*)(const Vector& point, Vector& screen);
    using _ConPrintEvent = int(__stdcall*)(IGameEvent* ev);
#else
    using _GetActiveSplitScreenPlayerSlot = int (*)(void* thisptr);
    using _ScreenPosition = int(__stdcall*)(void* thisptr, const Vector& point, Vector& screen);
    using _ConPrintEvent = int(__cdecl*)(void* thisptr, IGameEvent* ev);
#endif

    _GetScreenSize GetScreenSize;
    _ClientCmd ClientCmd;
    _GetLocalPlayer GetLocalPlayer;
    _GetViewAngles GetViewAngles;
    _SetViewAngles SetViewAngles;
    _GetMaxClients GetMaxClients;
    _GetGameDirectory GetGameDirectory;
    _GetActiveSplitScreenPlayerSlot GetActiveSplitScreenPlayerSlot;
    _AddListener AddListener;
    _RemoveListener RemoveListener;
    _Cbuf_AddText Cbuf_AddText;
    _AddText AddText;
    _ScreenPosition ScreenPosition;
    _ConPrintEvent ConPrintEvent;

    EngineDemoPlayer* demoplayer;
    EngineDemoRecorder* demorecorder;

    int* tickcount;
    float* interval_per_tick;
    char* m_szLevelName;
    bool* m_bLoadgame;
    CHostState* hoststate;
    void* s_CommandBuffer;
    bool* m_bWaitEnabled;

    void ExecuteCommand(const char* cmd);
    void ClientCommand(const char* fmt, ...);
    int GetSessionTick();
    float ToTime(int tick);
    int GetLocalPlayerIndex();
    QAngle GetAngles();
    void SetAngles(QAngle va);
    void SendToCommandBuffer(const char* text, int delay);
    int PointToScreen(const Vector& point, Vector& screen);
    void SafeUnload(const char* postCommand = nullptr);

private:
    // CClientState::Disconnect
    DECL_DETOUR(Disconnect, bool bShowMainMenu)
#ifdef _WIN32
    DECL_DETOUR(Disconnect2, int unk1, int unk2, int unk3)
    DECL_DETOUR_COMMAND(connect)
#else
    DECL_DETOUR(Disconnect2, int unk, bool bShowMainMenu)
#endif

    // CClientState::SetSignonState
    DECL_DETOUR(SetSignonState, int state, int count, void* unk)
    DECL_DETOUR(SetSignonState2, int state, int count)

    // CEngine::Frame
    DECL_DETOUR(Frame)

    DECL_DETOUR_COMMAND(plugin_load)
    DECL_DETOUR_COMMAND(plugin_unload)
    DECL_DETOUR_COMMAND(exit)
    DECL_DETOUR_COMMAND(quit)
    DECL_DETOUR_COMMAND(help)

    bool Init() override;
    void Shutdown() override;
};

extern Engine* engine;
