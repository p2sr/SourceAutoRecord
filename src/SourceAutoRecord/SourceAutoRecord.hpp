#pragma once
#include "minhook/MinHook.h"

#include "Modules/Console.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"

#include "Callbacks.hpp"
#include "Commands.hpp"
#include "Offsets.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace SAR
{
	std::string ExePath;

	ScanResult cjb, pnt, sst, cdf, str, sdf, stp;
	ScanResult enc, ggd, crt, ldg;
	ScanResult rec, cvr, cnv, mss, cnc, cnc2;

	bool LoadHooks()
	{
		cjb = Scan(Patterns::CheckJumpButton);
		if (!cjb.Found) {
			Console::Warning("SAR: %s\n", Patterns::CheckJumpButton.GetResult());
			return false;
		}
		pnt = Scan(Patterns::Paint);
		if (!pnt.Found) {
			Console::Warning("SAR: %s\n", Patterns::Paint.GetResult());
			return false;
		}
		sst = Scan(Patterns::SetSignonState);
		if (!sst.Found) {
			Console::Warning("SAR: %s\n", Patterns::SetSignonState.GetResult());
			return false;
		}
		cdf = Scan(Patterns::CloseDemoFile);
		if (!cdf.Found) {
			Console::Warning("SAR: %s\n", Patterns::CloseDemoFile.GetResult());
			return false;
		}
		str = Scan(Patterns::StopRecording);
		if (!str.Found) {
			Console::Warning("SAR: %s\n", Patterns::StopRecording.GetResult());
			return false;
		}
		sdf = Scan(Patterns::StartupDemoFile);
		if (!sdf.Found) {
			Console::Warning("SAR: %s\n", Patterns::StartupDemoFile.GetResult());
			return false;
		}
		stp = Scan(Patterns::Stop);
		if (!stp.Found) {
			Console::Warning("SAR: %s\n", Patterns::Stop.GetResult());
			return false;
		}

		Offsets::Init(cjb.Index);
		return true;
	}
	bool LoadEngine()
	{
		enc = Scan(Patterns::EngineClientPtr);
		if (!enc.Found) {
			Console::Warning("SAR: %s\n", Patterns::EngineClientPtr.GetResult());
			return 1;
		}
		ggd = Scan(Patterns::GetGameDir);
		if (!ggd.Found) {
			Console::Warning("SAR: %s\n", Patterns::GetGameDir.GetResult());
			return 1;
		}
		crt = Scan(Patterns::CurtimePtr);
		if (!crt.Found) {
			Console::Warning("SAR: %s\n", Patterns::CurtimePtr.GetResult());
			return 1;
		}
		ldg = Scan(Patterns::LoadgamePtr);
		if (!ldg.Found) {
			Console::Warning("SAR: %s\n", Patterns::LoadgamePtr.GetResult());
			return 1;
		}
		rec = Scan(Patterns::Record);
		if (!rec.Found) {
			Console::Warning("SAR: %s\n", Patterns::Record.GetResult());
			return 1;
		}

		Engine::Set(enc.Address, ggd.Address, crt.Address, ldg.Address);
		Recorder::Set(rec.Address);
		return true;
	}
	bool LoadTier1()
	{
		cvr = Scan(Patterns::CvarPtr);
		if (!cvr.Found) {
			Console::Warning("SAR: %s\n", Patterns::CvarPtr.GetResult());
			return 1;
		}
		cnv = Scan(Patterns::ConVar_Ctor3);
		if (!cnv.Found) {
			Console::Warning("SAR: %s\n", Patterns::ConVar_Ctor3.GetResult());
			return 1;
		}
		cnc = Scan(Patterns::ConCommand_Ctor1);
		if (!cnc.Found) {
			Console::Warning("SAR: %s\n", Patterns::ConCommand_Ctor1.GetResult());
			return 1;
		}
		cnc2 = Scan(Patterns::ConCommand_Ctor2);
		if (!cnc.Found) {
			Console::Warning("SAR: %s\n", Patterns::ConCommand_Ctor2.GetResult());
			return 1;
		}

		Cvar::Set(cvr.Address);
		Tier1::SetConVar(cnv.Address);
		Tier1::SetConCommand(cnc.Address, cnc2.Address);
		return true;
	}
	bool LoadRest()
	{
		mss = Scan(Patterns::MatSystemSurfacePtr);
		if (!mss.Found) {
			Console::Warning("SAR: %s\n", Patterns::MatSystemSurfacePtr.GetResult());
			return false;
		}

		Surface::Set(mss.Address);
		return true;
	}
	bool EnableHooks()
	{
		MH_Initialize();
		MH_CreateHook(
			reinterpret_cast<LPVOID>(cjb.Address),
			Server::Detour::CheckJumpButton,
			reinterpret_cast<LPVOID*>(&Server::Original::CheckJumpButton)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(pnt.Address),
			Client::Detour::Paint,
			reinterpret_cast<LPVOID*>(&Client::Original::Paint)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(sst.Address),
			Engine::Detour::SetSignonState,
			reinterpret_cast<LPVOID*>(&Engine::Original::SetSignonState)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(cdf.Address),
			Engine::Detour::CloseDemoFile,
			reinterpret_cast<LPVOID*>(&Engine::Original::CloseDemoFile)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(str.Address),
			Engine::Detour::StopRecording,
			reinterpret_cast<LPVOID*>(&Engine::Original::StopRecording)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(sdf.Address),
			Engine::Detour::StartupDemoFile,
			reinterpret_cast<LPVOID*>(&Engine::Original::StartupDemoFile)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(stp.Address),
			Engine::Detour::ConCommandStop,
			reinterpret_cast<LPVOID*>(&Engine::Original::ConCommandStop)
		);
		auto hook1 = MH_EnableHook(reinterpret_cast<LPVOID>(cjb.Address));
		auto hook2 = MH_EnableHook(reinterpret_cast<LPVOID>(pnt.Address));
		auto hook3 = MH_EnableHook(reinterpret_cast<LPVOID>(sst.Address));
		auto hook4 = MH_EnableHook(reinterpret_cast<LPVOID>(cdf.Address));
		auto hook5 = MH_EnableHook(reinterpret_cast<LPVOID>(str.Address));
		auto hook6 = MH_EnableHook(reinterpret_cast<LPVOID>(sdf.Address));
		auto hook7 = MH_EnableHook(reinterpret_cast<LPVOID>(stp.Address));
		if (hook1 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook1));
			return false;
		}
		if (hook2 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook2));
			return false;
		}
		if (hook3 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook3));
			return false;
		}
		if (hook4 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook4));
			return false;
		}
		if (hook5 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook5));
			return false;
		}
		if (hook6 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook6));
			return false;
		}
		if (hook7 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook7));
			return false;
		}
		return true;
	}
	void RegisterCommands()
	{
		// Save rebinding
		sar_rebinder_save = CreateFloat(
			"sar_rebinder_save",
			"0",
			0,
			1,
			"Automatic save rebinding after loading from a save.\n");
		sar_rebinder_reload = CreateFloat(
			"sar_rebinder_reload",
			"0",
			0,
			1,
			"Automatic save-reload rebinding after loading from a save.\n");
		/*sar_bind_save = CreateString(
			"sar_bind_save",
			"C segment_",
			"Automatic save rebinding after loading from a save. Usage: <key> <save_name>.\n");*/
		sar_bind_reload = CreateString(
			"sar_bind_reload",
			"Q segment_",
			"Automatic save-reload rebinding after loading from a save. Usage: <key> <save_name>.\n");
		sar_bind_save = CreateCommandArgs("sar_bind_save", Callbacks::SetSaveRebind, "Automatic save rebinding after loading from a save. Usage: <key> <save_name>.\n");

		// Commands
		sar_time_demo = CreateCommand(
			"sar_time_demo",
			Callbacks::PrintDemoTime,
			"Prints the tick count of a demo.\n");
		sar_server_tick = CreateCommand(
			"sar_server_tick",
			Callbacks::PrintCurrentTick,
			"Prints the current tick of the server.\n");
		sar_about = CreateCommand(
			"sar_about",
			Callbacks::PrintAbout,
			"Prints info about this plugin.\n");
		
		// Cheats
		sv_autojump = CreateBoolean(
			"sv_autojump",
			"0",
			"Enables automatic jumping on the server (requires sv_cheats).\n");

		// Experimental
		cl_showtime = CreateBoolean(
			"cl_showtime",
			"Draw the current server time.\n");

		// From the game
		_sv_cheats = ConVar("sv_cheats");
		_sv_bonus_challenge = ConVar("sv_bonus_challenge");
	}
}