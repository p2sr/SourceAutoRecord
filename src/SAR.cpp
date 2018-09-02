#include "SAR.hpp"

#include <cstring>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Tier1.hpp"
#include "Modules/VGui.hpp"

#include "Features/Config.hpp"
#include "Features/Cvars.hpp"
#include "Features/Listener.hpp"
#include "Features/Rebinder.hpp"
#include "Features/Routing/Tracer.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Tas/ReplaySystem.hpp"
#include "Features/Teleporter.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/WorkshopList.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Variable.hpp"

Variable sar_autorecord("sar_autorecord", "0", "Enables automatic demo recording.\n");
Variable sar_autojump("sar_autojump", "0", "Enables automatic jumping on the server.\n");
Variable sar_jumpboost("sar_jumpboost", "0", 0, "Enables special game movement on the server.\n"
                                                "0 = Default,\n"
                                                "1 = Orange Box Engine,\n"
                                                "2 = Pre-OBE\n");
Variable sar_aircontrol("sar_aircontrol", "0",
#ifdef _WIN32
    0,
#endif
    "Enables more air-control on the server.\n");
Variable sar_disable_challenge_stats_hud("sar_disable_challenge_stats_hud", "0", "Disables opening the challenge mode stats HUD.\n");
Variable sar_debug_event_queue("sar_debug_event_queue", "0", "Prints entitity events when they are fired, similar to developer.\n");

SAR sar;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(SAR, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, sar);

SAR::SAR()
{
    this->modules = new Modules();
    this->features = new Features();
    this->cheats = new Cheats();
    this->plugin = new Plugin();
    this->game = Game::CreateNew();
}

