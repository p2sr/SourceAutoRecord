#include "Cheats.hpp"

#include <cstring>

#include "Features/Cvars.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Hud/InspectionHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Imitator.hpp"
#include "Features/Listener.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Tas/AutoStrafer.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/Tas/TasTools.hpp"
#include "Features/WorkshopList.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Variable sar_autorecord("sar_autorecord", "0", "Enables automatic demo recording for loading a save.\n");
Variable sar_autojump("sar_autojump", "0", "Enables automatic jumping on the server.\n");
Variable sar_jumpboost("sar_jumpboost", "0", 0, "Enables special game movement on the server.\n"
                                                "0 = Default,\n"
                                                "1 = Orange Box Engine,\n"
                                                "2 = Pre-OBE.\n");
Variable sar_aircontrol("sar_aircontrol", "0", 0, "Enables more air-control on the server.\n");
Variable sar_duckjump("sar_duckjump", "0", "Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.\n");
Variable sar_disable_challenge_stats_hud("sar_disable_challenge_stats_hud", "0", "Disables opening the challenge mode stats HUD.\n");

// TSP only
void IN_BhopDown(const CCommand& args) { client->KeyDown(client->in_jump, (args.ArgC() > 1) ? args[1] : nullptr); }
void IN_BhopUp(const CCommand& args) { client->KeyUp(client->in_jump, (args.ArgC() > 1) ? args[1] : nullptr); }

Command startbhop("+bhop", IN_BhopDown, "Client sends a key-down event for the in_jump state.\n");
Command endbhop("-bhop", IN_BhopUp, "Client sends a key-up event for the in_jump state.\n");

CON_COMMAND(sar_anti_anti_cheat, "Sets sv_cheats to 1.\n")
{
    sv_cheats.ThisPtr()->m_nValue = 1;
}

// TSP & TBG only
DECLARE_AUTOCOMPLETION_FUNCTION(map, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2, "maps", bsp);

// P2 only
CON_COMMAND(sar_togglewait, "Enables or disables \"wait\" for the command buffer.\n")
{
    auto state = !*engine->m_bWaitEnabled;
    *engine->m_bWaitEnabled = *engine->m_bWaitEnabled2 = state;
    console->Print("%s wait!\n", (state) ? "Enabled" : "Disabled");
}

// P2 and Half-Life 2 Engine only
CON_COMMAND(sar_delete_alias_cmds, "Deletes all alias commands.\n")
{
    if (!engine->cmd_alias->next) {
        return console->Print("Nothing to delete.\n");
    }

    auto count = 0;
    auto cur = engine->cmd_alias->next;
    do {
        auto next = cur->next;
        // Better than valve because no mem-leak :^)
        delete[] cur->value;
        delete cur;
        cur = next;
        ++count;
    } while (cur);

    engine->cmd_alias->next = nullptr;

    console->Print("Deleted %i alias commands!\n", count);
}

void Cheats::Init()
{
    cl_showpos = Variable("cl_showpos");
    sv_cheats = Variable("sv_cheats");
    sv_footsteps = Variable("sv_footsteps");
    sv_alternateticks = Variable("sv_alternateticks");
    sv_bonus_challenge = Variable("sv_bonus_challenge");
    sv_accelerate = Variable("sv_accelerate");
    sv_airaccelerate = Variable("sv_airaccelerate");
    sv_friction = Variable("sv_friction");
    sv_maxspeed = Variable("sv_maxspeed");
    sv_stopspeed = Variable("sv_stopspeed");
    sv_maxvelocity = Variable("sv_maxvelocity");
    sv_edgefriction = Variable("sv_edgefriction");
    cl_sidespeed = Variable("cl_sidespeed");
    cl_forwardspeed = Variable("cl_forwardspeed");
    host_framerate = Variable("host_framerate");

    if (sar.game->version & SourceGame_Portal2Game) {
        sv_transition_fade_time = Variable("sv_transition_fade_time");
        sv_laser_cube_autoaim = Variable("sv_laser_cube_autoaim");
        ui_loadingscreen_transition_time = Variable("ui_loadingscreen_transition_time");
        hide_gun_when_holding = Variable("hide_gun_when_holding");
    } else if (sar.game->version & (SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide)) {
        Command::ActivateAutoCompleteFile("map", map_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel", changelevel_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel2", changelevel2_CompletionFunc);
    }

    sar_jumpboost.UniqueFor(SourceGame_Portal2Engine);
    sar_aircontrol.UniqueFor(SourceGame_Portal2Engine);
    sar_hud_portals.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_disable_challenge_stats_hud.UniqueFor(SourceGame_Portal2);
    sar_debug_game_events.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_sr_hud.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_sr_hud_x.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_sr_hud_y.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_sr_hud_font_color.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_sr_hud_font_index.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_autostart.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_autostop.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_standard.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_duckjump.UniqueFor(SourceGame_Portal2Game);
    sar_replay_viewmode.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_mimic.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_tas_strafe_vectorial.UniqueFor(SourceGame_Portal2Engine);

    startbhop.UniqueFor(SourceGame_TheStanleyParable);
    endbhop.UniqueFor(SourceGame_TheStanleyParable);
    sar_anti_anti_cheat.UniqueFor(SourceGame_TheStanleyParable);
    sar_workshop.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_workshop_update.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_workshop_list.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_speedrun_result.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_export.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_export_pb.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_import.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_category.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_categories.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_offset.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_start.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_stop.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_split.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_pause.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_resume.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_speedrun_reset.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_togglewait.UniqueFor(SourceGame_Portal2Game);
    sar_tas_ss.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_delete_alias_cmds.UniqueFor(SourceGame_Portal2Game | SourceGame_HalfLife2Engine);
    sar_tas_strafe.UniqueFor(SourceGame_Portal2Engine);
    startautostrafe.UniqueFor(SourceGame_Portal2Engine);
    endautostrafe.UniqueFor(SourceGame_Portal2Engine);

    cvars->Unlock();

    Variable::RegisterAll();
    Command::RegisterAll();
}
void Cheats::Shutdown()
{
    if (sar.game->version & (SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide)) {
        Command::DectivateAutoCompleteFile("map");
        Command::DectivateAutoCompleteFile("changelevel");
        Command::DectivateAutoCompleteFile("changelevel2");
    }

    cvars->Lock();

    Variable::UnregisterAll();
    Command::UnregisterAll();
}
