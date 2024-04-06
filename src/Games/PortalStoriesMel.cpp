#include "PortalStoriesMel.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalStoriesMel::PortalStoriesMel() {
	this->version = SourceGame_PortalStoriesMel;
	Game::maps = {
		{"st_a1_tramride",      "Tram Ride"},
		{"st_a1_mel_intro",     "Mel Intro"},
		{"st_a1_lift",          "Lift"},
		{"st_a1_garden",        "Garden"},
		{"st_a2_garden_de",     "Destroyed Garden"},
		{"st_a2_underbounce",   "Underbounce"},
		{"st_a2_once_upon",     "Once Upon"},
		{"st_a2_past_power",    "Past Power"},
		{"st_a2_ramp",          "Ramp"},
		{"st_a2_firestorm",     "Firestorm"},
		{"st_a3_junkyard",      "Junkyard"},
		{"st_a3_concepts",      "Concepts"},
		{"st_a3_paint_fling",   "Paint Fling"},
		{"st_a3_faith_plate",   "Faith Plate"},
		{"st_a3_transition",    "Transition"},
		{"st_a4_overgrown",     "Overgrown"},
		{"st_a4_tb_over_goo",   "Funnel Over Goo"},
		{"st_a4_two_of_a_kind", "Two of a Kind"},
		{"st_a4_destroyed",     "Destroyed"},
		{"st_a4_factory",       "Factory"},
		{"st_a4_core_access",   "Core Access"},
		{"st_a4_finale",        "Finale"},
		{"sp_a1_tramride",      "Advanced Tram Ride"},
		{"sp_a1_mel_intro",     "Advanced Mel Intro"},
		{"sp_a1_lift",          "Advanced Lift"},
		{"sp_a1_garden",        "Advanced Garden"},
		{"sp_a2_garden_de",     "Advanced Destroyed Garden"},
		{"sp_a2_underbounce",   "Advanced Underbounce"},
		{"sp_a2_once_upon",     "Advanced Once Upon"},
		{"sp_a2_past_power",    "Advanced Past Power"},
		{"sp_a2_ramp",          "Advanced Ramp"},
		{"sp_a2_firestorm",     "Advanced Firestorm"},
		{"sp_a3_junkyard",      "Advanced Junkyard"},
		{"sp_a3_concepts",      "Advanced Concepts"},
		{"sp_a3_paint_fling",   "Advanced Paint Fling"},
		{"sp_a3_faith_plate",   "Advanced Faith Plate"},
		{"sp_a3_transition",    "Advanced Transition"},
		{"sp_a4_overgrown",     "Advanced Overgrown"},
		{"sp_a4_tb_over_goo",   "Advanced Funnel Over Goo"},
		{"sp_a4_two_of_a_kind", "Advanced Two of a Kind"},
		{"sp_a4_destroyed",     "Advanced Destroyed"},
		{"sp_a4_factory",       "Advanced Factory"},
		{"sp_a4_core_access",   "Advanced Core Access"},
		{"sp_a4_finale",        "Advanced Finale"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}

	Game::achievements = {
		{"ACH.A3_DOORS", "Welcome to Aperture"},
		{"ACH.ADDICTED_TO_SPUDS", "Long-Term Relaxation"},
		{"ACH.BLUE_GEL", "Voices from Above"},
		{"ACH.SUMMER_SALE", "Firefighting 101"},
		{"ACH.BREAK_MONITORS", "Back on Track"},
		{"ACH.WHITE_GEL", "Persistent"},
		{"ACH.WHEATLEY_TRIES_TO", "Testing the Waters"},
		{"ACH.WAKE_UP", "Forever Alone"},
		{"ACH.BREAK_OUT", "Deja-Vu"},
		{"ACH.BRIDGE", "Organic Complications"},
		{"ACH.YOU_MONSTER", "Back off track"},
		{"ACH.CATCH_CRAZY_BOX", "Welcome to my Domain"},
		{"ACH.CLEAN_UP", "System Shutdown"},
		{"ACH.TRIVIAL_TEST", "Under the Stairs"},
		{"ACH.TRACTOR_BEAM", "You Shouldn't be Here"},
		{"ACH.SURVIVE_CONTAINER_RIDE", "Single rainbow"},
		{"ACH.STALEMATE_ASSOCIATE", "Burned in Goo"},
		{"ACH.SPEED_RUN_LEVEL", "Crushed"},
		{"ACH.SHOOT_THE_MOON", "Shot"},
		{"ACH.SAVE_REDEMPTION_TURRET", "Electrocution"},
		{"ACH.ORANGE_GEL", "In the Vents"},
		{"ACH.TEAM_BUILDING", "Beyond your Range of Hearing"},
		{"ACH.NO_BOAT", "Into Darkness"},
		{"ACH.PORTRAIT", "Ignorant"},
		{"ACH.HI_FIVE_YOUR_PARTNER", "Curious"},
		{"ACH.REENTER_TEST_CHAMBERS", "Determined"},
		{"ACH.NOT_THE_DROID", "2056"},
		{"ACH.HUG_NAME", "Story Shutdown"}
	};
}
void PortalStoriesMel::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

#ifdef _WIN32
	// engine.dll
	OnGameOverlayActivated = 144;  // CSteam3Client
#else
#define OFFSET_DEFAULT(name, win, linux)
#define OFFSET_EMPTY(name)
#define OFFSET_LINMOD(name, off) name = off;
#include "OffsetsData.hpp"
#endif
}
const char *PortalStoriesMel::Version() {
	return "Portal Stories: Mel (8151)";
}
const char *PortalStoriesMel::ModDir() {
	return "portal_stories";
}
