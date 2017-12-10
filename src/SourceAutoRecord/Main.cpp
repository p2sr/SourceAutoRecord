#pragma once
#include "SourceAutoRecord.hpp"

unsigned __stdcall Main(void* args)
{
	// Get developer console for output
	if (!Console::Init()) return Error("Could not initialize console!", "SourceAutoRecord");

	// Signature scanning
	Hooks::Load();

	Console::DevMsg("SAR: %s\n", Patterns::CheckJumpButton.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Paint.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::SetSignonState.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::CloseDemoFile.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StopRecording.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StartupDemoFile.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Stop.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StartPlayback.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::PlayDemo.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Disconnect.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::StopPlayback.GetResult());

	SAR::LoadEngine();
	Console::DevMsg("SAR: %s\n", Patterns::EngineClientPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::GetGameDir.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::CurtimePtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::LoadgamePtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::DemoRecorderPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::InputSystemPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::Key_SetBinding.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::DemoPlayerPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::MapnamePtr.GetResult());
	
	SAR::LoadTier1();
	Console::DevMsg("SAR: %s\n", Patterns::CvarPtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::ConVar_Ctor3.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::ConCommand_Ctor1.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::ConCommand_Ctor2.GetResult());
	
	SAR::LoadClient();
	Console::DevMsg("SAR: %s\n", Patterns::MatSystemSurfacePtr.GetResult());
	Console::DevMsg("SAR: %s\n", Patterns::SetSize.GetResult());

	// Hook all functions
	Hooks::CreateAndEnable();

	// Plugin commands
	SAR::RegisterCommands();

	// Nobody likes cheaters
	SAR::EnableAntiCheat();

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