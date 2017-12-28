#pragma once
#include "SourceAutoRecord.hpp"

unsigned __stdcall Main(void* args)
{
	if (!Game::IsSupported()) return Error("Game not supported!", "SourceAutoRecord");
	if (!Console::Init()) return Error("Could not initialize console!", "SourceAutoRecord");

	Offsets::Init();
	Patterns::Init();

	// ConCommand and ConVar
	if (SAR::LoadTier1()) {

		// Cheats
		SAR::CreateCommands();
		SAR::EnableGameCheats();

		// Hooks
		if (SAR::LoadClient() && SAR::LoadEngine()) {
			Hooks::CreateAll();

			// Nobody likes silly bugs
			SAR::LoadPatches();

			Hooks::EnableAll();

			Console::PrintActive("Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
			return 0;
		}
		else {
			Console::DevWarning("Could not hook any functions!\n");
		}
	}
	else {
		Console::DevWarning("Could not register any commands!\n");
	}
	Console::Warning("Failed to load SourceAutoRecord!\n");
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