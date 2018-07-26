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

#include "Cheats.hpp"
#include "Command.hpp"
#include "Commands.hpp"
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Listener.hpp"
#include "SAR.hpp"
#include "Variable.hpp"

class CSourceAutoRecord : public IServerPluginCallbacks {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
    {
        console = new Console();
        plugin = new Plugin();
        Speedrun::timer = new Speedrun::Timer();

        if (!console->Init())
            return false;

        Interfaces::Init();

        if (Game::IsSupported()) {
            if (Tier1::Init()) {

                Commands::Init();
                Cheats::Init();

                auto vars = Variable::RegisterAll();
                auto commands = Command::RegisterAll();
                console->DevMsg("SAR: Registered %i ConVars and %i ConCommands!\n", vars, commands);

                InputSystem::Hook();
                Scheme::Hook();
                Surface::Hook();
                VGui::Hook();
                Engine::Hook();
                Client::Hook();
                Server::Hook();

                //listener = new SourceAutoRecordListener();

                //Config::Load();
                SAR::SearchPlugin();

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

CON_COMMAND(sar_exit, "Removes all function hooks, registered commands and unloads the module.\n")
{
    //delete listener;

    Client::Unhook();
    Engine::Unhook();
    Server::Unhook();
    InputSystem::Unhook();
    Scheme::Unhook();
    Surface::Unhook();
    VGui::Unhook();

    Cheats::Unload();

    Variable::UnregisterAll();
    Command::UnregisterAll();

    Tier1::Shutdown();

    console->Print("Cya :)\n");
    delete Speedrun::timer;
    delete plugin;
    delete console;
}
