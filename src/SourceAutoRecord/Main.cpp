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

#include "Features/Config.hpp"

#include "Callbacks.hpp"
#include "Cheats.hpp"
#include "Commands.hpp"
#include "Interfaces.hpp"
#include "Game.hpp"

int __attribute__((constructor)) Main()
{
	if (!Console::Init()) return 1;

	if (Game::IsSupported()) {
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

			Config::Load();

			Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
			return 0;
		}
		else {
			Console::Warning("SAR: Could not register any commands!\n");
		}
	}
	else {
		Console::Warning("SAR: Game not supported!\n");
	}

	Console::Warning("SAR: Failed to load SourceAutoRecord!\n");
	return 1;
}