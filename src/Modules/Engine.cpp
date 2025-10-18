#include "Engine.hpp"

#include "Client.hpp"
#include "Console.hpp"
#include "EngineDemoPlayer.hpp"
#include "EngineDemoRecorder.hpp"
#include "Event.hpp"
#include "FileSystem.hpp"
#include "InputSystem.hpp"
#include "Features/AchievementTracker.hpp"
#include "Features/Camera.hpp"
#include "Features/Cvars.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Hud/PerformanceHud.hpp"
#include "Features/NetMessage.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Renderer.hpp"
#include "Features/SegmentedTools.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stitcher.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Game.hpp"
#include "Hook.hpp"
#include "Interface.hpp"
#include "SAR.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#include <cstring>
#include <cmath>

#define FPS_CHECK_WINDOW 0.5f

Variable host_timescale;
Variable host_framerate;
Variable net_showmsg;
Variable sv_portal_players;
Variable fps_max;
Variable mat_norendering;
Variable mat_filtertextures;
Variable phys_timescale;

Variable sar_record_at("sar_record_at", "-1", -1, "Start recording a demo at the tick specified. Will use sar_record_at_demo_name.\n", 0);
Variable sar_record_at_demo_name("sar_record_at_demo_name", "chamber", "Name of the demo automatically recorded.\n", 0);
Variable sar_record_at_increment("sar_record_at_increment", "0", "Increment automatically the demo name.\n");

Variable sar_pause_at("sar_pause_at", "-1", -1, "Pause at the specified tick. -1 to deactivate it.\n");
Variable sar_pause_for("sar_pause_for", "0", 0, "Pause for this amount of ticks.\n");

Variable sar_tick_debug("sar_tick_debug", "0", 0, 3, "Output debugging information to the console related to ticks and frames.\n");
Variable sar_frametime_debug("sar_frametime_debug", "0", "Output debugging information to the console related to frametime.\n"); // see also host_print_frame_times
Variable sar_frametime_uncap("sar_frametime_uncap", "0", "EXPERIMENTAL - USE AT OWN RISK. Removes the 10-1000 FPS cap on frametime. More info https://wiki.portal2.sr/Frametime\n");
Variable sar_command_debug("sar_command_debug", "0", 0, 2, "Output debugging information to the console related to commands. **Breaks svar_capture**\n");

Variable sar_cm_rightwarp("sar_cm_rightwarp", "0", "Fix CM wrongwarp.\n");

float g_cur_fps = 0.0f;

LoadState g_loadstate = LOADED;
int g_coop_pausable = -1;

REDECL(Engine::Disconnect);
REDECL(Engine::SetSignonState);
REDECL(Engine::ClientCommandKeyValues);
#ifndef _WIN32
REDECL(Engine::GetMouseDelta);
#endif
REDECL(Engine::Frame);
REDECL(Engine::PurgeUnusedModels);
REDECL(Engine::OnGameOverlayActivated);
REDECL(Engine::OnGameOverlayActivatedBase);
REDECL(Engine::ReadCustomData);
REDECL(Engine::ReadConsoleCommand);
REDECL(Engine::plugin_load_callback);
REDECL(Engine::plugin_unload_callback);
REDECL(Engine::exit_callback);
REDECL(Engine::quit_callback);
REDECL(Engine::restart_callback);
REDECL(Engine::help_callback);
REDECL(Engine::gameui_activate_callback);
REDECL(Engine::unpause_callback);
REDECL(Engine::playvideo_end_level_transition_callback);
REDECL(Engine::stop_transition_videos_fadeout_callback);
REDECL(Engine::load_callback);
REDECL(Engine::give_callback);
REDECL(Engine::exec_callback);
REDECL(Engine::changelevel_command_callback);
REDECL(Engine::changelevel2_command_callback);
#ifdef _WIN32
REDECL(Engine::ParseSmoothingInfo_Skip);
REDECL(Engine::ParseSmoothingInfo_Default);
REDECL(Engine::ParseSmoothingInfo_Continue);
REDECL(Engine::ParseSmoothingInfo_Mid);
REDECL(Engine::ParseSmoothingInfo_Mid_Trampoline);
#endif

void Engine::ExecuteCommand(const char *cmd, bool immediately) {
	this->SendToCommandBuffer(cmd, 0);
	if (immediately) {
		// HACKHACK: This effectively just does Cbuf_Execute, could we get at that by itself?
		this->ExecuteClientCmd(this->engineClient->ThisPtr(), "");
	}
}
float Engine::GetIPT() { // IntervalPerTick
	if (this->interval_per_tick) {
		return *this->interval_per_tick;
	}
	return 1.0f / sar.game->Tickrate();
}
int Engine::GetMaxClients() {
	if (this->GetMaxClientsOrig() == 0 && this->hoststate->m_activeGame) {
		return server->gpGlobals->maxClients;
	}
	return this->GetMaxClientsOrig();
}
int Engine::GetTick() {
	return (this->GetMaxClients() < 2 || engine->demoplayer->IsPlaying()) ? *this->tickcount : TIME_TO_TICKS(*this->net_time);
}
float Engine::ToTime(int tick) {
	return tick * this->GetIPT();
}
int Engine::GetLocalPlayerIndex() {
	return this->GetLocalPlayer(this->engineClient->ThisPtr());
}
edict_t *Engine::PEntityOfEntIndex(int iEntIndex) {
	if (iEntIndex >= 0 && iEntIndex < server->gpGlobals->maxEntities) {
		auto pEdict = reinterpret_cast<edict_t *>((uintptr_t)server->gpGlobals->pEdicts + iEntIndex * sizeof(edict_t));
		if (!pEdict->IsFree()) {
			return pEdict;
		}
	}

	return nullptr;
}
QAngle Engine::GetAngles(int nSlot) {
	auto va = QAngle();
	if (this->GetLocalClient) {
		auto client = this->GetLocalClient(nSlot);
		if (client) {
			va = *reinterpret_cast<QAngle *>((uintptr_t)client + Offsets::viewangles);
		}
	} else {
		this->GetViewAngles(this->engineClient->ThisPtr(), va);
	}
	return va;
}
void Engine::SetAngles(int nSlot, QAngle va) {
	if (this->GetLocalClient) {
		auto client = this->GetLocalClient(nSlot);
		if (client) {
			auto viewangles = reinterpret_cast<QAngle *>((uintptr_t)client + Offsets::viewangles);
			viewangles->x = Math::AngleNormalize(va.x);
			viewangles->y = Math::AngleNormalize(va.y);
			viewangles->z = Math::AngleNormalize(va.z);
		}
	} else {
		this->SetViewAngles(this->engineClient->ThisPtr(), va);
	}
}
void Engine::SendToCommandBuffer(const char *text, int delay) {
	auto slot = this->GetActiveSplitScreenPlayerSlot(nullptr);
	this->Cbuf_AddText(slot, text, delay);
}
int Engine::PointToScreen(const Vector &point, Vector &screen) {
	return this->ScreenPosition(nullptr, point, screen);
}
void Engine::SafeUnload(const char *postCommand) {

	// give events some time to execute before plugin is disabled
	Event::Trigger<Event::SAR_UNLOAD>({});
	this->ExecuteCommand("sar_exit");

	if (postCommand) {
		this->SendToCommandBuffer(postCommand, SAFE_UNLOAD_TICK_DELAY);
	}
}
bool Engine::isRunning() {
	auto hoststate_run = HS_RUN;
	if (sar.game->Is(SourceGame_INFRA)) hoststate_run = INFRA_HS_RUN;
	return engine->hoststate->m_activeGame && engine->hoststate->m_currentState == hoststate_run;
}
bool Engine::IsGamePaused() {
	return this->IsPaused(this->engineClient->ThisPtr());
}

