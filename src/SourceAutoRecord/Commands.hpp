#pragma once
#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"

using namespace Tier1;

namespace Commands
{
	// Rebinder
	ConCommand sar_bind_save;
	ConCommand sar_bind_reload;
	ConCommand sar_unbind_save;
	ConCommand sar_unbind_reload;
	ConVar sar_save_flag;

	// Info
	ConCommand sar_time_demo;
	ConCommand sar_session_tick;
	ConCommand sar_about;

	// Cheats
	ConVar sv_autojump;
	ConVar sv_abh;

	// Anti-Cheat
	ConVar sv_cheats;
	ConVar sv_bonus_challenge;

	// Others
	ConVar sv_accelerate;
	ConVar sv_airaccelerate;
	ConVar sv_friction;
	ConVar sv_maxspeed;
	ConVar sv_stopspeed;
}