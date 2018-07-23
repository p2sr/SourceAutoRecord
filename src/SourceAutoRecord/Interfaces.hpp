#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Game.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#define CreateInterfaceInternal_Offset 5
#ifdef _WIN32
#define s_pInterfaceRegs_Offset 6
#else
#define s_pInterfaceRegs_Offset 11
#endif

namespace Interfaces {

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
typedef void* (*InstantiateInterfaceFn)();

struct InterfaceReg {
    InstantiateInterfaceFn m_CreateFn;
    const char* m_pName;
    InterfaceReg* m_pNext;
    static InterfaceReg* s_pInterfaceRegs;
};

void* IGameMovement;
void* IVEngineClient;
void* IInputSystem;
void* ISurface;
void* IEngineVGui;
void* IBaseClientDLL;
void* IEngineTool;
void* ISchemeManager;
void* IClientEntityList;
void* IServerGameDLL;
void* ICVar;
void* IServerPluginHelpers;
void* IGameEventManager2;

void* Get(const char* filename, const char* interfaceSymbol)
{
    auto handle = Memory::GetModuleHandleByName(filename);
    if (!handle) {
        console->DevWarning("SAR: Failed to open module %s!\n", filename);
        return nullptr;
    }

    auto CreateInterface = Memory::GetSymbolAddress(handle, "CreateInterface");
    Memory::CloseModuleHandle(handle);

    if (!CreateInterface) {
        console->DevWarning("SAR: Failed to find symbol CreateInterface for %s!\n", filename);
        return nullptr;
    }

    auto CreateInterfaceInternal = Memory::ReadAbsoluteAddress((uintptr_t)CreateInterface + CreateInterfaceInternal_Offset);
    auto s_pInterfaceRegs = **reinterpret_cast<InterfaceReg***>(CreateInterfaceInternal + s_pInterfaceRegs_Offset);

    void* result = nullptr;
    for (auto current = s_pInterfaceRegs; current; current = current->m_pNext) {
        if (strncmp(current->m_pName, interfaceSymbol, strlen(interfaceSymbol)) == 0) {
            result = current->m_CreateFn();
            //console->DevMsg("SAR: Found interface %s at %p in %s!\n", current->m_pName, result, filename);
            break;
        }
    }

    if (!result)
        console->DevWarning("SAR: Failed to find interface with symbol %s in %s!\n", interfaceSymbol, filename);

    return result;
}
void Init()
{
    IVEngineClient = Get(MODULE("engine"), "VEngineClient0");
    IEngineVGui = Get(MODULE("engine"), "VEngineVGui0");
    IEngineTool = Get(MODULE("engine"), "VENGINETOOL0");
    IInputSystem = Get(MODULE("inputsystem"), "InputSystemVersion0");
    ISurface = Get(MODULE("vguimatsurface"), "VGUI_Surface0");
    ISchemeManager = Get(MODULE("vgui2"), "VGUI_Scheme0");
    IBaseClientDLL = Get(MODULE("client"), "VClient0");
    IClientEntityList = Get(MODULE("client"), "VClientEntityList0");
    IGameMovement = Get(MODULE("server"), "GameMovement0");
    IServerGameDLL = Get(MODULE("server"), "ServerGameDLL0");
#if _WIN32
    ICVar = Get(MODULE("vstdlib"), "VEngineCvar0");
#else
    ICVar = Get(MODULE("libvstdlib"), "VEngineCvar0");
#endif
    IServerPluginHelpers = Get(MODULE("engine"), "ISERVERPLUGINHELPERS0");
    IGameEventManager2 = Get(MODULE("engine"), "GAMEEVENTSMANAGER002");
}
}
