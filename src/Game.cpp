#include "Game.hpp"

#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(Portal)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)

const char* Game::Version()
{
    return "Unknown";
}
Game* Game::CreateNew()
{
    auto proc = Memory::GetProcessName();

    if (proc == Portal2::Process()) {
        return new Portal2();
    }
    if (proc == HalfLife2::Process()) {
        if (Memory::TryGetModule(MODULE("sourcevr"), nullptr)) {
            return new HalfLife2();
        }
        return new Portal();
    }
    if (proc == TheStanleyParable::Process()) {
        return new TheStanleyParable();
    }
    if (proc == TheBeginnersGuide::Process()) {
        return new TheBeginnersGuide();
    }
    return nullptr;
}
