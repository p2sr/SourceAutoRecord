#include "TheBeginnersGuide.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

TheBeginnersGuide::TheBeginnersGuide()
{
    this->version = SourceGame_TheBeginnersGuide;
    Game::mapNames = {
        "Intro"
        "Whisper",
        "Backwards",
        "Entering",
        "Stairs",
        "Puzzle",
        "Exiting",
        "Down",
        "Notes",
        "Escape",
        "House",
        "Lecture",
        "Theater",
        "Mobius",
        "Island",
        "Machine",
        "Tower",
        "Epilogue",
    };
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
