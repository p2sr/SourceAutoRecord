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
#include "Utils.hpp"

#define IServerMessageHandler_VMT_Offset 8

namespace Engine {

VMT engine;
VMT cl;
VMT s_GameEventManager;

using _GetScreenSize = int(__stdcall*)(int& width, int& height);
using _ClientCmd = int(__func*)(void* thisptr, const char* szCmdString);
using _GetLocalPlayer = int(__func*)(void* thisptr);
using _GetViewAngles = int(__func*)(void* thisptr, QAngle& va);
using _SetViewAngles = int(__func*)(void* thisptr, QAngle& va);
using _AddListener = bool(__func*)(void* thisptr, IGameEventListener2* listener, const char* name, bool serverside);
using _RemoveListener = bool(__func*)(void* thisptr, IGameEventListener2* listener);

_GetScreenSize GetScreenSize;
_ClientCmd ClientCmd;
_GetLocalPlayer GetLocalPlayer;
_GetViewAngles GetViewAngles;
_SetViewAngles SetViewAngles;
_AddListener AddListener;
_RemoveListener RemoveListener;

int* tickcount;
float* interval_per_tick;
char** m_szLevelName;

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

bool IsInGame = false;
unsigned int CurrentFrame = 0;
unsigned int LastFrame = 0;

void SessionStarted()
{
    Console::Print("Session Started!\n");
    Session::Rebase(*tickcount);
    Timer::Rebase(*tickcount);

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
    IsInGame = true;
    CurrentFrame = 0;
}
void SessionEnded()
{
    if (!DemoPlayer::IsPlaying() && IsInGame) {
        int tick = GetSessionTick();

        if (tick != 0) {
            Console::Print("Session: %i (%.3f)\n", tick, Engine::ToTime(tick));
            Session::LastSession = tick;
        }

        if (Summary::IsRunning) {
            Summary::Add(tick, Engine::ToTime(tick), *m_szLevelName);
            Console::Print("Total: %i (%.3f)\n", Summary::TotalTicks, Engine::ToTime(Summary::TotalTicks));
        }

        if (Timer::IsRunning) {
            if (Cheats::sar_timer_always_running.GetBool()) {
                Timer::Save(*tickcount);
                Console::Print("Timer paused: %i (%.3f)!\n", Timer::TotalTicks, Engine::ToTime(Timer::TotalTicks));
            } else {
                Timer::Stop(*tickcount);
                Console::Print("Timer stopped!\n");
            }
        }

        auto reset = Cheats::sar_stats_auto_reset.GetInt();
        if ((reset == 1 && !*m_bLoadgame) || reset >= 2) {
            Stats::ResetAll();
        }

        DemoRecorder::CurrentDemo = "";
        LastFrame = CurrentFrame;
        CurrentFrame = 0;
    }

    TAS::Reset();
    TAS2::Stop();
    IsInGame = false;
}

namespace Detour {
    int LastState;
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
    if (state != LastState && LastState == SignonState::Full) {
        SessionEnded();
    }

    // Demo recorder starts syncing from this tick
    if (state == SignonState::Full) {
        SessionStarted();
    }

