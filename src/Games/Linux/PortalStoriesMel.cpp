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
    return "Portal Stories: Mel (5723)";
}
const char* PortalStoriesMel::Process()
{
    return "portal2_linux";
}
const float PortalStoriesMel::Tickrate()
{
    return 60;
}
