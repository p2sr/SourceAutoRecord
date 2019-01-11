#include "ApertureTag.hpp"

#include "Game.hpp"

ApertureTag::ApertureTag()
{
    this->version = SourceGame_ApertureTag;
}
void ApertureTag::LoadOffsets()
{
    Portal2::LoadOffsets();
}
const char* ApertureTag::Version()
{
    return "Aperture Tag (7054)";
}
const char* ApertureTag::Process()
{
    return "portal2.exe";
}
const float ApertureTag::Tickrate()
{
    return 60;
}
