#pragma once
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Client.hpp"

#include "Features/TAS.hpp"

namespace Callbacks
{
	void AddTas(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Print("sar_tas_add_frame <frames_to_wait> [command_to_execute] : Experimental.\n");
			return;
		}
		Console::DevMsg("TAS ADD [%i] -> %s\n", atoi(args.At(1)), args.At(2));
		TAS::AddFrame(atoi(args.At(1)), std::string(args.At(2)));
	}
	void DelayTas(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 1) {
			Console::Print("sar_tas_delay <frames_to_wait> : Experimental.\n");
			return;
		}
		TAS::Delay(atoi(args.At(1)));
	}
	void StartTas()
	{
		Console::DevMsg("---TAS START---\n");
		TAS::Start();
	}
	void ResetTas()
	{
		Console::DevMsg("---TAS RESET---\n");
		TAS::Reset();
	}
}