int Engine::GetMapIndex(const std::string map) {
	std::string map_lower = map;
	std::transform(map_lower.begin(), map_lower.end(), map_lower.begin(), tolower);
	auto it = std::find(Game::mapNames.begin(), Game::mapNames.end(), map_lower);
	if (it != Game::mapNames.end()) {
		return std::distance(Game::mapNames.begin(), it);
	} else {
		return -1;
	}
}

std::string Engine::GetCurrentMapName() {
	static std::string last_map;

	std::string map = this->GetLevelNameShort(this->engineClient->ThisPtr());

	if (session->isRunning || map != "") {
		// Forward-ify all the slashes
		std::replace(map.begin(), map.end(), '\\', '/');

		last_map = map;
	}

	return last_map;
}

std::string Engine::GetMapTitle(std::string map) {
	std::string map_lower = map;
	std::transform(map_lower.begin(), map_lower.end(), map_lower.begin(), tolower);
	auto it = std::find_if(Game::maps.begin(), Game::maps.end(), [&map_lower](const MapData &data) {
		return data.fileName == map_lower;
	});
	if (it != Game::maps.end()) {
		return it->displayName;
	}
	return map;
}

std::string Engine::GetCurrentMapTitle() {
	return this->GetMapTitle(this->GetCurrentMapName());
}

bool Engine::IsCoop() {
	if (GetCurrentMapName().size() == 0) return false;
	if (client->gamerules && *client->gamerules) {
		using _IsMultiplayer = bool (__rescall *)(void *thisptr);
		return Memory::VMT<_IsMultiplayer>(*client->gamerules, Offsets::IsMultiplayer)(*client->gamerules);
	}
	return sv_portal_players.GetInt() == 2 || (engine->demoplayer->IsPlaying() && engine->GetMaxClients() >= 2);
}

bool Engine::IsOrange() {
	static bool isOrange;
	if (engine->demoplayer->IsPlaying()) {
		isOrange = GET_SLOT() == 1;
	} else if (session->signonState == SIGNONSTATE_FULL) {
		isOrange = this->IsCoop() && !engine->hoststate->m_activeGame && !engine->demoplayer->IsPlaying();
	}
	return isOrange;
}

bool Engine::IsSplitscreen() {
	if (!engine->IsCoop()) return false;

	for (int i = 0; i < 2; ++i) {
		ClientEnt *player = client->GetPlayer(i + 1);
		if (!player) continue;

		// m_hSplitScreenPlayers
		auto &splitscreen_players = player->fieldOff<CUtlVector<CBaseHandle>>("m_szLastPlaceName", 40);
		if (splitscreen_players.m_Size > 0) return true;
	}

	return false;
}

ON_EVENT(PRE_TICK) {
	if (!engine->demoplayer->IsPlaying()) {
		if (sar_pause_at.GetInt() == -1 || (!sv_cheats.GetBool() && sar_pause_at.GetInt() > 0)) {
			if (sar_pause_at.GetInt() != -1 && !engine->hasPaused) {
				console->Print("sar_pause_at values over 0 are only usable with sv_cheats\n");
			}
			engine->hasPaused = true;  // We don't want to randomly pause if the user sets sar_pause_at in this session
			engine->isPausing = false;
		} else {
			if (!engine->hasPaused && session->isRunning && event.tick >= sar_pause_at.GetInt()) {
				engine->ExecuteCommand("pause", true);
				engine->hasPaused = true;
				engine->isPausing = true;
				engine->pauseTick = server->tickCount;
			} else if (sar_pause_for.GetInt() > 0 && engine->isPausing && server->tickCount >= sar_pause_for.GetInt() + engine->pauseTick) {
				engine->ExecuteCommand("unpause", true);
				engine->isPausing = false;
			}
		}
	}
}

ON_EVENT(PRE_TICK) {
	if (engine->shouldPauseForSync && event.tick >= 0) {
		if (!engine->IsCoop() || (!engine->IsOrange() && g_orangeReady)) {
			if (engine->IsCoop()) {
				g_coop_pausable = Variable("sv_pausable").GetBool();
				engine->ExecuteCommand("stopvideos"); // loading animation goes over sync screen
				Variable("sv_pausable").SetValue("1");
			}
			engine->ExecuteCommand("pause", true);
			engine->shouldPauseForSync = false;
		}
	}
}

bool Engine::ConsoleVisible() {
	return this->Con_IsVisible(this->engineClient->ThisPtr());
}

float Engine::GetHostFrameTime() {
	return this->HostFrameTime(this->engineTool->ThisPtr());
}

float Engine::GetClientTime() {
	return this->ClientTime(this->engineTool->ThisPtr());
}

float Engine::GetHostTime() {
	return this->engineTool->Original<float (__rescall *)(void *thisptr)>(Offsets::HostTick - 1)(this->engineTool->ThisPtr());
}

// CClientState::Disconnect
DETOUR(Engine::Disconnect, bool bShowMainMenu) {
	session->Ended();
	return Engine::Disconnect(thisptr, bShowMainMenu);
}

// CClientState::SetSignonState
DETOUR(Engine::SetSignonState, int state, int count, void *unk) {
	if (sar_tick_debug.GetInt() >= 2) {
		int host, server, client;
		engine->GetTicks(host, server, client);
		console->Print("CClientState::SetSignonState %d (host=%d server=%d client=%d)\n", state, host, server, client);
	}
	auto ret = Engine::SetSignonState(thisptr, state, count, unk);
	session->Changed(state);
	return ret;
}

DETOUR_COMMAND(Engine::changelevel_command) {
	if (args.ArgC() >= 2) {
		std::string map = args[1];
		if (fileSystem->MapExists(map) && engine->GetCurrentMapName() != map) engine->isLevelTransition = true;
	}
	return Engine::changelevel_command_callback(args);
}
DETOUR_COMMAND(Engine::changelevel2_command) {
	if (args.ArgC() >= 2) {
		std::string map = args[1];
		if (fileSystem->MapExists(map) && engine->GetCurrentMapName() != map) engine->isLevelTransition = true;
	}
	return Engine::changelevel2_command_callback(args);
}

// CVEngineServer::ClientCommandKeyValues
DETOUR(Engine::ClientCommandKeyValues, void* pEdict, KeyValues* pKeyValues) {
	AchievementTracker::CheckKeyValuesForAchievement(pKeyValues);
	return Engine::ClientCommandKeyValues(thisptr, pEdict, pKeyValues);
}