bool SAR::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
    console = new Console();
    if (!console->Init())
        return false;

    if (this->game) {
        this->game->LoadOffsets();

        tier1 = new Tier1();
        if (tier1->Init()) {
            this->cheats->Init();

            this->features->AddFeature<Config>(&config);
            this->features->AddFeature<Cvars>(&cvars);
            this->features->AddFeature<Rebinder>(&rebinder);
            this->features->AddFeature<Session>(&session);
            this->features->AddFeature<StepCounter>(&stepCounter);
            this->features->AddFeature<Summary>(&summary);
            this->features->AddFeature<Teleporter>(&teleporter);
            this->features->AddFeature<Tracer>(&tracer);
            this->features->AddFeature<SpeedrunTimer>(&speedrun);
            this->features->AddFeature<Stats>(&stats);
            this->features->AddFeature<CommandQueuer>(&tasQueuer);
            this->features->AddFeature<ReplaySystem>(&tasReplaySystem);
            this->features->AddFeature<Timer>(&timer);

            this->game->LoadRules();

            this->modules->AddModule<InputSystem>(&inputSystem);
            this->modules->AddModule<Scheme>(&scheme);
            this->modules->AddModule<Surface>(&surface);
            this->modules->AddModule<VGui>(&vgui);
            this->modules->AddModule<Engine>(&engine);
            this->modules->AddModule<Client>(&client);
            this->modules->AddModule<Server>(&server);
            this->modules->InitAll();

            if (engine && engine->hasLoaded) {
                engine->demoplayer->Init();
                engine->demorecorder->Init();

                if (this->game->version & SourceGame_Portal2) {
                    this->features->AddFeature<Listener>(&listener);
                    this->features->AddFeature<WorkshopList>(&workshop);

                    listener->Init();
                }

                config->Load();

                this->SearchPlugin();

                console->PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
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
    return false;
}

// SAR has to disable itself in the plugin list or the game might crash because of missing callbacks
// This is a race condition though
bool SAR::GetPlugin()
{
    static Interface* s_ServerPlugin = Interface::Create(MODULE("engine"), "ISERVERPLUGINHELPERS0", false);
    if (s_ServerPlugin) {
        auto m_Size = *reinterpret_cast<int*>((uintptr_t)s_ServerPlugin->ThisPtr() + CServerPlugin_m_Size);
        if (m_Size > 0) {
            auto m_Plugins = *reinterpret_cast<uintptr_t*>((uintptr_t)s_ServerPlugin->ThisPtr() + CServerPlugin_m_Plugins);
            for (int i = 0; i < m_Size; i++) {
                auto ptr = *reinterpret_cast<CPlugin**>(m_Plugins + sizeof(uintptr_t) * i);
                if (!std::strcmp(ptr->m_szName, SAR_PLUGIN_SIGNATURE)) {
                    this->plugin->ptr = ptr;
                    this->plugin->index = i;
                    return true;
                }
            }
        }
    }
    return false;
}
void SAR::SearchPlugin()
{
    this->findPluginThread = std::thread([this]() {
        GO_THE_FUCK_TO_SLEEP(1000);
        if (!this->GetPlugin()) {
            console->DevWarning("SAR: Failed to find SAR in the plugin list!\nTry again with \"plugin_load\".\n");
        } else {
            this->plugin->ptr->m_bDisable = true;
        }
    });
    this->findPluginThread.detach();
}

CON_COMMAND(sar_session, "Prints the current tick of the server since it has loaded.\n")
{
    int tick = engine->GetSessionTick();
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
CON_COMMAND(sar_about, "Prints info about this tool.\n")
{
    console->Print("SourceAutoRecord tells the engine to keep recording when loading a save.\n");
    console->Print("More information at: %s\n", sar.Website());
    console->Print("Game: %s\n", sar.game->Version());
    console->Print("Version: %s\n", sar.Version());
    console->Print("Build: %s\n", sar.Build());
}
CON_COMMAND(sar_cvars_save, "Saves important SAR cvars.\n")
{
    if (!config->Save()) {
        console->Print("Failed to create config file!\n");
    } else {
        console->Print("Saved important settings to cfg/_sar_cvars.cfg!\n");
    }
}
CON_COMMAND(sar_cvars_load, "Loads important SAR cvars.\n")
{
    if (!config->Load()) {
        console->Print("Config file not found!\n");
    }
}
CON_COMMAND(sar_cvars_dump, "Dumps all cvars to a file.\n")
{
    std::ofstream file("game.cvars", std::ios::out | std::ios::trunc | std::ios::binary);
    auto result = cvars->Dump(file);
    file.close();

    console->Print("Dumped %i cvars to game.cvars!\n", result);
}
CON_COMMAND(sar_cvarlist, "Lists all SAR cvars and unlocked engine cvars.\n")
{
    cvars->ListAll();
}
CON_COMMAND(sar_rename, "Changes your name.\n")
{
    if (args.ArgC() != 2) {
        console->Print("Changes your name. Usage: sar_rename <name>\n");
    }

    auto name = Variable("name");
    if (!!name) {
        name.DisableChange();
        name.SetValue(args[1]);
        name.EnableChange();
    }
}
CON_COMMAND(sar_exit, "Removes all function hooks, registered commands and unloads the module.\n")
{
    SAFE_DELETE(sar.features)

    if (sar.cheats) {
        sar.cheats->Shutdown();
    }
    if (sar.modules) {
        sar.modules->ShutdownAll();
    }

    if (sar.GetPlugin()) {
        // SAR has to unhook CEngine some ticks before unloading the module
        auto unload = std::string("plugin_unload ") + std::to_string(sar.plugin->index);
        engine->SendToCommandBuffer(unload.c_str(), SAFE_UNLOAD_TICK_DELAY);
    }

    SAFE_DELETE(sar.cheats)
    SAFE_DELETE(sar.modules)
    SAFE_DELETE(sar.plugin)
    SAFE_DELETE(sar.game)

    console->Print("Cya :)\n");

    SAFE_DELETE(tier1)
    SAFE_DELETE(console)
}

// TSP only
void IN_BhopDown(const CCommand& args) { client->KeyDown(client->in_jump, (args.ArgC() > 1) ? args[1] : NULL); }
void IN_BhopUp(const CCommand& args) { client->KeyUp(client->in_jump, (args.ArgC() > 1) ? args[1] : NULL); }

Command startbhop("+bhop", IN_BhopDown, "Client sends a key-down event for the in_jump state.\n");
Command endbhop("-bhop", IN_BhopUp, "Client sends a key-up event for the in_jump state.\n");

CON_COMMAND(sar_anti_anti_cheat, "Sets sv_cheats to 1.\n")
{
    sv_cheats.ThisPtr()->m_nValue = 1;
}

// TSP & TBG only
DECLARE_AUTOCOMPLETION_FUNCTION(map, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2, "maps", bsp);

// P2 only
CON_COMMAND(sar_togglewait, "Enables or disables \"wait\" for the command buffer.\n")
{
    auto state = !*engine->m_bWaitEnabled;
    *engine->m_bWaitEnabled = state;
    console->Print("%s wait!\n", (state) ? "Enabled" : "Disabled");
}

#pragma region Unused callbacks
void SAR::Unload()
{
}
void SAR::Pause()
{
}
void SAR::UnPause()
{
}
const char* SAR::GetPluginDescription()
{
    return SAR_PLUGIN_SIGNATURE;
}
void SAR::LevelInit(char const* pMapName)
{
}
void SAR::ServerActivate(void* pEdictList, int edictCount, int clientMax)
{
}
void SAR::GameFrame(bool simulating)
{
}
void SAR::LevelShutdown()
{
}
void SAR::ClientFullyConnect(void* pEdict)
{
}
void SAR::ClientActive(void* pEntity)
{
}
void SAR::ClientDisconnect(void* pEntity)
{
}
void SAR::ClientPutInServer(void* pEntity, char const* playername)
{
}
void SAR::SetCommandClient(int index)
{
}
void SAR::ClientSettingsChanged(void* pEdict)
{
}
int SAR::ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen)
{
    return 0;
}
int SAR::ClientCommand(void* pEntity, const void*& args)
{
    return 0;
}
int SAR::NetworkIDValidated(const char* pszUserName, const char* pszNetworkID)
{
    return 0;
}
void SAR::OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue)
{
}
void SAR::OnEdictAllocated(void* edict)
{
}
void SAR::OnEdictFreed(const void* edict)
{
}
#pragma endregion
