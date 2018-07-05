#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Interfaces.hpp"
#include "Patterns.hpp"
#include "Plugin.hpp"
#include "Utils.hpp"

#define SAR_VERSION "1.7"
#define SAR_BUILD __TIME__ " " __DATE__
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
bool NewVMT(void* ptr, std::unique_ptr<VMTHook>& hook)
{
    if (ptr) {
        hook = std::make_unique<VMTHook>(ptr);
        Console::DevMsg("SAR: Created new VMT for 0x%p with %i functions.\n", ptr, hook->GetTotalFunctions());
        return true;
    }

    Console::DevWarning("SAR: Skipped creating new VMT for 0x%p.\n", ptr);
    return false;
}
void DeleteVMT(std::unique_ptr<VMTHook>& hook)
{
    Console::DevMsg("SAR: Released VMT for 0x%p.\n", hook->GetThisPtr());
    hook.reset();
}

bool LoadedAsPlugin = false;
void IsPlugin()
{
    if (LoadedAsPlugin) {
        Console::DevMsg("SAR: Loaded SAR as plugin! Trying to disable itself...\n");
        if (Interfaces::IServerPluginHelpers) {
            auto m_Size = *reinterpret_cast<int*>((uintptr_t)Interfaces::IServerPluginHelpers + 16);
            Console::Print("m_Size = %i\n", m_Size);
            if (m_Size > 0) {
                auto sar = reinterpret_cast<CPlugin*>((uintptr_t)Interfaces::IServerPluginHelpers + 4);
                Console::Print("sar->m_szName = %s\n", sar->m_szName);
                Console::Print("sar->m_bDisable = %i\n", sar->m_bDisable);
                sar->m_bDisable = true;
            }
        }
        Console::DevWarning("SAR: Could not disable SAR from the plugin list! Try manually with plugin_unload.\n");
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
        return "Are you happy now, Jamer?";
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