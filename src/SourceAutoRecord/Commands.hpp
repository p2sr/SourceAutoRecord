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
	ConCommand sar_time_demos;
	ConCommand sar_session;
	ConCommand sar_about;

	// Summary
	ConCommand sar_sum_here;
	ConCommand sar_sum_stop;
	ConCommand sar_sum_result;
	ConVar sar_sum_during_session;

	// Cheats
	ConVar sar_autojump;

	// From the game
	ConVar sv_cheats;
	ConVar sv_bonus_challenge;
	ConVar sv_accelerate;
	ConVar sv_airaccelerate;
	ConVar sv_friction;
	ConVar sv_maxspeed;
	ConVar sv_stopspeed;
}