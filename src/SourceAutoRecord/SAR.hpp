#pragma once
#include <thread>

#include "vmthook/vmthook.h"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Speedrun.hpp"

#include "Game.hpp"
#include "Interfaces.hpp"
#include "Patterns.hpp"
#include "Plugin.hpp"
#include "Utils.hpp"

#define SAR_VERSION "1.7"
#define SAR_BUILD __TIME__ " " __DATE__

namespace SAR {

std::thread selfDisable;
bool exited;

Memory::ScanResult Find(const char* pattern)
{
    auto result = Memory::Scan(Patterns::Get(pattern));
    if (result.Found) {
        console->DevMsg("SAR: %s\n", result.Message);
    } else {
        console->DevWarning("SAR: %s\n", result.Message);
    }
    return result;
}
bool NewVMT(void* ptr, VMT& hook, const char* caller = "unknown")
{
    if (ptr) {
        hook = std::make_unique<VMTHook>(ptr);
        console->DevMsg("SAR: Created new VMT for %s %p with %i functions.\n", caller, ptr, hook->GetTotalFunctions());
        return true;
    }

    console->DevWarning("SAR: Failed to create VMT for %s.\n", caller);
    return false;
}
void DeleteVMT(VMT& hook, const char* caller = "unknown")
{
    if (!hook)
        return;

    console->DevMsg("SAR: Deleted VMT for %s %p.\n", caller, hook->GetThisPtr());
    hook.reset();
}

// SAR has to disable itself in the plugin list or the game might crash because of missing callbacks
// This is a race condition though
void Disable()
{
    selfDisable = std::thread([]() {
#if _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        if (Interfaces::IServerPluginHelpers) {
            auto m_Size = *reinterpret_cast<int*>((uintptr_t)Interfaces::IServerPluginHelpers + CServerPlugin_m_Size);
            console->Print("%i\n", m_Size);
            if (m_Size > 0) {
                auto m_Plugins = *reinterpret_cast<uintptr_t*>((uintptr_t)Interfaces::IServerPluginHelpers + CServerPlugin_m_Plugins);
                for (int i = 0; i < m_Size; i++) {
                    auto plugin = *reinterpret_cast<CPlugin**>(m_Plugins + sizeof(uintptr_t) * i);
                    console->Print("%s\n", plugin->m_szName);
                    if (!std::strcmp(plugin->m_szName, SAR_PLUGIN_SIGNATURE)) {
                        plugin->m_bDisable = true;
                        console->DevMsg("SAR: Disabled SAR in the plugin list!\n");
                        return;
                    }
                }
            }
        }
        console->DevWarning("SAR: Could not disable SAR in the plugin list!\nTry manually with plugin_pause.\n");
    });
    selfDisable.detach();
}
}

#define CREATE_VMT(ptr, vmt) if (SAR::NewVMT(ptr, vmt, #vmt))
#define DELETE_VMT(vmt) SAR::DeleteVMT(vmt, #vmt);