#pragma once
#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"

using namespace Tier1;

namespace Commands
{
	// Rebinder
	ConVar sar_rebinder_save;
	ConVar sar_rebinder_reload;
	ConCommand sar_bind_save;
	ConCommand sar_bind_reload;
	ConVar sar_save_flag;

	// Info
	ConCommand sar_time_demo;
	ConCommand sar_session_tick;
	ConCommand sar_about;

	// Experimental
	//ConVar cl_showtime;

	// Cheats
	ConVar sv_autojump;

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