#ifndef _WIN32
// CVEngineClient::GetMouseDelta
DETOUR_T(void, Engine::GetMouseDelta, int &x, int &y, bool ignore_next) {
	Engine::GetMouseDelta(thisptr, x, y, ignore_next);
	inputSystem->DPIScaleDeltas(x, y);
}
#endif

void Engine::GetTicks(int &host, int &server, int &client) {
	auto &et = this->engineTool;
	using _Fn = int(__rescall *)(void *thisptr);
	host = et->Original<_Fn>(Offsets::HostTick)(et->ThisPtr());
	server = et->Original<_Fn>(Offsets::ServerTick)(et->ThisPtr());
	client = et->Original<_Fn>(Offsets::ClientTick)(et->ThisPtr());
}

// CEngine::Frame
DETOUR(Engine::Frame) {
	auto startTime = NOW();
	if (sar_tick_debug.GetInt() >= 2) {
		static int lastServer, lastClient;
		int host, server, client;
		engine->GetTicks(host, server, client);
		if (server != lastServer || client != lastClient || sar_tick_debug.GetInt() >= 3) {
			console->Print("CEngine::Frame (host=%d server=%d client=%d)\n", host, server, client);
			lastServer = server;
			lastClient = client;
		}
	}

	if (engine->hoststate->m_currentState != session->prevState) {
		session->Changed();
	}
	session->prevState = engine->hoststate->m_currentState;

	if (engine->hoststate->m_activeGame || std::strlen(engine->m_szLevelName) == 0) {
		SpeedrunTimer::Update();
	}

	if ((engine->demoplayer->IsPlaying() || engine->IsOrange()) && engine->lastTick != session->GetTick()) {
		Event::Trigger<Event::PRE_TICK>({false, session->GetTick()});
		Event::Trigger<Event::POST_TICK>({false, session->GetTick()});
	}

	//demoplayer
	if (engine->demoplayer->demoQueueSize > 0 && !engine->demoplayer->IsPlaying() && engine->demoplayer->IsPlaybackFixReady()) {
		DemoParser parser;
		auto name = engine->demoplayer->demoQueue[engine->demoplayer->currentDemoID];
		engine->ExecuteCommand(Utils::ssprintf("playdemo \"%s\"", name.c_str()).c_str(), true);
		if (++engine->demoplayer->currentDemoID >= engine->demoplayer->demoQueueSize) {
			engine->demoplayer->ClearDemoQueue();
		}
	}

	engine->lastTick = session->GetTick();

	Renderer::Frame();
	engine->demoplayer->HandlePlaybackFix();
	Event::Trigger<Event::FRAME>({});

	if (!engine->IsForcingNoRendering() && session->isRunning) {
		Event::Trigger<Event::RENDER>({});
	}

	NetMessage::Update();

	// stopping TAS player if outside of the game
	if (!engine->hoststate->m_activeGame && tasPlayer->IsRunning()) {
		tasPlayer->Stop(true);
	}

	performanceHud->AddMetric(performanceHud->times_render, std::chrono::duration_cast<std::chrono::microseconds>(NOW() - startTime).count() / 1000000.0f);

	return Engine::Frame(thisptr);
}

