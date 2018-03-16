#include "SourceAutoRecord.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoPlayer.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Server.hpp"

#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"
#include "Modules/Cvar.hpp"

#include "Callbacks.hpp"
#include "Cheats.hpp"
#include "Commands.hpp"
#include "Game.hpp"
#include "Hooks.hpp"

int __attribute__((constructor)) Main()
{	
	if (!Game::IsSupported()) return 1;
	if (!Console::Init()) return 1;

	Offsets::Init();
	Patterns::Init();

	// ConCommand and ConVar
	if (Hooks::LoadTier1()) {

		/* for (MODULEINFO& item: Cache::Modules) {
			Console::PrintActive("%s -> %p (%i)\n", item.moduleName, item.lpBaseOfDll, item.SizeOfImage);
		} */

		Cheats::Create();
		Cheats::UnlockAll();
		Hooks::Load();
		
		Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
		return 0;
	}
	else {
		Console::DevWarning("Could not register any commands!\n");
	}
	Console::Warning("Failed to load SourceAutoRecord!\n");
	return 1;
}