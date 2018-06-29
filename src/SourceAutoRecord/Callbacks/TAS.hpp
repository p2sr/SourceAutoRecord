#pragma once
#include "Modules/Client.hpp"
#include "Modules/Tier1.hpp"

#include "Features/TAS.hpp"

namespace Callbacks
{
	void AddFrameAtTas(const CCommand& args)
	{
		if (args.ArgC() != 3) {
			Console::Print("sar_tas_frame_at <frame> [command_to_execute] : Adds command frame to TAS.\n");
			return;
		}

		TAS::AddFrame(atoi(args[1]), std::string(args[2]));
	}
	void AddFrameAfterTas(const CCommand& args)
	{
		if (args.ArgC() != 3) {
			Console::Print("sar_tas_frame_after <frames_to_wait> [command_to_execute] : Adds command frame to TAS.\n");
			return;
		}

		TAS::AddFrame(atoi(args[1]), std::string(args[2]), true);
	}
	void StartTas()
	{
		TAS::Start();
	}
	void ResetTas()
	{
		TAS::Reset();
	}
}