#include "ApertureTag.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ApertureTag::ApertureTag()
{
    this->version = SourceGame_ApertureTag;
}
void ApertureTag::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // client.dll

    m_pCommands = 228; // CInput::DecodeUserCmdFromBuffer
}
const char* ApertureTag::Version()
{
    return "Aperture Tag (7054)";
}
const char* ApertureTag::GameDir()
{
    return "Aperture Tag";
}
