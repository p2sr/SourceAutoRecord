#include "Engine.hpp"

#include <stdarg.h>

#include "Console.hpp"
#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"

#include "Features/Cvars.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

REDECL(Engine::Disconnect)
REDECL(Engine::Disconnect2)
REDECL(Engine::SetSignonState)
REDECL(Engine::SetSignonState2)
REDECL(Engine::Frame)
REDECL(Engine::plugin_load_callback)
REDECL(Engine::plugin_unload_callback)
REDECL(Engine::exit_callback)
REDECL(Engine::quit_callback)
REDECL(Engine::help_callback)
#ifdef _WIN32
REDECL(Engine::connect)
#endif

void Engine::ExecuteCommand(const char* cmd)
{
    this->ClientCmd(this->engineClient->ThisPtr(), cmd);
}
void Engine::ClientCommand(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);
    this->ClientCmd(this->engineClient->ThisPtr(), data);
}
int Engine::GetSessionTick()
{
    int result = *this->tickcount - session->baseTick;
    return (result >= 0) ? result : 0;
}
float Engine::ToTime(int tick)
{
    return tick * *this->interval_per_tick;
}
int Engine::GetLocalPlayerIndex()
{
    return this->GetLocalPlayer(this->engineClient->ThisPtr());
}
QAngle Engine::GetAngles()
{
    auto va = QAngle();
    this->GetViewAngles(this->engineClient->ThisPtr(), va);
    return va;
}
void Engine::SetAngles(QAngle va)
{
    this->SetViewAngles(this->engineClient->ThisPtr(), va);
}
void Engine::SendToCommandBuffer(const char* text, int delay)
{
    if (sar.game->version & SourceGame_Portal2Engine) {
#ifdef _WIN32
        auto slot = this->GetActiveSplitScreenPlayerSlot();
#else
        auto slot = this->GetActiveSplitScreenPlayerSlot(nullptr);
#endif
        this->Cbuf_AddText(slot, text, delay);
    } else if (sar.game->version & SourceGame_HalfLife2Engine) {
        this->AddText(this->s_CommandBuffer, text, delay);
    }
}
int Engine::PointToScreen(const Vector& point, Vector& screen)
{
#ifdef _WIN32
    return this->ScreenPosition(point, screen);
#else
    return this->ScreenPosition(nullptr, point, screen);
#endif
}
void Engine::SafeUnload(const char* postCommand)
{
    // The exit command will handle everything
    this->ExecuteCommand("sar_exit");

    if (postCommand) {
        this->SendToCommandBuffer(postCommand, SAFE_UNLOAD_TICK_DELAY);
    }
}

// CClientState::Disconnect
DETOUR(Engine::Disconnect, bool bShowMainMenu)
{
    session->Ended();
    return Engine::Disconnect(thisptr, bShowMainMenu);
}
#ifdef _WIN32
DETOUR(Engine::Disconnect2, int unk1, int unk2, int unk3)
{
    engine->SessionEnded();
    return Engine::Disconnect2(thisptr, unk1, unk2, unk3);
}
DETOUR_COMMAND(Engine::connect)
{
    engine->SessionEnded();
    Engine::connect_callback(args);
}
#else
DETOUR(Engine::Disconnect2, int unk, bool bShowMainMenu)
{
    session->Ended();
    return Engine::Disconnect2(thisptr, unk, bShowMainMenu);
}
#endif

// CClientState::SetSignonState
DETOUR(Engine::SetSignonState, int state, int count, void* unk)
{
    session->Changed(state);
    return Engine::SetSignonState(thisptr, state, count, unk);
}
DETOUR(Engine::SetSignonState2, int state, int count)
{
    session->Changed(state);
    return Engine::SetSignonState2(thisptr, state, count);
}

// CEngine::Frame
DETOUR(Engine::Frame)
{
    if (engine->hoststate->m_currentState != session->prevState) {
        session->Changed();
    }
    session->prevState = engine->hoststate->m_currentState;

    if (engine->hoststate->m_activeGame) {
        speedrun->Update(engine->tickcount, engine->hoststate->m_levelName);
    }

    return Engine::Frame(thisptr);
}

