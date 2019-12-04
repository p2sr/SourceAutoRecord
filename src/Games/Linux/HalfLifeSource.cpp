#include "HalfLifeSource.hpp"

#include "Game.hpp"

HalfLifeSource::HalfLifeSource()
{
    this->version = SourceGame_HalfLifeSource;
}
const char* HalfLifeSource::Version()
{
    return "Half-Life: Source (5377866)";
}
const char* HalfLifeSource::ModDir()
{
    return "hl1";
}
