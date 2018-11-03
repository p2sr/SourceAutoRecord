#include "Cheats.hpp"

#include <cstring>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Features/Hud/Hud.hpp"
#include "Features/Hud/InspectionHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Listener.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Tas/CommandQueuer.hpp"
#include "Features/WorkshopList.hpp"

#include "Game.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"

Variable sar_autorecord("sar_autorecord", "0", "Enables automatic demo recording.\n");
Variable sar_autojump("sar_autojump", "0", "Enables automatic jumping on the server.\n");
Variable sar_jumpboost("sar_jumpboost", "0", 0, "Enables special game movement on the server.\n"
                                                "0 = Default,\n"
                                                "1 = Orange Box Engine,\n"
                                                "2 = Pre-OBE\n");
Variable sar_aircontrol("sar_aircontrol", "0",
#ifdef _WIN32
    0,
#endif
    "Enables more air-control on the server.\n");
Variable sar_duckjump("sar_duckjump", "0", "Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.\n");
Variable sar_disable_challenge_stats_hud("sar_disable_challenge_stats_hud", "0", "Disables opening the challenge mode stats HUD.\n");
Variable sar_debug_event_queue("sar_debug_event_queue", "0", "Prints entitity events when they are fired, similar to developer.\n");

// TSP only
void IN_BhopDown(const CCommand& args) { client->KeyDown(client->in_jump, (args.ArgC() > 1) ? args[1] : NULL); }
void IN_BhopUp(const CCommand& args) { client->KeyUp(client->in_jump, (args.ArgC() > 1) ? args[1] : NULL); }

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
    *engine->m_bWaitEnabled = state;
    console->Print("%s wait!\n", (state) ? "Enabled" : "Disabled");
}
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

