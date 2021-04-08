#include "Engine.hpp"

#include <cstring>

#include "Features/Cvars.hpp"
#include "Features/ConditionalExec.hpp"
#include "Features/SegmentedTools.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/ZachStats.hpp"
#include "Features/Renderer.hpp"

#include "Client.hpp"
#include "Console.hpp"
#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Server.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "SAR.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#ifdef _WIN32
#include <Memoryapi.h>
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

Variable host_framerate;
Variable net_showmsg;
Variable sv_portal_players;
Variable fps_max;
Variable mat_norendering;

Variable sar_record_at("sar_record_at", "-1", -1, "Start recording a demo at the tick specified. Will use sar_record_at_demo_name.\n", 0);
Variable sar_record_at_demo_name("sar_record_at_demo_name", "chamber", "Name of the demo automatically recorded.\n", 0);
Variable sar_record_at_increment("sar_record_at_increment", "0", "Increment automatically the demo name.\n");

Variable sar_pause_at("sar_pause_at", "-1", -1, "Pause at the specified tick. -1 to deactivate it.\n");
Variable sar_pause_for("sar_pause_for", "0", 0, "Pause for this amount of ticks.\n");

REDECL(Engine::Disconnect);
REDECL(Engine::Disconnect2);
REDECL(Engine::SetSignonState);
REDECL(Engine::SetSignonState2);
REDECL(Engine::Frame);
REDECL(Engine::PurgeUnusedModels);
REDECL(Engine::OnGameOverlayActivated);
REDECL(Engine::OnGameOverlayActivatedBase);
REDECL(Engine::ReadCustomData);
REDECL(Engine::plugin_load_callback);
REDECL(Engine::plugin_unload_callback);
REDECL(Engine::exit_callback);
REDECL(Engine::quit_callback);
REDECL(Engine::help_callback);
REDECL(Engine::gameui_activate_callback);
REDECL(Engine::unpause_callback);
REDECL(Engine::playvideo_end_level_transition_callback);
REDECL(Engine::playvideo_exitcommand_nointerrupt_callback);
REDECL(Engine::load_callback);
#ifdef _WIN32
REDECL(Engine::connect_callback);
REDECL(Engine::ParseSmoothingInfo_Skip);
REDECL(Engine::ParseSmoothingInfo_Default);
REDECL(Engine::ParseSmoothingInfo_Continue);
REDECL(Engine::ParseSmoothingInfo_Mid);
REDECL(Engine::ParseSmoothingInfo_Mid_Trampoline);
#endif

void Engine::ExecuteCommand(const char* cmd, bool immediately)
{
    if (immediately) {
        this->ExecuteClientCmd(this->engineClient->ThisPtr(), cmd);
    } else {
        this->ClientCmd(this->engineClient->ThisPtr(), cmd);
    }
}
int Engine::GetTick()
{
    return (this->GetMaxClients() < 2) ? *this->tickcount : TIME_TO_TICKS(*this->net_time);
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
    RunExitExecs();

    // The exit command will handle everything
    this->ExecuteCommand("sar_exit");

    if (postCommand) {
        this->SendToCommandBuffer(postCommand, SAFE_UNLOAD_TICK_DELAY);
    }
}
bool Engine::isRunning()
{
    return engine->hoststate->m_activeGame && engine->hoststate->m_currentState == HOSTSTATES::HS_RUN;
}
bool Engine::IsGamePaused()
{
    return this->IsPaused(this->engineClient->ThisPtr());
}

int Engine::GetMapIndex(const std::string map)
{
    auto it = std::find(Game::mapNames.begin(), Game::mapNames.end(), map);
    if (it != Game::mapNames.end()) {
        return std::distance(Game::mapNames.begin(), it);
    } else {
        return -1;
    }
}

std::string Engine::GetCurrentMapName()
{
    if (engine->demoplayer->IsPlaying()) {
        return engine->demoplayer->GetLevelName();
    } else if (engine->IsOrange()) {
        return client->lastLevelName;
    } else {
        return engine->m_szLevelName;
    }
}

bool Engine::IsCoop()
{
    return sv_portal_players.GetInt() == 2;
}

bool Engine::IsOrange()
{
    return this->IsCoop() && session->signonState == SIGNONSTATE_FULL && !engine->hoststate->m_activeGame;
}

