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
#include "Hooks.hpp"
#include "Offsets.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace SAR
{
	ScanResult enc, ggd, crt, ldg, drc, ins, ksb, dpl, mpn;
	ScanResult cvr, cnv, cnc, cnc2;
	ScanResult mss;

	void LoadEngine()
	{
		enc = Scan(Patterns::EngineClientPtr);
		ggd = Scan(Patterns::GetGameDir);
		crt = Scan(Patterns::CurtimePtr);
		ldg = Scan(Patterns::LoadgamePtr);
		drc = Scan(Patterns::DemoRecorderPtr);
		ins = Scan(Patterns::InputSystemPtr);
		ksb = Scan(Patterns::Key_SetBinding);
		dpl = Scan(Patterns::DemoPlayerPtr);
		mpn = Scan(Patterns::MapnamePtr);

		Engine::Set(enc.Address, ggd.Address, crt.Address, ldg.Address, mpn.Address);
		DemoRecorder::Set(drc.Address);
		InputSystem::Set(ins.Address, ksb.Address);
		DemoPlayer::Set(dpl.Address);
	}
	void LoadTier1()
	{
		cvr = Scan(Patterns::CvarPtr);
		cnv = Scan(Patterns::ConVar_Ctor3);
		cnc = Scan(Patterns::ConCommand_Ctor1);
		cnc2 = Scan(Patterns::ConCommand_Ctor2);

		Cvar::Set(cvr.Address);
		Tier1::SetConVar(cnv.Address);
		Tier1::SetConCommand(cnc.Address, cnc2.Address);
	}
	void LoadRest()
	{
		mss = Scan(Patterns::MatSystemSurfacePtr);

		Surface::Set(mss.Address);
	}
	void RegisterCommands()
	{
		// Rebinder
		sar_bind_save = CreateCommandArgs(
			"sar_bind_save", Callbacks::BindSaveRebinder,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_save <key> [save_name]\n");
		sar_bind_reload = CreateCommandArgs(
			"sar_bind_reload", Callbacks::BindReloadRebinder,
			"Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_reload <key> [save_name]\n");
		sar_unbind_save = CreateCommand(
			"sar_unbind_save", Callbacks::UnbindSaveRebinder,
			"Unbinds current save rebinder.\n");
		sar_unbind_reload = CreateCommand(
			"sar_unbind_reload", Callbacks::UnbindReloadRebinder,
			"Unbinds current save-reload rebinder.\n");
		sar_save_flag = CreateString(
			"sar_save_flag", "#SAVE#",
			"Echo message when using sar_binder_save. Default is \"#SAVE#\", a SourceRuns standard. Keep this empty if no echo message should be binded.\n");

		// Info
		sar_time_demo = CreateCommandArgs(
			"sar_time_demo",
			Callbacks::PrintDemoInfo,
			"Parses a demo and prints some information about it.\n");
		sar_time_demos = CreateCommandArgs(
			"sar_time_demos",
			Callbacks::PrintDemoInfos,
			"Parses multiple demos and prints the total sum of them.\n");
		sar_session_tick = CreateCommand(
			"sar_session_tick",
			Callbacks::PrintSessionTick,
			"Prints the current tick of the server since it has loaded.\n");
		sar_about = CreateCommand(
			"sar_about",
			Callbacks::PrintAbout,
			"Prints info about this plugin.\n");

		// Summary
		sar_sum_here = CreateCommand(
			"sar_sum_here",
			Callbacks::StartSummary,
			"Starts counting total ticks of sessions.\n");
		sar_sum_reset = CreateCommand(
			"sar_sum_reset",
			Callbacks::ResetSummary,
			"Stops current running summary counter and resets.\n");
		sar_sum_result = CreateCommand(
			"sar_sum_result",
			Callbacks::PrintSummary,
			"Prints result of summary.\n");

		// Cheats
		sar_autojump = CreateBoolean(
			"sar_autojump",
			"0",
			"Enables automatic jumping on the server.\n");

		// From the game
		sv_cheats = ConVar("sv_cheats");
		sv_bonus_challenge = ConVar("sv_bonus_challenge");
		sv_accelerate = ConVar("sv_accelerate");
		sv_airaccelerate = ConVar("sv_airaccelerate");
		sv_friction = ConVar("sv_friction");
		sv_maxspeed = ConVar("sv_maxspeed");
		sv_stopspeed = ConVar("sv_stopspeed");
	}
	void LoadAntiCheat()
	{
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
	void LoadPatches()
	{
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