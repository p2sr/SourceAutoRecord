#pragma once
#include "SourceAutoRecord.hpp"

unsigned __stdcall Main(void* args)
{
	// Get developer console for output
	if (!Console::Init()) return Error("Could not initialize console!", "SourceAutoRecord");

	// Signature scanning
	if (!SAR::LoadHooks()) return 1;

	Console::DevMsg("SAR: %s\n", Patterns::CheckJumpButton.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Paint.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::SetSignonState.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::CloseDemoFile.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StopRecording.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StartupDemoFile.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Stop.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StartPlayback.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::PlayDemo.GetResult());

	if (!SAR::LoadEngine()) return 1;
	Console::DevMsg("SAR: %s\n", Patterns::EngineClientPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::GetGameDir.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::CurtimePtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::LoadgamePtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::DemoRecorderPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::InputSystemPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Key_SetBinding.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::DemoPlayerPtr.GetResult());
	
	if (!SAR::LoadTier1()) return 1;
	Console::DevMsg("SAR: %s\n", Patterns::CvarPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::ConVar_Ctor3.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::ConCommand_Ctor1.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::ConCommand_Ctor2.GetResult());
	
	if (!SAR::LoadRest()) return 1;
	Console::DevMsg("SAR: %s\n", Patterns::MatSystemSurfacePtr.GetResult());

	// Hook all functions
	if (!SAR::EnableHooks()) return 1;
	Console::DevMsg("SAR: Enabled hook for CheckJumpButton!\n");
	Console::DevMsg("SAR: Enabled hook for Paint!\n");
	Console::DevMsg("SAR: Enabled hook for SetSignOnState!\n");
	Console::DevMsg("SAR: Enabled hook for CloseDemoFile!\n");

	// Plugin commands
	SAR::RegisterCommands();

	// Sorry, not sorry
	SAR::LoadAntiCheat();

	// Game patches because nobody likes silly bugs
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