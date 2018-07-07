#pragma once
#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)

#define PROC_IS(name, game) \
    if (proc == name) { \
        Version = SourceGame::game; \
        game::Patterns(); \
        game::Offsets(); \
    }

namespace Game {

enum SourceGame {
    Unknown,
    Portal2,
    Portal,
    TheStanleyParable,
    TheBeginnersGuide,
    HalfLife2
};

SourceGame Version = SourceGame::Unknown;

bool IsSupported()
{
    auto proc = Memory::GetProcessName();

#ifdef _WIN32
    PROC_IS("portal2.exe", Portal2);
    PROC_IS("hl2.exe", HalfLife2);
    PROC_IS("stanley.exe", TheStanleyParable);
    PROC_IS("beginnersguide.exe", TheBeginnersGuide);
#else
    PROC_IS("portal2_linx", Portal2);
    PROC_IS("hl2_linux", HalfLife2);
    PROC_IS("stanley_linux", TheStanleyParable);
    PROC_IS("beginnersguide.bin", TheBeginnersGuide);
#endif

    return Version != SourceGame::Unknown;
}
const char* GetVersion()
{
    switch (Version) {
    case SourceGame::Portal2:
        return "Portal 2 (7054)";
    case SourceGame::Portal:
        return "Portal (1910503)";
#ifdef _WIN32
    case SourceGame::TheStanleyParable:
        return "The Stanley Parable (5454)";
    case SourceGame::TheBeginnersGuide:
        return "The Beginners Guide (6167)";
#else
    case SourceGame::TheStanleyParable:
        return "The Stanley Parable (6130)";
    case SourceGame::TheBeginnersGuide:
        return "The Beginners Guide (6172)";
#endif
    case SourceGame::HalfLife2:
        return "Half-Life 2 (2257546)";
    default:
        break;
    }
    return "Unknown";
}
bool IsPortal2Engine()
{
    return Version == SourceGame::Portal2
        || Version == SourceGame::TheStanleyParable
        || Version == SourceGame::TheBeginnersGuide;
}
bool IsHalfLife2Engine()
{
    return Version == SourceGame::HalfLife2
        || Version == SourceGame::Portal;
}
bool HasChallengeMode()
{
    return Version == SourceGame::Portal2;
}
bool HasJumpDisabled()
{
    return Version == SourceGame::TheStanleyParable;
}
bool IsPortalGame()
{
    return Version == SourceGame::Portal2
        || Version == SourceGame::Portal;
}
}