#include "Game.hpp"

#include <cstring>
#include <string>

#include "Command.hpp"
#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(HalfLife2Episodic)
#include GAME(HalfLifeSource)
#include GAME(Portal)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)
#include GAME(ApertureTag)
#include GAME(PortalStoriesMel)
#include GAME(ThinkingWithTimeMachine)
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

#define TARGET_MOD  MODULE("server")
#define TARGET_MOD2 MODULE("engine")

const char* Game::Version()
{
    return "Unknown";
}
Game* Game::CreateNew()
{
    auto GetModDir = [](const char* targetMod) {
        auto modDir = std::string();
        auto target = Memory::ModuleInfo();
        if (Memory::TryGetModule(targetMod, &target)) {
            modDir = std::string(target.path);
            modDir = modDir.substr(0, modDir.length() - std::strlen(targetMod) - 5);
            modDir = modDir.substr(modDir.find_last_of("\\/") + 1);
        }
        return modDir;
    };

    auto modDir = GetModDir(TARGET_MOD);

    if (Utils::ICompare(modDir, Portal2::ModDir())) {
        return new Portal2();
    }

    if (Utils::ICompare(modDir, PortalStoriesMel::ModDir())) {
        return new PortalStoriesMel();
    }

    if (Utils::ICompare(modDir, HalfLife2::ModDir())) {
#ifdef _WIN32
        if (Memory::TryGetModule(MODULE("filesystem_steam"), nullptr)) {
            return new HalfLife2Unpack();
        }
#endif
        return new HalfLife2();
    }

    if (Utils::ICompare(modDir, HalfLife2Episodic::ModDir())) {
        return new HalfLife2Episodic();
    }

    if (Utils::ICompare(modDir, HalfLifeSource::ModDir())) {
        return new HalfLifeSource();
    }

    if (Utils::ICompare(modDir, Portal::ModDir())) {
#ifdef _WIN32
        if (Memory::TryGetModule(MODULE("filesystem_steam"), nullptr)) {
            return new PortalUnpack();
        }
#endif
        return new Portal();
    }

    if (Utils::ICompare(modDir, TheStanleyParable::ModDir())) {
        return new TheStanleyParable();
    }

    if (Utils::ICompare(modDir, TheBeginnersGuide::ModDir())) {
        return new TheBeginnersGuide();
    }

#ifdef _WIN32
    if (Utils::ICompare(modDir, INFRA::ModDir())) {
        return new INFRA();
    }
#endif

    modDir = GetModDir(TARGET_MOD2);
    if (Utils::ICompare(modDir, ApertureTag::GameDir())) {
        return new ApertureTag();
    }
    if (Utils::ICompare(modDir, ThinkingWithTimeMachine::GameDir())) {
        return new ThinkingWithTimeMachine();
    }

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
        HAS_GAME_FLAG(SourceGame_ThinkingWithTimeMachine,          "Thinking with Time Machine")
        HAS_GAME_FLAG(SourceGame_HalfLife2Episodic,                "Half-Life 2: Episode One/Two")
    }
    return games;
}
