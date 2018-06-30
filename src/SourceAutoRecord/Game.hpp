#pragma once
#include "Games/HalfLife2.hpp"
#include "Games/Portal.hpp"
#include "Games/Portal2.hpp"
#include "Games/TheBeginnersGuide.hpp"
#include "Games/TheStanleyParable.hpp"

namespace Game {

enum SourceGame {
    Unknown,
    Portal2, // Portal 2 (7054)
    Portal, // Portal (1910503)
    TheStanleyParable, // The Stanley Parable (6130)
    TheBeginnersGuide, // The Beginners Guide (6172)
    HalfLife2 // Half-Life 2 (2257546)
};

SourceGame Version = SourceGame::Unknown;

void Init()
{
    auto exe = Memory::GetProcessName();
    if (exe == "portal2_linux") {
        Version = SourceGame::Portal2;
        Portal2::Patterns();
        Portal2::Offsets();
    } else if (exe == "hl2_linux") {
        Version = SourceGame::HalfLife2;
        HalfLife2::Patterns();
        HalfLife2::Offsets();
    } else if (exe == "stanley_linux") {
        Version = SourceGame::TheStanleyParable;
        TheStanleyParable::Patterns();
        TheStanleyParable::Offsets();
    } else if (exe == "beginnersguide.bin") {
        Version = SourceGame::TheBeginnersGuide;
        TheBeginnersGuide::Patterns();
        TheBeginnersGuide::Offsets();
    }
}
const char* GetVersion()
{
    switch (Version) {
    case SourceGame::Portal2:
        return "Portal 2 (7054)";
    case SourceGame::Portal:
        return "Portal (1910503)";
    case SourceGame::TheStanleyParable:
        return "The Stanley Parable (6130)";
    case SourceGame::TheBeginnersGuide:
        return "The Beginners Guide (6172)";
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
}