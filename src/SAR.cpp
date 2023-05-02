#include "SAR.hpp"

#include "Version.hpp"

#include <cstring>
#include <ctime>
#include <curl/curl.h>

#ifdef _WIN32
#	include <filesystem>
#endif

#include "Cheats.hpp"
#include "Checksum.hpp"
#include "Command.hpp"
#include "CrashHandler.hpp"
#include "Event.hpp"
#include "Features.hpp"
#include "Features/Stats/StatsCounter.hpp"
#include "Features/SeasonalASCII.hpp"
#include "Game.hpp"
#include "Hook.hpp"
#include "Interface.hpp"
#include "Modules.hpp"
#include "Variable.hpp"

SAR sar;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(SAR, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, sar);


SAR::SAR()
	: modules(new Modules())
	, features(new Features())
	, cheats(new Cheats())
	, plugin(new Plugin())
	, game(Game::CreateNew()) {
}

bool SAR::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
	console = new Console();
	if (!console->Init())
		return false;

#ifdef _WIN32
	// The auto-updater can create this file on Windows; we should try
	// to delete it.
	try {
		if (std::filesystem::exists("sar.dll.old-auto")) {
			std::filesystem::remove("sar.dll.old-auto");
		}
		if (std::filesystem::exists("sar.pdb.old-auto")) {
			std::filesystem::remove("sar.pdb.old-auto");
		}
	} catch (...) {
	}
#endif

	if (this->game) {
		this->game->LoadOffsets();

		CrashHandler::Init();

		SarInitHandler::RunAll();

		curl_global_init(CURL_GLOBAL_ALL);

		tier1 = new Tier1();
		if (tier1->Init()) {
			this->features->AddFeature<Cvars>(&cvars);
			this->features->AddFeature<Session>(&session);
			this->features->AddFeature<StepCounter>(&stepCounter);
			this->features->AddFeature<Summary>(&summary);
			this->features->AddFeature<Teleporter>(&teleporter);
			SpeedrunTimer::Init();
			this->features->AddFeature<Stats>(&stats);
			this->features->AddFeature<StatsCounter>(&statsCounter);
			this->features->AddFeature<Sync>(&synchro);
			this->features->AddFeature<ReloadedFix>(&reloadedFix);
			this->features->AddFeature<Timer>(&timer);
			this->features->AddFeature<EntityInspector>(&inspector);
			this->features->AddFeature<SeamshotFind>(&seamshotFind);
			this->features->AddFeature<ClassDumper>(&classDumper);
			this->features->AddFeature<EntityList>(&entityList);
			this->features->AddFeature<TasController>(&tasControllers[0]);
			this->features->AddFeature<TasController>(&tasControllers[1]);
			this->features->AddFeature<TasPlayer>(&tasPlayer);
			this->features->AddFeature<PauseTimer>(&pauseTimer);
			this->features->AddFeature<DataMapDumper>(&dataMapDumper);
			this->features->AddFeature<FovChanger>(&fovChanger);
			this->features->AddFeature<Camera>(&camera);
			this->features->AddFeature<GroundFramesCounter>(&groundFramesCounter);
			this->features->AddFeature<TimescaleDetect>(&timescaleDetect);
			this->features->AddFeature<PlayerTrace>(&playerTrace);
			toastHud.InitMessageHandler();

			this->modules->AddModule<InputSystem>(&inputSystem);
			this->modules->AddModule<Scheme>(&scheme);
			this->modules->AddModule<Surface>(&surface);
			this->modules->AddModule<VGui>(&vgui);
			this->modules->AddModule<Engine>(&engine);
			this->modules->AddModule<Client>(&client);
			this->modules->AddModule<Server>(&server);
			this->modules->AddModule<MaterialSystem>(&materialSystem);
			this->modules->AddModule<FileSystem>(&fileSystem);
			this->modules->InitAll();

			InitSARChecksum();

			if (engine && engine->hasLoaded) {
				engine->demoplayer->Init();
				engine->demorecorder->Init();

				this->cheats->Init();

				this->features->AddFeature<Listener>(&listener);

				if (this->game->Is(SourceGame_Portal2 | SourceGame_ApertureTag)) {
					this->features->AddFeature<WorkshopList>(&workshop);
				}

				if (listener) {
					listener->Init();
				}

				this->SearchPlugin();

				console->PrintActive("Loaded SourceAutoRecord, Version %s\n", SAR_VERSION);
				
				SeasonalASCII::Init();

				return true;
			} else {
				console->Warning("SAR: Failed to load engine module!\n");
			}
		} else {
			console->Warning("SAR: Failed to load tier1 module!\n");
		}
	} else {
		console->Warning("SAR: Game not supported!\n");
	}

	console->Warning("SAR: Failed to load SourceAutoRecord!\n");

	if (sar.cheats) {
		sar.cheats->Shutdown();
	}
	if (sar.features) {
		sar.features->DeleteAll();
	}

	if (sar.modules) {
		sar.modules->ShutdownAll();
	}

	// This isn't in sar.modules
	if (tier1) {
		tier1->Shutdown();
	}

	Variable::ClearAllCallbacks();
	SAFE_DELETE(sar.features)
	SAFE_DELETE(sar.cheats)
	SAFE_DELETE(sar.modules)
	SAFE_DELETE(sar.plugin)
	SAFE_DELETE(sar.game)
	SAFE_DELETE(tier1)
	SAFE_DELETE(console)
	CrashHandler::Cleanup();
	return false;
}

