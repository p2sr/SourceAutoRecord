#include "Game.hpp"

#include <cstring>
#include <string>

#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(Portal)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)
#include GAME(ApertureTag)
#include GAME(PortalStoriesMel)
#ifdef _WIN32
#include GAME(INFRA)
#endif

#define HAS_GAME_FLAG(flag, name)        \
    if (version & (flag)) {                \
        games += std::string(name "\n"); \
        version &= ~(flag);                \
    }
#define HAS_GAME_FLAGS(flags, name)       \
    if (version == (flags)) {               \
        games += std::string(name "\n"); \
        version &= ~(flags);                \
    }

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

#ifdef _WIN32
    if (proc == INFRA::Process()) {
        return new INFRA();
    }
#endif

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
std::string Game::VersionToString(int version)
{
    auto games = std::string("");
    while (version > 0) {
        HAS_GAME_FLAGS(SourceGame_Portal2Game | SourceGame_Portal, "Portal Game")
        HAS_GAME_FLAGS(SourceGame_Portal2Engine, "Portal 2 Engine")
        HAS_GAME_FLAGS(SourceGame_Portal2Game, "Portal 2 Game")
        HAS_GAME_FLAGS(SourceGame_HalfLife2Engine, "Half-Life 2 Engine")
        HAS_GAME_FLAG(SourceGame_Portal2, "Portal 2")
        HAS_GAME_FLAG(SourceGame_Portal, "Portal")
        HAS_GAME_FLAG(SourceGame_TheStanleyParable, "The Stanley Parable")
        HAS_GAME_FLAG(SourceGame_TheBeginnersGuide, "The Beginners Guide")
        HAS_GAME_FLAG(SourceGame_HalfLife2, "Half-Life 2")
        HAS_GAME_FLAG(SourceGame_ApertureTag, "Aperture Tag")
        HAS_GAME_FLAG(SourceGame_PortalStoriesMel, "Portal Stories: Mel")
        HAS_GAME_FLAG(SourceGame_INFRA, "INFRA")
    }
    return games;
}
