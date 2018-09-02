#include "Cheats.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Features/Hud/Hud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Listener.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/WorkshopList.hpp"

#include "Game.hpp"
#include "SAR.hpp"

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

    if (sar.game->version == SourceGame::Portal2) {
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
    } else if (sar.game->version == SourceGame::HalfLife2) {
        auto sv_portal_debug_touch = Variable("sv_portal_debug_touch");
        if (!!sv_portal_debug_touch) {
            sar.game->version = SourceGame::Portal;
            console->DevMsg("SAR: Detected Portal version!\n");
        }
    } else if (sar.game->version == SourceGame::TheStanleyParable
        || sar.game->version == SourceGame::TheBeginnersGuide) {
        Command::ActivateAutoCompleteFile("map", map_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel", changelevel_CompletionFunc);
        Command::ActivateAutoCompleteFile("changelevel2", changelevel2_CompletionFunc);
    }

    sar_jumpboost.UniqueFor([]() { return sar.game->IsPortal2Engine(); });
    sar_aircontrol.UniqueFor([]() { return sar.game->IsPortal2Engine(); });
    sar_hud_portals.UniqueFor([]() { return sar.game->IsPortalGame(); });
    sar_disable_challenge_stats_hud.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_debug_game_events.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_sr_hud.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_sr_hud_x.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_sr_hud_y.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_sr_hud_font_color.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_sr_hud_font_index.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_autostart.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_autostop.UniqueFor([]() { return sar.game->HasChallengeMode(); });

    startbhop.UniqueFor([]() { return sar.game->HasJumpDisabled(); });
    endbhop.UniqueFor([]() { return sar.game->HasJumpDisabled(); });
    sar_anti_anti_cheat.UniqueFor([]() { return sar.game->HasJumpDisabled(); });
    sar_workshop.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_workshop_update.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_workshop_list.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_result.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_export.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_export_pb.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_import.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_speedrun_rules.UniqueFor([]() { return sar.game->HasChallengeMode(); });
    sar_togglewait.UniqueFor([]() { return sar.game->HasChallengeMode(); });

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

    if (sar.game->version == SourceGame::Portal2) {
        sv_bonus_challenge.Lock();
        sv_transition_fade_time.Lock();
        sv_laser_cube_autoaim.Lock();
        ui_loadingscreen_transition_time.Lock();
        hide_gun_when_holding.Lock();
    } else if (sar.game->version == SourceGame::TheStanleyParable
        || sar.game->version == SourceGame::TheBeginnersGuide) {
        Command::DectivateAutoCompleteFile("map");
        Command::DectivateAutoCompleteFile("changelevel");
        Command::DectivateAutoCompleteFile("changelevel2");
    }

    Variable::UnregisterAll();
    Command::UnregisterAll();
}
