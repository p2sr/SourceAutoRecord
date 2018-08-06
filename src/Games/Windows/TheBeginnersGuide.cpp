#include "TheBeginnersGuide.hpp"

#include "Game.hpp"

TheBeginnersGuide::TheBeginnersGuide()
{
    this->version = SourceGame::TheBeginnersGuide;
}
void TheBeginnersGuide::LoadOffsets()
{
    TheStanleyParable::LoadOffsets();
}
void TheBeginnersGuide::LoadRules()
{
}
const char* TheBeginnersGuide::Version()
{
    return "The Beginners Guide (6167)";
}
