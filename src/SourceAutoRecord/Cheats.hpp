#pragma once
#include "Command.hpp"
#include "Game.hpp"
#include "Variable.hpp"

namespace Cheats {

Variable sar_autorecord("sar_autorecord", "0", "Enables automatic demo recording.\n");
Variable sar_save_flag("sar_save_flag", "#SAVE#", "Echo message when using sar_bind_save. "
    "Default is \"#SAVE#\", a SourceRuns standard. Keep this empty if no echo message should be binded.\n", 0);
Variable sar_time_demo_dev("sar_time_demo_dev", "0", 0, "Printing mode when using sar_time_demo. "
    "0 = default, 1 = console commands, 2 = console commands & packets.\n");
Variable sar_sum_during_session("sar_sum_during_session", "1", "Updates the summary counter automatically during a session.\n");
Variable sar_timer_always_running("sar_timer_always_running", "1", "Timer will save current value when disconnecting.\n");
Variable sar_hud_text("sar_hud_text", "", "Draws specified text when not empty.\n", 0);
Variable sar_hud_position("sar_hud_position", "0", 0, "Draws absolute position of the client.\n");
Variable sar_hud_angles("sar_hud_angles", "0", "Draws absolute view angles of the client.\n");
Variable sar_hud_velocity("sar_hud_velocity", "0", 0, "Draws velocity of the client. 0 = default, 1 = x/y/z , 2 = x/y\n");
Variable sar_hud_session("sar_hud_session", "0", "Draws current session value.\n");
Variable sar_hud_last_session("sar_hud_last_session", "0", "Draws value of latest completed session.\n");
Variable sar_hud_sum("sar_hud_sum", "0", "Draws summary value of sessions.\n");
Variable sar_hud_timer("sar_hud_timer", "0", "Draws current value of timer.\n");
Variable sar_hud_avg("sar_hud_avg", "0", "Draws calculated average of timer.\n");
Variable sar_hud_cps("sar_hud_cps", "0", "Draws latest checkpoint of timer.\n");
Variable sar_hud_demo("sar_hud_demo", "0", "Draws name, tick and time of current demo.\n");
Variable sar_hud_jumps("sar_hud_jumps", "0", "Draws total jump count.\n");
Variable sar_hud_portals("sar_hud_portals", "0", "Draws total portal count.\n");
Variable sar_hud_steps("sar_hud_steps", "0", "Draws total step count.\n");
Variable sar_hud_jump("sar_hud_jump", "0", "Draws current jump distance.\n");
Variable sar_hud_jump_peak("sar_hud_jump_peak", "0", "Draws longest jump distance.\n");
Variable sar_hud_trace("sar_hud_trace", "0", "Draws distance values of tracer.\n");
Variable sar_hud_velocity_peak("sar_hud_velocity_peak", "0", "Draws last saved velocity peak.\n");
Variable sar_hud_default_spacing("sar_hud_default_spacing", "4", 0, "Spacing between elements of HUD.\n");
Variable sar_hud_default_padding_x("sar_hud_default_padding_x", "2", 0, "X padding of HUD.\n");
Variable sar_hud_default_padding_y("sar_hud_default_padding_y", "2", 0, "Y padding of HUD.\n");
Variable sar_hud_default_font_index("sar_hud_default_font_index", "0", 0, "Font index of HUD.\n");
Variable sar_hud_default_font_color("sar_hud_default_font_color", "255 255 255 255", "RGBA font color of HUD.\n", 0);
Variable sar_stats_jumps_xy("sar_stats_jumps_xy", "0", "Saves jump distance as 2D vector.\n");
Variable sar_stats_velocity_peak_xy("sar_stats_velocity_peak_xy", "0", "Saves velocity peak as 2D vector.\n");
Variable sar_stats_auto_reset("sar_stats_auto_reset", "0", 0, "Resets all stats automatically. "
    "0 = default, 1 = restart or disconnect only, 2 = any load & sar_timer_start. "
    "Note: Portal counter is not part of the \"stats\" feature.\n");
Variable sar_autojump("sar_autojump", "0", "Enables automatic jumping on the server.\n");
Variable sar_jumpboost("sar_jumpboost", "0", 0, "Enables special game movement on the server. "
    "0 = Default, 1 = Orange Box Engine, 2 = Pre-OBE\n");
Variable sar_aircontrol("sar_aircontrol", "0",
#ifdef _WIN32
    0,
#endif
    "Enables more air-control on the server.\n");
Variable sar_disable_challenge_stats_hud("sar_disable_challenge_stats_hud", "0", "Disables opening the challenge mode stats HUD.\n");
Variable sar_tas_autostart("sar_tas_autostart", "1", "Starts queued commands automatically on first frame after a load.\n");

Variable cl_showpos;
Variable sv_cheats;
Variable sv_footsteps;
Variable sv_bonus_challenge;
Variable sv_accelerate;
Variable sv_airaccelerate;
Variable sv_friction;
Variable sv_maxspeed;
Variable sv_stopspeed;
Variable sv_maxvelocity;
Variable sv_transition_fade_time;
Variable sv_laser_cube_autoaim;
Variable ui_loadingscreen_transition_time;
Variable hide_gun_when_holding;

// The Stanley Parable
DECLARE_AUTOCOMPLETION_FUNCTION(map, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2, "maps", bsp);

void Init()
{
    sar_jumpboost.UniqueFor(Game::IsPortal2Engine);
    sar_aircontrol.UniqueFor(Game::IsPortal2Engine);
    sar_disable_challenge_stats_hud.UniqueFor(Game::HasChallengeMode);
    sar_hud_portals.UniqueFor(Game::IsPortalGame);

    cl_showpos = Variable("cl_showpos");
    sv_cheats = Variable("sv_cheats");
    sv_footsteps = Variable("sv_footsteps");
    sv_bonus_challenge = Variable("sv_bonus_challenge");
    sv_accelerate = Variable("sv_accelerate");
    sv_airaccelerate = Variable("sv_airaccelerate");
    sv_friction = Variable("sv_friction");
    sv_maxspeed = Variable("sv_maxspeed");
    sv_stopspeed = Variable("sv_stopspeed");
    sv_maxvelocity = Variable("sv_maxvelocity");

    sv_accelerate.Unlock();
    sv_airaccelerate.Unlock();
    sv_friction.Unlock();
    sv_maxspeed.Unlock();
    sv_stopspeed.Unlock();
    sv_maxvelocity.Unlock();
    sv_footsteps.Unlock();

    if (Game::Version == Game::Portal2) {
        sv_transition_fade_time = Variable("sv_transition_fade_time");
        sv_laser_cube_autoaim = Variable("sv_laser_cube_autoaim");
        ui_loadingscreen_transition_time = Variable("ui_loadingscreen_transition_time");
        hide_gun_when_holding = Variable("hide_gun_when_holding");

        // Don't find a way to abuse this, ok?
        sv_bonus_challenge.Unlock(false);
        sv_transition_fade_time.Unlock();
        sv_laser_cube_autoaim.Unlock();
        ui_loadingscreen_transition_time.Unlock();
        // Not a real cheat, right?
        hide_gun_when_holding.Unlock(false);
    } else if (Game::Version == Game::HalfLife2) {
        // Detecting Portal game really late but it adds nothing really
        auto sv_portal_debug_touch = Variable("sv_portal_debug_touch");
        if (sv_portal_debug_touch.GetPtr()) {
            Game::Version = Game::Portal;
            Console::DevMsg("SAR: Detected Portal version!\n");
        }
    } else if (Game::Version == Game::TheStanleyParable) {
        ACTIVATE_AUTOCOMPLETEFILE(map);
        ACTIVATE_AUTOCOMPLETEFILE(changelevel);
        ACTIVATE_AUTOCOMPLETEFILE(changelevel2);
    }
}
void Unload()
{
    sv_accelerate.Lock();
    sv_airaccelerate.Lock();
    sv_friction.Lock();
    sv_maxspeed.Lock();
    sv_stopspeed.Lock();
    sv_maxvelocity.Lock();
    sv_footsteps.Lock();

    if (Game::Version == Game::Portal2) {
        sv_bonus_challenge.Lock();
        sv_transition_fade_time.Lock();
        sv_laser_cube_autoaim.Lock();
        ui_loadingscreen_transition_time.Lock();
        hide_gun_when_holding.Lock();
    }
}
}