// SAR has to disable itself in the plugin list or the game might crash because of missing callbacks
// This is a race condition though
bool SAR::GetPlugin() {
	auto s_ServerPlugin = reinterpret_cast<uintptr_t>(engine->s_ServerPlugin->ThisPtr());
	auto m_Size = *reinterpret_cast<int *>(s_ServerPlugin + CServerPlugin_m_Size);
	if (m_Size > 0) {
		auto m_Plugins = *reinterpret_cast<uintptr_t *>(s_ServerPlugin + CServerPlugin_m_Plugins);
		for (auto i = 0; i < m_Size; ++i) {
			auto ptr = *reinterpret_cast<CPlugin **>(m_Plugins + sizeof(uintptr_t) * i);
			if (!std::strcmp(ptr->m_szName, SAR_PLUGIN_SIGNATURE)) {
				this->plugin->ptr = ptr;
				this->plugin->index = i;
				return true;
			}
		}
	}
	return false;
}
void SAR::SearchPlugin() {
	this->findPluginThread = std::thread([this]() {
		GO_THE_FUCK_TO_SLEEP(1000);
		if (this->GetPlugin()) {
			this->plugin->ptr->m_bDisable = true;
		} else {
			console->DevWarning("SAR: Failed to find SAR in the plugin list!\nTry again with \"plugin_load\".\n");
		}
	});
	this->findPluginThread.detach();
}

void SAR::Unload() {
	if (unloading) return;
	unloading = true;

	curl_global_cleanup();
	statsCounter->RecordDatas(session->GetTick());
	statsCounter->ExportToFile(sar_statcounter_filePath.GetString());

	networkManager.Disconnect();

	Variable::ClearAllCallbacks();

	Hook::DisableAll();

	if (sar.cheats) {
		sar.cheats->Shutdown();
	}
	if (sar.features) {
		sar.features->DeleteAll();
	}

	if (sar.GetPlugin()) {
		// SAR has to unhook CEngine some ticks before unloading the module
		auto unload = std::string("plugin_unload ") + std::to_string(sar.plugin->index);
		engine->SendToCommandBuffer(unload.c_str(), SAFE_UNLOAD_TICK_DELAY);
	}

	if (sar.modules) {
		sar.modules->ShutdownAll();
	}

	// This isn't in sar.modules
	if (tier1) {
		tier1->Shutdown();
	}

	SAFE_DELETE(sar.features)
	SAFE_DELETE(sar.cheats)
	SAFE_DELETE(sar.modules)
	SAFE_DELETE(sar.plugin)
	SAFE_DELETE(sar.game)

	console->Print("Cya :)\n");

	SAFE_DELETE(tier1)
	SAFE_DELETE(console)
	CrashHandler::Cleanup();
}