    LastState = state;
    return Original::SetSignonState(thisptr, state, count, unk);
}
DETOUR(SetSignonState2, int state, int count)
{
    if (state != LastState && LastState == SignonState::Full) {
        SessionEnded();
    }

    // Demo recorder starts syncing from this tick
    if (state == SignonState::Full) {
        SessionStarted();
    }

    LastState = state;
    return Original::SetSignonState2(thisptr, state, count);
}

#ifdef _WIN32
HOSTSTATES prevHostState;

DETOUR_MH(HostStateFrame, float time)
{
    auto hs = reinterpret_cast<CHostState*>(thisptr);
    
    if (hs->m_currentState != prevHostState) {
        Console::Print("hs->m_currentState = %i\n", hs->m_currentState);
        if (hs->m_currentState == HOSTSTATES::HS_CHANGE_LEVEL_SP) {
            SessionEnded();
        } else if (hs->m_currentState == HOSTSTATES::HS_RUN && hs->m_activeGame && !DemoPlayer::IsPlaying()) {
            Console::Print("Detected menu!\n");
            Session::Rebase(*tickcount);
        }
    }
    prevHostState = hs->m_currentState;

    if (Speedrun::timer && tickcount && m_szLevelName && m_bLoadgame) {
        Speedrun::timer->Update(*tickcount, *m_szLevelName, *m_bLoadgame);
    }

    return Trampoline::HostStateFrame(thisptr, time);
}
#endif

void Hook()
{
    CREATE_VMT(Interfaces::IVEngineClient, engine) {
        GetScreenSize = engine->GetOriginalFunction<_GetScreenSize>(Offsets::GetScreenSize);
        ClientCmd = engine->GetOriginalFunction<_ClientCmd>(Offsets::ClientCmd);
        GetLocalPlayer = engine->GetOriginalFunction<_GetLocalPlayer>(Offsets::GetLocalPlayer);
        GetViewAngles = engine->GetOriginalFunction<_GetViewAngles>(Offsets::GetViewAngles);
        SetViewAngles = engine->GetOriginalFunction<_GetViewAngles>(Offsets::SetViewAngles);
        GetGameDirectory = engine->GetOriginalFunction<_GetGameDirectory>(Offsets::GetGameDirectory);

        void* clPtr = nullptr;
        if (Game::IsPortal2Engine()) {
            typedef void* (*_GetClientState)();
            auto addr = Memory::ReadAbsoluteAddress((uintptr_t)ClientCmd + Offsets::GetClientStateFunction);
            auto GetClientState = reinterpret_cast<_GetClientState>(addr);
            clPtr = GetClientState();
        } else if (Game::IsHalfLife2Engine()) {
            auto ServerCmdKeyValues = engine->GetOriginalFunction<uintptr_t>(Offsets::ServerCmdKeyValues);
            clPtr = *reinterpret_cast<void**>(ServerCmdKeyValues + Offsets::cl);
        }

        CREATE_VMT(clPtr, cl) {
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
            DemoPlayer::Hook(**reinterpret_cast<void***>(disconnect + Offsets::demoplayer));
            DemoRecorder::Hook(**reinterpret_cast<void***>(disconnect + Offsets::demorecorder));

            auto IServerMessageHandler_VMT = *reinterpret_cast<uintptr_t*>((uintptr_t)cl->GetThisPtr() + IServerMessageHandler_VMT_Offset);
            auto ProcessTick = *reinterpret_cast<uintptr_t*>(IServerMessageHandler_VMT + sizeof(uintptr_t) * Offsets::ProcessTick);
            tickcount = *reinterpret_cast<int**>(ProcessTick + Offsets::tickcount);
            interval_per_tick = *reinterpret_cast<float**>(ProcessTick + Offsets::interval_per_tick);
            Speedrun::timer = new Speedrun::Timer(*Engine::interval_per_tick);

#ifdef _WIN32
            auto fru = SAR::Find("FrameUpdate");
            if (fru.Found) {
                MH_HOOK(HostStateFrame, fru.Address);
            }
#endif
        }
    }

    VMT tool;
    CREATE_VMT(Interfaces::IEngineTool, tool) {
        auto GetCurrentMap = tool->GetOriginalFunction<uintptr_t>(Offsets::GetCurrentMap);
        m_szLevelName = reinterpret_cast<char**>(GetCurrentMap + Offsets::m_szLevelName);
    }

    auto ldg = SAR::Find("m_bLoadgame");
    if (ldg.Found) {
        m_bLoadgame = *reinterpret_cast<bool**>(ldg.Address);
    }

    CREATE_VMT(Interfaces::IGameEventManager2, s_GameEventManager) {
        AddListener = s_GameEventManager->GetOriginalFunction<_AddListener>(Offsets::AddListener);
        RemoveListener = s_GameEventManager->GetOriginalFunction<_RemoveListener>(Offsets::RemoveListener);
    }
}
void Unhook()
{
    UNHOOK_O(cl, Offsets::Disconnect - 1);
    UNHOOK_O(cl, Offsets::Disconnect);
    UNHOOK_COMMAND(connect);
#ifdef _WIN32
    MH_UNHOOK(HostStateFrame);
#endif
    DELETE_VMT(engine);
    DELETE_VMT(cl);
    DELETE_VMT(s_GameEventManager);

    DemoPlayer::Unhook();
    DemoRecorder::Unhook();
}
}