bool Engine::Trace(Vector& pos, QAngle& angle, float distMax, CTraceFilterSimple& filter, CGameTrace& tr)
{
    float X = DEG2RAD(angle.x), Y = DEG2RAD(angle.y);
    auto cosX = std::cos(X), cosY = std::cos(Y);
    auto sinX = std::sin(X), sinY = std::sin(Y);

    Vector dir(cosY * cosX, sinY * cosX, -sinX);

    Vector finalDir = Vector(dir.x, dir.y, dir.z).Normalize() * distMax;

    Ray_t ray;
    ray.m_IsRay = true;
    ray.m_IsSwept = true;
    ray.m_Start = VectorAligned(pos.x, pos.y, pos.z);
    ray.m_Delta = VectorAligned(finalDir.x, finalDir.y, finalDir.z);
    ray.m_StartOffset = VectorAligned();
    ray.m_Extents = VectorAligned();

    engine->TraceRay(this->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

    if (tr.fraction >= 1) {
        return false;
    }
    return true;
}

bool Engine::TraceFromCamera(float distMax, CGameTrace& tr)
{
    void* player = server->GetPlayer(GET_SLOT() + 1);

    if (player == nullptr || (int)player == -1)
        return false;

    Vector camPos = server->GetAbsOrigin(player) + server->GetViewOffset(player);
    QAngle angle = engine->GetAngles(GET_SLOT());

    CTraceFilterSimple filter;
    filter.SetPassEntity(server->GetPlayer(GET_SLOT() + 1));

    return this->Trace(camPos, angle, distMax, filter, tr);
}

void Engine::NewTick(const int tick)
{
    //sar_record

    if (sar_record_at.GetFloat() != -1 && !engine->hasRecorded && sar_record_at.GetFloat() == tick) {
        std::string cmd = std::string("record ") + sar_record_at_demo_name.GetString();
        engine->ExecuteCommand(cmd.c_str(), true);
        engine->hasRecorded = true;
    }

    //sar_pause

    if (sar_pause_at.GetInt() != -1 && !engine->demoplayer->IsPlaying()) {
        if (!engine->hasPaused && sar_pause_at.GetInt() == tick) {
            engine->ExecuteCommand("pause", true);
            engine->hasPaused = true;
            engine->isPausing = true;
            engine->pauseTick = server->tickCount;
        } else if (sar_pause_for.GetInt() > 0 && engine->isPausing && server->tickCount == sar_pause_for.GetInt() + engine->pauseTick) {
            engine->ExecuteCommand("unpause", true);
            engine->isPausing = false;
        }
    }

    if (segmentedTools->waitTick == tick && !engine->hasWaited) {
        if (!sv_cheats.GetBool()) {
            console->Print("\"wait\" needs sv_cheats 1.\n");
            engine->hasWaited = true;
        } else {
            engine->hasWaited = true;
            engine->ExecuteCommand(segmentedTools->pendingCommands.c_str(), true);
        }
    }

    networkManager.DispatchQueuedEvents();

    if (networkManager.isConnected && engine->isRunning()) {
        networkManager.UpdateGhostsPosition();

        if (networkManager.isCountdownReady) {
            networkManager.UpdateCountdown();
        }
    }

    if (demoGhostPlayer.IsPlaying() && engine->isRunning()) {
        demoGhostPlayer.UpdateGhostsPosition();
    }

    zachStats->UpdateTriggers();
}

bool Engine::ConsoleVisible()
{
    return this->Con_IsVisible(this->engineClient->ThisPtr());
}

float Engine::GetHostFrameTime()
{
    return this->HostFrameTime(this->engineTool->ThisPtr());
}

float Engine::GetClientTime()
{
    return this->ClientTime(this->engineTool->ThisPtr());
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
    if (engine->hoststate->m_currentState != session->prevState) {
        session->Changed();
    }
    session->prevState = engine->hoststate->m_currentState;


    if (engine->hoststate->m_activeGame || std::strlen(engine->m_szLevelName) == 0) {
        SpeedrunTimer::Update();
    }

    if ((engine->demoplayer->IsPlaying() || engine->IsOrange()) && engine->lastTick != session->GetTick()) {
        engine->NewTick(session->GetTick());
    }

    //demoplayer
    if (engine->demoplayer->demoQueueSize > 0 && !engine->demoplayer->IsPlaying()) {
        DemoParser parser;
        auto name = engine->demoplayer->demoQueue[engine->demoplayer->currentDemoID];
        engine->ExecuteCommand(std::string("playdemo " + name).c_str());
        if (++engine->demoplayer->currentDemoID >= engine->demoplayer->demoQueueSize) {
            engine->demoplayer->ClearDemoQueue();
        }
    }

    engine->lastTick = session->GetTick();

    Renderer::Frame();

    return Engine::Frame(thisptr);
}

DETOUR(Engine::PurgeUnusedModels)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto result = Engine::PurgeUnusedModels(thisptr);
    auto stop = std::chrono::high_resolution_clock::now();
    console->DevMsg("PurgeUnusedModels - %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
    return result;
}

DETOUR(Engine::ReadCustomData, int* callbackIndex, char** data)
{
    auto size = Engine::ReadCustomData(thisptr, callbackIndex, data);
    if (callbackIndex && data && *callbackIndex == 0 && size > 8) {
        engine->demoplayer->CustomDemoData(*data + 8, size - 8);
    }
    return size;
}

#ifdef _WIN32
// CDemoFile::ReadCustomData
void __fastcall ReadCustomData_Wrapper(int demoFile, int edx, int unk1, int unk2)
{
    Engine::ReadCustomData((void*)demoFile, nullptr, nullptr);
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

_orig: // Original overwritten instructions
        add eax, -3
        cmp eax, 6
        ja _def

        jmp Engine::ParseSmoothingInfo_Continue

_def:
        jmp Engine::ParseSmoothingInfo_Default
    }
}
#endif

