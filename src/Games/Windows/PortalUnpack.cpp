#include "PortalUnpack.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalUnpack::PortalUnpack()
{
    this->version = SourceGame_Portal;
    Game::mapNames = {
        "testchmb_a_00",
        "testchmb_a_01",
        "testchmb_a_02",
        "testchmb_a_03",
        "testchmb_a_04",
        "testchmb_a_05",
        "testchmb_a_06",
        "testchmb_a_07",
        "testchmb_a_08",
        "testchmb_a_09",
        "testchmb_a_10",
        "testchmb_a_11",
        "testchmb_a_13",
        "testchmb_a_14",
        "testchmb_a_15",
        "escape_00",
        "escape_01",
        "escape_02",
    };
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
