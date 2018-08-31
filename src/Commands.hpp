#pragma once
#include "Commands/Binding.hpp"
#include "Commands/CommandQueuer.hpp"
#include "Commands/Demo.hpp"
#include "Commands/Exclusive.hpp"
#include "Commands/ReplaySystem.hpp"
#include "Commands/Speedrun.hpp"
#include "Commands/Stats.hpp"
#include "Commands/Summary.hpp"
#include "Commands/Teleporter.hpp"
#include "Commands/Timer.hpp"
#include "Commands/Tracer.hpp"

#include "Game.hpp"

class Commands {
public:
    void Init();
    void Shutdown();
};

void Commands::Init()
{
    startbhop.UniqueFor(Game::HasJumpDisabled);
    endbhop.UniqueFor(Game::HasJumpDisabled);
    sar_anti_anti_cheat.UniqueFor(Game::HasJumpDisabled);
    sar_workshop.UniqueFor(Game::HasChallengeMode);
    sar_workshop_update.UniqueFor(Game::HasChallengeMode);
    sar_workshop_list.UniqueFor(Game::HasChallengeMode);
    sar_speedrun_result.UniqueFor(Game::HasChallengeMode);
    sar_speedrun_export.UniqueFor(Game::HasChallengeMode);
    sar_speedrun_export_pb.UniqueFor(Game::HasChallengeMode);
    sar_speedrun_import.UniqueFor(Game::HasChallengeMode);
    sar_speedrun_rules.UniqueFor(Game::HasChallengeMode);
    sar_togglewait.UniqueFor(Game::HasChallengeMode);

    if (game->version == SourceGame::TheStanleyParable
        || game->version == SourceGame::TheBeginnersGuide) {
        ACTIVATE_AUTOCOMPLETEFILE(map);
        ACTIVATE_AUTOCOMPLETEFILE(changelevel);
        ACTIVATE_AUTOCOMPLETEFILE(changelevel2);
    }

    Command::RegisterAll();
}
void Commands::Shutdown()
{
    if (game->version == SourceGame::TheStanleyParable
        || game->version == SourceGame::TheBeginnersGuide) {
        DEACTIVATE_AUTOCOMPLETEFILE(map);
        DEACTIVATE_AUTOCOMPLETEFILE(changelevel);
        DEACTIVATE_AUTOCOMPLETEFILE(changelevel2);
    }

    Command::UnregisterAll();
}

Commands* commands;
extern Commands* commands;
