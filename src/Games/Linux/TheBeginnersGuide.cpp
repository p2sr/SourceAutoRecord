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

    // vguimatsurface.so

    StartDrawing = 692; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 619; // CMatSystemSurface::PaintTraverseEx
}
const char* TheBeginnersGuide::Version()
{
    return "The Beginners Guide (6172)";
}
const char* TheBeginnersGuide::ModDir()
{
    return "beginnersguide";
}
