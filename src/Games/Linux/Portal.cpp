#include "Portal.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal::Portal()
{
    this->version = SourceGame_Portal;
}
void Portal::LoadOffsets()
{
    HalfLife2::LoadOffsets();

    using namespace Offsets;

    // server.so

    iNumPortalsPlaced = 4816; // CPortal_Player::IncrementPortalsPlaced
}
const char* Portal::Version()
{
    return "Portal (1910503)";
}
const char* Portal::ModDir()
{
    return "portal";
}
