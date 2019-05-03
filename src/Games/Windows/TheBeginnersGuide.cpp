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

    using namespace Offsets;

    // client.dll

    KeyDown = 269; // CInput::JoyStickApplyMovement
    KeyUp = 248; // CInput::JoyStickApplyMovement
}
const char* TheBeginnersGuide::Version()
{
    return "The Beginners Guide (6167)";
}
const char* TheBeginnersGuide::ModDir()
{
    return "beginnersguide";
}
