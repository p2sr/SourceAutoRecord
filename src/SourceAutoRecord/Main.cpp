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
#include "Game.hpp"
#include "Interfaces.hpp"

unsigned __stdcall Main(void* args)
{
    if (!Console::Init())
        return 1;

    Interfaces::Init();
    Game::Init();

    if (Tier1::Init()) {

        Cheats::Create();
        Cheats::UnlockAll();

        Client::Hook();
        Engine::Hook();
        InputSystem::Hook();
        Scheme::Hook();
        Server::Hook();
        Surface::Hook();
        VGui::Hook();

        Config::Load();

        Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
        return 0;
    } else {
        Console::Warning("SAR: Could not register any commands!\n");
    }

    Console::Warning("SAR: Failed to load SourceAutoRecord!\n");
    return 1;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        CreateThread(0, 0, LPTHREAD_START_ROUTINE(Main), 0, 0, 0);
    }
    return TRUE;
}