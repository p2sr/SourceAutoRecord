#include "PortalStoriesMel.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalStoriesMel::PortalStoriesMel()
{
    this->version = SourceGame_PortalStoriesMel;
}
void PortalStoriesMel::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // client.dll

    m_pCommands = 228; // CInput::DecodeUserCmdFromBuffer
}
const char* PortalStoriesMel::Version()
{
    return "Portal Stories: Mel (7054)";
}
const char* PortalStoriesMel::ModDir()
{
    return "portal_stories";
}
