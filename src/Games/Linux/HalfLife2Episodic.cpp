#include "HalfLife2Episodic.hpp"

#include "Game.hpp"

HalfLife2Episodic::HalfLife2Episodic()
{
    this->version = SourceGame_HalfLife2Episodic;
}
const char* HalfLife2Episodic::Version()
{
    return "Half-Life 2: Episode One/Two (5377866)";
}
const char* HalfLife2Episodic::ModDir()
{
    return "episodic";
}
