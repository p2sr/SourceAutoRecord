#include "Portal2.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

#ifdef _WIN32
#define OFFSET_DEFAULT(name, win, linux) name = win;
#else
#define OFFSET_DEFAULT(name, win, linux) name = linux;
#endif

#define OFFSET_EMPTY(name)
#define OFFSET_LINMOD(name, off)

Portal2::Portal2() {
	this->version = SourceGame_Portal2;
	Game::mapNames = {
		"sp_a1_intro1",
		"sp_a1_intro2",
		"sp_a1_intro3",
		"sp_a1_intro4",
		"sp_a1_intro5",
		"sp_a1_intro6",
		"sp_a1_intro7",
		"sp_a1_wakeup",
		"sp_a2_intro",
		"sp_a2_laser_intro",
		"sp_a2_laser_stairs",
		"sp_a2_dual_lasers",
		"sp_a2_laser_over_goo",
		"sp_a2_catapult_intro",
		"sp_a2_trust_fling",
		"sp_a2_pit_flings",
		"sp_a2_fizzler_intro",
		"sp_a2_sphere_peek",
		"sp_a2_ricochet",
		"sp_a2_bridge_intro",
		"sp_a2_bridge_the_gap",
		"sp_a2_turret_intro",
		"sp_a2_laser_relays",
		"sp_a2_turret_blocker",
		"sp_a2_laser_vs_turret",
		"sp_a2_pull_the_rug",
		"sp_a2_column_blocker",
		"sp_a2_laser_chaining",
		"sp_a2_triple_laser",
		"sp_a2_bts1",
		"sp_a2_bts2",
		"sp_a2_bts3",
		"sp_a2_bts4",
		"sp_a2_bts5",
		"sp_a2_bts6",
		"sp_a2_core",
		"sp_a3_00",
		"sp_a3_01",
		"sp_a3_03",
		"sp_a3_jump_intro",
		"sp_a3_bomb_flings",
		"sp_a3_crazy_box",
		"sp_a3_transition01",
		"sp_a3_speed_ramp",
		"sp_a3_speed_flings",
		"sp_a3_portal_intro",
		"sp_a3_end",
		"sp_a4_intro",
		"sp_a4_tb_intro",
		"sp_a4_tb_trust_drop",
		"sp_a4_tb_wall_button",
		"sp_a4_tb_polarity",
		"sp_a4_tb_catch",
		"sp_a4_stop_the_box",
		"sp_a4_laser_catapult",
		"sp_a4_laser_platform",
		"sp_a4_speed_catch",
		"sp_a4_jump_polarity",
		"sp_a4_finale1",
		"sp_a4_finale2",
		"sp_a4_finale3",
		"sp_a4_finale4",
	};

	Game::achievements = {
		{"ACH.SURVIVE_CONTAINER_RIDE", "Wake Up Call", false},
		{"ACH.WAKE_UP", "You Monster", false},
		{"ACH.LASER", "Undiscouraged", false},
		{"ACH.BRIDGE", "Bridge Over Troubling Water", false},
		{"ACH.BREAK_OUT", "SaBOTour", false},
		{"ACH.STALEMATE_ASSOCIATE", "Stalemate Associate", false},
		{"ACH.ADDICTED_TO_SPUDS", "Tater Tote", false},
		{"ACH.BLUE_GEL", "Vertically Unchallenged", false},
		{"ACH.ORANGE_GEL", "Stranger Than Friction", false},
		{"ACH.WHITE_GEL", "White Out", false},
		{"ACH.TRACTOR_BEAM", "Tunnel of Funnel", false},
		{"ACH.TRIVIAL_TEST", "Dual Pit Experiment", false},
		{"ACH.WHEATLEY_TRIES_TO", "The Part Where He Kills You", false},
		{"ACH.SHOOT_THE_MOON", "Lunacy", false},
		{"ACH.BOX_HOLE_IN_ONE", "Drop Box", false},
		{"ACH.SPEED_RUN_LEVEL", "Overclocker", false},
		{"ACH.COMPLIANT", "Pit Boss", false},
		{"ACH.SAVE_CUBE", "Preservation of Mass", false},
		{"ACH.LAUNCH_TURRET", "Pturretdactyl", false},
		{"ACH.CLEAN_UP", "Final Transmission", false},
		{"ACH.REENTER_TEST_CHAMBERS", "Good Listener", false},
		{"ACH.NOT_THE_DROID", "Scanned Alone", false},
		{"ACH.SAVE_REDEMPTION_TURRET", "No Hard Feelings", false},
		{"ACH.CATCH_CRAZY_BOX", "Schrodinger's Catch", false},
		{"ACH.NO_BOAT", "Ship Overboard", false},
		{"ACH.A3_DOORS", "Door Prize", false},
		{"ACH.PORTRAIT", "Portrait of a Lady", false},
		{"ACH.DEFIANT", "You Made Your Point", false},
		{"ACH.BREAK_MONITORS", "Smash TV", false},
		{"ACH.HI_FIVE_YOUR_PARTNER", "High Five", true},
		{"ACH.TEAM_BUILDING", "Team Building", true},
		{"ACH.MASS_AND_VELOCITY", "Confidence Building", true},
		{"ACH.HUG_NAME", "Bridge Building", true},
		{"ACH.EXCURSION_FUNNELS", "Obstacle Building", true},
		{"ACH.NEW_BLOOD", "You Saved Science", true},
		{"ACH.NICE_CATCH", "Iron Grip", true},
		{"ACH.TAUNTS", "Gesticul-8", true},
		{"ACH.YOU_MONSTER", "Can't Touch This", true},
		{"ACH.PARTNER_DROP", "Empty Gesture", true},
		{"ACH.PARTY_OF_THREE", "Party of Three", true},
		{"ACH.PORTAL_TAUNT", "Narbacular Drop", true},
		{"ACH.TEACHER", "Professor Portal", true},
		{"ACH.WITH_STYLE", "Air Show", true},
		{"ACH.LIMITED_PORTALS", "Portal Conservation Society", true},
		{"ACH.FOUR_PORTALS", "Four Ring Circus", true},
		{"ACH.SPEED_RUN_COOP", "Triple Crown", true},
		{"ACH.STAYING_ALIVE", "Still Alive", true},
		{"ACH.TAUNT_CAMERA", "Asking for Trouble", true},
		{"ACH.ROCK_CRUSHES_ROBOT", "Rock Portal Scissors", true},
		{"ACH.SPREAD_THE_LOVE", "Friends List With Benefits", true},
		{"ACH.SUMMER_SALE", "Talent Show", true}
	};
}
void Portal2::LoadOffsets() {
	using namespace Offsets;

	#include "OffsetsData.hpp"
}
const char *Portal2::Version() {
	return "Portal 2 (8491)";
}
const float Portal2::Tickrate() {
	return 60;
}
const char *Portal2::ModDir() {
	return "portal2";
}
