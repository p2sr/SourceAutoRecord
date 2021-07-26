#pragma once
#include "Cheats.hpp"
#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Modules/Console.hpp"
#include "Modules/Module.hpp"
#include "Plugin.hpp"
#include "Variable.hpp"

#include <thread>

#define SAR_BUILT __TIME__ " " __DATE__
#define SAR_WEB "https://nekzor.github.io/SourceAutoRecord or https://wiki.portal2.sr/SAR"

#define SAFE_UNLOAD_TICK_DELAY 33

class SAR : public IServerPluginCallbacks {
public:
	Modules *modules;
	Features *features;
	Cheats *cheats;
	Plugin *plugin;
	Game *game;

private:
	std::thread findPluginThread;

public:
	SAR();

	virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void Unload();
	virtual void Pause();
	virtual void UnPause();
	virtual const char *GetPluginDescription();
	virtual void LevelInit(char const *pMapName);
	virtual void ServerActivate(void *pEdictList, int edictCount, int clientMax);
	virtual void GameFrame(bool simulating);
	virtual void LevelShutdown();
	virtual void ClientFullyConnect(void *pEdict);
	virtual void ClientActive(void *pEntity);
	virtual void ClientDisconnect(void *pEntity);
	virtual void ClientPutInServer(void *pEntity, char const *playername);
	virtual void SetCommandClient(int index);
	virtual void ClientSettingsChanged(void *pEdict);
	virtual int ClientConnect(bool *bAllowConnect, void *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen);
	virtual int ClientCommand(void *pEntity, const void *&args);
	virtual int NetworkIDValidated(const char *pszUserName, const char *pszNetworkID);
	virtual void OnQueryCvarValueFinished(int iCookie, void *pPlayerEntity, int eStatus, const char *pCvarName, const char *pCvarValue);
	virtual void OnEdictAllocated(void *edict);
	virtual void OnEdictFreed(const void *edict);

	bool GetPlugin();
	void SearchPlugin();
};

extern SAR sar;