DETOUR_COMMAND(Engine::plugin_load)
{
    // Prevent crash when trying to load SAR twice or try to find the module in
    // the plugin list if the initial search thread failed
    if (args.ArgC() >= 2) {
        auto file = std::string(args[1]);
        if (ends_with(file, std::string(MODULE("sar"))) || ends_with(file, std::string("sar"))) {
            if (sar.GetPlugin()) {
                sar.plugin->ptr->m_bDisable = true;
                console->PrintActive("SAR: Plugin fully loaded!\n");
            }
            return;
        }
    }

    Engine::plugin_load_callback(args);
}
DETOUR_COMMAND(Engine::plugin_unload)
{
    if (args.ArgC() >= 2 && sar.GetPlugin() && atoi(args[1]) == sar.plugin->index) {
        engine->SafeUnload();
    } else {
        engine->plugin_unload_callback(args);
    }
}
DETOUR_COMMAND(Engine::exit)
{
    engine->SafeUnload("exit");
}
DETOUR_COMMAND(Engine::quit)
{
    engine->SafeUnload("quit");
}
DETOUR_COMMAND(Engine::help)
{
    cvars->PrintHelp(args);
}

bool Engine::Init()
{
    this->engineClient = Interface::Create(this->Name(), "VEngineClient0", false);
    if (this->engineClient) {
        this->GetScreenSize = this->engineClient->Original<_GetScreenSize>(Offsets::GetScreenSize);
        this->ClientCmd = this->engineClient->Original<_ClientCmd>(Offsets::ClientCmd);
        this->GetLocalPlayer = this->engineClient->Original<_GetLocalPlayer>(Offsets::GetLocalPlayer);
        this->GetViewAngles = this->engineClient->Original<_GetViewAngles>(Offsets::GetViewAngles);
        this->SetViewAngles = this->engineClient->Original<_SetViewAngles>(Offsets::SetViewAngles);
        this->GetMaxClients = this->engineClient->Original<_GetMaxClients>(Offsets::GetMaxClients);
        this->GetGameDirectory = this->engineClient->Original<_GetGameDirectory>(Offsets::GetGameDirectory);

        Memory::Read<_Cbuf_AddText>((uintptr_t)this->ClientCmd + Offsets::Cbuf_AddText, &this->Cbuf_AddText);
        Memory::Deref<void*>((uintptr_t)this->Cbuf_AddText + Offsets::s_CommandBuffer, &this->s_CommandBuffer);
        if (sar.game->version & SourceGame_Portal2) {
            this->m_bWaitEnabled = reinterpret_cast<bool*>((uintptr_t)s_CommandBuffer + Offsets::m_bWaitEnabled);
        }

        void* clPtr = nullptr;
        if (sar.game->version & SourceGame_Portal2Engine) {
            typedef void* (*_GetClientState)();
            auto GetClientState = Memory::Read<_GetClientState>((uintptr_t)this->ClientCmd + Offsets::GetClientStateFunction);
            clPtr = GetClientState();

            this->GetActiveSplitScreenPlayerSlot = this->engineClient->Original<_GetActiveSplitScreenPlayerSlot>(Offsets::GetActiveSplitScreenPlayerSlot);
        } else if (sar.game->version & SourceGame_HalfLife2Engine) {
            auto ServerCmdKeyValues = this->engineClient->Original(Offsets::ServerCmdKeyValues);
            clPtr = Memory::Deref<void*>(ServerCmdKeyValues + Offsets::cl);

            Memory::Read<_AddText>((uintptr_t)this->Cbuf_AddText + Offsets::AddText, &this->AddText);
        }

        if (this->cl = Interface::Create(clPtr)) {
            if (!this->demoplayer)
                this->demoplayer = new EngineDemoPlayer();
            if (!this->demorecorder)
                this->demorecorder = new EngineDemoRecorder();

            if (sar.game->version & SourceGame_Portal2Engine) {
                this->cl->Hook(Engine::SetSignonState_Hook, Engine::SetSignonState, Offsets::Disconnect - 1);
                this->cl->Hook(Engine::Disconnect_Hook, Engine::Disconnect, Offsets::Disconnect);
            } else if (sar.game->version & SourceGame_HalfLife2Engine) {
                this->cl->Hook(Engine::SetSignonState2_Hook, Engine::SetSignonState2, Offsets::Disconnect - 1);
#ifdef _WIN32
                Command::Hook("connect", Engine::connect_callback_hook, &Engine::connect_callback);
#else
                this->cl->Hook(Engine::Disconnect2_Hook, Engine::Disconnect2, Offsets::Disconnect);
#endif
            }

#if _WIN32
            auto IServerMessageHandler_VMT = Memory::Deref<uintptr_t>((uintptr_t)this->cl->ThisPtr() + IServerMessageHandler_VMT_Offset);
            auto ProcessTick = Memory::Deref<uintptr_t>(IServerMessageHandler_VMT + sizeof(uintptr_t) * Offsets::ProcessTick);
#else
            auto ProcessTick = this->cl->Original(Offsets::ProcessTick);
#endif
            tickcount = Memory::Deref<int*>(ProcessTick + Offsets::tickcount);
            interval_per_tick = Memory::Deref<float*>(ProcessTick + Offsets::interval_per_tick);
            speedrun->SetIntervalPerTick(interval_per_tick);

            auto SetSignonState = this->cl->Original(Offsets::Disconnect - 1);
            auto HostState_OnClientConnected = Memory::Read(SetSignonState + Offsets::HostState_OnClientConnected);
            Memory::Deref<CHostState*>(HostState_OnClientConnected + Offsets::hoststate, &hoststate);

            if (auto s_EngineAPI = Interface::Create(MODULE("engine"), "VENGINE_LAUNCHER_API_VERSION0", false)) {
                auto IsRunningSimulation = s_EngineAPI->Original(Offsets::IsRunningSimulation);
                auto engAddr = Memory::DerefDeref<void*>(IsRunningSimulation + Offsets::eng);

                if (this->eng = Interface::Create(engAddr)) {
                    this->eng->Hook(Engine::Frame_Hook, Engine::Frame, Offsets::Frame);
                }
            }
        }
    }

    if (auto tool = Interface::Create(MODULE("engine"), "VENGINETOOL0", false)) {
        auto GetCurrentMap = tool->Original(Offsets::GetCurrentMap);
        this->m_szLevelName = Memory::Deref<char*>(GetCurrentMap + Offsets::m_szLevelName);
        this->m_bLoadgame = reinterpret_cast<bool*>((uintptr_t)this->m_szLevelName + Offsets::m_bLoadGame);
    }

    if (sar.game->version == SourceGame_Portal2) {
        this->s_GameEventManager = Interface::Create(MODULE("engine"), "GAMEEVENTSMANAGER002", false);
        if (this->s_GameEventManager) {
            this->AddListener = this->s_GameEventManager->Original<_AddListener>(Offsets::AddListener);
            this->RemoveListener = this->s_GameEventManager->Original<_RemoveListener>(Offsets::RemoveListener);

            auto FireEventClientSide = s_GameEventManager->Original(Offsets::FireEventClientSide);
            auto FireEventIntern = Memory::Read(FireEventClientSide + Offsets::FireEventIntern);
            Memory::Read<_ConPrintEvent>(FireEventIntern + Offsets::ConPrintEvent, &this->ConPrintEvent);
        }
    }

    Command::Hook("plugin_load", Engine::plugin_load_callback_hook, Engine::plugin_load_callback);
    Command::Hook("plugin_unload", Engine::plugin_unload_callback_hook, Engine::plugin_unload_callback);
    Command::Hook("exit", Engine::exit_callback_hook, Engine::exit_callback);
    Command::Hook("quit", Engine::quit_callback_hook, Engine::quit_callback);
    Command::Hook("help", Engine::help_callback_hook, Engine::help_callback);

    return this->hasLoaded = this->engineClient && this->demoplayer && this->demorecorder;
}
void Engine::Shutdown()
{
    Interface::Delete(this->engineClient);
    Interface::Delete(this->cl);
    Interface::Delete(this->eng);
    Interface::Delete(this->s_GameEventManager);

#ifdef _WIN32
    Command::Unhook("connect", Engine::connect_callback);
#endif
    Command::Unhook("plugin_load", Engine::plugin_load_callback);
    Command::Unhook("plugin_unload", Engine::plugin_unload_callback);
    Command::Unhook("exit", Engine::exit_callback);
    Command::Unhook("quit", Engine::quit_callback);
    Command::Unhook("help", Engine::help_callback);

    if (this->demoplayer) {
        this->demoplayer->Shutdown();
        delete this->demoplayer;
        this->demoplayer = nullptr;
    }
    if (this->demorecorder) {
        this->demorecorder->Shutdown();
        delete this->demorecorder;
        this->demorecorder = nullptr;
    }
}

Engine* engine;
