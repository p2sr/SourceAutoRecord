#pragma once
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Client.hpp"

#include "Features/TAS.hpp"

namespace Callbacks
{
	void AddFrameAtTas(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Print("sar_tas_frame_at <frame> [command_to_execute] : Adds command frame to TAS.\n");
			return;
		}

		TAS::AddFrame(atoi(args.At(1)), std::string(args.At(2)));
	}
	void AddFrameAfterTas(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Print("sar_tas_frame_after <frames_to_wait> [command_to_execute] : Adds command frame to TAS.\n");
			return;
		}

		TAS::AddFrame(atoi(args.At(1)), std::string(args.At(2)), true);
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