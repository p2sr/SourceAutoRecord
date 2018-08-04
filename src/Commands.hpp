#pragma once
#include "Commands/Binding.hpp"
#include "Commands/Config.hpp"
#include "Commands/Demo.hpp"
#include "Commands/Info.hpp"
#include "Commands/Routing.hpp"
#include "Commands/Speedrun.hpp"
#include "Commands/Stanley.hpp"
#include "Commands/Stats.hpp"
#include "Commands/Summary.hpp"
#include "Commands/Tas.hpp"
#include "Commands/Teleporter.hpp"
#include "Commands/Timer.hpp"

#include "Game.hpp"

class Commands {
public:
    void Init();
    void Shutdown();
};

// The Stanley Parable
DECLARE_AUTOCOMPLETION_FUNCTION(map, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2, "maps", bsp);

void Commands::Init()
{
    startbhop.UniqueFor(Game::HasJumpDisabled);
    endbhop.UniqueFor(Game::HasJumpDisabled);
    sar_anti_anti_cheat.UniqueFor(Game::HasJumpDisabled);

    if (game->version == SourceGame::TheStanleyParable) {
        ACTIVATE_AUTOCOMPLETEFILE(map);
        ACTIVATE_AUTOCOMPLETEFILE(changelevel);
        ACTIVATE_AUTOCOMPLETEFILE(changelevel2);
    }
    Command::RegisterAll();
}
void Commands::Shutdown()
{
    if (game->version == SourceGame::TheStanleyParable) {
        DEACTIVATE_AUTOCOMPLETEFILE(map);
        DEACTIVATE_AUTOCOMPLETEFILE(changelevel);
        DEACTIVATE_AUTOCOMPLETEFILE(changelevel2);
    }

    Command::UnregisterAll();
}

Commands* commands;
extern Commands* commands;
