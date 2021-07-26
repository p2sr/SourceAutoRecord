#include "ApertureTag.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ApertureTag::ApertureTag() {
	this->version = SourceGame_ApertureTag;
	Game::mapNames = {
		"gg_intro_wakeup",
		"gg_blue_only",
		"gg_blue_only_2",
		"gg_blue_only_3",
		"gg_blue_only_2_pt2",
		"gg_a1_intro4",
		"gg_blue_upplatform",
		"gg_red_only",
		"gg_red_surf",
		"gg_all_intro",
		"gg_all_rotating_wall",
		"gg_all_fizzler",
		"gg_all_intro_2",
		"gg_a2_column_blocker",
		"gg_all_puzzle2",
		"gg_all2_puzzle1",
		"gg_all_puzzle1",
		"gg_all2_escape",
		"gg_stage_reveal",
		"gg_stage_bridgebounce_2",
		"gg_stage_redfirst",
		"gg_stage_laserrelay",
		"gg_stage_beamscotty",
		"gg_stage_bridgebounce",
		"gg_stage_roofbounce",
		"gg_stage_pickbounce",
		"gg_stage_theend",
	};
}
void ApertureTag::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

	// client.dll

	m_pCommands = 228;  // CInput::DecodeUserCmdFromBuffer
}
const char *ApertureTag::Version() {
	return "Aperture Tag (7054)";
}
const char *ApertureTag::GameDir() {
	return "Aperture Tag";
}
