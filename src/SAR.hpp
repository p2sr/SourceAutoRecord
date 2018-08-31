#pragma once
#include <thread>

#include "Modules/Console.hpp"
#include "Modules/Module.hpp"

#include "Features/Feature.hpp"

#include "Interface.hpp"
#include "Plugin.hpp"

#define SAR_VERSION "1.8"
#define SAR_BUILD __TIME__ " " __DATE__
#define SAR_WEB "https://nekzor.github.io/SourceAutoRecord"

#define SAFE_UNLOAD_TICK_DELAY 33

class SAR {
public:
    Modules* modules;
    Features* features;

private:
    std::thread findPluginThread;

public:
    SAR()
    {
        this->modules = new Modules();
        this->features = new Features();
    }
    ~SAR()
    {
        SAFE_DELETE(this->modules);
        SAFE_DELETE(this->features);
    }
    const char* Version()
    {
        return SAR_VERSION;
    }
    const char* Build()
    {
        return SAR_BUILD;
    }
    const char* Website()
    {
        return SAR_WEB;
    }
    // SAR has to disable itself in the plugin list or the game might crash because of missing callbacks
    // This is a race condition though
    bool GetPlugin()
    {
        static Interface* s_ServerPlugin = Interface::Create(MODULE("engine"), "ISERVERPLUGINHELPERS0", false);
        if (s_ServerPlugin) {
            auto m_Size = *reinterpret_cast<int*>((uintptr_t)s_ServerPlugin->ThisPtr() + CServerPlugin_m_Size);
            if (m_Size > 0) {
                auto m_Plugins = *reinterpret_cast<uintptr_t*>((uintptr_t)s_ServerPlugin->ThisPtr() + CServerPlugin_m_Plugins);
                for (int i = 0; i < m_Size; i++) {
                    auto ptr = *reinterpret_cast<CPlugin**>(m_Plugins + sizeof(uintptr_t) * i);
                    if (!std::strcmp(ptr->m_szName, SAR_PLUGIN_SIGNATURE)) {
                        plugin->ptr = ptr;
                        plugin->index = i;
                        return true;
                    }
                }
            }
        }
        return false;
    }
    void SearchPlugin()
    {
        findPluginThread = std::thread([this]() {
#if _WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
            if (!this->GetPlugin()) {
                console->DevWarning("SAR: Failed to find SAR in the plugin list!\nTry again with \"plugin_load\".\n");
            } else {
                plugin->ptr->m_bDisable = true;
            }
        });
        findPluginThread.detach();
    }
};

SAR* sar;
extern SAR* sar;
