#include "Game.hpp"

#include <cstring>

#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(Portal)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)
#include GAME(ApertureTag)
#include GAME(PortalStoriesMel)

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
        return (Memory::TryGetModule(MODULE("sourcevr"), nullptr)) ? new HalfLife2() : new Portal();
    }

    if (proc == TheStanleyParable::Process()) {
        return new TheStanleyParable();
    }

    if (proc == TheBeginnersGuide::Process()) {
        return new TheBeginnersGuide();
    }

    return nullptr;
}
Game* Game::CreateNewMod(const char* dir)
{
    auto mod = std::string(dir);

    if (Utils::EndsWith(mod, std::string("aperturetag"))) {
        return new ApertureTag();
    }

    if (Utils::EndsWith(mod, std::string("portal_stories"))) {
        return new PortalStoriesMel();
    }

    return nullptr;
}
