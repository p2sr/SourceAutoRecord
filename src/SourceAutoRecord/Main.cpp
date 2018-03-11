#include "SourceAutoRecord.hpp"

int __attribute__((constructor)) Main()
{	
	if (!Game::IsSupported()) return 1;
	if (!Console::Init()) return 1;

	Offsets::Init();
	Patterns::Init();

	// ConCommand and ConVar
	if (SAR::LoadTier1()) {

		// Cheats
		SAR::CreateCommands();
		SAR::EnableGameCheats();

		/* for (MODULEINFO& item: Cache::Modules) {
			Console::PrintActive("%s -> %p (%i)\n", item.moduleName, item.lpBaseOfDll, item.SizeOfImage);
		} */

		//SAR::LoadPatches();

		Hooks::Load();
		Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
		return 0;

		// Hooks
		/* if (SAR::LoadClient() && SAR::LoadEngine()) {
			//Hooks::CreateAll();

			// Nobody likes silly bugs
			//SAR::LoadPatches();

			//Hooks::InstallAll();

			Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
			return 0;
		}
		else {
			Console::DevWarning("Could not hook any functions!\n");
		} */
	}
	else {
		Console::DevWarning("Could not register any commands!\n");
	}
	Console::Warning("Failed to load SourceAutoRecord!\n");
	return 1;
}