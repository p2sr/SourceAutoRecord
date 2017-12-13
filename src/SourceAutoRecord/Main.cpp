#pragma once
#include "SourceAutoRecord.hpp"

unsigned __stdcall Main(void* args)
{
	if (!Offsets::Init()) return Error("Game not supported!", "SourceAutoRecord");
	if (!Console::Init()) return Error("Could not initialize console!", "SourceAutoRecord");

	// Signature scans and hooks
	if (SAR::LoadTier1()) {
		SAR::RegisterCommands();
		SAR::EnableGameCheats();

		if (SAR::LoadClient() && SAR::LoadEngine()) {
			Hooks::Load();
		}
		else {
			Console::DevWarning("Could not load any hooks!\n");
		}
	}
	else {
		Console::DevWarning("Could not register any commands!\n");
	}

	// Nobody likes silly bugs
	SAR::LoadPatches();
		
	Console::ColorMsg(COL_GREEN, "Loaded SourceAutoRecord, Version %s (by NeKz)\n", SAR_VERSION);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(module);
		CreateThread(0, 0, LPTHREAD_START_ROUTINE(Main), 0, 0, 0);
	}
	return TRUE;
}