#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"

#include "Game.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

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

void* Get(const char* filename, const char* interfaceSymbol)
{
    auto handle = Memory::GetModuleHandleByName(filename);
    if (!handle) {
        Console::DevWarning("SAR: Failed to open module %s!\n", filename);
        return nullptr;
    }

    auto CreateInterface = Memory::GetSymbolAddress(handle, "CreateInterface");
    Memory::CloseModuleHandle(handle);

    if (!CreateInterface) {
        Console::DevWarning("SAR: Failed to find symbol CreateInterface for %s!\n", filename);
        return nullptr;
    }

    auto CreateInterfaceInternal = Memory::ReadAbsoluteAddress((uintptr_t)CreateInterface + 5);
    auto s_pInterfaceRegs = **reinterpret_cast<InterfaceReg***>(CreateInterfaceInternal + 6);

    void* result = nullptr;
    for (auto current = s_pInterfaceRegs; current; current = current->m_pNext) {
        if (strncmp(current->m_pName, interfaceSymbol, strlen(interfaceSymbol)) == 0) {
            result = current->m_CreateFn();
            Console::DevMsg("SAR: Found interface %s at %p in %s!\n", current->m_pName, result, filename);
            break;
        }
    }

    if (!result)
        Console::DevWarning("SAR: Failed to find interface with symbol %s in %s!\n", interfaceSymbol, filename);

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
    ICVar = Get(MODULE("vstdlib"), "VEngineCvar0");
    IServerPluginHelpers = Get(MODULE("engine"), "ISERVERPLUGINHELPERS0");
}
}