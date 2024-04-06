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
	Game::maps = {
		{"sp_a1_intro1",                 "Container Ride",       "62761"},
		{"sp_a1_intro2",                 "Portal Carousel",      "62758"},
		{"sp_a1_intro3",                 "Portal Gun",           "47458"},
		{"sp_a1_intro4",                 "Smooth Jazz",          "47455"},
		{"sp_a1_intro5",                 "Cube Momentum",        "47452"},
		{"sp_a1_intro6",                 "Future Starter",       "47106"},
		{"sp_a1_intro7",                 "Secret Panel",         "62763"},
		{"sp_a1_wakeup",                 "Wakeup",               "62759"},
		{"sp_a2_intro",                  "Incinerator",          "47735"},
		{"sp_a2_laser_intro",            "Laser Intro",          "62765"},
		{"sp_a2_laser_stairs",           "Laser Stairs",         "47736"},
		{"sp_a2_dual_lasers",            "Dual Lasers",          "47738"},
		{"sp_a2_laser_over_goo",         "Laser Over Goo",       "47742"},
		{"sp_a2_catapult_intro",         "Catapult Intro",       "62767"},
		{"sp_a2_trust_fling",            "Trust Fling",          "47744"},
		{"sp_a2_pit_flings",             "Pit Flings",           "47465"},
		{"sp_a2_fizzler_intro",          "Fizzler Intro",        "47746"},
		{"sp_a2_sphere_peek",            "Ceiling Catapult",     "47748"},
		{"sp_a2_ricochet",               "Ricochet",             "47751"},
		{"sp_a2_bridge_intro",           "Bridge Intro",         "47752"},
		{"sp_a2_bridge_the_gap",         "Bridge The Gap",       "47755"},
		{"sp_a2_turret_intro",           "Turret Intro",         "47756"},
		{"sp_a2_laser_relays",           "Laser Relays",         "47759"},
		{"sp_a2_turret_blocker",         "Turret Blocker",       "47760"},
		{"sp_a2_laser_vs_turret",        "Laser vs Turret",      "47763"},
		{"sp_a2_pull_the_rug",           "Pull the Rug",         "47764"},
		{"sp_a2_column_blocker",         "Column Blocker",       "47766"},
		{"sp_a2_laser_chaining",         "Laser Chaining",       "47768"},
		{"sp_a2_triple_laser",           "Triple Laser",         "47770"},
		{"sp_a2_bts1",                   "Jailbreak",            "47773"},
		{"sp_a2_bts2",                   "Escape",               "47774"},
		{"sp_a2_bts3",                   "Turret Factory",       "47776"},
		{"sp_a2_bts4",                   "Turret Sabotage",      "47779"},
		{"sp_a2_bts5",                   "Neurotoxin Sabotage",  "47780"},
		{"sp_a2_bts6",                   "Tube Ride",            ""},
		{"sp_a2_core",                   "Core",                 "62771"},
		{"sp_a3_00",                     "Long Fall",            ""},
		{"sp_a3_01",                     "Underground",          "47783"},
		{"sp_a3_03",                     "Cave Johnson",         "47784"},
		{"sp_a3_jump_intro",             "Repulsion Intro",      "47787"},
		{"sp_a3_bomb_flings",            "Bomb Flings",          "47468"},
		{"sp_a3_crazy_box",              "Crazy Box",            "47469"},
		{"sp_a3_transition01",           "PotatOS",              "47472"},
		{"sp_a3_speed_ramp",             "Propulsion Intro",     "47791"},
		{"sp_a3_speed_flings",           "Propulsion Flings",    "47793"},
		{"sp_a3_portal_intro",           "Conversion Intro",     "47795"},
		{"sp_a3_end",                    "Three Gels",           "47798"},
		{"sp_a4_intro",                  "Test",                 "88350"},
		{"sp_a4_tb_intro",               "Funnel Intro",         "47800"},
		{"sp_a4_tb_trust_drop",          "Ceiling Button",       "47802"},
		{"sp_a4_tb_wall_button",         "Wall Button",          "47804"},
		{"sp_a4_tb_polarity",            "Polarity",             "47806"},
		{"sp_a4_tb_catch",               "Funnel Catch",         "47808"},
		{"sp_a4_stop_the_box",           "Stop the Box",         "47811"},
		{"sp_a4_laser_catapult",         "Laser Catapult",       "47813"},
		{"sp_a4_laser_platform",         "Laser Platform",       "47815"},
		{"sp_a4_speed_tb_catch",         "Propulsion Catch",     "47817"},
		{"sp_a4_jump_polarity",          "Repulsion Polarity",   "47819"},
		{"sp_a4_finale1",                "Finale 1",             "62776"},
		{"sp_a4_finale2",                "Finale 2",             "47821"},
		{"sp_a4_finale3",                "Finale 3",             "47824"},
		{"sp_a4_finale4",                "Finale 4",             "47456"},
		{"mp_coop_start",                "Calibration",          ""},
		{"mp_coop_lobby_2",              "Lobby (pre-DLC)",      ""},
		{"mp_coop_lobby_3",              "Lobby",                ""},
		{"mp_coop_doors",                "Doors",                "47741"},
		{"mp_coop_race_2",               "Buttons",              "47825"},
		{"mp_coop_laser_2",              "Lasers",               "47828"},
		{"mp_coop_rat_maze",             "Rat Maze",             "47829"},
		{"mp_coop_laser_crusher",        "Laser Crusher",        "45467"},
		{"mp_coop_teambts",              "Behind the Scenes",    "46362"},
		{"mp_coop_fling_3",              "Flings",               "47831"},
		{"mp_coop_infinifling_train",    "Infinifling",          "47833"},
		{"mp_coop_come_along",           "Team Retrieval",       "47835"},
		{"mp_coop_fling_1",              "Vertical Flings",      "47837"},
		{"mp_coop_catapult_1",           "Catapults",            "47840"},
		{"mp_coop_multifling_1",         "Multifling",           "47841"},
		{"mp_coop_fling_crushers",       "Fling Crushers",       "47844"},
		{"mp_coop_fan",                  "Industrial Fan",       "47845"},
		{"mp_coop_wall_intro",           "Cooperative Bridges",  "47848"},
		{"mp_coop_wall_2",               "Bridge Swap",          "47849"},
		{"mp_coop_catapult_wall_intro",  "Fling Block",          "47854"},
		{"mp_coop_wall_block",           "Catapult Block",       "47856"},
		{"mp_coop_catapult_2",           "Bridge Fling",         "47858"},
		{"mp_coop_turret_walls",         "Turret Walls",         "47861"},
		{"mp_coop_turret_ball",          "Turret Assassin",      "52642"},
		{"mp_coop_wall_5",               "Bridge Testing",       "52660"},
		{"mp_coop_tbeam_redirect",       "Cooperative Funnels",  "52662"},
		{"mp_coop_tbeam_drill",          "Funnel Drill",         "52663"},
		{"mp_coop_tbeam_catch_grind_1",  "Funnel Catch Coop",    "52665"},
		{"mp_coop_tbeam_laser_1",        "Funnel Laser",         "52667"},
		{"mp_coop_tbeam_polarity",       "Cooperative Polarity", "52671"},
		{"mp_coop_tbeam_polarity2",      "Funnel Hop",           "52687"},
		{"mp_coop_tbeam_polarity3",      "Advanced Polarity",    "52689"},
		{"mp_coop_tbeam_maze",           "Funnel Maze",          "52691"},
		{"mp_coop_tbeam_end",            "Turret Warehouse",     "52777"},
		{"mp_coop_paint_come_along",     "Repulsion Jumps",      "52694"},
		{"mp_coop_paint_redirect",       "Double Bounce",        "52711"},
		{"mp_coop_paint_bridge",         "Bridge Repulsion",     "52714"},
		{"mp_coop_paint_walljumps",      "Wall Repulsion",       "52715"},
		{"mp_coop_paint_speed_fling",    "Propulsion Crushers",  "52717"},
		{"mp_coop_paint_red_racer",      "Turret Ninja",         "52735"},
		{"mp_coop_paint_speed_catch",    "Propulsion Retrieval", "52738"},
		{"mp_coop_paint_longjump_intro", "Vault Entrance",       "52740"},
		{"mp_coop_separation_1",         "Separation",           "49341"},
		{"mp_coop_tripleaxis",           "Triple Axis",          "49343"},
		{"mp_coop_catapult_catch",       "Catapult Catch",       "49345"},
		{"mp_coop_2paints_1bridge",      "Bridge Gels",          "49347"},
		{"mp_coop_paint_conversion",     "Maintenance",          "49349"},
		{"mp_coop_bridge_catch",         "Bridge Catch",         "49351"},
		{"mp_coop_laser_tbeam",          "Double Lift",          "52757"},
		{"mp_coop_paint_rat_maze",       "Gel Maze",             "52759"},
		{"mp_coop_paint_crazy_box",      "Crazier Box",          "48287"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}

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
