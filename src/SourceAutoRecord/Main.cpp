#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Vars.hpp"
#include "Modules/VGui.hpp"

#include "Modules/Tier1.hpp"

#include "Features/Config.hpp"

#include "Callbacks.hpp"
#include "Cheats.hpp"
#include "Interfaces.hpp"
#include "Game.hpp"

int __attribute__((constructor)) Main()
{
	if (!Console::Init()) return 1;

	Game::Init();
	Interfaces::Init();

    if (Tier1::Init()) {

        Cheats::Create();
        Cheats::UnlockAll();

        Client::Hook();
        Engine::Hook();
        InputSystem::Hook();
        Scheme::Hook();
        Server::Hook();
        Surface::Hook();
        Vars::Hook();
        VGui::Hook();

        Config::Load();

        Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
        return 0;
    }
    else {
        Console::Warning("SAR: Could not register any commands!\n");
    }

	Console::Warning("SAR: Failed to load SourceAutoRecord!\n");
	return 1;
}