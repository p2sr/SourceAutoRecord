#pragma once
#include "SourceAutoRecord.h"

unsigned __stdcall Main(void* args)
{
	// Get developer console for output
	if (!Console::Init()) return Error("Could not initialize console!", "SourceAutoRecord");

	// Sigscan the necessary functions
	if (!SAR::Setup()) return 1;

	Console::DevMsg("SAR: Found CheckJumpButton at 0x%p in %s using %s\n", SAR::cjb.Address, Patterns::CheckJumpButton.Module, Patterns::CheckJumpButton.Signatures[SAR::cjb.Index].Comment);
	Console::DevMsg("SAR: Found Paint at 0x%p in %s using %s\n", SAR::pnt.Address, Patterns::Paint.Module, Patterns::Paint.Signatures[SAR::pnt.Index].Comment);
	Console::DevMsg("SAR: Found SetSignonState at 0x%p in %s using %s\n", SAR::sst.Address, Patterns::SetSignonState.Module, Patterns::SetSignonState.Signatures[SAR::sst.Index].Comment);

	// Hook all functions
	if (!SAR::InitHooks()) return 1;

	Console::DevMsg("SAR: Enabled hook for CheckJumpButton!\n");
	Console::DevMsg("SAR: Enabled hook for Paint!\n");
	Console::DevMsg("SAR: Enabled hook for SetSignOnState!\n");

	if (!SAR::LoadCvar()) return 1;
	Console::DevMsg("SAR: Found CvarPtr at 0x%p in %s using %s\n", SAR::cvr.Address, Patterns::CvarPtr.Module, Patterns::CvarPtr.Signatures[SAR::cvr.Index].Comment);

	if (!SAR::LoadConVar()) return 1;
	Console::DevMsg("SAR: Found ConVar_Ctor3 at 0x%p in %s using %s\n", SAR::cnv.Address, Patterns::ConVar_Ctor3.Module, Patterns::ConVar_Ctor3.Signatures[SAR::cnv.Index].Comment);

	if (!SAR::LoadConCommand()) return 1;
	Console::DevMsg("SAR: Found ConCommand_Ctor1 at 0x%p in %s using %s\n", SAR::cnc.Address, Patterns::ConCommand_Ctor1.Module, Patterns::ConCommand_Ctor1.Signatures[SAR::cnc.Index].Comment);

	if (!SAR::LoadDrawing()) return 1;
	Console::DevMsg("SAR: Found MatSystemSurfacePtr at 0x%p in %s using %s\n", SAR::mss.Address, Patterns::MatSystemSurfacePtr.Module, Patterns::MatSystemSurfacePtr.Signatures[SAR::mss.Index].Comment);

	SAR::LoadCommands();
	Console::ColorMsg(COL_GREEN, "SourceAutoRecord Version 1.0 (by NeKz)\n");
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