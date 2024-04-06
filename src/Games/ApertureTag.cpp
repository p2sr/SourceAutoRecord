#include "ApertureTag.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ApertureTag::ApertureTag() {
	this->version = SourceGame_ApertureTag;
	Game::maps = {
		{"gg_intro_wakeup",         "Wakeup"},
		{"gg_blue_only",            "Blue Only 1"},
		{"gg_blue_only_2",          "Blue Only 2"},
		{"gg_blue_only_3",          "Blue Only 3"},
		{"gg_blue_only_2_pt2",      "Blue Only 4"},
		{"gg_a1_intro4",            "Smooth Jazz"},
		{"gg_blue_upplatform",      "Blue Platform"},
		{"gg_red_only",             "Red Only"},
		{"gg_red_surf",             "Surf"},
		{"gg_all_intro",            "Both Intro"},
		{"gg_all_rotating_wall",    "Rotating Wall"},
		{"gg_all_fizzler",          "Fizzler"},
		{"gg_all_intro_2",          "Both Intro 2"},
		{"gg_a2_column_blocker",    "Column Blocker"},
		{"gg_all_puzzle2",          "Puzzle 1"},
		{"gg_all2_puzzle1",         "Puzzle 2"},
		{"gg_all_puzzle1",          "Puzzle 3"},
		{"gg_all2_escape",          "Escape"},
		{"gg_stage_reveal",         "Reveal"},
		{"gg_stage_bridgebounce_2", "Bridge Bounce"},
		{"gg_stage_redfirst",       "Red First"},
		{"gg_stage_laserrelay",     "Laser Relay"},
		{"gg_stage_beamscotty",     "Beam Scotty"},
		{"gg_stage_bridgebounce",   "Bridge Bounce 2"},
		{"gg_stage_roofbounce",     "Roof Bounce"},
		{"gg_stage_pickbounce",     "Pick Bounce"},
		{"gg_stage_theend",         "Finale"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}
}
void ApertureTag::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

#ifdef _WIN32
	// engine.dll
	OnGameOverlayActivated = 144;  // CSteam3Client

	// client.dll
	m_pCommands = 228;  // CInput::DecodeUserCmdFromBuffer

	// materialsystem.dll
	RemoveMaterial = 155;
#else
	// client.so
	m_pCommands = 228;  // CInput::DecodeUserCmdFromBuffer

#define OFFSET_DEFAULT(name, win, linux)
#define OFFSET_EMPTY(name)
#define OFFSET_LINMOD(name, off) name = off;
#include "OffsetsData.hpp"
	
	// materialsystem.so
	RemoveMaterial = 156;
	g_pTextureManager = 10;
#endif
}
const char *ApertureTag::Version() {
	return "Aperture Tag (7054)";
}
const char *ApertureTag::GameDir() {
	return "Aperture Tag";
}
