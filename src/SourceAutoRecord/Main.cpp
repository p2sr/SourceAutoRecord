#include "SourceAutoRecord.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoPlayer.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Vars.hpp"
#include "Modules/VGui.hpp"

#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"
#include "Modules/Cvar.hpp"

#include "Callbacks.hpp"
#include "Cheats.hpp"
#include "Commands.hpp"
#include "Interfaces.hpp"
#include "Game.hpp"

int __attribute__((constructor)) Main()
{	
	if (!Game::IsSupported()) return 1;
	if (!Console::Init()) return 1;

	Offsets::Init();
	Patterns::Init();

	if (Cvar::Loaded() && Tier1::ConCommandLoaded() && Tier1::ConVarLoaded()) {

		Cheats::Create();
		Cheats::UnlockAll();

		Interfaces::Load();

		Client::Hook();
		Engine::Hook();
		InputSystem::Hook();
		Scheme::Hook();
		Server::Hook();
		Surface::Hook();
		Vars::Hook();
		VGui::Hook();
		
		Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
		return 0;
	}
	else {
		Console::DevWarning("Could not register any commands!\n");
	}
	Console::Warning("Failed to load SourceAutoRecord!\n");
	return 1;
}