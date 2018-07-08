#pragma once
#include "Commands/Binding.hpp"
#include "Commands/Config.hpp"
#include "Commands/Demo.hpp"
#include "Commands/Info.hpp"
#include "Commands/Routing.hpp"
#include "Commands/Stanley.hpp"
#include "Commands/Stats.hpp"
#include "Commands/Summary.hpp"
#include "Commands/Tas.hpp"
#include "Commands/Teleporter.hpp"
#include "Commands/Timer.hpp"

namespace Commands {

void Init()
{
    startbhop.UniqueFor(Game::HasJumpDisabled);
    endbhop.UniqueFor(Game::HasJumpDisabled);
    sar_anti_anti_cheat.UniqueFor(Game::HasJumpDisabled);
}
}