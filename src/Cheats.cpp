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
#include "Features/Timer/Timer.hpp"
#include "Features/WorkshopList.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Variable sar_autorecord("sar_autorecord", "0", "Enables automatic demo recording.\n");
Variable sar_autojump("sar_autojump", "0", "Enables automatic jumping on the server.\n");
Variable sar_jumpboost("sar_jumpboost", "0", 0, "Enables special game movement on the server.\n"
                                                "0 = Default,\n"
                                                "1 = Orange Box Engine,\n"
                                                "2 = Pre-OBE.\n");
Variable sar_aircontrol("sar_aircontrol", "0", 0, "Enables more air-control on the server.\n");
Variable sar_duckjump("sar_duckjump", "0", "Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.\n");
Variable sar_disable_challenge_stats_hud("sar_disable_challenge_stats_hud", "0", "Disables opening the challenge mode stats HUD.\n");
Variable sar_disable_steam_pause("sar_disable_steam_pause", "0", "Prevents pauses from steam overlay.\n");
Variable sar_disable_no_focus_sleep("sar_disable_no_focus_sleep", "0", "Does not yield the CPU when game is not focused.\n");

Variable sv_laser_cube_autoaim;
Variable ui_loadingscreen_transition_time;
Variable ui_loadingscreen_fadein_time;
Variable ui_loadingscreen_mintransition_time;
Variable hide_gun_when_holding;

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

// P2, INFRA and HL2 only
#ifdef _WIN32
#define TRACE_SHUTDOWN_PATTERN "6A 00 68 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? "
#define TRACE_SHUTDOWN_OFFSET1 3
#define TRACE_SHUTDOWN_OFFSET2 10
#else
#define TRACE_SHUTDOWN_PATTERN "C7 44 24 04 00 00 00 00 C7 04 24 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? C7"
#define TRACE_SHUTDOWN_OFFSET1 11
#define TRACE_SHUTDOWN_OFFSET2 10
#endif
CON_COMMAND(sar_delete_alias_cmds, "Deletes all alias commands.\n")
{
    using _Cmd_Shutdown = int (*)();
    static _Cmd_Shutdown Cmd_Shutdown = nullptr;

    if (!Cmd_Shutdown) {
        auto result = Memory::MultiScan(engine->Name(), TRACE_SHUTDOWN_PATTERN, TRACE_SHUTDOWN_OFFSET1);
        if (!result.empty()) {
            for (auto const& addr : result) {
                if (!std::strcmp(*reinterpret_cast<char**>(addr), "Cmd_Shutdown()")) {
                    Cmd_Shutdown = Memory::Read<_Cmd_Shutdown>(addr + TRACE_SHUTDOWN_OFFSET2);
                    break;
                }
            }
        }
    }

    if (Cmd_Shutdown) {
        Cmd_Shutdown();
    } else {
        console->Print("Unable to find Cmd_Shutdown() function!\n");
    }
}

void Cheats::Init()
{
    if (sar.game->Is(SourceGame_Portal2Game)) {
        sv_laser_cube_autoaim = Variable("sv_laser_cube_autoaim");
        ui_loadingscreen_transition_time = Variable("ui_loadingscreen_transition_time");
        ui_loadingscreen_fadein_time = Variable("ui_loadingscreen_fadein_time");
        ui_loadingscreen_mintransition_time = Variable("ui_loadingscreen_mintransition_time");
        hide_gun_when_holding = Variable("hide_gun_when_holding");
    } else if (sar.game->Is(SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide)) {
        Command::ActivateAutoCompleteFile("map", map_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel", changelevel_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel2", changelevel2_CompletionFunc);
    }

    auto s3 = SourceGame_Portal2Game | SourceGame_Portal;

    sar_jumpboost.UniqueFor(SourceGame_Portal2Engine);
    sar_aircontrol.UniqueFor(SourceGame_Portal2Engine);
    sar_hud_portals.UniqueFor(SourceGame_Portal2Game | SourceGame_Portal);
    sar_disable_challenge_stats_hud.UniqueFor(SourceGame_Portal2);
    sar_disable_steam_pause.UniqueFor(SourceGame_Portal2Game);
    sar_debug_listener.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_sr_hud.UniqueFor(s3);
    sar_sr_hud_x.UniqueFor(s3);
    sar_sr_hud_y.UniqueFor(s3);
    sar_sr_hud_font_color.UniqueFor(s3);
    sar_sr_hud_font_index.UniqueFor(s3);
    sar_speedrun_autostart.UniqueFor(s3);
    sar_speedrun_autostop.UniqueFor(s3);
    sar_speedrun_standard.UniqueFor(s3);
    sar_duckjump.UniqueFor(SourceGame_Portal2Engine);
    sar_replay_viewmode.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_mimic.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_tas_ss_forceuser.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_hud_pause_timer.UniqueFor(s3);
    sar_speedrun_time_pauses.UniqueFor(s3);
    sar_speedrun_smartsplit.UniqueFor(s3);
    sar_disable_no_focus_sleep.UniqueFor(SourceGame_Portal2Engine);

    startbhop.UniqueFor(SourceGame_TheStanleyParable);
    endbhop.UniqueFor(SourceGame_TheStanleyParable);
    sar_anti_anti_cheat.UniqueFor(SourceGame_TheStanleyParable);
    sar_workshop.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_workshop_update.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_workshop_list.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_speedrun_result.UniqueFor(s3);
    sar_speedrun_export.UniqueFor(s3);
    sar_speedrun_export_pb.UniqueFor(s3);
    sar_speedrun_import.UniqueFor(s3);
    sar_speedrun_category.UniqueFor(s3);
    sar_speedrun_offset.UniqueFor(s3);
    sar_speedrun_start.UniqueFor(s3);
    sar_speedrun_stop.UniqueFor(s3);
    sar_speedrun_split.UniqueFor(s3);
    sar_speedrun_pause.UniqueFor(s3);
    sar_speedrun_resume.UniqueFor(s3);
    sar_speedrun_reset.UniqueFor(s3);
    sar_togglewait.UniqueFor(SourceGame_Portal2Game | SourceGame_INFRA);
    sar_tas_ss.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
    sar_delete_alias_cmds.UniqueFor(SourceGame_Portal2Game | SourceGame_HalfLife2Engine);
    sar_tas_strafe.UniqueFor(SourceGame_Portal2Engine);
    sar_tas_strafe_vectorial.UniqueFor(SourceGame_Portal2Engine);
    startautostrafe.UniqueFor(SourceGame_Portal2Engine);
    endautostrafe.UniqueFor(SourceGame_Portal2Engine);
    sar_dump_events.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);

    cvars->Unlock();

    Variable::RegisterAll();
    Command::RegisterAll();
}
void Cheats::Shutdown()
{
    if (sar.game->Is(SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide)) {
        Command::DectivateAutoCompleteFile("map");
        Command::DectivateAutoCompleteFile("changelevel");
        Command::DectivateAutoCompleteFile("changelevel2");
    }

    cvars->Lock();

    Variable::UnregisterAll();
    Command::UnregisterAll();
}
