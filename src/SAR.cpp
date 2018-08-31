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
#include "Features/Routing/Tracer.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Tas/ReplaySystem.hpp"
#include "Features/Teleporter.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/WorkshopList.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Commands.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Listener.hpp"
#include "SAR.hpp"
#include "Variable.hpp"

class CSourceAutoRecord : public IServerPluginCallbacks {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
    {
        console = new Console();
        if (!console->Init())
            return false;

        if (Game::IsSupported()) {
            sar = new SAR();

            sar->modules->AddModule<Tier1>(&tier1);
            sar->modules->InitAll();

            if (tier1->hasLoaded) {
                plugin = new Plugin();

                sar->features->AddFeature<Stats>(&stats);
                sar->features->AddFeature<StepCounter>(&stepCounter);
                sar->features->AddFeature<SpeedrunTimer>(&speedrun);
                sar->features->AddFeature<Rebinder>(&rebinder);
                sar->features->AddFeature<Cvars>(&cvars);
                sar->features->AddFeature<CommandQueuer>(&tasQueuer);
                sar->features->AddFeature<ReplaySystem>(&tasReplaySystem);
                sar->features->AddFeature<Session>(&session);
                sar->features->AddFeature<Tracer>(&tracer);
                sar->features->AddFeature<Teleporter>(&teleporter);
                sar->features->AddFeature<Config>(&config);
                sar->features->AddFeature<Summary>(&summary);
                sar->features->AddFeature<Timer>(&timer);

                game->LoadRules();

                commands = new Commands();
                cheats = new Cheats();

                commands->Init();
                cheats->Init();

                sar->modules->AddModule<InputSystem>(&inputSystem);
                sar->modules->AddModule<Scheme>(&scheme);
                sar->modules->AddModule<Surface>(&surface);
                sar->modules->InitAll();

                VGui::Init();
                Engine::Init();
                Client::Init();
                Server::Init();

                if (game->version == SourceGame::Portal2) {
                    listener = new Listener();
                    listener->Init();

                    sar->features->AddFeature<WorkshopList>(&workshop);
                }

                config->Load();

                sar->SearchPlugin();

                console->PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
                return true;
            } else {
                console->Warning("SAR: Could not register any commands!\n");
            }
        } else {
            console->Warning("SAR: Game not supported!\n");
        }

        console->Warning("SAR: Failed to load SourceAutoRecord!\n");
        return false;
    }
    virtual void Unload()
    {
    }
    virtual void Pause()
    {
    }
    virtual void UnPause()
    {
    }
    virtual const char* GetPluginDescription()
    {
        return SAR_PLUGIN_SIGNATURE;
    }
    virtual void LevelInit(char const* pMapName)
    {
    }
    virtual void ServerActivate(void* pEdictList, int edictCount, int clientMax)
    {
    }
    virtual void GameFrame(bool simulating)
    {
    }
    virtual void LevelShutdown()
    {
    }
    virtual void ClientFullyConnect(void* pEdict)
    {
    }
    virtual void ClientActive(void* pEntity)
    {
    }
    virtual void ClientDisconnect(void* pEntity)
    {
    }
    virtual void ClientPutInServer(void* pEntity, char const* playername)
    {
    }
    virtual void SetCommandClient(int index)
    {
    }
    virtual void ClientSettingsChanged(void* pEdict)
    {
    }
    virtual int ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen)
    {
        return 0;
    }
    virtual int ClientCommand(void* pEntity, const void*& args)
    {
        return 0;
    }
    virtual int NetworkIDValidated(const char* pszUserName, const char* pszNetworkID)
    {
        return 0;
    }
    virtual void OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue)
    {
    }
    virtual void OnEdictAllocated(void* edict)
    {
    }
    virtual void OnEdictFreed(const void* edict)
    {
    }
};

CSourceAutoRecord g_SourceAutoRecord;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSourceAutoRecord, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_SourceAutoRecord);

CON_COMMAND(sar_session, "Prints the current tick of the server since it has loaded.\n")
{
    int tick = Engine::GetSessionTick();
    console->Print("Session Tick: %i (%.3f)\n", tick, Engine::ToTime(tick));
    if (*Engine::DemoRecorder::m_bRecording) {
        tick = Engine::DemoRecorder::GetTick();
        console->Print("Demo Recorder Tick: %i (%.3f)\n", tick, Engine::ToTime(tick));
    }
    if (Engine::DemoPlayer::IsPlaying()) {
        tick = Engine::DemoPlayer::GetTick();
        console->Print("Demo Player Tick: %i (%.3f)\n", tick, Engine::ToTime(tick));
    }
}

CON_COMMAND(sar_about, "Prints info about this tool.\n")
{
    console->Print("SourceAutoRecord tells the engine to keep recording when loading a save.\n");
    console->Print("More information at: %s\n", sar->Website());
    console->Print("Game: %s\n", game->Version());
    console->Print("Version: %s\n", sar->Version());
    console->Print("Build: %s\n", sar->Build());
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
    SAFE_DELETE(listener);

    Client::Shutdown();
    Engine::Shutdown();
    Server::Shutdown();
    VGui::Shutdown();

    cheats->Shutdown();
    commands->Shutdown();

    sar->modules->ShutdownAll();

    if (sar->GetPlugin()) {
        // SAR has to unhook CEngine some ticks before unloading the module
        auto unload = std::string("plugin_unload ") + std::to_string(plugin->index);
        Engine::SendToCommandBuffer(unload.c_str(), SAFE_UNLOAD_TICK_DELAY);
    }

    console->Print("Cya :)\n");

    SAFE_DELETE(cheats);
    SAFE_DELETE(commands);
    SAFE_DELETE(config);
    SAFE_DELETE(plugin);
    SAFE_DELETE(sar);
    SAFE_DELETE(game);
    SAFE_DELETE(console);
}
