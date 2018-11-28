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
    return "Aperture Tag (TODO)";
}
const char* ApertureTag::Process()
{
    return "portal2_linux";
}
const float ApertureTag::Tickrate()
{
    return 60;
}
