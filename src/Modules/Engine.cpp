#include "Engine.hpp"

#include <cstring>

#include "Features/Cvars.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Console.hpp"
#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"
#include "Server.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

REDECL(Engine::Disconnect);
REDECL(Engine::Disconnect2);
REDECL(Engine::SetSignonState);
REDECL(Engine::SetSignonState2);
REDECL(Engine::Frame);
REDECL(Engine::plugin_load_callback);
REDECL(Engine::plugin_unload_callback);
REDECL(Engine::exit_callback);
REDECL(Engine::quit_callback);
REDECL(Engine::help_callback);
#ifdef _WIN32
REDECL(Engine::connect_callback);
REDECL(Engine::ParseSmoothingInfo_Skip);
REDECL(Engine::ParseSmoothingInfo_Default);
REDECL(Engine::ParseSmoothingInfo_Continue);
REDECL(Engine::ParseSmoothingInfo_Mid);
REDECL(Engine::ParseSmoothingInfo_Mid_Trampoline);
REDECL(Engine::ReadCustomData);
#endif

void Engine::ExecuteCommand(const char* cmd)
{
    this->ClientCmd(this->engineClient->ThisPtr(), cmd);
}
int Engine::GetSessionTick()
{
    auto result = *this->tickcount - session->baseTick;
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
edict_t* Engine::PEntityOfEntIndex(int iEntIndex)
{
    if (iEntIndex >= 0 && iEntIndex < server->gpGlobals->maxEntities) {
        auto pEdict = reinterpret_cast<edict_t*>((uintptr_t)server->gpGlobals->pEdicts + iEntIndex * sizeof(edict_t));
        if (!pEdict->IsFree()) {
            return pEdict;
        }
    }

    return nullptr;
}
QAngle Engine::GetAngles(int nSlot)
{
    auto va = QAngle();
    if (this->GetLocalClient) {
        auto client = this->GetLocalClient(nSlot);
        if (client) {
            va = *reinterpret_cast<QAngle*>((uintptr_t)client + Offsets::viewangles);
        }
    } else {
        this->GetViewAngles(this->engineClient->ThisPtr(), va);
    }
    return va;
}
void Engine::SetAngles(int nSlot, QAngle va)
{
    if (this->GetLocalClient) {
        auto client = this->GetLocalClient(nSlot);
        if (client) {
            auto viewangles = reinterpret_cast<QAngle*>((uintptr_t)client + Offsets::viewangles);
            viewangles->x = Math::AngleNormalize(va.x);
            viewangles->y = Math::AngleNormalize(va.y);
            viewangles->z = Math::AngleNormalize(va.z);
        }
    } else {
        this->SetViewAngles(this->engineClient->ThisPtr(), va);
    }
}
void Engine::SendToCommandBuffer(const char* text, int delay)
{
    if (sar.game->Is(SourceGame_Portal2Engine)) {
#ifdef _WIN32
        auto slot = this->GetActiveSplitScreenPlayerSlot();
#else
        auto slot = this->GetActiveSplitScreenPlayerSlot(nullptr);
#endif
        this->Cbuf_AddText(slot, text, delay);
    } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
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
    session->Ended();
    return Engine::Disconnect2(thisptr, unk1, unk2, unk3);
}
DETOUR_COMMAND(Engine::connect)
{
    session->Ended();
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
    speedrun->PreUpdate(engine->tickcount, engine->m_szLevelName);

    if (engine->hoststate->m_currentState != session->prevState) {
        session->Changed();
    }
    session->prevState = engine->hoststate->m_currentState;

    if (engine->hoststate->m_activeGame || std::strlen(engine->m_szLevelName) == 0) {
        speedrun->PostUpdate(engine->tickcount, engine->m_szLevelName);
    }

    return Engine::Frame(thisptr);
}

#ifdef _WIN32
// CDemoFile::ReadCustomData
void __fastcall ReadCustomData_Wrapper(int demoFile, int edx, int unk1, int unk2)
{
    Engine::ReadCustomData((void*)demoFile, 0, nullptr, nullptr);
}
// CDemoSmootherPanel::ParseSmoothingInfo
DETOUR_MID_MH(Engine::ParseSmoothingInfo_Mid)
{
    __asm {
        // Check if we have dem_customdata
        cmp eax, 8
        jne _orig

        // Parse stuff that does not get parsed (thanks valve)
        push edi
        push edi
        mov ecx, esi
        call ReadCustomData_Wrapper

        jmp Engine::ParseSmoothingInfo_Skip

_orig:  // Original overwritten instructions
        add eax, -3
        cmp eax, 6
        ja _def

        jmp Engine::ParseSmoothingInfo_Continue

_def:
        jmp Engine::ParseSmoothingInfo_Default
    }
}
#endif

DETOUR_COMMAND(Engine::plugin_load)
{
    // Prevent crash when trying to load SAR twice or try to find the module in
    // the plugin list if the initial search thread failed
    if (args.ArgC() >= 2) {
        auto file = std::string(args[1]);
        if (Utils::EndsWith(file, std::string(MODULE("sar"))) || Utils::EndsWith(file, std::string("sar"))) {
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
    if (args.ArgC() >= 2 && sar.GetPlugin() && std::atoi(args[1]) == sar.plugin->index) {
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
    this->s_ServerPlugin = Interface::Create(this->Name(), "ISERVERPLUGINHELPERS0", false);

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

        if (sar.game->Is(SourceGame_Portal2Engine)) {
            Memory::Read((uintptr_t)this->SetViewAngles + Offsets::GetLocalClient, &this->GetLocalClient);

            if (sar.game->Is(SourceGame_Portal2Game | SourceGame_INFRA)) {
                this->m_bWaitEnabled = reinterpret_cast<bool*>((uintptr_t)s_CommandBuffer + Offsets::m_bWaitEnabled);
                this->m_bWaitEnabled2 = reinterpret_cast<bool*>((uintptr_t)this->m_bWaitEnabled + Offsets::CCommandBufferSize);
            }

            if (auto g_VEngineServer = Interface::Create(this->Name(), "VEngineServer0", false)) {
                this->ClientCommand = g_VEngineServer->Original<_ClientCommand>(Offsets::ClientCommand);
                Interface::Delete(g_VEngineServer);
            }
        }

        void* clPtr = nullptr;
        if (sar.game->Is(SourceGame_Portal2Engine)) {
            typedef void* (*_GetClientState)();
            auto GetClientState = Memory::Read<_GetClientState>((uintptr_t)this->ClientCmd + Offsets::GetClientStateFunction);
            clPtr = GetClientState();

            this->GetActiveSplitScreenPlayerSlot = this->engineClient->Original<_GetActiveSplitScreenPlayerSlot>(Offsets::GetActiveSplitScreenPlayerSlot);
        } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
            auto ServerCmdKeyValues = this->engineClient->Original(Offsets::ServerCmdKeyValues);
            clPtr = Memory::Deref<void*>(ServerCmdKeyValues + Offsets::cl);

            Memory::Read<_AddText>((uintptr_t)this->Cbuf_AddText + Offsets::AddText, &this->AddText);
        }

        if (this->cl = Interface::Create(clPtr)) {
            if (!this->demoplayer)
                this->demoplayer = new EngineDemoPlayer();
            if (!this->demorecorder)
                this->demorecorder = new EngineDemoRecorder();

            if (sar.game->Is(SourceGame_Portal2Engine)) {
                this->cl->Hook(Engine::SetSignonState_Hook, Engine::SetSignonState, Offsets::Disconnect - 1);
                this->cl->Hook(Engine::Disconnect_Hook, Engine::Disconnect, Offsets::Disconnect);
            } else if (sar.game->Is(SourceGame_HalfLife2Engine)) {
                this->cl->Hook(Engine::SetSignonState2_Hook, Engine::SetSignonState2, Offsets::Disconnect - 1);
#ifdef _WIN32
                Command::Hook("connect", Engine::connect_callback_hook, Engine::connect_callback);
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
        }
    }

    if (auto tool = Interface::Create(this->Name(), "VENGINETOOL0", false)) {
        auto GetCurrentMap = tool->Original(Offsets::GetCurrentMap);
        this->m_szLevelName = Memory::Deref<char*>(GetCurrentMap + Offsets::m_szLevelName);
        this->m_bLoadgame = reinterpret_cast<bool*>((uintptr_t)this->m_szLevelName + Offsets::m_bLoadGame);
        Interface::Delete(tool);
    }

    if (auto s_EngineAPI = Interface::Create(this->Name(), "VENGINE_LAUNCHER_API_VERSION0", false)) {
        auto IsRunningSimulation = s_EngineAPI->Original(Offsets::IsRunningSimulation);
        auto engAddr = Memory::DerefDeref<void*>(IsRunningSimulation + Offsets::eng);

        if (this->eng = Interface::Create(engAddr)) {
            if (this->tickcount && this->hoststate && this->m_szLevelName) {
                this->eng->Hook(Engine::Frame_Hook, Engine::Frame, Offsets::Frame);
            }
        }
        Interface::Delete(s_EngineAPI);
    }

    if (sar.game->Is(SourceGame_Portal2 | SourceGame_ApertureTag)) {
        this->s_GameEventManager = Interface::Create(this->Name(), "GAMEEVENTSMANAGER002", false);
        if (this->s_GameEventManager) {
            this->AddListener = this->s_GameEventManager->Original<_AddListener>(Offsets::AddListener);
            this->RemoveListener = this->s_GameEventManager->Original<_RemoveListener>(Offsets::RemoveListener);

            auto FireEventClientSide = s_GameEventManager->Original(Offsets::FireEventClientSide);
            auto FireEventIntern = Memory::Read(FireEventClientSide + Offsets::FireEventIntern);
            Memory::Read<_ConPrintEvent>(FireEventIntern + Offsets::ConPrintEvent, &this->ConPrintEvent);
        }
    }

#ifdef _WIN32
    if (sar.game->Is(SourceGame_Portal2Game)) {
        auto parseSmoothingInfoAddr = Memory::Scan(this->Name(), "55 8B EC 0F 57 C0 81 EC ? ? ? ? B9 ? ? ? ? 8D 85 ? ? ? ? EB", 178);
        auto readCustomDataAddr = Memory::Scan(this->Name(), "55 8B EC F6 05 ? ? ? ? ? 53 56 57 8B F1 75 2F");

        console->DevMsg("CDemoSmootherPanel::ParseSmoothingInfo = %p\n", parseSmoothingInfoAddr);
        console->DevMsg("CDemoFile::ReadCustomData = %p\n", readCustomDataAddr);

        if (parseSmoothingInfoAddr && readCustomDataAddr) {
            MH_HOOK_MID(Engine::ParseSmoothingInfo_Mid, parseSmoothingInfoAddr);            // Hook switch-case
            Engine::ParseSmoothingInfo_Continue = parseSmoothingInfoAddr + 8;               // Back to original function
            Engine::ParseSmoothingInfo_Default = parseSmoothingInfoAddr + 133;              // Default case
            Engine::ParseSmoothingInfo_Skip = parseSmoothingInfoAddr - 29;                  // Continue loop
            Engine::ReadCustomData = reinterpret_cast<_ReadCustomData>(readCustomDataAddr); // Function that handles dem_customdata

            this->demoSmootherPatch = new Memory::Patch();
            unsigned char nop3[] = { 0x90, 0x90, 0x90 };
            this->demoSmootherPatch->Execute(parseSmoothingInfoAddr + 5, nop3);             // Nop rest
        }
    }
#endif

    if (auto debugoverlay = Interface::Create(this->Name(), "VDebugOverlay0", false)) {
        ScreenPosition = debugoverlay->Original<_ScreenPosition>(Offsets::ScreenPosition);
        Interface::Delete(debugoverlay);
    }

    Command::Hook("plugin_load", Engine::plugin_load_callback_hook, Engine::plugin_load_callback);
    Command::Hook("plugin_unload", Engine::plugin_unload_callback_hook, Engine::plugin_unload_callback);
    Command::Hook("exit", Engine::exit_callback_hook, Engine::exit_callback);
    Command::Hook("quit", Engine::quit_callback_hook, Engine::quit_callback);
    Command::Hook("help", Engine::help_callback_hook, Engine::help_callback);

    return this->hasLoaded = this->engineClient && this->s_ServerPlugin && this->demoplayer && this->demorecorder;
}
void Engine::Shutdown()
{
    Interface::Delete(this->engineClient);
    Interface::Delete(this->s_ServerPlugin);
    Interface::Delete(this->cl);
    Interface::Delete(this->eng);
    Interface::Delete(this->s_GameEventManager);

#ifdef _WIN32
    Command::Unhook("connect", Engine::connect_callback);

    MH_UNHOOK(Engine::ParseSmoothingInfo_Mid);

    if (this->demoSmootherPatch) {
        this->demoSmootherPatch->Restore();
    }
    SAFE_DELETE(this->demoSmootherPatch)
#endif
    Command::Unhook("plugin_load", Engine::plugin_load_callback);
    Command::Unhook("plugin_unload", Engine::plugin_unload_callback);
    Command::Unhook("exit", Engine::exit_callback);
    Command::Unhook("quit", Engine::quit_callback);
    Command::Unhook("help", Engine::help_callback);

    if (this->demoplayer) {
        this->demoplayer->Shutdown();
    }
    if (this->demorecorder) {
        this->demorecorder->Shutdown();
    }

    SAFE_DELETE(this->demoplayer)
    SAFE_DELETE(this->demorecorder)
}

Engine* engine;
