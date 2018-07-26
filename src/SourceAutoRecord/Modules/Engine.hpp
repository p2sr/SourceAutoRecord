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

#define IServerMessageHandler_VMT_Offset 8

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

_GetScreenSize GetScreenSize;
_ClientCmd ClientCmd;
_GetLocalPlayer GetLocalPlayer;
_GetViewAngles GetViewAngles;
_SetViewAngles SetViewAngles;
_AddListener AddListener;
_RemoveListener RemoveListener;

int* tickcount;
float* interval_per_tick;
char* m_szLevelName;

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
    IsInGame = true;
    CurrentFrame = 0;
}
void SessionEnded()
{
    if (!DemoPlayer::IsPlaying() && IsInGame) {
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

// CEngine::Frame
CHostState* hoststate;
HOSTSTATES prevState;
DETOUR(Frame)
{
    if (hoststate->m_currentState != prevState) {
        console->Print("hs->m_currentState = %i\n", hoststate->m_currentState);
        if (hoststate->m_currentState == HOSTSTATES::HS_CHANGE_LEVEL_SP) {
            SessionEnded();
        } else if (hoststate->m_currentState == HOSTSTATES::HS_RUN
            && !hoststate->m_activeGame
            && !DemoPlayer::IsPlaying()) {
            console->Print("Detected menu!\n");
            Session::Rebase(*tickcount);
            Speedrun::timer->Unpause(tickcount);
        }
    }
    prevState = hoststate->m_currentState;

    // Observe other game states here
    Speedrun::timer->Update(tickcount, m_bLoadgame, m_szLevelName);

    return Original::Frame(thisptr);
}

void SendToCommandBuffer(const char* text, int delay)
{
    using _Cbuf_AddText = void(__cdecl*)(int slot, const char* pText, int nTickDelay);
    auto Cbuf_AddText = Memory::ReadAbsoluteAddress<_Cbuf_AddText>((uintptr_t)ClientCmd + Offsets::Cbuf_AddText);

    if (Game::IsPortal2Engine()) {
        auto GetActiveSplitScreenPlayerSlot = engine->GetOriginalFunction<int (*)()>(Offsets::GetActiveSplitScreenPlayerSlot);
        Cbuf_AddText(GetActiveSplitScreenPlayerSlot(), text, delay);
    } else if (Game::IsHalfLife2Engine()) {
        using _AddText = void(__func*)(void* thisptr, const char* pText, int nTickDelay);
        auto AddText = Memory::ReadAbsoluteAddress<_AddText>((uintptr_t)Cbuf_AddText + Offsets::AddText);
        auto s_CommandBuffer = Memory::Deref<void*>((uintptr_t)Cbuf_AddText + Offsets::s_CommandBuffer);
        AddText(s_CommandBuffer, text, delay);
    }
}

// SAR has to unhook CEngine::Frame one tick before unloading the module or the game
// will crash 100% of the time because the hooked function gets called so often
#define SAFE_UNLOAD_TICK_DELAY 69
void SafeUnload(bool exit = false)
{
    if (SAR::PluginFound()) {
        auto unload = std::string("plugin_unload ") + std::to_string(plugin->index);

        // Everything gets deleted at this point
        ExecuteCommand("sar_exit");

        if (Game::IsPortal2Engine()) {
            SendToCommandBuffer(unload.c_str(), SAFE_UNLOAD_TICK_DELAY);
        } else {
        }
    } else {
        console->Warning("SAR: This should never happen :(\n");
    }

    if (exit)
        SendToCommandBuffer("exit", SAFE_UNLOAD_TICK_DELAY);
}

DETOUR_COMMAND(plugin_load)
{
    // Prevent crash when trying to load SAR twice or find the module in
    // the plugin list if the search thread failed before because of RC
    if (args.ArgC() >= 2) {
        auto file = std::string(args[1]);
        if (endsWith(file, std::string(MODULE("sar"))) || endsWith(file, std::string("sar"))) {
            if (plugin->found) {
                console->Warning("SAR: Cannot load the same plugin twice!\n");
            } else if (SAR::PluginFound()) {
                plugin->ptr->m_bDisable = true;
                console->PrintActive("SAR: Loaded SAR successfully!\n");
            } else {
                console->Warning("SAR: This should never happen :(\n");
            }
            return;
        }
    }

    return Original::plugin_load_callback(args);
}

DETOUR_COMMAND(plugin_unload)
{
    if (args.ArgC() >= 2) {
        if (SAR::PluginFound()) {
            if (atoi(args[1]) == plugin->index) {
                SafeUnload();
                return;
            }
        } else {
            console->Warning("SAR: This should never happen :(\n");
        }
    }

    return Original::plugin_unload_callback(args);
}

DETOUR_COMMAND(exit)
{
    SafeUnload(true);
}

DETOUR_COMMAND(quit)
{
    SafeUnload(true);
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
            DemoPlayer::Hook(**reinterpret_cast<void***>(disconnect + Offsets::demoplayer));
            DemoRecorder::Hook(**reinterpret_cast<void***>(disconnect + Offsets::demorecorder));

#if _WIN32
            auto IServerMessageHandler_VMT = *reinterpret_cast<uintptr_t*>((uintptr_t)cl->GetThisPtr() + IServerMessageHandler_VMT_Offset);
            auto ProcessTick = *reinterpret_cast<uintptr_t*>(IServerMessageHandler_VMT + sizeof(uintptr_t) * Offsets::ProcessTick);
#else
            auto ProcessTick = cl->GetOriginalFunction<uintptr_t>(Offsets::ProcessTick);
#endif
            tickcount = *reinterpret_cast<int**>(ProcessTick + Offsets::tickcount);
            interval_per_tick = *reinterpret_cast<float**>(ProcessTick + Offsets::interval_per_tick);
            Speedrun::timer->SetIntervalPerTick(interval_per_tick);

            auto hsf = SAR::Find("HostState_Frame");
            if (hsf.Found) {
                hoststate = *reinterpret_cast<CHostState**>(hsf.Address + Offsets::hoststate);
                auto FrameUpdate = Memory::ReadAbsoluteAddress(hsf.Address + Offsets::FrameUpdate);
#if _WIN32
                auto engAddr = **reinterpret_cast<void***>(FrameUpdate + Offsets::eng);
#else
                auto State_Shutdown = Memory::ReadAbsoluteAddress(FrameUpdate + Offsets::State_Shutdown);
                auto engAddr = **reinterpret_cast<void***>(State_Shutdown + Offsets::eng);
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
        m_szLevelName = *reinterpret_cast<char**>(GetCurrentMap + Offsets::m_szLevelName);
    }

    auto ldg = SAR::Find("m_bLoadgame");
    if (ldg.Found) {
        m_bLoadgame = *reinterpret_cast<bool**>(ldg.Address);
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
