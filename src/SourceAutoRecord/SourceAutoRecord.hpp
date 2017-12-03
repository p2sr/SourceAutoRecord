#pragma once
#include "minhook/MinHook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoPlayer.hpp"
#include "Modules/DemoRecorder.hpp"
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

	ScanResult cjb, pnt, sst, cdf, str, sdf, stp, spb, pld, dsc;
	ScanResult enc, ggd, crt, ldg, drc, ins, ksb, dpl;
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
		spb = Scan(Patterns::StartPlayback);
		if (!spb.Found) {
			Console::Warning("SAR: %s\n", Patterns::StartPlayback.GetResult());
			return 1;
		}
		pld = Scan(Patterns::PlayDemo);
		if (!pld.Found) {
			Console::Warning("SAR: %s\n", Patterns::PlayDemo.GetResult());
			return 1;
		}
		dsc = Scan(Patterns::Disconnect);
		if (!dsc.Found) {
			Console::Warning("SAR: %s\n", Patterns::Disconnect.GetResult());
			return 1;
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
		drc = Scan(Patterns::DemoRecorderPtr);
		if (!drc.Found) {
			Console::Warning("SAR: %s\n", Patterns::DemoRecorderPtr.GetResult());
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
		dpl = Scan(Patterns::DemoPlayerPtr);
		if (!dpl.Found) {
			Console::Warning("SAR: %s\n", Patterns::DemoPlayerPtr.GetResult());
			return 1;
		}

		Engine::Set(enc.Address, ggd.Address, crt.Address, ldg.Address);
		DemoRecorder::Set(drc.Address);
		DemoPlayer::Set(dpl.Address);
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
		MH_CreateHook(
			reinterpret_cast<LPVOID>(spb.Address),
			Engine::Detour::StartPlayback,
			reinterpret_cast<LPVOID*>(&Engine::Original::StartPlayback)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(pld.Address),
			Engine::Detour::PlayDemo,
			reinterpret_cast<LPVOID*>(&Engine::Original::PlayDemo)
		);
		MH_CreateHook(
			reinterpret_cast<LPVOID>(dsc.Address),
			Engine::Detour::Disconnect,
			reinterpret_cast<LPVOID*>(&Engine::Original::Disconnect)
		);
		auto hook1 = MH_EnableHook(reinterpret_cast<LPVOID>(cjb.Address));
		auto hook2 = MH_EnableHook(reinterpret_cast<LPVOID>(pnt.Address));
		auto hook3 = MH_EnableHook(reinterpret_cast<LPVOID>(sst.Address));
		auto hook4 = MH_EnableHook(reinterpret_cast<LPVOID>(cdf.Address));
		auto hook5 = MH_EnableHook(reinterpret_cast<LPVOID>(str.Address));
		auto hook6 = MH_EnableHook(reinterpret_cast<LPVOID>(sdf.Address));
		auto hook7 = MH_EnableHook(reinterpret_cast<LPVOID>(stp.Address));
		auto hook8 = MH_EnableHook(reinterpret_cast<LPVOID>(spb.Address));
		auto hook9 = MH_EnableHook(reinterpret_cast<LPVOID>(pld.Address));
		auto hook10 = MH_EnableHook(reinterpret_cast<LPVOID>(dsc.Address));
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
		if (hook8 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook8));
			return false;
		}
		if (hook9 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook9));
			return false;
		}
		if (hook10 != MH_OK) {
			Console::Warning("SAR: %s\n", MH_StatusToString(hook10));
			return false;
		}
		return true;
	}
	void RegisterCommands()
	{
		// Rebinding
		sar_bind_save = CreateCommandArgs(
			"sar_bind_save", Callbacks::BindSaveRebinder,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_save <key> [save_name]\n");
		sar_bind_reload = CreateCommandArgs(
			"sar_bind_reload", Callbacks::BindReloadRebinder,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_reload <key> [save_name]\n");
		sar_unbind_save = CreateCommand(
			"sar_unbind_save", Callbacks::UnbindSaveRebinder,
			"Unbinds current save rebinder.\n");
		sar_unbind_reload = CreateCommand(
			"sar_unbind_reload", Callbacks::UnbindReloadRebinder,
			"Unbinds current save-reload rebinder.\n");
		sar_save_flag = CreateString(
			"sar_save_flag", "#SAVE#",
			"Echo message when using sar_binder_save. Default is \"#SAVE#\", a SourceRuns standard. Keep this empty if no echo message should be binded.\n");

		// Others
		sar_time_demo = CreateCommandArgs(
			"sar_time_demo",
			Callbacks::PrintDemoInfo,
			"Parses a demo and prints some information about it.\n");
		sar_session_tick = CreateCommand(
			"sar_session_tick",
			Callbacks::PrintSessionTick,
			"Prints the current tick of the server since it has loaded.\n");
		sar_about = CreateCommand(
			"sar_about",
			Callbacks::PrintAbout,
			"Prints info about this plugin.\n");
		
		// Cheats
		sv_autojump = CreateBoolean(
			"sv_autojump",
			"0",
			"Enables automatic jumping on the server.\n");
		sv_abh = CreateBoolean(
			"sv_abh",
			"0",
			"Enables accelerated back hopping on the server.\n");

		// From the game
		sv_cheats = ConVar("sv_cheats");
		sv_bonus_challenge = ConVar("sv_bonus_challenge");
		sv_accelerate = ConVar("sv_accelerate");
		sv_airaccelerate = ConVar("sv_airaccelerate");
		sv_friction = ConVar("sv_friction");
		sv_maxspeed = ConVar("sv_maxspeed");
		sv_stopspeed = ConVar("sv_stopspeed");
	}
	void LoadAntiCheat() {
		sv_bonus_challenge.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_accelerate.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_airaccelerate.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_friction.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_maxspeed.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_stopspeed.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);

		//sv_bonus_challenge.AddFlag(FCVAR_CHEAT);
		sv_accelerate.AddFlag(FCVAR_CHEAT);
		sv_airaccelerate.AddFlag(FCVAR_CHEAT);
		sv_friction.AddFlag(FCVAR_CHEAT);
		sv_maxspeed.AddFlag(FCVAR_CHEAT);
		sv_stopspeed.AddFlag(FCVAR_CHEAT);
	}
	void LoadPatches() {
		switch (Offsets::Variant) {
		// Portal 2 6879
		case 0:

			// Fix limited help string output in ConVar_PrintDescription
			// Changing 80 to 1024
			const char* thanksForNothingValve = "%-80s - %.1024s\n";

			ScanResult prd = Scan(Patterns::ConVar_PrintDescription);
			Console::DevMsg("SAR: %s\n", Patterns::ConVar_PrintDescription.GetResult());

			if (prd.Found && WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(prd.Address), &thanksForNothingValve, 4, 0)) {
				Console::DevMsg("SAR: Patched ConVar_PrintDescription!\n");
			}

			// Fix executing commands in demo playback
			// Removing Cbuf_AddText and Cbuf_Execute
			BYTE ignoreEvilCommandsInDemos[22] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

			ScanResult rdp = Scan(Patterns::ReadPacket);
			Console::DevMsg("SAR: %s\n", Patterns::ReadPacket.GetResult());
			if (prd.Found && WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(rdp.Address), ignoreEvilCommandsInDemos, 22, 0)) {
				Console::DevMsg("SAR: Patched ReadPacket!\n");
			}

			break;
		}
	}
}