#include "Game.hpp"

#include "Utils.hpp"

#include GAME(HalfLife2)
#include GAME(Portal2)
#include GAME(TheBeginnersGuide)
#include GAME(TheStanleyParable)

const char* Game::Version()
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

    if (proc == Portal2::Process()) {
        game = new Portal2();
    }
    if (proc == HalfLife2::Process()) {
        game = new HalfLife2();
    }
    if (proc == TheStanleyParable::Process()) {
        game = new TheStanleyParable();
    }
    if (proc == TheBeginnersGuide::Process()) {
        game = new TheBeginnersGuide();
    }

    if (game) {
        game->LoadOffsets();
        return true;
    }

    return false;
}

Game* game;
