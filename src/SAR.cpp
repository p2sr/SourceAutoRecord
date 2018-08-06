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
#include "Features/Speedrun/SpeedrunTimer.hpp"
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
            modules = new Modules();
            modules->AddModule<Tier1>(&tier1);
            modules->InitAll();

            if (tier1->hasLoaded) {
                sar = new SAR();
                plugin = new Plugin();
                speedrun = new SpeedrunTimer();
                game->LoadRules();

                commands = new Commands();
                cheats = new Cheats();

                commands->Init();
                cheats->Init();

                InputSystem::Init();
                Scheme::Init();
                Surface::Init();
                VGui::Init();
                Engine::Init();
                Client::Init();
                Server::Init();

                listener = new Listener();
                listener->Init();

                config = new Config();
                config->Load();

                if (game->version == SourceGame::Portal2) {
                    workshop = new WorkshopList();
                }

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

CON_COMMAND(sar_exit, "Removes all function hooks, registered commands and unloads the module.\n")
{
    //SAFE_DELETE(listener);

    Client::Shutdown();
    Engine::Shutdown();
    Server::Shutdown();
    InputSystem::Shutdown();
    Scheme::Shutdown();
    Surface::Shutdown();
    VGui::Shutdown();

    cheats->Shutdown();
    commands->Shutdown();

    modules->ShutdownAll();

    if (sar->PluginFound()) {
        // SAR has to unhook CEngine some ticks before unloading the module
        auto unload = std::string("plugin_unload ") + std::to_string(plugin->index);
        Engine::SendToCommandBuffer(unload.c_str(), SAFE_UNLOAD_TICK_DELAY);
    } else {
        console->Warning("SAR: This should never happen :(\n");
    }

    console->Print("Cya :)\n");

    SAFE_DELETE(workshop);
    SAFE_DELETE(config);
    SAFE_DELETE(cheats);
    SAFE_DELETE(commands);
    SAFE_DELETE(game);
    SAFE_DELETE(speedrun);
    SAFE_DELETE(plugin);
    SAFE_DELETE(sar);
    SAFE_DELETE(console);
}
