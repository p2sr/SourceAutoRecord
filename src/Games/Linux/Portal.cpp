#include "Portal.hpp"

#include "Game.hpp"

Portal::Portal()
{
    this->version = SourceGame_Portal;
}
void Portal::LoadOffsets()
{
    HalfLife2::LoadOffsets();
}
void Portal::LoadRules()
{
}
const char* Portal::Version()
{
    return "Portal (1910503)";
}
const char* Portal::Process()
{
    return "hl2_linux";
}
