#pragma once
#include <thread>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Speedrun.hpp"

#include "Interfaces.hpp"
#include "Plugin.hpp"
#include "Utils.hpp"

#define SAR_VERSION "1.7"
#define SAR_BUILD __TIME__ " " __DATE__

#define SAFE_UNLOAD_TICK_DELAY 33

namespace SAR {

std::thread findPluginThread;

bool NewVMT(void* ptr, VMT& hook, const char* caller = "unknown", bool vtableOnly = false)
{
    if (ptr) {
        hook = std::make_unique<VMTHook>(ptr, !vtableOnly);
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
bool PluginFound()
{
    if (!plugin->found && Interfaces::IServerPluginHelpers) {
        auto m_Size = *reinterpret_cast<int*>((uintptr_t)Interfaces::IServerPluginHelpers + CServerPlugin_m_Size);
        if (m_Size > 0) {
            auto m_Plugins = *reinterpret_cast<uintptr_t*>((uintptr_t)Interfaces::IServerPluginHelpers + CServerPlugin_m_Plugins);
            for (int i = 0; i < m_Size; i++) {
                auto ptr = *reinterpret_cast<CPlugin**>(m_Plugins + sizeof(uintptr_t) * i);
                if (!std::strcmp(ptr->m_szName, SAR_PLUGIN_SIGNATURE)) {
                    plugin->index = i;
                    plugin->ptr = ptr;
                    plugin->found = true;
                    break;
                }
            }
        }
    }
    return plugin->found;
}

// SAR has to disable itself in the plugin list or the game might crash because of missing callbacks
// This is a race condition though
void SearchPlugin()
{
    findPluginThread = std::thread([]() {
#if _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        if (!PluginFound()) {
            console->DevWarning("SAR: Failed to find SAR in the plugin list!\nTry again with \"plugin_load\".\n");
        } else {
            plugin->ptr->m_bDisable = true;
        }
    });
    findPluginThread.detach();
}
}

#define CREATE_VMT(ptr, vmt) if (SAR::NewVMT(ptr, vmt, #vmt))
#define COPY_VMT(ptr, vmt) if (SAR::NewVMT(ptr, vmt, #vmt, true))
#define DELETE_VMT(vmt) SAR::DeleteVMT(vmt, #vmt);
