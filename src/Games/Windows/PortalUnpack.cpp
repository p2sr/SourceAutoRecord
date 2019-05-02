#include "PortalUnpack.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalUnpack::PortalUnpack()
{
    this->version = SourceGame_Portal;
}
void PortalUnpack::LoadOffsets()
{
    HalfLife2Unpack::LoadOffsets();

    using namespace Offsets;

    // server.dll

    iNumPortalsPlaced = 4736; // CPortal_Player::IncrementPortalsPlaced
}
const char* PortalUnpack::Version()
{
    return "Portal (5135)";
}
const char* PortalUnpack::ModDir()
{
    return "portal";
}
