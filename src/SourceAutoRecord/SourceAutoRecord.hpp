#pragma once
#include "minhook/MinHook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
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
	ScanResult enc, ggd, crt, ldg, rec, ins, ksb;
	ScanResult cvr, cnv, mss, cnc, cnc2;

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
		ins = Scan(Patterns::InputSystemPtr);
		if (!ins.Found) {
			Console::Warning("SAR: %s\n", Patterns::InputSystemPtr.GetResult());
			return 1;
		}
		ksb = Scan(Patterns::Key_SetBinding);
		if (!ksb.Found) {
			Console::Warning("SAR: %s\n", Patterns::Key_SetBinding.GetResult());
			return 1;
		}

		Engine::Set(enc.Address, ggd.Address, crt.Address, ldg.Address);
		Recorder::Set(rec.Address);
		InputSystem::Set(ins.Address, ksb.Address);
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
		// Rebinder feature
		sar_rebinder_save = CreateFloat(
			"sar_rebinder_save", "0", 0, 1,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos.\n");
		sar_rebinder_reload = CreateFloat(
			"sar_rebinder_reload", "0", 0, 1,
			"Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos.\n");

		// Binding commands
		sar_bind_save = CreateCommandArgs(
			"sar_bind_save", Callbacks::SetSaveRebind,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_save <key> [save_name]\n");
		sar_bind_reload = CreateCommandArgs(
			"sar_bind_reload", Callbacks::SetReloadRebind,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_reload <key> [save_name]\n");

		// Optional binding config
		sar_save_flag = CreateString(
			"sar_save_flag", "#SAVE#",
			"Echo message when using sar_rebinder_save and sar_bind_save. Default is \"#SAVE#\", a SourceRuns standard. Keep this empty if no echo message should be binded.\n");

		// Others
		sar_time_demo = CreateCommandArgs(
			"sar_time_demo",
			Callbacks::PrintDemoTime,
			"Parses a demo and prints some information about it.\n");
		sar_session_tick = CreateCommand(
			"sar_session_tick",
			Callbacks::PrintCurrentTick,
			"Prints the current tick of the server since it has loaded.\n");
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
		/*cl_showtime = CreateBoolean(
			"cl_showtime",
			"Draw the current server time.\n");*/

		// From the game
		sv_cheats = ConVar("sv_cheats");
		sv_bonus_challenge = ConVar("sv_bonus_challenge");
		sv_accelerate = ConVar("sv_accelerate");
		sv_airaccelerate = ConVar("sv_airaccelerate");
		sv_friction = ConVar("sv_friction");
		sv_maxspeed = ConVar("sv_maxspeed");
		sv_stopspeed = ConVar("sv_stopspeed");

		// Even more cheats
		sv_bonus_challenge.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_accelerate.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_airaccelerate.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_friction.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_maxspeed.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_stopspeed.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
	}
}