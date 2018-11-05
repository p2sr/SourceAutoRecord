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

    // server.dll

    gpGlobals = 50; // UTIL_PlayerByIndex (TODO)
    ServiceEventQueue = 207; // CServerGameDLL::GameFrame
}
const char* TheBeginnersGuide::Version()
{
    return "The Beginners Guide (6167)";
}
const char* TheBeginnersGuide::Process()
{
    return "beginnersguide.exe";
}