// CSteam3Client::OnGameOverlayActivated
DETOUR_B(Engine::OnGameOverlayActivated, GameOverlayActivated_t* pGameOverlayActivated)
{
    engine->overlayActivated = pGameOverlayActivated->m_bActive;
    return Engine::OnGameOverlayActivatedBase(thisptr, pGameOverlayActivated);
}

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
DETOUR_COMMAND(Engine::gameui_activate)
{
    if (sar_disable_steam_pause.GetBool() && engine->overlayActivated) {
        return;
    }

    Engine::gameui_activate_callback(args);
}
DETOUR_COMMAND(Engine::playvideo_end_level_transition)
{
    if (engine->GetMaxClients() >= 2) {
        SpeedrunTimer::Pause();
    }

    Engine::playvideo_end_level_transition_callback(args);
}
DETOUR_COMMAND(Engine::playvideo_exitcommand_nointerrupt)
{
    if (engine->GetMaxClients() >= 2 && args.ArgC() == 4 && !strcmp(args[1], "dlc1_endmovie")) {
        console->Print("%d: course 6 end\n", session->GetTick());
        //course6End = true;
    } else if (engine->GetMaxClients() >= 2 && args.ArgC() == 4 && !strcmp(args[1], "coop_outro")) {
        console->Print("%d: course 5 end\n", session->GetTick());
        //course5End = true;
    }

    Engine::playvideo_exitcommand_nointerrupt_callback(args);
}
DETOUR_COMMAND(Engine::load)
{
    // Loading a save should bypass ghost_sync if there's no map
    // list for this game
    if (Game::mapNames.empty() && networkManager.isConnected) {
        networkManager.disableSyncForLoad = true;
    }
    Engine::load_callback(args);
}

DECL_CVAR_CALLBACK(ss_force_primary_fullscreen)
{
    if (engine->GetMaxClients() >= 2 && server->GetChallengeStatus() != CMStatus::CHALLENGE && ss_force_primary_fullscreen.GetInt() == 0) {
        ++engine->nForcePrimaryFullscreen;
        if (engine->nForcePrimaryFullscreen == 2) {
            SpeedrunTimer::Resume();
            SpeedrunTimer::OnLoad();
        }
    }
}

