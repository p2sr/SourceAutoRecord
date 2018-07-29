#pragma once
#include "Console.hpp"
#include "DemoPlayer.hpp"
#include "DemoRecorder.hpp"
#include "Tier1.hpp"

#include "Features/Session.hpp"
#include "Features/Speedrun.hpp"
#include "Features/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas.hpp"
#include "Features/Timer.hpp"

#include "Cheats.hpp"
#include "Game.hpp"
#include "Interfaces.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#if _WIN32
#define IServerMessageHandler_VMT_Offset 8
#endif

namespace Engine {

VMT engine;
VMT cl;
VMT s_GameEventManager;
VMT eng;

using _GetScreenSize = int(__stdcall*)(int& width, int& height);
using _ClientCmd = int(__func*)(void* thisptr, const char* szCmdString);
using _GetLocalPlayer = int(__func*)(void* thisptr);
using _GetViewAngles = int(__func*)(void* thisptr, QAngle& va);
using _SetViewAngles = int(__func*)(void* thisptr, QAngle& va);
using _AddListener = bool(__func*)(void* thisptr, IGameEventListener2* listener, const char* name, bool serverside);
using _RemoveListener = bool(__func*)(void* thisptr, IGameEventListener2* listener);
using _Cbuf_AddText = void(__cdecl*)(int slot, const char* pText, int nTickDelay);
using _AddText = void(__func*)(void* thisptr, const char* pText, int nTickDelay);
using _GetActiveSplitScreenPlayerSlot = int (*)();

_GetScreenSize GetScreenSize;
_ClientCmd ClientCmd;
_GetLocalPlayer GetLocalPlayer;
_GetViewAngles GetViewAngles;
_SetViewAngles SetViewAngles;
_GetActiveSplitScreenPlayerSlot GetActiveSplitScreenPlayerSlot;
_AddListener AddListener;
_RemoveListener RemoveListener;
_Cbuf_AddText Cbuf_AddText;
_AddText AddText;

int* tickcount;
float* interval_per_tick;
char* m_szLevelName;
CHostState* hoststate;
void* s_CommandBuffer;

void ExecuteCommand(const char* cmd)
{
    ClientCmd(engine->GetThisPtr(), cmd);
}
int GetSessionTick()
{
    int result = *tickcount - Session::BaseTick;
    return (result >= 0) ? result : 0;
}
float ToTime(int tick)
{
    return tick * *interval_per_tick;
}
int GetLocalPlayerIndex()
{
    return GetLocalPlayer(engine->GetThisPtr());
}
QAngle GetAngles()
{
    auto va = QAngle();
    GetViewAngles(engine->GetThisPtr(), va);
    return va;
}
void SetAngles(QAngle va)
{
    SetViewAngles(engine->GetThisPtr(), va);
}
void SendToCommandBuffer(const char* text, int delay)
{
    if (Game::IsPortal2Engine()) {
        Cbuf_AddText(GetActiveSplitScreenPlayerSlot(), text, delay);
    } else if (Game::IsHalfLife2Engine()) {
        AddText(s_CommandBuffer, text, delay);
    }
}

bool isInSession = false;
unsigned currentFrame = 0;
unsigned lastFrame = 0;
int prevSignonState = 0;
HOSTSTATES prevState;

void SessionStarted()
{
    console->Print("Session Started!\n");
    Session::Rebase(*tickcount);
    Timer::Rebase(*tickcount);
    Speedrun::timer->Unpause(tickcount);

    if (Rebinder::IsSaveBinding || Rebinder::IsReloadBinding) {
        if (DemoRecorder::IsRecordingDemo) {
            Rebinder::UpdateIndex(*DemoRecorder::m_nDemoNumber);
        } else {
            Rebinder::UpdateIndex(Rebinder::LastIndexNumber + 1);
        }

        Rebinder::RebindSave();
        Rebinder::RebindReload();
    }

    if (Cheats::sar_tas_autostart.GetBool()) {
        TAS::Start();
    }
    if (Cheats::sar_tas_autorecord.GetBool()) {
        TAS2::StartRecording();
    }
    if (Cheats::sar_tas_autoplay.GetBool()) {
        TAS2::StartPlaying();
    }

    StepCounter::ResetTimer();
    isInSession = true;
    currentFrame = 0;
}
void SessionEnded()
{
    if (!DemoPlayer::IsPlaying() && isInSession) {
        int tick = GetSessionTick();

        if (tick != 0) {
            console->Print("Session: %i (%.3f)\n", tick, Engine::ToTime(tick));
            Session::LastSession = tick;
        }

        if (Summary::IsRunning) {
            Summary::Add(tick, Engine::ToTime(tick), m_szLevelName);
            console->Print("Total: %i (%.3f)\n", Summary::TotalTicks, Engine::ToTime(Summary::TotalTicks));
        }

        if (Timer::IsRunning) {
            if (Cheats::sar_timer_always_running.GetBool()) {
                Timer::Save(*tickcount);
                console->Print("Timer paused: %i (%.3f)!\n", Timer::TotalTicks, Engine::ToTime(Timer::TotalTicks));
            } else {
                Timer::Stop(*tickcount);
                console->Print("Timer stopped!\n");
            }
        }

        auto reset = Cheats::sar_stats_auto_reset.GetInt();
        if ((reset == 1 && !*m_bLoadgame) || reset >= 2) {
            Stats::ResetAll();
        }

        DemoRecorder::CurrentDemo = "";
        lastFrame = currentFrame;
        currentFrame = 0;
    }

    TAS::Reset();
    TAS2::Stop();
    isInSession = false;
    Speedrun::timer->Pause();
}
void SessionChanged(int state)
{
    console->Print("state = %i\n", state);
    if (state != prevSignonState && prevSignonState == SignonState::Full) {
        SessionEnded();
    }

    // Demo recorder starts syncing from this tick
    if (state == SignonState::Full) {
        SessionStarted();
    }

    prevSignonState = state;
}
void SafeUnload(const char* postCommand = nullptr)
{
    // The exit command will handle everything
    ExecuteCommand("sar_exit");

    if (postCommand) {
        SendToCommandBuffer(postCommand, SAFE_UNLOAD_TICK_DELAY);
    }
}

// CClientState::Disconnect
DETOUR(Disconnect, bool bShowMainMenu)
{
    SessionEnded();
    return Original::Disconnect(thisptr, bShowMainMenu);
}
#ifdef _WIN32
DETOUR(Disconnect2, int unk1, int unk2, int unk3)
{
    SessionEnded();
    return Original::Disconnect2(thisptr, unk1, unk2, unk3);
}
DETOUR_COMMAND(connect)
{
    SessionEnded();
    Original::connect_callback(args);
}
#else
DETOUR(Disconnect2, int unk, bool bShowMainMenu)
{
    SessionEnded();
    return Original::Disconnect2(thisptr, unk, bShowMainMenu);
}
#endif

// CClientState::SetSignonState
DETOUR(SetSignonState, int state, int count, void* unk)
{
    SessionChanged(state);
    return Original::SetSignonState(thisptr, state, count, unk);
}
DETOUR(SetSignonState2, int state, int count)
{
    SessionChanged(state);
    return Original::SetSignonState2(thisptr, state, count);
}

// CEngine::Frame
DETOUR(Frame)
{
    if (hoststate->m_currentState != prevState) {
        console->Print("m_currentState = %i\n", hoststate->m_currentState);
        if (hoststate->m_currentState == HOSTSTATES::HS_CHANGE_LEVEL_SP) {
            SessionEnded();
        } else if (hoststate->m_currentState == HOSTSTATES::HS_RUN
            && !hoststate->m_activeGame
            && !DemoPlayer::IsPlaying()) {
            console->Print("Session started! (menu)\n");
            Session::Rebase(*tickcount);
            Speedrun::timer->Unpause(tickcount);
        }
    }
    prevState = hoststate->m_currentState;

    if (hoststate->m_activeGame) {
        Speedrun::timer->Update(tickcount, m_bLoadgame, m_szLevelName);
    }

    return Original::Frame(thisptr);
}

DETOUR_COMMAND(plugin_load)
{
    // Prevent crash when trying to load SAR twice or try to find the module in
    // the plugin list if the initial search thread failed
    if (args.ArgC() >= 2) {
        auto file = std::string(args[1]);
        if (endsWith(file, std::string(MODULE("sar"))) || endsWith(file, std::string("sar"))) {
            if (plugin->found) {
                console->Warning("SAR: Plugin already loaded!\n");
            } else if (SAR::PluginFound()) {
                plugin->ptr->m_bDisable = true;
                console->PrintActive("SAR: Plugin fully loaded!\n");
            } else {
                console->Warning("SAR: This should never happen :(\n");
            }
            return;
        }
    }

    Original::plugin_load_callback(args);
}
DETOUR_COMMAND(plugin_unload)
{
    if (args.ArgC() >= 2 && atoi(args[1]) == plugin->index) {
        SafeUnload();
    } else {
        Original::plugin_unload_callback(args);
    }
}
DETOUR_COMMAND(exit)
{
    SafeUnload("exit");
}
DETOUR_COMMAND(quit)
{
    SafeUnload("quit");
}

void Hook()
{
    CREATE_VMT(Interfaces::IVEngineClient, engine)
    {
        GetScreenSize = engine->GetOriginalFunction<_GetScreenSize>(Offsets::GetScreenSize);
        ClientCmd = engine->GetOriginalFunction<_ClientCmd>(Offsets::ClientCmd);
        GetLocalPlayer = engine->GetOriginalFunction<_GetLocalPlayer>(Offsets::GetLocalPlayer);
        GetViewAngles = engine->GetOriginalFunction<_GetViewAngles>(Offsets::GetViewAngles);
        SetViewAngles = engine->GetOriginalFunction<_SetViewAngles>(Offsets::SetViewAngles);
        GetGameDirectory = engine->GetOriginalFunction<_GetGameDirectory>(Offsets::GetGameDirectory);

        Cbuf_AddText = Memory::ReadAbsoluteAddress<_Cbuf_AddText>((uintptr_t)ClientCmd + Offsets::Cbuf_AddText);

        void* clPtr = nullptr;
        if (Game::IsPortal2Engine()) {
            typedef void* (*_GetClientState)();
            auto GetClientState = Memory::ReadAbsoluteAddress<_GetClientState>((uintptr_t)ClientCmd + Offsets::GetClientStateFunction);
            clPtr = GetClientState();

            GetActiveSplitScreenPlayerSlot = engine->GetOriginalFunction<_GetActiveSplitScreenPlayerSlot>(Offsets::GetActiveSplitScreenPlayerSlot);
        } else if (Game::IsHalfLife2Engine()) {
            auto ServerCmdKeyValues = engine->GetOriginalFunction<uintptr_t>(Offsets::ServerCmdKeyValues);
            clPtr = Memory::Deref<void*>(ServerCmdKeyValues + Offsets::cl);

            AddText = Memory::ReadAbsoluteAddress<_AddText>((uintptr_t)Cbuf_AddText + Offsets::AddText);
            s_CommandBuffer = Memory::Deref<void*>((uintptr_t)Cbuf_AddText + Offsets::s_CommandBuffer);
        }

        CREATE_VMT(clPtr, cl)
        {
            if (Game::IsPortal2Engine()) {
                HOOK_O(cl, SetSignonState, Offsets::Disconnect - 1);
                HOOK(cl, Disconnect);
            } else {
                HOOK_O(cl, SetSignonState2, Offsets::Disconnect - 1);
#ifdef _WIN32
                HOOK_COMMAND(connect);
#else
                HOOK_O(cl, Disconnect2, Offsets::Disconnect);
#endif
            }

            auto disconnect = cl->GetOriginalFunction<uintptr_t>(Offsets::Disconnect);
            DemoPlayer::Hook(Memory::DerefDeref<void*>(disconnect + Offsets::demoplayer));
            DemoRecorder::Hook(Memory::DerefDeref<void*>(disconnect + Offsets::demorecorder));

#if _WIN32
            auto IServerMessageHandler_VMT = Memory::Deref<uintptr_t>((uintptr_t)cl->GetThisPtr() + IServerMessageHandler_VMT_Offset);
            auto ProcessTick = Memory::Deref<uintptr_t>(IServerMessageHandler_VMT + sizeof(uintptr_t) * Offsets::ProcessTick);
#else
            auto ProcessTick = cl->GetOriginalFunction<uintptr_t>(Offsets::ProcessTick);
#endif
            tickcount = Memory::Deref<int*>(ProcessTick + Offsets::tickcount);
            interval_per_tick = Memory::Deref<float*>(ProcessTick + Offsets::interval_per_tick);
            Speedrun::timer->SetIntervalPerTick(interval_per_tick);

            auto hsf = SAR::Find("HostState_Frame");
            if (hsf.Found) {
                hoststate = Memory::Deref<CHostState*>(hsf.Address + Offsets::hoststate);
                auto FrameUpdate = Memory::ReadAbsoluteAddress(hsf.Address + Offsets::FrameUpdate);
#if _WIN32
                auto engAddr = Memory::DerefDeref<void*>(FrameUpdate + Offsets::eng);
#else
                auto State_Shutdown = Memory::ReadAbsoluteAddress(FrameUpdate + Offsets::State_Shutdown);
                auto engAddr = Memory::DerefDeref<void*>(State_Shutdown + Offsets::eng);
#endif
                CREATE_VMT(engAddr, eng)
                {
                    HOOK(eng, Frame);
                }
            }
        }
    }

    VMT tool;
    CREATE_VMT(Interfaces::IEngineTool, tool)
    {
        auto GetCurrentMap = tool->GetOriginalFunction<uintptr_t>(Offsets::GetCurrentMap);
        m_szLevelName = Memory::Deref<char*>(GetCurrentMap + Offsets::m_szLevelName);
    }

    auto ldg = SAR::Find("m_bLoadgame");
    if (ldg.Found) {
        m_bLoadgame = Memory::Deref<bool*>(ldg.Address);
    }

    CREATE_VMT(Interfaces::IGameEventManager2, s_GameEventManager)
    {
        AddListener = s_GameEventManager->GetOriginalFunction<_AddListener>(Offsets::AddListener);
        RemoveListener = s_GameEventManager->GetOriginalFunction<_RemoveListener>(Offsets::RemoveListener);
    }

    HOOK_COMMAND(plugin_load);
    HOOK_COMMAND(plugin_unload);
    HOOK_COMMAND(exit);
    HOOK_COMMAND(quit);
}
void Unhook()
{
    UNHOOK_O(cl, Offsets::Disconnect - 1);
#ifdef _WIN32
    UNHOOK_COMMAND(connect);
#else
    UNHOOK_O(cl, Offsets::Disconnect);
#endif
    UNHOOK(eng, Frame);

    DELETE_VMT(engine);
    DELETE_VMT(cl);
    DELETE_VMT(s_GameEventManager);
    DELETE_VMT(eng);

    UNHOOK_COMMAND(plugin_load);
    UNHOOK_COMMAND(plugin_unload);
    UNHOOK_COMMAND(exit);
    UNHOOK_COMMAND(quit);

    DemoPlayer::Unhook();
    DemoRecorder::Unhook();
}
}
