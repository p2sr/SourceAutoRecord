#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Teleporter.hpp"

namespace Callbacks
{
	void Teleport()
	{
		if (sv_cheats.GetBool()) {
			if (Teleporter::IsSet) {
				Engine::ExecuteCommand(Teleporter::GetSetpos().c_str());
				Engine::ExecuteCommand(Teleporter::GetSetang().c_str());
			}
			else {
				Console::Print("Location not set. Use sar_set_teleport.\n");
			}
		}
		else {
			Console::Print("Cannot teleport without sv_cheats 1.\n");
		}
	}
	void SetTeleport()
	{
		Teleporter::Save();
	}
}