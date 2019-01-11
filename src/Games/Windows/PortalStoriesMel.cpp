#include "PortalStoriesMel.hpp"

#include "Game.hpp"

PortalStoriesMel::PortalStoriesMel()
{
    this->version = SourceGame_PortalStoriesMel;
}
void PortalStoriesMel::LoadOffsets()
{
    Portal2::LoadOffsets();
}
const char* PortalStoriesMel::Version()
{
    return "Portal Stories: Mel (7054)";
}
const char* PortalStoriesMel::Process()
{
    return "portal2.exe";
}
const float PortalStoriesMel::Tickrate()
{
    return 60;
}
