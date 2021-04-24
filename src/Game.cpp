#include "Game.hpp"

#include <cstring>
#include <string>

#include "Command.hpp"
#include "Utils.hpp"

#include GAME(Portal2)
#include GAME(ApertureTag)
#include GAME(PortalStoriesMel)
#include GAME(PortalReloaded)
#include GAME(ThinkingWithTimeMachine)

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

#define TARGET_MOD MODULE("server")
#define TARGET_MOD2 MODULE("engine")

std::vector<std::string> Game::mapNames;

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

    auto modDir = GetModDir(TARGET_MOD2);

    // This check is at the top because aptag's server dll is in a
    // portal2 bin folder so it gets detected as portal 2 otherwise
    if (Utils::ICompare(modDir, ApertureTag::GameDir())) {
        return new ApertureTag();
    }

    // May as well do the same thing here too then
    if (Utils::ICompare(modDir, ThinkingWithTimeMachine::GameDir())) {
        return new ThinkingWithTimeMachine();
    }

    modDir = GetModDir(TARGET_MOD);

    if (Utils::ICompare(modDir, Portal2::ModDir())) {
        return new Portal2();
    }

    if (Utils::ICompare(modDir, PortalStoriesMel::ModDir())) {
        return new PortalStoriesMel();
    }

    if (Utils::ICompare(modDir, PortalReloaded::ModDir())) {
        return new PortalReloaded();
    }

    return nullptr;
}
std::string Game::VersionToString(int version)
{
    auto games = std::string("");
    while (version > 0) {
        HAS_GAME_FLAG(SourceGame_Portal2,                 "Portal 2")
        HAS_GAME_FLAG(SourceGame_ApertureTag,             "Aperture Tag")
        HAS_GAME_FLAG(SourceGame_PortalStoriesMel,        "Portal Stories: Mel")
        HAS_GAME_FLAG(SourceGame_ThinkingWithTimeMachine, "Thinking with Time Machine")
        HAS_GAME_FLAG(SourceGame_PortalReloaded,          "Portal Reloaded")
    }
    return games;
}
