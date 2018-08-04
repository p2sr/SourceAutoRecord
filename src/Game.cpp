#include "Game.hpp"

#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)

const char* Game::GetVersion()
{
    return "Unknown";
}
bool Game::IsPortal2Engine()
{
    return game->version == SourceGame::Portal2
        || game->version == SourceGame::TheStanleyParable
        || game->version == SourceGame::TheBeginnersGuide;
}
bool Game::IsHalfLife2Engine()
{
    return game->version == SourceGame::HalfLife2
        || game->version == SourceGame::Portal;
}
bool Game::HasChallengeMode()
{
    return game->version == SourceGame::Portal2;
}
bool Game::HasJumpDisabled()
{
    return game->version == SourceGame::TheStanleyParable;
}
bool Game::IsPortalGame()
{
    return game->version == SourceGame::Portal2
        || game->version == SourceGame::Portal;
}
bool Game::IsSupported()
{
    auto proc = Memory::GetProcessName();

#ifdef _WIN32
    if (proc == "portal2.exe") {
        game = new Portal2();
    }
    if (proc == "hl2.exe") {
        game = new HalfLife2();
    }
    if (proc == "stanley.exe") {
        game = new TheStanleyParable();
    }
    if (proc == "beginnersguide.exe") {
        game = new TheBeginnersGuide();
    }
#else
    if (proc == "portal2_linux") {
        game = new Portal2();
    }
    if (proc == "hl2_linux") {
        game = new HalfLife2();
    }
    if (proc == "stanley_linux") {
        game = new TheStanleyParable();
    }
    if (proc == "beginnersguide.bin") {
        game = new TheBeginnersGuide();
    }
#endif

    if (game) {
        game->LoadOffsets();
        game->LoadRules();
        return true;
    }

    return false;
}

Game* game;
