#pragma once
#include "Utils.hpp"

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define INTERFACEVERSION_ISERVERPLUGINCALLBACKS "ISERVERPLUGINCALLBACKS002"

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
typedef void* (*InstantiateInterfaceFn)();

struct InterfaceReg {
    InstantiateInterfaceFn m_CreateFn;
    const char* m_pName;
    InterfaceReg* m_pNext;
    static InterfaceReg* s_pInterfaceRegs;

    InterfaceReg(InstantiateInterfaceFn fn, const char *pName) : m_pName(pName)
    {
        m_CreateFn = fn;
        m_pNext = s_pInterfaceRegs;
        s_pInterfaceRegs = this;
    }
};

InterfaceReg* InterfaceReg::s_pInterfaceRegs = nullptr;

void* CreateInterfaceInternal(const char *pName, int *pReturnCode)
{
    InterfaceReg *pCur;

    for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext) {
        if (strcmp(pCur->m_pName, pName) == 0) {
            if (pReturnCode) {
                *pReturnCode = 0;
            }
            return pCur->m_CreateFn();
        }
    }

    if (pReturnCode) {
        *pReturnCode = 1;
    }
    return NULL;
}

DLL_EXPORT void* CreateInterface(const char *pName, int *pReturnCode)
{
    return CreateInterfaceInternal(pName, pReturnCode);
}

#define EXPOSE_INTERFACE_FN(functionName, interfaceName, versionName) \
    static InterfaceReg __g_Create##interfaceName##_reg(functionName, versionName);

#define EXPOSE_INTERFACE(className, interfaceName, versionName) \
    static void* __Create##className##_interface() { return static_cast<interfaceName*>(new className); } \
    static InterfaceReg __g_Create##className##_reg(__Create##className##_interface, versionName);

#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName) \
    static void* __Create##className##interfaceName##_interface() { return static_cast<interfaceName*>(&globalVarName); } \
    static InterfaceReg __g_Create##className##interfaceName##_reg(__Create##className##interfaceName##_interface, versionName);

#define EXPOSE_SINGLE_INTERFACE(className, interfaceName, versionName) \
    static className __g_##className##_singleton; \
    EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)

struct IServerPluginCallbacks {
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) = 0;
    virtual void Unload() = 0;
    virtual void Pause() = 0;
    virtual void UnPause() = 0;
    virtual const char* GetPluginDescription() = 0;
    virtual void LevelInit(char const* pMapName) = 0;
    virtual void ServerActivate(void* pEdictList, int edictCount, int clientMax) = 0;
    virtual void GameFrame(bool simulating) = 0;
    virtual void LevelShutdown() = 0;
    virtual void ClientActive(void* pEntity) = 0;
    virtual void ClientDisconnect(void* pEntity) = 0;
    virtual void ClientPutInServer(void* pEntity, char const* playername) = 0;
    virtual void SetCommandClient(int index) = 0;
    virtual void ClientSettingsChanged(void* pEdict) = 0;
    virtual int ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) = 0;
    virtual int ClientCommand(void* pEntity, const void*& args) = 0;
    virtual int NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) = 0;
    virtual void OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) = 0;
};

// CServerPlugin
#define m_Size_Offset 16
#define m_Plugins_Offset 4

struct CPlugin {
    char m_szName[128];
    bool m_bDisable;
    IServerPluginCallbacks* m_pPlugin;
    int m_iPluginInterfaceVersion;
    void* m_pPluginModule;
};