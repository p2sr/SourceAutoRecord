#pragma once
#include "Commands/Bhop.hpp"
#include "Commands/Binding.hpp"
#include "Commands/Teleporter.hpp"
#include "Commands/Config.hpp"
#include "Commands/Demo.hpp"
#include "Commands/Info.hpp"
#include "Commands/Routing.hpp"
#include "Commands/Stats.hpp"
#include "Commands/Summary.hpp"
#include "Commands/Tas.hpp"
#include "Commands/Timer.hpp"

namespace Commands {

void Init()
{
    startbhop.UniqueFor(Game::HasJumpDisabled);
    endbhop.UniqueFor(Game::HasJumpDisabled);
}
}