CON_COMMAND(sar_session, "sar_session - prints the current tick of the server since it has loaded\n") {
	auto tick = session->GetTick();
	console->Print("Session Tick: %i (%.3f)\n", tick, engine->ToTime(tick));
	if (*engine->demorecorder->m_bRecording) {
		tick = engine->demorecorder->GetTick();
		console->Print("Demo Recorder Tick: %i (%.3f)\n", tick, engine->ToTime(tick));
	}
	if (engine->demoplayer->IsPlaying()) {
		tick = engine->demoplayer->GetTick();
		console->Print("Demo Player Tick: %i (%.3f)\n", tick, engine->ToTime(tick));
	}
}
CON_COMMAND(sar_about, "sar_about - prints info about SAR plugin\n") {
	console->Print("SourceAutoRecord is a speedrun plugin for Source Engine games.\n");
	console->Print("More information at: https://github.com/p2sr/SourceAutoRecord or https://wiki.portal2.sr/SAR\n");
	console->Print("Game: %s\n", sar.game->Version());
	console->Print("Version: " SAR_VERSION "\n");
	console->Print("Built: " SAR_BUILT "\n");
}
CON_COMMAND(sar_cvars_dump, "sar_cvars_dump - dumps all cvars to a file\n") {
	std::ofstream file("game.cvars", std::ios::out | std::ios::trunc | std::ios::binary);
	auto result = cvars->Dump(file);
	file.close();

	console->Print("Dumped %i cvars to game.cvars!\n", result);
}
CON_COMMAND(sar_cvars_dump_doc, "sar_cvars_dump_doc - dumps all SAR cvars to a file\n") {
	std::ofstream file("sar.cvars", std::ios::out | std::ios::trunc | std::ios::binary);
	auto result = cvars->DumpDoc(file);
	file.close();

	console->Print("Dumped %i cvars to sar.cvars!\n", result);
}
CON_COMMAND(sar_cvars_lock, "sar_cvars_lock - restores default flags of unlocked cvars\n") {
	cvars->Lock();
}
CON_COMMAND(sar_cvars_unlock, "sar_cvars_unlock - unlocks all special cvars\n") {
	cvars->Unlock();
}
CON_COMMAND(sar_cvarlist, "sar_cvarlist - lists all SAR cvars and unlocked engine cvars\n") {
	cvars->ListAll();
}
CON_COMMAND(sar_rename, "sar_rename <name> - changes your name\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_rename.ThisPtr()->m_pszHelpString);
	}

	Variable name("name");
	if (!!name) {
		name.DisableChange();
		name.SetValue(args[1]);
		name.EnableChange();
	}
}
CON_COMMAND(sar_exit, "sar_exit - removes all function hooks, registered commands and unloads the module\n") {
	sar.Unload();
}

#pragma region Unused callbacks
void SAR::Pause() {
}
void SAR::UnPause() {
}
const char *SAR::GetPluginDescription() {
	return SAR_PLUGIN_SIGNATURE;
}
void SAR::LevelInit(char const *pMapName) {
}
void SAR::ServerActivate(void *pEdictList, int edictCount, int clientMax) {
}
void SAR::GameFrame(bool simulating) {
}
void SAR::LevelShutdown() {
}
void SAR::ClientFullyConnect(void *pEdict) {
}
void SAR::ClientActive(void *pEntity) {
}
void SAR::ClientDisconnect(void *pEntity) {
}
void SAR::ClientPutInServer(void *pEntity, char const *playername) {
}
void SAR::SetCommandClient(int index) {
}
void SAR::ClientSettingsChanged(void *pEdict) {
}
int SAR::ClientConnect(bool *bAllowConnect, void *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) {
	return 0;
}
int SAR::ClientCommand(void *pEntity, const void *&args) {
	return 0;
}
int SAR::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) {
	return 0;
}
void SAR::OnQueryCvarValueFinished(int iCookie, void *pPlayerEntity, int eStatus, const char *pCvarName, const char *pCvarValue) {
}
void SAR::OnEdictAllocated(void *edict) {
}
void SAR::OnEdictFreed(const void *edict) {
}
#pragma endregion
