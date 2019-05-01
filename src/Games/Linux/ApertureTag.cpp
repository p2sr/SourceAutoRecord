#include "ApertureTag.hpp"

#include "Game.hpp"

ApertureTag::ApertureTag()
{
    this->version = SourceGame_ApertureTag;
}
const char* ApertureTag::Version()
{
    return "Aperture Tag (7054)";
}
const char* ApertureTag::ModDir()
{
    return "aperturetag";
}