DETOUR(Engine::PurgeUnusedModels) {
	auto start = std::chrono::high_resolution_clock::now();
	auto result = Engine::PurgeUnusedModels(thisptr);
	auto stop = std::chrono::high_resolution_clock::now();
	console->DevMsg("PurgeUnusedModels - %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
	return result;
}

Memory::Patch *g_ReadCustomDataPatch = nullptr;
Memory::Patch *g_ReadConsoleCommandPatch = nullptr;

DETOUR(Engine::ReadCustomData, int *callbackIndex, char **data) {
	auto size = Engine::ReadCustomData(thisptr, callbackIndex, data);
	if (callbackIndex && data && *callbackIndex == 0 && size > 8) {
		engine->demoplayer->CustomDemoData(*data + 8, size - 8);
	}
	return size;
}

DETOUR_T(const char *, Engine::ReadConsoleCommand) {
	const char *cmd = Engine::ReadConsoleCommand(thisptr);
	if (engine->demoplayer->ShouldBlacklistCommand(cmd)) {
		return "";
	}
	return cmd;
}

#ifdef _WIN32
// CDemoFile::ReadCustomData
void __fastcall ReadCustomData_Wrapper(int demoFile, int edx, int unk1, int unk2) {
	Engine::ReadCustomData((void *)demoFile, nullptr, nullptr);
}
// CDemoSmootherPanel::ParseSmoothingInfo
DETOUR_MID_MH(Engine::ParseSmoothingInfo_Mid) {
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

// CSteam3Client::OnGameOverlayActivated
DETOUR_B(Engine::OnGameOverlayActivated, GameOverlayActivated_t *pGameOverlayActivated) {
	engine->shouldSuppressPause = sar_disable_steam_pause.GetBool() && pGameOverlayActivated->m_bActive;
	return Engine::OnGameOverlayActivatedBase(thisptr, pGameOverlayActivated);
}

DETOUR_COMMAND(Engine::plugin_load) {
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
DETOUR_COMMAND(Engine::plugin_unload) {
	if (args.ArgC() >= 2 && sar.GetPlugin() && std::atoi(args[1]) == sar.plugin->index) {
		engine->SafeUnload();
	} else {
		engine->plugin_unload_callback(args);
	}
}
DETOUR_COMMAND(Engine::exit) {
	engine->SafeUnload("exit");
}
DETOUR_COMMAND(Engine::quit) {
	engine->SafeUnload("quit");
}
DETOUR_COMMAND(Engine::restart) {
	engine->SafeUnload("_restart");
}
DETOUR_COMMAND(Engine::help) {
	cvars->PrintHelp(args);
}
DETOUR_COMMAND(Engine::gameui_activate) {
	if (engine->shouldSuppressPause) {
		engine->shouldSuppressPause = false;
		return;
	}

	Engine::gameui_activate_callback(args);
}
DETOUR_COMMAND(Engine::playvideo_end_level_transition) {
	if (engine->GetMaxClients() >= 2 && !engine->IsOrange() && client->GetChallengeStatus() != CMStatus::CHALLENGE) {
		SpeedrunTimer::Pause();
	}

	if (engine->GetMaxClients() >= 2) {
		engine->isLevelTransition = true;
	}

	Engine::playvideo_end_level_transition_callback(args);
}
DETOUR_COMMAND(Engine::stop_transition_videos_fadeout) {
	engine->startedTransitionFadeout = true;
	Engine::stop_transition_videos_fadeout_callback(args);
}
DETOUR_COMMAND(Engine::load) {
	// Loading a save should bypass ghost_sync if there's no map
	// list for this game
	if (Game::mapNames.empty() && networkManager.isConnected) {
		networkManager.disableSyncForLoad = true;
	}
	if (sar_cm_rightwarp.GetBool() && sv_bonus_challenge.GetBool()) {
		sv_bonus_challenge.SetValue(false);
	}
	engine->tickLoadStarted = engine->GetTick();
	Engine::load_callback(args);
}
ON_EVENT(PRE_TICK) {
	if (engine->tickLoadStarted >= 0 && (engine->GetTick() < engine->tickLoadStarted || engine->GetTick() > engine->tickLoadStarted + 15)) {
		engine->tickLoadStarted = -1;
	}
}
DETOUR_COMMAND(Engine::give) {
	if (!sv_cheats.GetBool() && !strcasecmp(args[1], "challenge_mode_end_node")) {
		console->Print("This is cheating! If you really want to do it, set sv_cheats 1\n");
		return;
	}
	Engine::give_callback(args);
}
DETOUR_COMMAND(Engine::exec) {
	bool is_config_exec = (args.ArgC() == 3 && !strcmp(args[1], "config.cfg")) || (args.ArgC() == 2 && !strcmp(args[1], "config_default.cfg"));

	Engine::exec_callback(args);

	static bool has_execd_config = false;
	if (is_config_exec && !has_execd_config) {
		has_execd_config = true;
		Event::Trigger<Event::CONFIG_EXEC>({});
	}
}

DECL_CVAR_CALLBACK(ss_force_primary_fullscreen) {
	if (engine->GetMaxClients() >= 2 && client->GetChallengeStatus() != CMStatus::CHALLENGE && ss_force_primary_fullscreen.GetInt() == 0) {
		if (engine->startedTransitionFadeout && !engine->coopResumed && !engine->IsOrange()) {
			// if the game is not Portal Reloaded (see src/Features/ReloadedFix.cpp)
			if (sar.game->GetVersion() != SourceGame_PortalReloaded) {
				engine->coopResumed = true;
				SpeedrunTimer::Resume();
				SpeedrunTimer::OnLoad();
			}
		}
	}
}

static bool(__rescall *g_ProcessTick)(void *thisptr, void *pack);

#ifdef _WIN32
bool __fastcall ProcessTick_Detour(void *thisptr, void *unused, void *pack);
#else
bool ProcessTick_Detour(void *thisptr, void *pack);
#endif

static Hook ProcessTick_Hook(&ProcessTick_Detour);

#ifdef _WIN32
bool __fastcall ProcessTick_Detour(void *thisptr, void *unused, void *pack)
#else
bool ProcessTick_Detour(void *thisptr, void *pack)
#endif
{
	if (sar_tick_debug.GetInt() >= 1) {
		int host, server, client;
		engine->GetTicks(host, server, client);
		console->Print("NET_Tick %d (host=%d server=%d client=%d)\n", *(int *)((char *)pack + 16), host, server, client);
	}
	ProcessTick_Hook.Disable();
	bool ret = g_ProcessTick(thisptr, pack);
	ProcessTick_Hook.Enable();
	return ret;
}

static unsigned g_advance = 0;
static bool g_advancing = false;
static bool g_skipping = false;

void Engine::SetAdvancing(bool advancing) {
	g_advancing = advancing;
	if (!advancing) g_advance = 0;
}

bool Engine::IsAdvancing() {
	return g_advancing;
}

void Engine::AdvanceTick() {
	if (g_advancing) {
		if (!engine->IsCoop() && sv_alternateticks.GetBool()) {
			g_advance += 2;
		} else {
			g_advance += 1;
		}
	}
}

void Engine::SetSkipping(bool skipping) {
	g_skipping = skipping;
}

bool Engine::IsSkipping() {
	return g_skipping;
}

bool Engine::IsForcingNoRendering() {
	return g_skipping && !g_advancing && !engine->IsGamePaused();
}

static float *host_frametime;
static float *host_frametime_unbounded;
void Host_AccumulateTime_Detour(float dt);
void (*Host_AccumulateTime)(float dt);
static Hook Host_AccumulateTime_Hook(&Host_AccumulateTime_Detour);
void Host_AccumulateTime_Detour(float dt) {
	performanceHud->OnFrame(dt);
	if (!g_advancing || !session->isRunning) {
		Host_AccumulateTime_Hook.Disable();
		Host_AccumulateTime(dt);
		Host_AccumulateTime_Hook.Enable();
	} else if (g_advance > 0) {
		Host_AccumulateTime_Hook.Disable();
		Host_AccumulateTime(engine->GetIPT());
		Host_AccumulateTime_Hook.Enable();
		--g_advance;
	} else {
		*host_frametime = 0;
		*host_frametime_unbounded = 0;
	}

	// HACK: Force frametime to equal a tick while loading
	// Limits host_timescale effect on load times, faster loads
	if (g_loadstate == LOADING && sar_loads_uncap.GetBool()) {
		*host_frametime = *host_frametime_unbounded = engine->GetIPT();
	}

	if (*host_frametime != *host_frametime_unbounded) {
		if (sar_frametime_uncap.GetBool() && g_loadstate == LOADED) {
			if (sar_frametime_debug.GetBool()) console->Print("Host_AccumulateTime: %f (uncapped from %f)\n", *host_frametime_unbounded, *host_frametime);
			*host_frametime = *host_frametime_unbounded;
		} else {
			if (sar_frametime_debug.GetBool()) console->Print("Host_AccumulateTime: %f (capped to %f)\n", *host_frametime_unbounded, *host_frametime);
			if (engine->demorecorder->isRecordingDemo && g_loadstate == LOADED) {
				char data[5];
				data[0] = 0x0F;
				*(float *)(data + 1) = *host_frametime_unbounded;
				engine->demorecorder->RecordData(data, sizeof data);
			}
		}
	} else {
		if (sar_frametime_debug.GetBool()) console->Print("Host_AccumulateTime: %f\n", *host_frametime);
	}

	if (g_loadstate == LOAD_END) g_loadstate = LOADED;
}

const ConCommandBase *(*g_Cmd_ExecuteCommand)(int eTarget, const CCommand &command, int nClientSlot /* = -1 */);
const ConCommandBase *Cmd_ExecuteCommand_Detour(int eTarget, const CCommand &command, int nClientSlot /* = -1 */);
static Hook Cmd_ExecuteCommand_Hook(&Cmd_ExecuteCommand_Detour);
const ConCommandBase *Cmd_ExecuteCommand_Detour(int eTarget, const CCommand &command, int nClientSlot /* = -1 */) {
	if (!engine->demorecorder->isRecordingDemo) {
		auto cmd = reinterpret_cast<ConVar *>(tier1->FindCommandBase(tier1->g_pCVar->ThisPtr(), command.Arg(0)));
		if (cmd && !(cmd->m_nFlags & FCVAR_DONTRECORD)) {
			engine->demorecorder->queuedCommands.push_back(command.m_pArgSBuffer);
		}
	}
	if (sar_command_debug.GetInt() >= 1) {
		auto cmd = std::string(command.m_pArgSBuffer);
		cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
		cmd.erase(0, cmd.find_first_not_of(" \t"));
		console->Print("Cmd_ExecuteCommand (%s) target: %d slot: %d\n", cmd.c_str(), eTarget, nClientSlot);
	}
	if (command.ArgC() >= 1 && !strcmp(command.Arg(0), "restart_level") && 
	    engine->IsCoop() && !engine->IsOrange() &&
	    (g_partnerHasSAR && !g_orangeReady)) {
		return 0;
	}
	Cmd_ExecuteCommand_Hook.Disable();
	auto ret = g_Cmd_ExecuteCommand(eTarget, command, nClientSlot);
	Cmd_ExecuteCommand_Hook.Enable();
	return ret;
}

static bool(__rescall *g_InsertCommand)(void *thisptr, char *pArgS, int nCommandSize, int nTick);
#ifdef _WIN32
bool __fastcall InsertCommand_Detour(void *thisptr, void *unused, char *pArgS, int nCommandSize, int nTick);
#else
bool InsertCommand_Detour(void *thisptr, char *pArgS, int nCommandSize, int nTick);
#endif
static Hook InsertCommand_Hook(&InsertCommand_Detour);
#ifdef _WIN32
bool __fastcall InsertCommand_Detour(void *thisptr, void *unused, char *pArgS, int nCommandSize, int nTick) {
#else
bool InsertCommand_Detour(void *thisptr, char *pArgS, int nCommandSize, int nTick) {
#endif
	if (sar_command_debug.GetInt() >= 2) {
		auto cmd = std::string(pArgS);
		cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
		cmd.erase(0, cmd.find_first_not_of(" \t"));
		console->Print("InsertCommand      (%s) tick: %d\n", cmd.c_str(), nTick);
	}
	InsertCommand_Hook.Disable();
	bool ret = g_InsertCommand(thisptr, pArgS, nCommandSize, nTick);
	InsertCommand_Hook.Enable();
	return ret;
}

void _Host_RunFrame_Render_Detour();
void (*_Host_RunFrame_Render)();
static Hook _Host_RunFrame_Render_Hook(&_Host_RunFrame_Render_Detour);
void _Host_RunFrame_Render_Detour() {
	static uint64_t total_frames = 0;

	uint64_t init_frames = total_frames;
	total_frames += 1;

	unsigned nticks = roundf(FPS_CHECK_WINDOW / engine->GetIPT());
	Scheduler::InHostTicks(nticks, [=]() {
		uint64_t nframes = total_frames - init_frames;
		g_cur_fps = (float)nframes / FPS_CHECK_WINDOW;
	});
	if (engine->IsForcingNoRendering()) {
		// We need to do this or else the client doesn't update viewangles
		// in response to portal teleportations (and it probably breaks some
		// other stuff too). This would normally be done within
		// SCR_UpdateScreen, wrapping the main rendering calls
		client->ClFrameStageNotify(5); // FRAME_RENDER_START
		client->ClFrameStageNotify(6); // FRAME_RENDER_END
	} else {
		// Just do a normal render
		_Host_RunFrame_Render_Hook.Disable();
		_Host_RunFrame_Render();
		_Host_RunFrame_Render_Hook.Enable();
	}
}

static std::map<void *, float> g_bink_last_frames;
static bool g_bink_override_active = false;

ON_EVENT(SESSION_END) {
	// we have no better way of detecting this - just hope all videos die
	// on session end, else i guess we'll be skipping a frame
	g_bink_last_frames.clear();
}

Variable sar_bink_respect_host_time("sar_bink_respect_host_time", "1", "Make BINK video playback respect host time.\n");

ON_EVENT(FRAME) {
	if (!sar_bink_respect_host_time.GetBool()) {
		g_bink_override_active = false;
		g_bink_last_frames.clear();
		return;
	}

	// only do the bink overrides if host_timescale, host_framerate, or
	// frame advance is active - it's a very hacky patch, i don't trust it
	bool host_ts = sv_cheats.GetBool() && host_timescale.GetFloat() > 0.0f && host_timescale.GetFloat() != 1.0f;
	bool host_fr = (sv_cheats.GetBool() || engine->demoplayer->IsPlaying()) && host_framerate.GetFloat() != 0.0f;
	if (engine->IsAdvancing() || host_ts || host_fr) {
		g_bink_override_active = true;
	} else {
		g_bink_override_active = false;
		g_bink_last_frames.clear();
	}
}

static int framesToRun(void *bink) {
	// BINK datastructure in bink.h from SE2007 leak
	//uint32_t nframes = ((uint32_t *)bink)[2];
	//uint32_t last_frame = ((uint32_t *)bink)[4];
	double framerate = (double)((uint32_t *)bink)[5] / (double)((uint32_t *)bink)[6];

	double now = engine->GetHostTime();
	double last;

	auto it = g_bink_last_frames.find(bink);
	if (it == g_bink_last_frames.end()) {
		g_bink_last_frames[bink] = now;
		last = now;
	} else {
		last = it->second;
	}

	double to_run = (now - last) * framerate;
	//int possible = nframes - last_frame - 1;

	//if (to_run > possible) return possible;
	return (int)to_run;
}

static void advFrame(void *bink) {
	// BINK datastructure in bink.h from SE2007 leak
	//uint32_t nframes = ((uint32_t *)bink)[2];
	//uint32_t last_frame = ((uint32_t *)bink)[4];
	double framerate = (double)((uint32_t *)bink)[5] / (double)((uint32_t *)bink)[6];

	auto it = g_bink_last_frames.find(bink);
	if (it == g_bink_last_frames.end()) {
		g_bink_last_frames[bink] = engine->GetHostTime();
	} else {
		g_bink_last_frames[bink] += 1.0f / framerate;
	}
}

void (__stdcall *BinkNextFrame)(void *bink);
void __stdcall BinkNextFrame_Detour(void *bink);
static Hook BinkNextFrame_Hook(&BinkNextFrame_Detour);
void __stdcall BinkNextFrame_Detour(void *bink) {
	BinkNextFrame_Hook.Disable();
	BinkNextFrame(bink);
	BinkNextFrame_Hook.Enable();
	if (g_bink_override_active) advFrame(bink);
}

int (__stdcall *BinkShouldSkip)(void *bink);
int __stdcall BinkShouldSkip_Detour(void *bink);
static Hook BinkShouldSkip_Hook(&BinkShouldSkip_Detour);
int __stdcall BinkShouldSkip_Detour(void *bink) {
	if (g_bink_override_active) {
		return framesToRun(bink) > 1;
	} else {
		BinkShouldSkip_Hook.Disable();
		int ret = BinkShouldSkip(bink);
		BinkShouldSkip_Hook.Enable();
		return ret;
	}
}

int (__stdcall *BinkWait)(void *bink);
int __stdcall BinkWait_Detour(void *bink);
static Hook BinkWait_Hook(&BinkWait_Detour);
int __stdcall BinkWait_Detour(void *bink) {
	if (g_bink_override_active) {
		return framesToRun(bink) == 0;
	} else {
		BinkWait_Hook.Disable();
		int ret = BinkWait(bink);
		BinkWait_Hook.Enable();
		return ret;
	}
}

Color Engine::GetLightAtPoint(Vector point) {
#ifdef _WIN32
	// MSVC bug workaround - COM interfaces apparently don't quite follow
	// the behaviour implemented by '__thiscall' in some cases, and
	// returning structs is one of them! The compiler flips around the
	// first two args otherwise, with predictably disastrous results.
	Vector light;
	engine->GetLightForPoint(engine->engineClient->ThisPtr(), light, point, true);
#else
	Vector light = engine->GetLightForPoint(engine->engineClient->ThisPtr(), point, true);
#endif

	return Color{(uint8_t)(light.x * 255), (uint8_t)(light.y * 255), (uint8_t)(light.z * 255), 255};
}

bool Engine::GetPlayerInfo(int ent_num, player_info_t* pInfo)
{
	return this->GetInfo(this->engineClient->ThisPtr(), ent_num, pInfo);
}

std::string Engine::GetPartnerSteamID32() {
	player_info_t pInfo;

	if (IsOrange()) {
		GetPlayerInfo(1, &pInfo);
	} else {
		GetPlayerInfo(2, &pInfo);
	}
	return std::to_string(pInfo.friendsID);
}

static _CommandCompletionCallback playdemo_orig_completion;
DECL_COMMAND_FILE_COMPLETION(playdemo, ".dem", "", 1)

// FIXME: Includes "Portal 2/cfg". Bit of a conundrum because "Portal 2" is used
// for other commands
static _CommandCompletionCallback exec_orig_completion;
DECL_COMMAND_FILE_COMPLETION(exec, ".cfg", "cfg", 1)

bool Engine::Init() {
	this->engineClient = Interface::Create(this->Name(), "VEngineClient015");
	this->s_ServerPlugin = Interface::Create(this->Name(), "ISERVERPLUGINHELPERS001", false);

	if (this->engineClient) {
		this->GetScreenSize = this->engineClient->Original<_GetScreenSize>(Offsets::GetScreenSize);
		this->ClientCmd = this->engineClient->Original<_ClientCmd>(Offsets::ClientCmd);
		this->ExecuteClientCmd = this->engineClient->Original<_ExecuteClientCmd>(Offsets::ExecuteClientCmd);
		this->GetLocalPlayer = this->engineClient->Original<_GetLocalPlayer>(Offsets::GetLocalPlayer);
		this->GetViewAngles = this->engineClient->Original<_GetViewAngles>(Offsets::GetViewAngles);
		this->SetViewAngles = this->engineClient->Original<_SetViewAngles>(Offsets::SetViewAngles);
		this->GetMaxClientsOrig = this->engineClient->Original<_GetMaxClients>(Offsets::GetMaxClients);
		this->GetGameDirectory = this->engineClient->Original<_GetGameDirectory>(Offsets::GetGameDirectory);
		this->GetSaveDirName = this->engineClient->Original<_GetSaveDirName>(Offsets::GetSaveDirName);
		this->DebugDrawPhysCollide = this->engineClient->Original<_DebugDrawPhysCollide>(Offsets::DebugDrawPhysCollide);
		this->IsPaused = this->engineClient->Original<_IsPaused>(Offsets::IsPaused);
		this->Con_IsVisible = this->engineClient->Original<_Con_IsVisible>(Offsets::Con_IsVisible);
		this->GetLevelNameShort = this->engineClient->Original<_GetLevelNameShort>(Offsets::GetLevelNameShort);
		this->GetLightForPoint = this->engineClient->Original<_GetLightForPoint>(Offsets::GetLightForPoint);
		this->GetInfo = this->engineClient->Original<_GetPlayerInfo>(Offsets::GetPlayerInfo);

#ifndef _WIN32
		this->engineClient->Hook(Engine::GetMouseDelta_Hook, Engine::GetMouseDelta, Offsets::GetMouseDelta);
#endif

		Memory::Read<_Cbuf_AddText>((uintptr_t)this->ClientCmd + Offsets::Cbuf_AddText, &this->Cbuf_AddText);
		Memory::Deref<void *>((uintptr_t)this->Cbuf_AddText + Offsets::s_CommandBuffer, &this->s_CommandBuffer);

		Memory::Read((uintptr_t)this->SetViewAngles + Offsets::GetLocalClient, &this->GetLocalClient);

		this->m_bWaitEnabled = reinterpret_cast<bool *>((uintptr_t)s_CommandBuffer + Offsets::m_bWaitEnabled);
		this->m_bWaitEnabled2 = reinterpret_cast<bool *>((uintptr_t)this->m_bWaitEnabled + Offsets::CCommandBufferSize);

		auto GetSteamAPIContext = this->engineClient->Original<uintptr_t (*)()>(Offsets::GetSteamAPIContext);
		auto OnGameOverlayActivated = reinterpret_cast<_OnGameOverlayActivated *>(GetSteamAPIContext() + Offsets::OnGameOverlayActivated);

		Engine::OnGameOverlayActivatedBase = *OnGameOverlayActivated;
		*OnGameOverlayActivated = reinterpret_cast<_OnGameOverlayActivated>(Engine::OnGameOverlayActivated_Hook);

		if (this->g_VEngineServer = Interface::Create(this->Name(), "VEngineServer022")) {
			this->g_VEngineServer->Hook(Engine::ClientCommandKeyValues_Hook, Engine::ClientCommandKeyValues, Offsets::ClientCommandKeyValues);
			this->ClientCommand = this->g_VEngineServer->Original<_ClientCommand>(Offsets::ClientCommand);
			this->IsServerPaused = this->g_VEngineServer->Original<_IsServerPaused>(Offsets::IsServerPaused);
			this->ServerPause = this->g_VEngineServer->Original<_ServerPause>(Offsets::ServerPause);
		}

		typedef void *(*_GetClientState)();
		auto GetClientState = Memory::Read<_GetClientState>((uintptr_t)this->ClientCmd + Offsets::GetClientStateFunction);
		void *clPtr = GetClientState();

		this->GetActiveSplitScreenPlayerSlot = this->engineClient->Original<_GetActiveSplitScreenPlayerSlot>(Offsets::GetActiveSplitScreenPlayerSlot);

		if (this->cl = Interface::Create(clPtr)) {
			if (!this->demoplayer)
				this->demoplayer = new EngineDemoPlayer();
			if (!this->demorecorder)
				this->demorecorder = new EngineDemoRecorder();

			this->cl->Hook(Engine::SetSignonState_Hook, Engine::SetSignonState, Offsets::Disconnect - 1);
			this->cl->Hook(Engine::Disconnect_Hook, Engine::Disconnect, Offsets::Disconnect);
#if _WIN32
			auto IServerMessageHandler_VMT = Memory::Deref<uintptr_t>((uintptr_t)this->cl->ThisPtr() + Offsets::IServerMessageHandler);
			auto ProcessTick = Memory::Deref<uintptr_t>(IServerMessageHandler_VMT + sizeof(uintptr_t) * Offsets::ProcessTick);
#else
			auto ProcessTick = this->cl->Original(Offsets::ProcessTick);
#endif

			g_ProcessTick = (decltype(g_ProcessTick))ProcessTick;
			ProcessTick_Hook.SetFunc(ProcessTick);

			tickcount = Memory::Deref<int *>(ProcessTick + Offsets::tickcount);

			interval_per_tick = Memory::Deref<float *>(ProcessTick + Offsets::interval_per_tick);

			auto SetSignonState = this->cl->Original(Offsets::Disconnect - 1);
			auto HostState_OnClientConnected = Memory::Read(SetSignonState + Offsets::HostState_OnClientConnected);
			Memory::Deref<CHostState *>(HostState_OnClientConnected + Offsets::hoststate, &hoststate);
		}

		if (this->engineTrace = Interface::Create(this->Name(), "EngineTraceServer004")) {
			this->TraceRay = this->engineTrace->Original<_TraceRay>(Offsets::TraceRay);
			this->PointOutsideWorld = this->engineTrace->Original<_PointOutsideWorld>(Offsets::TraceRay + 14);
		}

		if (this->engineTraceClient = Interface::Create(this->Name(), "EngineTraceClient004")) {
			this->TraceRayClient = this->engineTraceClient->Original<_TraceRay>(Offsets::TraceRay);
		}
	}

	if (this->engineTool = Interface::Create(this->Name(), "VENGINETOOL003", false)) {
		auto GetCurrentMap = this->engineTool->Original(Offsets::GetCurrentMap);
		this->m_szLevelName = Memory::Deref<char *>(GetCurrentMap + Offsets::m_szLevelName);

		this->m_bLoadgame = reinterpret_cast<bool *>((uintptr_t)this->m_szLevelName + Offsets::m_bLoadGame);

		this->HostFrameTime = this->engineTool->Original<_HostFrameTime>(Offsets::HostFrameTime);
		this->ClientTime = this->engineTool->Original<_ClientTime>(Offsets::ClientTime);

		this->PrecacheModel = this->engineTool->Original<_PrecacheModel>(Offsets::PrecacheModel);
	}

	if (auto s_EngineAPI = Interface::Create(this->Name(), "VENGINE_LAUNCHER_API_VERSION004", false)) {
		auto IsRunningSimulation = s_EngineAPI->Original(Offsets::IsRunningSimulation);
		auto engAddr = Memory::DerefDeref<void *>(IsRunningSimulation + Offsets::eng);

		if (this->eng = Interface::Create(engAddr)) {
			if (this->tickcount && this->hoststate && this->m_szLevelName) {
				this->eng->Hook(Engine::Frame_Hook, Engine::Frame, Offsets::Frame);
			}
		}

		uintptr_t Init = s_EngineAPI->Original(Offsets::Init);
		uintptr_t VideoMode_Create = Memory::Read(Init + Offsets::VideoMode_Create);
		auto videomode = Memory::Deref<void **>(VideoMode_Create + Offsets::videomode);
		Renderer::Init(videomode);
		Stitcher::Init(videomode);

		Interface::Delete(s_EngineAPI);
	}

	this->s_GameEventManager = Interface::Create(this->Name(), "GAMEEVENTSMANAGER002", false);
	if (this->s_GameEventManager) {
		this->AddListener = this->s_GameEventManager->Original<_AddListener>(Offsets::AddListener);
		this->RemoveListener = this->s_GameEventManager->Original<_RemoveListener>(Offsets::RemoveListener);

		auto FireEventClientSide = s_GameEventManager->Original(Offsets::FireEventClientSide);
		auto FireEventIntern = Memory::Read(FireEventClientSide + Offsets::FireEventIntern);
		Memory::Read<_ConPrintEvent>(FireEventIntern + Offsets::ConPrintEvent, &this->ConPrintEvent);
	}

#ifdef _WIN32
	// Note: we don't get readCustomDataAddr anymore as we find this
	// below anyway
	auto ParseSmoothingInfo = Memory::Scan(this->Name(), Offsets::ParseSmoothingInfoSig, Offsets::ParseSmoothingInfoOff);
	console->DevMsg("CDemoSmootherPanel::ParseSmoothingInfo = %p\n", ParseSmoothingInfo);

	if (ParseSmoothingInfo) {
		MH_HOOK_MID(Engine::ParseSmoothingInfo_Mid, ParseSmoothingInfo);  // Hook switch-case
		Engine::ParseSmoothingInfo_Continue = ParseSmoothingInfo + 8;     // Back to original function
		Engine::ParseSmoothingInfo_Default = ParseSmoothingInfo + 133;    // Default case
		Engine::ParseSmoothingInfo_Skip = ParseSmoothingInfo - 29;        // Continue loop

		this->demoSmootherPatch = new Memory::Patch();
		unsigned char nop3[] = {0x90, 0x90, 0x90};
		this->demoSmootherPatch->Execute(ParseSmoothingInfo + 5, nop3);  // Nop rest
	}
#endif

	Host_AccumulateTime = (void (*)(float))Memory::Scan(this->Name(), Offsets::Host_AccumulateTime);
	if (Host_AccumulateTime) {
		host_frametime = Memory::Deref<float *>((uintptr_t)Host_AccumulateTime + Offsets::host_frametime);
		host_frametime_unbounded = host_frametime + Offsets::host_frametime_unbounded;
	}

	Host_AccumulateTime_Hook.SetFunc(Host_AccumulateTime);

	_Host_RunFrame_Render = (void (*)())Memory::Scan(this->Name(), Offsets::_Host_RunFrame_Render);

	_Host_RunFrame_Render_Hook.SetFunc(_Host_RunFrame_Render);


	g_Cmd_ExecuteCommand = (decltype(g_Cmd_ExecuteCommand))Memory::Scan(this->Name(), Offsets::Cmd_ExecuteCommand);
	g_InsertCommand = (decltype(g_InsertCommand))Memory::Scan(this->Name(), Offsets::InsertCommand);

	Cmd_ExecuteCommand_Hook.SetFunc(g_Cmd_ExecuteCommand);
	InsertCommand_Hook.SetFunc(g_InsertCommand);

	g_ReadCustomDataPatch = new Memory::Patch();
	auto readCustomDataInjectAddr = Memory::Scan(this->Name(), Offsets::readCustomDataInjectSig, Offsets::readCustomDataInjectOff);
	if (readCustomDataInjectAddr) {
		Engine::ReadCustomData = (_ReadCustomData)Memory::Read(readCustomDataInjectAddr);
		auto ReadCustomDataInject = (uint32_t)&ReadCustomData_Hook - (readCustomDataInjectAddr + 4);
		g_ReadCustomDataPatch->Execute(readCustomDataInjectAddr, (unsigned char *)&ReadCustomDataInject, 4);
	}

	g_ReadConsoleCommandPatch = new Memory::Patch();
	auto readConsoleCommandInjectAddr = Memory::Scan(this->Name(), Offsets::readConsoleCommandInjectSig, Offsets::readConsoleCommandInjectOff);
	if (readConsoleCommandInjectAddr) {
		Engine::ReadConsoleCommand = (_ReadConsoleCommand)Memory::Read(readConsoleCommandInjectAddr);
		auto ReadConsoleCommandInject = (uint32_t)&ReadConsoleCommand_Hook - (readConsoleCommandInjectAddr + 4);
		g_ReadConsoleCommandPatch->Execute(readConsoleCommandInjectAddr, (unsigned char *)&ReadConsoleCommandInject, 4);
	}

	if (auto debugoverlay = Interface::Create(this->Name(), "VDebugOverlay004", false)) {
		ScreenPosition = debugoverlay->Original<_ScreenPosition>(Offsets::ScreenPosition);
		Interface::Delete(debugoverlay);
	}

	Command::Hook("plugin_load", Engine::plugin_load_callback_hook, Engine::plugin_load_callback);
	Command::Hook("plugin_unload", Engine::plugin_unload_callback_hook, Engine::plugin_unload_callback);
	Command::Hook("exit", Engine::exit_callback_hook, Engine::exit_callback);
	Command::Hook("quit", Engine::quit_callback_hook, Engine::quit_callback);
	Command::Hook("_restart", Engine::restart_callback_hook, Engine::restart_callback);
	Command::Hook("help", Engine::help_callback_hook, Engine::help_callback);
	Command::Hook("load", Engine::load_callback_hook, Engine::load_callback);
	Command::Hook("give", Engine::give_callback_hook, Engine::give_callback);
	Command::Hook("exec", Engine::exec_callback_hook, Engine::exec_callback);
	Command::HookCompletion("playdemo", AUTOCOMPLETION_FUNCTION(playdemo), playdemo_orig_completion);
	Command::HookCompletion("exec", AUTOCOMPLETION_FUNCTION(exec), exec_orig_completion);

	Command::Hook("gameui_activate", Engine::gameui_activate_callback_hook, Engine::gameui_activate_callback);
	Command::Hook("playvideo_end_level_transition", Engine::playvideo_end_level_transition_callback_hook, Engine::playvideo_end_level_transition_callback);
	Command::Hook("stop_transition_videos_fadeout", Engine::stop_transition_videos_fadeout_callback_hook, Engine::stop_transition_videos_fadeout_callback);
	Command::Hook("changelevel", Engine::changelevel_command_callback_hook, Engine::changelevel_command_callback);
	Command::Hook("changelevel2", Engine::changelevel2_command_callback_hook, Engine::changelevel2_command_callback);
	CVAR_HOOK_AND_CALLBACK(ss_force_primary_fullscreen);

	host_timescale = Variable("host_timescale");
	host_framerate = Variable("host_framerate");
	net_showmsg = Variable("net_showmsg");
	sv_portal_players = Variable("sv_portal_players");
	fps_max = Variable("fps_max");
	mat_norendering = Variable("mat_norendering");
	mat_filtertextures = Variable("mat_filtertextures");
	phys_timescale = Variable("phys_timescale"); 

	// Dumb fix for valve cutting off convar descriptions at 80
	// characters for some reason
	/* TODO Memory::Scan data segments
	char *s = (char *)Memory::Scan(this->Name(), Offsets::Convar_PrintDescription);  // "%-80s - %.80s"
	if (s) {
		Memory::UnProtect(s, 11);
		strcpy(s, "%-80s - %s");
	}*/

	if (this->g_physCollision = Interface::Create(MODULE("vphysics"), "VPhysicsCollision007")) {
		this->CreateDebugMesh = this->g_physCollision->Original<_CreateDebugMesh>(Offsets::CreateDebugMesh);
		this->DestroyDebugMesh = this->g_physCollision->Original<_DestroyDebugMesh>(Offsets::DestroyDebugMesh);
	}

#ifdef _WIN32
	auto bink_mod = Memory::GetModuleHandleByName(MODULE("binkw32"));
#else
	auto bink_mod = Memory::GetModuleHandleByName(MODULE("valve_avi"));
#endif
	if (bink_mod) {
		BinkNextFrame = Memory::GetSymbolAddress<void (__stdcall *)(void *bink)>(bink_mod, STDCALL_NAME("BinkNextFrame", 4));
		BinkNextFrame_Hook.SetFunc(BinkNextFrame);
		BinkShouldSkip = Memory::GetSymbolAddress<int (__stdcall *)(void *bink)>(bink_mod, STDCALL_NAME("BinkShouldSkip", 4));
		BinkShouldSkip_Hook.SetFunc(BinkShouldSkip);
		BinkWait = Memory::GetSymbolAddress<int (__stdcall *)(void *bink)>(bink_mod, STDCALL_NAME("BinkWait", 4));
		BinkWait_Hook.SetFunc(BinkWait);
		Memory::CloseModuleHandle(bink_mod);
	}

	return this->hasLoaded = this->engineClient && this->s_ServerPlugin && this->demoplayer && this->demorecorder && this->engineTrace && this->engineTraceClient;
}
void Engine::Shutdown() {
	if (this->engineClient) {
		auto GetSteamAPIContext = this->engineClient->Original<uintptr_t (*)()>(Offsets::GetSteamAPIContext);
		if (Engine::OnGameOverlayActivatedBase) {
			auto OnGameOverlayActivated = reinterpret_cast<_OnGameOverlayActivated *>(GetSteamAPIContext() + Offsets::OnGameOverlayActivated);
			*OnGameOverlayActivated = Engine::OnGameOverlayActivatedBase;
		}
	}

	Renderer::Cleanup();

	Interface::Delete(this->engineClient);
	Interface::Delete(this->s_ServerPlugin);
	Interface::Delete(this->cl);
	Interface::Delete(this->eng);
	Interface::Delete(this->s_GameEventManager);
	Interface::Delete(this->engineTool);
	Interface::Delete(this->engineTrace);
	Interface::Delete(this->g_VEngineServer);
	Interface::Delete(this->g_physCollision);

	// Reset to the offsets that were originally in the code
	g_ReadCustomDataPatch->Restore();
	g_ReadConsoleCommandPatch->Restore();
	SAFE_DELETE(g_ReadCustomDataPatch)
	SAFE_DELETE(g_ReadConsoleCommandPatch)

#ifdef _WIN32
	MH_UNHOOK(Engine::ParseSmoothingInfo_Mid);

	this->demoSmootherPatch->Restore();
	SAFE_DELETE(this->demoSmootherPatch)
#endif
	Command::Unhook("plugin_load", Engine::plugin_load_callback);
	Command::Unhook("plugin_unload", Engine::plugin_unload_callback);
	Command::Unhook("exit", Engine::exit_callback);
	Command::Unhook("quit", Engine::quit_callback);
	Command::Unhook("_restart", Engine::restart_callback);
	Command::Unhook("help", Engine::help_callback);
	Command::Unhook("load", Engine::load_callback);
	Command::Unhook("give", Engine::give_callback);
	Command::Unhook("exec", Engine::exec_callback);
	Command::UnhookCompletion("playdemo", playdemo_orig_completion);
	Command::UnhookCompletion("exec", exec_orig_completion);
	Command::Unhook("gameui_activate", Engine::gameui_activate_callback);
	Command::Unhook("playvideo_end_level_transition", Engine::playvideo_end_level_transition_callback);
	Command::Unhook("stop_transition_videos_fadeout", Engine::stop_transition_videos_fadeout_callback);
	Command::Unhook("changelevel", Engine::changelevel_command_callback);
	Command::Unhook("changelevel2", Engine::changelevel2_command_callback);

	if (this->demoplayer) {
		this->demoplayer->Shutdown();
	}
	if (this->demorecorder) {
		this->demorecorder->Shutdown();
	}

	SAFE_DELETE(this->demoplayer)
	SAFE_DELETE(this->demorecorder)
}

Engine *engine;