bool Engine::Init()
{
    this->engineClient = Interface::Create(this->Name(), "VEngineClient0", false);
    this->s_ServerPlugin = Interface::Create(this->Name(), "ISERVERPLUGINHELPERS0", false);

    if (this->engineClient) {
        this->GetScreenSize = this->engineClient->Original<_GetScreenSize>(Offsets::GetScreenSize);
        this->ClientCmd = this->engineClient->Original<_ClientCmd>(Offsets::ClientCmd);
        this->ExecuteClientCmd = this->engineClient->Original<_ExecuteClientCmd>(Offsets::ExecuteClientCmd);
        this->GetLocalPlayer = this->engineClient->Original<_GetLocalPlayer>(Offsets::GetLocalPlayer);
        this->GetViewAngles = this->engineClient->Original<_GetViewAngles>(Offsets::GetViewAngles);
        this->SetViewAngles = this->engineClient->Original<_SetViewAngles>(Offsets::SetViewAngles);
        this->GetMaxClients = this->engineClient->Original<_GetMaxClients>(Offsets::GetMaxClients);
        this->GetGameDirectory = this->engineClient->Original<_GetGameDirectory>(Offsets::GetGameDirectory);
        this->IsPaused = this->engineClient->Original<_IsPaused>(Offsets::IsPaused);
        this->Con_IsVisible = this->engineClient->Original<_Con_IsVisible>(Offsets::Con_IsVisible);

        Memory::Read<_Cbuf_AddText>((uintptr_t)this->ClientCmd + Offsets::Cbuf_AddText, &this->Cbuf_AddText);
        Memory::Deref<void*>((uintptr_t)this->Cbuf_AddText + Offsets::s_CommandBuffer, &this->s_CommandBuffer);

        if (sar.game->Is(SourceGame_Portal2Engine)) {
            Memory::Read((uintptr_t)this->SetViewAngles + Offsets::GetLocalClient, &this->GetLocalClient);

            if (sar.game->Is(SourceGame_Portal2Game | SourceGame_INFRA)) {
                this->m_bWaitEnabled = reinterpret_cast<bool*>((uintptr_t)s_CommandBuffer + Offsets::m_bWaitEnabled);
                this->m_bWaitEnabled2 = reinterpret_cast<bool*>((uintptr_t)this->m_bWaitEnabled + Offsets::CCommandBufferSize);
            }

            if (sar.game->Is(SourceGame_Portal2Game)) {
                auto GetSteamAPIContext = this->engineClient->Original<uintptr_t (*)()>(Offsets::GetSteamAPIContext);
                auto OnGameOverlayActivated = reinterpret_cast<_OnGameOverlayActivated*>(GetSteamAPIContext() + Offsets::OnGameOverlayActivated);

                Engine::OnGameOverlayActivatedBase = *OnGameOverlayActivated;
                *OnGameOverlayActivated = reinterpret_cast<_OnGameOverlayActivated>(Engine::OnGameOverlayActivated_Hook);
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
            SpeedrunTimer::SetIpt(*interval_per_tick);

            auto SetSignonState = this->cl->Original(Offsets::Disconnect - 1);
            auto HostState_OnClientConnected = Memory::Read(SetSignonState + Offsets::HostState_OnClientConnected);
            Memory::Deref<CHostState*>(HostState_OnClientConnected + Offsets::hoststate, &hoststate);
        }

        if (this->engineTrace = Interface::Create(this->Name(), "EngineTraceServer004")) {
            this->TraceRay = this->engineTrace->Original<_TraceRay>(Offsets::TraceRay);
        }
    }

    if (this->engineTool = Interface::Create(this->Name(), "VENGINETOOL0", false)) {
        auto GetCurrentMap = this->engineTool->Original(Offsets::GetCurrentMap);
        this->m_szLevelName = Memory::Deref<char*>(GetCurrentMap + Offsets::m_szLevelName);

        if (sar.game->Is(SourceGame_HalfLife2Engine) && std::strlen(this->m_szLevelName) != 0) {
            console->Warning("SAR: DO NOT load this plugin when the server is active!\n");
            return false;
        }

        this->m_bLoadgame = reinterpret_cast<bool*>((uintptr_t)this->m_szLevelName + Offsets::m_bLoadGame);

        this->HostFrameTime = this->engineTool->Original<_HostFrameTime>(Offsets::HostFrameTime);
        this->ClientTime = this->engineTool->Original<_ClientTime>(Offsets::ClientTime);

        this->PrecacheModel = this->engineTool->Original<_PrecacheModel>(Offsets::PrecacheModel);
    }

    if (auto s_EngineAPI = Interface::Create(this->Name(), "VENGINE_LAUNCHER_API_VERSION0", false)) {
        auto IsRunningSimulation = s_EngineAPI->Original(Offsets::IsRunningSimulation);
        auto engAddr = Memory::DerefDeref<void*>(IsRunningSimulation + Offsets::eng);

        if (this->eng = Interface::Create(engAddr)) {
            if (this->tickcount && this->hoststate && this->m_szLevelName) {
                this->eng->Hook(Engine::Frame_Hook, Engine::Frame, Offsets::Frame);
            }
        }

        uintptr_t Init = s_EngineAPI->Original(Offsets::Init);
        uintptr_t VideoMode_Create = Memory::Read(Init + Offsets::VideoMode_Create);
        void **videomode = *(void ***)(VideoMode_Create + Offsets::videomode);
        Renderer::Init(videomode);

        Interface::Delete(s_EngineAPI);
    }

    if (sar.game->Is(SourceGame_Portal2Game)) {
        this->s_GameEventManager = Interface::Create(this->Name(), "GAMEEVENTSMANAGER002", false);
        if (this->s_GameEventManager) {
            this->AddListener = this->s_GameEventManager->Original<_AddListener>(Offsets::AddListener);
            this->RemoveListener = this->s_GameEventManager->Original<_RemoveListener>(Offsets::RemoveListener);

            auto FireEventClientSide = s_GameEventManager->Original(Offsets::FireEventClientSide);
            auto FireEventIntern = Memory::Read(FireEventClientSide + Offsets::FireEventIntern);
            Memory::Read<_ConPrintEvent>(FireEventIntern + Offsets::ConPrintEvent, &this->ConPrintEvent);
        }
    }

    if (sar.game->Is(SourceGame_Portal2Game)) {
#ifdef _WIN32
        // Note: we don't get readCustomDataAddr anymore as we find this
        // below anyway
        auto parseSmoothingInfoAddr = Memory::Scan(this->Name(), "55 8B EC 0F 57 C0 81 EC ? ? ? ? B9 ? ? ? ? 8D 85 ? ? ? ? EB", 178);
        //auto readCustomDataAddr = Memory::Scan(this->Name(), "55 8B EC F6 05 ? ? ? ? ? 53 56 57 8B F1 75 2F");

        console->DevMsg("CDemoSmootherPanel::ParseSmoothingInfo = %p\n", parseSmoothingInfoAddr);
        //console->DevMsg("CDemoFile::ReadCustomData = %p\n", readCustomDataAddr);

        if (parseSmoothingInfoAddr) {
            MH_HOOK_MID(Engine::ParseSmoothingInfo_Mid, parseSmoothingInfoAddr); // Hook switch-case
            Engine::ParseSmoothingInfo_Continue = parseSmoothingInfoAddr + 8; // Back to original function
            Engine::ParseSmoothingInfo_Default = parseSmoothingInfoAddr + 133; // Default case
            Engine::ParseSmoothingInfo_Skip = parseSmoothingInfoAddr - 29; // Continue loop
            //Engine::ReadCustomData = reinterpret_cast<_ReadCustomData>(readCustomDataAddr); // Function that handles dem_customdata

            this->demoSmootherPatch = new Memory::Patch();
            unsigned char nop3[] = { 0x90, 0x90, 0x90 };
            this->demoSmootherPatch->Execute(parseSmoothingInfoAddr + 5, nop3); // Nop rest
        }
#endif

        // This is the address of the one interesting call to ReadCustomData - the E8 byte indicates the start of the call instruction
#ifdef _WIN32
        this->readPacketInjectAddr = Memory::Scan(this->Name(), "8D 45 E8 50 8D 4D BC 51 8D 4F 04 E8 ? ? ? ? 8B 4D BC 83 F9 FF", 12);
#else
        if (sar.game->Is(SourceGame_PortalStoriesMel)) {
            this->readPacketInjectAddr = Memory::Scan(this->Name(), "8B 95 94 FE FF FF 8D 4D D8 8D 45 D4 89 4C 24 08 89 44 24 04 89 14 24 E8", 24);
        } else {
            this->readPacketInjectAddr = Memory::Scan(this->Name(), "89 44 24 08 8D 85 B0 FE FF FF 89 44 24 04 8B 85 8C FE FF FF 89 04 24 E8", 24);
        }
#endif

        // Pesky memory protection doesn't want us overwriting code - we
        // get around it with a call to mprotect or VirtualProtect
        Memory::UnProtect((void*)this->readPacketInjectAddr, 4);

        // It's a relative call, so we have to do some weird fuckery lol
        Engine::ReadCustomData = reinterpret_cast<_ReadCustomData>(*(uint32_t*)this->readPacketInjectAddr + (this->readPacketInjectAddr + 4));
        *(uint32_t*)this->readPacketInjectAddr = (uint32_t)&ReadCustomData_Hook - (this->readPacketInjectAddr + 4); // Add 4 to get address of next instruction
    }

    if (auto debugoverlay = Interface::Create(this->Name(), "VDebugOverlay0", false)) {
        ScreenPosition = debugoverlay->Original<_ScreenPosition>(Offsets::ScreenPosition);
        AddBoxOverlay = debugoverlay->Original<_AddBoxOverlay>(Offsets::AddBoxOverlay);
        AddSphereOverlay = debugoverlay->Original<_AddSphereOverlay>(Offsets::AddSphereOverlay);
        AddTriangleOverlay = debugoverlay->Original<_AddTriangleOverlay>(Offsets::AddTriangleOverlay);
        AddLineOverlay = debugoverlay->Original<_AddLineOverlay>(Offsets::AddLineOverlay);
        AddScreenTextOverlay = debugoverlay->Original<_AddScreenTextOverlay>(Offsets::AddScreenTextOverlay);
        ClearAllOverlays = debugoverlay->Original<_ClearAllOverlays>(Offsets::ClearAllOverlays);
        Interface::Delete(debugoverlay);
    }

    Command::Hook("plugin_load", Engine::plugin_load_callback_hook, Engine::plugin_load_callback);
    Command::Hook("plugin_unload", Engine::plugin_unload_callback_hook, Engine::plugin_unload_callback);
    Command::Hook("exit", Engine::exit_callback_hook, Engine::exit_callback);
    Command::Hook("quit", Engine::quit_callback_hook, Engine::quit_callback);
    Command::Hook("help", Engine::help_callback_hook, Engine::help_callback);
    Command::Hook("load", Engine::load_callback_hook, Engine::load_callback);

    if (sar.game->Is(SourceGame_Portal2Game)) {
        Command::Hook("gameui_activate", Engine::gameui_activate_callback_hook, Engine::gameui_activate_callback);
        Command::Hook("playvideo_end_level_transition", Engine::playvideo_end_level_transition_callback_hook, Engine::playvideo_end_level_transition_callback);
        Command::Hook("playvideo_exitcommand_nointerrupt", Engine::playvideo_exitcommand_nointerrupt_callback_hook, Engine::playvideo_exitcommand_nointerrupt_callback);
        CVAR_HOOK_AND_CALLBACK(ss_force_primary_fullscreen);
    }

    host_framerate = Variable("host_framerate");
    net_showmsg = Variable("net_showmsg");
    sv_portal_players = Variable("sv_portal_players");
    fps_max = Variable("fps_max");
    mat_norendering = Variable("mat_norendering");

    // Dumb fix for valve cutting off convar descriptions at 80
    // characters for some reason
    char* s = (char*)Memory::Scan(this->Name(), "25 2d 38 30 73 20 2d 20 25 2e 38 30 73 0a 00"); // "%-80s - %.80s"
    if (s) {
        Memory::UnProtect(s, 11);
        strcpy(s, "%-80s - %s");
    }

    return this->hasLoaded = this->engineClient && this->s_ServerPlugin && this->demoplayer && this->demorecorder && this->engineTrace;
}
void Engine::Shutdown()
{
    if (this->engineClient && sar.game->Is(SourceGame_Portal2Game)) {
        auto GetSteamAPIContext = this->engineClient->Original<uintptr_t (*)()>(Offsets::GetSteamAPIContext);
        auto OnGameOverlayActivated = reinterpret_cast<_OnGameOverlayActivated*>(GetSteamAPIContext() + Offsets::OnGameOverlayActivated);
        *OnGameOverlayActivated = Engine::OnGameOverlayActivatedBase;
    }

    Renderer::Cleanup();

    Interface::Delete(this->engineClient);
    Interface::Delete(this->s_ServerPlugin);
    Interface::Delete(this->cl);
    Interface::Delete(this->eng);
    Interface::Delete(this->s_GameEventManager);
    Interface::Delete(this->engineTool);
    Interface::Delete(this->engineTrace);

    // Reset to the offsets that were originally in the code
#ifdef _WIN32
    *(uint32_t*)this->readPacketInjectAddr = 0x50E8458D;
#else
    *(uint32_t*)this->readPacketInjectAddr = 0x08244489;
#endif

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
    Command::Unhook("load", Engine::load_callback);
    Command::Unhook("gameui_activate", Engine::gameui_activate_callback);
    Command::Unhook("playvideo_end_level_transition", Engine::playvideo_end_level_transition_callback);
    Command::Unhook("playvideo_exitcommand_nointerrupt", Engine::playvideo_exitcommand_nointerrupt_callback);

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
