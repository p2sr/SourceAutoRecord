#include "TheBeginnersGuide.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

TheBeginnersGuide::TheBeginnersGuide()
{
    this->version = SourceGame_TheBeginnersGuide;
}
void TheBeginnersGuide::LoadOffsets()
{
    TheStanleyParable::LoadOffsets();
}
const char* TheBeginnersGuide::Version()
{
    return "The Beginners Guide (6167)";
}
const char* TheBeginnersGuide::ModDir()
{
    return "beginnersguide";
}