CON_COMMAND(sar_dump_entlist, "Dumps entity list.\n")
{
    console->Print("[index] addr | m_iClassName | m_iName\n");

    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        console->Print("[%i] %p | %s | %s\n", index, info->m_pEntity,
            server->GetEntityClassName(info->m_pEntity),
            server->GetEntityName(info->m_pEntity));
    }
}
CON_COMMAND(sar_find_ent_by_name, "Finds first entity in the entity list by name.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("sar_find_ent_by_name <m_iName> : Finds first entity in the entity list by name.\n");
    }

    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        auto name = server->GetEntityName(info->m_pEntity);
        if (!name || std::strcmp(name, args[1]) != 0) {
            continue;
        }

        console->Print("[%i] %p\n", index, info->m_pEntity);
        console->Msg("    -> m_iClassName = %s\n", server->GetEntityClassName(info->m_pEntity));
        console->Msg("    -> m_iName = %s\n", name);
        break;
    }
}
CON_COMMAND(sar_find_ent_by_class_name, "Finds first entity in the entity list by class name.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("sar_find_ent_by_class_name <m_iClassName> : Finds first entity in the entity list by class name.\n");
    }

    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        auto cname = server->GetEntityClassName(info->m_pEntity);
        if (!cname || std::strcmp(cname, args[1]) != 0) {
            continue;
        }

        console->Print("[%i] %p\n", index, info->m_pEntity);
        console->Msg("    -> m_iClassName = %s\n", cname);
        console->Msg("    -> m_iName = %s\n", server->GetEntityName(info->m_pEntity));
        break;
    }
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

    sv_accelerate.Unlock();
    sv_airaccelerate.Unlock();
    sv_friction.Unlock();
    sv_maxspeed.Unlock();
    sv_stopspeed.Unlock();
    sv_maxvelocity.Unlock();
    sv_footsteps.Unlock();

    if (sar.game->version & SourceGame_Portal2) {
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
    } else if (sar.game->version & SourceGame_TheStanleyParable
        || sar.game->version & SourceGame_TheBeginnersGuide) {
        Command::ActivateAutoCompleteFile("map", map_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel", changelevel_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel2", changelevel2_CompletionFunc);
    }

    sar_jumpboost.UniqueFor(SourceGame_Portal2Engine);
    sar_aircontrol.UniqueFor(SourceGame_Portal2Engine);
    sar_hud_portals.UniqueFor(SourceGame_Portal2 | SourceGame_Portal);
    sar_disable_challenge_stats_hud.UniqueFor(SourceGame_Portal2);
    sar_debug_game_events.UniqueFor(SourceGame_Portal2);
    sar_sr_hud.UniqueFor(SourceGame_Portal2);
    sar_sr_hud_x.UniqueFor(SourceGame_Portal2);
    sar_sr_hud_y.UniqueFor(SourceGame_Portal2);
    sar_sr_hud_font_color.UniqueFor(SourceGame_Portal2);
    sar_sr_hud_font_index.UniqueFor(SourceGame_Portal2);
    sar_speedrun_autostart.UniqueFor(SourceGame_Portal2);
    sar_speedrun_autostop.UniqueFor(SourceGame_Portal2);
    sar_duckjump.UniqueFor(SourceGame_Portal2);
    sar_ei_hud.UniqueFor(SourceGame_Portal2);
    sar_ei_hud_font_color.UniqueFor(SourceGame_Portal2);
    sar_ei_hud_font_color2.UniqueFor(SourceGame_Portal2);
    sar_ei_hud_font_index.UniqueFor(SourceGame_Portal2);
    sar_ei_hud_x.UniqueFor(SourceGame_Portal2);
    sar_ei_hud_y.UniqueFor(SourceGame_Portal2);
    sar_ei_hud_z.UniqueFor(SourceGame_Portal2);
    sar_hud_inspection.UniqueFor(SourceGame_Portal2);

    startbhop.UniqueFor(SourceGame_TheStanleyParable);
    endbhop.UniqueFor(SourceGame_TheStanleyParable);
    sar_anti_anti_cheat.UniqueFor(SourceGame_TheStanleyParable);
    sar_workshop.UniqueFor(SourceGame_Portal2);
    sar_workshop_update.UniqueFor(SourceGame_Portal2);
    sar_workshop_list.UniqueFor(SourceGame_Portal2);
    sar_speedrun_result.UniqueFor(SourceGame_Portal2);
    sar_speedrun_export.UniqueFor(SourceGame_Portal2);
    sar_speedrun_export_pb.UniqueFor(SourceGame_Portal2);
    sar_speedrun_import.UniqueFor(SourceGame_Portal2);
    sar_speedrun_rules.UniqueFor(SourceGame_Portal2);
    sar_togglewait.UniqueFor(SourceGame_Portal2);
    sar_tas_ss.UniqueFor(SourceGame_Portal2);
    sar_delete_alias_cmds.UniqueFor(SourceGame_Portal2);
    sar_dump_entlist.UniqueFor(SourceGame_Portal2);
    sar_find_ent_by_name.UniqueFor(SourceGame_Portal2);
    sar_find_ent_by_class_name.UniqueFor(SourceGame_Portal2);
    sar_inspection_start.UniqueFor(SourceGame_Portal2);
    sar_inspection_stop.UniqueFor(SourceGame_Portal2);
    sar_inspection_print.UniqueFor(SourceGame_Portal2);
    sar_inspection_export.UniqueFor(SourceGame_Portal2);
    sar_inspection_index.UniqueFor(SourceGame_Portal2);

    Variable::RegisterAll();
    Command::RegisterAll();
}
void Cheats::Shutdown()
{
    sv_accelerate.Lock();
    sv_airaccelerate.Lock();
    sv_friction.Lock();
    sv_maxspeed.Lock();
    sv_stopspeed.Lock();
    sv_maxvelocity.Lock();
    sv_footsteps.Lock();

    if (sar.game->version & SourceGame_Portal2) {
        sv_bonus_challenge.Lock();
        sv_transition_fade_time.Lock();
        sv_laser_cube_autoaim.Lock();
        ui_loadingscreen_transition_time.Lock();
        hide_gun_when_holding.Lock();
    } else if (sar.game->version & SourceGame_TheStanleyParable
        || sar.game->version & SourceGame_TheBeginnersGuide) {
        Command::DectivateAutoCompleteFile("map");
        Command::DectivateAutoCompleteFile("changelevel");
        Command::DectivateAutoCompleteFile("changelevel2");
    }

    Variable::UnregisterAll();
    Command::UnregisterAll();
}
