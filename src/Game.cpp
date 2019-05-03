#include "Game.hpp"

#include <cstring>
#include <string>

#include "Command.hpp"
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
#include GAME(HalfLife2Unpack)
#include GAME(PortalUnpack)
#endif

#define HAS_GAME_FLAG(flag, name)        \
    if (version & (flag)) {              \
        games += std::string(name "\n"); \
        version &= ~(flag);              \
    }
#define HAS_GAME_FLAGS(flags, name)      \
    if (version == (flags)) {            \
        games += std::string(name "\n"); \
        version &= ~(flags);             \
    }

#define TARGET_MOD      MODULE("server")
#define TARGET_PATTERN  "\\bin\\" TARGET_MOD

const char* Game::Version()
{
    return "Unknown";
}
Game* Game::CreateNew()
{
    auto modDir = std::string();

    auto target = Memory::ModuleInfo();
    if (Memory::TryGetModule(TARGET_MOD, &target)) {
        modDir = std::string(target.path);
        modDir = modDir.substr(0, modDir.length() - std::strlen(TARGET_PATTERN));
        modDir = modDir.substr(modDir.find_last_of("\\/") + 1);
    }

    if (!modDir.compare(Portal2::ModDir())) {
        return new Portal2();
    }

    if (!modDir.compare(ApertureTag::ModDir())) {
        return new ApertureTag();
    }

    if (!modDir.compare(PortalStoriesMel::ModDir())) {
        return new PortalStoriesMel();
    }

    if (!modDir.compare(HalfLife2::ModDir())) {
#ifdef _WIN32
        if (Memory::TryGetModule(MODULE("filesystem_steam"), nullptr)) {
            return new HalfLife2Unpack();
        }
#endif
        return new HalfLife2();
    }

    if (!modDir.compare(Portal::ModDir())) {
#ifdef _WIN32
        if (Memory::TryGetModule(MODULE("filesystem_steam"), nullptr)) {
            return new PortalUnpack();
        }
#endif
        return new Portal();
    }

    if (!modDir.compare(TheStanleyParable::ModDir())) {
        return new TheStanleyParable();
    }

    if (!modDir.compare(TheBeginnersGuide::ModDir())) {
        return new TheBeginnersGuide();
    }

#ifdef _WIN32
    if (!modDir.compare(INFRA::ModDir())) {
        return new INFRA();
    }
#endif

    return nullptr;
}
std::string Game::VersionToString(int version)
{
    auto games = std::string("");
    while (version > 0) {
        HAS_GAME_FLAGS(SourceGame_Portal2Game | SourceGame_Portal, "Portal Game")
        HAS_GAME_FLAGS(SourceGame_Portal2Engine,                   "Portal 2 Engine")
        HAS_GAME_FLAGS(SourceGame_Portal2Game,                     "Portal 2 Game")
        HAS_GAME_FLAGS(SourceGame_HalfLife2Engine,                 "Half-Life 2 Engine")
        HAS_GAME_FLAG(SourceGame_Portal2,                          "Portal 2")
        HAS_GAME_FLAG(SourceGame_Portal,                           "Portal")
        HAS_GAME_FLAG(SourceGame_TheStanleyParable,                "The Stanley Parable")
        HAS_GAME_FLAG(SourceGame_TheBeginnersGuide,                "The Beginners Guide")
        HAS_GAME_FLAG(SourceGame_HalfLife2,                        "Half-Life 2")
        HAS_GAME_FLAG(SourceGame_ApertureTag,                      "Aperture Tag")
        HAS_GAME_FLAG(SourceGame_PortalStoriesMel,                 "Portal Stories: Mel")
        HAS_GAME_FLAG(SourceGame_INFRA,                            "INFRA")
    }
    return games;
}
