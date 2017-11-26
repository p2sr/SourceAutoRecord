#pragma once
#include "minhook/MinHook.h"

#include "Modules/Console.h"
#include "Modules/Client.h"
#include "Modules/Engine.h"
#include "Modules/Server.h"

#include "Modules/ConCommand.h"
#include "Modules/ConVar.h"

#include "Callbacks.h"
#include "Commands.h"
#include "Offsets.h"
#include "Patterns.h"
#include "Utils.h"

using namespace Commands;
using namespace Tier1;

namespace SAR
{
	ScanResult cjb;
	ScanResult pnt;
	ScanResult sst;
	ScanResult cvr;
	ScanResult cnv;
	ScanResult mss;
	ScanResult cnc;

	bool Setup()
	{
		cjb = Scan(Patterns::CheckJumpButton);
		if (!cjb.Found) {
			Console::Warning("SAR: Could not find the CheckJumpButton function!\n");
			return false;
		}
		pnt = Scan(Patterns::Paint);
		if (!pnt.Found) {
			Console::Warning("SAR: Could not find the Paint function!\n");
			return false;
		}
		sst = Scan(Patterns::SetSignonState);
		if (!sst.Found) {
			Console::Warning("SAR: Could not find the SetSignonState function!\n");
			return false;
		}
		Offsets::Init(cjb.Index);
		return true;
	}
	bool InitHooks()
	{
		MH_Initialize();
		MH_CreateHook
		(
			reinterpret_cast<LPVOID>(cjb.Address),
			Server::Detour_CheckJumpButton,
			reinterpret_cast<LPVOID*>(&Server::Original_CheckJumpButton)
		);
		MH_CreateHook
		(
			reinterpret_cast<LPVOID>(pnt.Address),
			Client::Detour_Paint,
			reinterpret_cast<LPVOID*>(&Client::Original_Paint)
		);
		MH_CreateHook
		(
			reinterpret_cast<LPVOID>(sst.Address),
			Engine::Detour_SetSignonState,
			reinterpret_cast<LPVOID*>(&Engine::Original_SetSignonState)
		);
		auto hook1 = MH_EnableHook(reinterpret_cast<LPVOID>(cjb.Address));
		auto hook2 = MH_EnableHook(reinterpret_cast<LPVOID>(pnt.Address));
		auto hook3 = MH_EnableHook(reinterpret_cast<LPVOID>(sst.Address));
		if (hook1 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook1));
			return false;
		}
		if (hook2 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook2));
			return false;
		}
		if (hook2 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook2));
			return false;
		}
		return true;
	}
	bool LoadCvar()
	{
		cvr = Scan(Patterns::CvarPtr);
		if (!cvr.Found) {
			Console::Warning("SAR: Could not find the Cvar pointer!\n");
			return 1;
		}
		Cvar::Set(cvr.Address);
		return true;
	}
	bool LoadConVar()
	{
		cnv = Scan(Patterns::ConVar_Ctor3);
		if (!cnv.Found) {
			Console::Warning("SAR: Could not find the ConVar constructor!\n");
			return 1;
		}
		InitConVar(cnv.Address);
		return true;
	}
	bool LoadConCommand()
	{
		cnc = Scan(Patterns::ConCommand_Ctor1);
		if (!cnc.Found) {
			Console::Warning("SAR: Could not find the ConCommand constructor!\n");
			return 1;
		}
		InitConCommand(cnc.Address);
		return true;
	}
	bool LoadDrawing()
	{
		mss = Scan(Patterns::MatSystemSurfacePtr);
		if (!mss.Found) {
			Console::Warning("SAR: Could not find the MatSystemSurface pointer!\n");
			return false;
		}
		Surface::Set(mss.Address);
		return true;
	}
	void LoadCommands()
	{
		// Demo recording
		AutoRecord = CreateFloat("sar_record", "0", 0, "Automatic demo recording after loading from a save.\n0 => Off.\n1 => On.\n2 => On and print time when finished.\n");
		DemoName = CreateString("sar_demo_name", "demo_", "Default demo name.\n");
		DemoStartIndex = CreateFloat("sar_demo_start_index", "0", 0, "Default demo start index for automatic demo recording.");

		// Save rebinding
		AutoSave = CreateString("sar_bind_save", "C segment_", "Usage: <key> <save_name>. Automatic save rebinding after loading from a save.\n");
		AutoSaveReload = CreateString("sar_bind_reload", "Q segment", "Usage: <key> <save_name>. Automatic save-reload rebinding after loading from a save.\n");

		// Demo parsing
		TimeDemo = CreateCommand("sar_time_demo", Callbacks::PrintDemoTime, "Prints the tick count of a demo.\n");
		
		// Cheats
		SvCheats = ConVar("sv_cheats");
		AutoJump = CreateBoolean("sv_autojump", "0", "Enables automatic jumping on the server (requires sv_cheats).\n");
		ABH = CreateBoolean("sv_abh", "0", "Enables automatic jumping on the server (requires sv_cheats).\n");

		// Drawing
		ShowTime = CreateBoolean("cl_showtime", "Draw the current server time.\n");
	}
}