#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Interfaces.hpp"
#include "Patterns.hpp"
#include "Plugin.hpp"
#include "Utils.hpp"

#define SAR_VERSION "1.7"
#define SAR_BUILD __TIME__ " " __DATE__
#define SAR_SIGNATURE new char[25] { 65, 114, 101, 32, 121, 111, 117, 32, 104, 97, 112, 112, 121, 32, 110, 111, 119, 44, 32, 74, 97, 109, 101, 114, 63 }
#define SAR_COLOR Color(247, 235, 69)
#define COL_ACTIVE Color(110, 247, 76)
#define COL_DEFAULT Color(255, 255, 255, 255)

namespace SAR {

Memory::ScanResult Find(const char* pattern)
{
    auto result = Memory::Scan(Patterns::Get(pattern));
    if (result.Found) {
        Console::DevMsg("SAR: %s\n", result.Message);
    } else {
        Console::DevWarning("SAR: %s\n", result.Message);
    }
    return result;
}
bool NewVMT(void* ptr, VMT& hook)
{
    if (ptr) {
        hook = std::make_unique<VMTHook>(ptr);
        Console::DevMsg("SAR: Created new VMT for %p with %i functions.\n", ptr, hook->GetTotalFunctions());
        return true;
    }

    Console::DevWarning("SAR: Skipped creating one VMT.\n");
    return false;
}
void DeleteVMT(VMT& hook)
{
    if (!hook) return;
    Console::DevMsg("SAR: Released VMT for %p.\n", hook->GetThisPtr());
    hook.reset();
}

// SAR has to disable itself in the plugin list or the game will crash because of missing callbacks
// This is a race condition though
bool LoadedAsPlugin = false;
void IsPlugin()
{
    if (LoadedAsPlugin) {
        Console::DevMsg("SAR: Loaded SAR as plugin! Trying to disable itself...\n");
        if (Interfaces::IServerPluginHelpers) {
            auto m_Size = *reinterpret_cast<int*>((uintptr_t)Interfaces::IServerPluginHelpers + m_Size_Offset);
            if (m_Size > 0) {
                auto m_Plugins = *reinterpret_cast<uintptr_t*>((uintptr_t)Interfaces::IServerPluginHelpers + m_Plugins_Offset);
                for (int i = 0; i < m_Size; i++) {
                    auto plugin = *reinterpret_cast<CPlugin**>(m_Plugins + sizeof(uintptr_t) * i);
                    Console::Print("%s\n", plugin->m_szName);
                    if (std::strcmp(plugin->m_szName, SAR_SIGNATURE) == 0) {
                        plugin->m_bDisable = true;
                        Console::DevMsg("SAR: Disabled SAR in the plugin list!\n");
                        return;
                    }
                }
            }
        }
        Console::DevWarning("SAR: Could not disable SAR in the plugin list! Try manually with plugin_pause.\n");
    }
}
}

struct CSourceAutoRecord : IServerPluginCallbacks {
    CSourceAutoRecord()
    {
    }
    ~CSourceAutoRecord()
    {
    }
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
    {
        return SAR::LoadedAsPlugin = true;
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
        return SAR_SIGNATURE;
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
};

CSourceAutoRecord g_SourceAutoRecord;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSourceAutoRecord, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_SourceAutoRecord);