#include "INFRA.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

INFRA::INFRA() {
	this->version = SourceGame_INFRA;
	Game::maps = {
		{"infra_c1_m1_office",         "Alcista Building"},
		{"infra_c2_m1_reserve1",       "Hammer Valley"},
		{"infra_c2_m2_reserve2",       "Hammer Valley Dam"},
		{"infra_c2_m3_reserve3",       "Hammer Valley National Park"},
		{"infra_c3_m1_tunnel",         "Bergmann Water Tunnel"},
		{"infra_c3_m2_tunnel2",        "Tunnel B2"},
		{"infra_c3_m3_tunnel3",        "Raft Ride"},
		{"infra_c3_m4_tunnel4",        "Bergmann Power Plant"},
		{"infra_c4_m2_furnace",        "Stalburg Steel"},
		{"infra_c4_m3_tower",          "Exhaust Tower"},
		{"infra_c5_m1_watertreatment", "Pitheath Water Treatment Plant"},
		{"infra_c5_m2_sewer",          "Pitheath Canal"},
		{"infra_c5_m2b_sewer2",        "Pitheath Sewer"},
		{"infra_c6_m1_sewer3",         "Hartman's Hideout"},
		{"infra_c6_m2_metro",          "Pitheath Metro Station"},
		{"infra_c6_m3_metroride",      "Metro Tunnels"},
		{"infra_c6_m4_waterplant",     "Central Waterplant"},
		{"infra_c6_m5_minitrain",      "Minitrain Tunnels"},
		{"infra_c6_m6_central",        "Central Metro Station"},
		{"infra_c7_m1_servicetunnel",  "Service Tunnels"},
		{"infra_c7_m1b_skyscraper",    "Skyscraper"},
		{"infra_c7_m2_bunker",         "SNW Bunker"},
		{"infra_c7_m3_stormdrain",     "West Stalburg Stormdrain"},
		{"infra_c7_m4_cistern",        "West Stalburg Cistern"},
		{"infra_c7_m5_powerstation",   "Coal Power Plant"},
		{"infra_c8_m1_powerstation2",  "Destroyed Power Plant"},
		{"infra_c8_m3_isle1",          "Castle Rock Bay"},
		{"infra_c8_m4_isle2",          "Castle Rock"},
		{"infra_c8_m5_isle3",          "Castle Rock Drawbridge"},
		{"infra_c8_m6_business",       "Point Elias"},
		{"infra_c8_m7_business2",      "Point Elias Metro Station"},
		{"infra_c8_m8_officeblackout", "Flooded Alcista Building"},
		{"infra_c9_m1_rails",          "Turnip Rail Yard"},
		{"infra_c9_m2_tenements",      "Turnip Hill"},
		{"infra_c9_m3_river",          "Turnip River"},
		{"infra_c9_m4_villa",          "Rosenthal's Villa"},
		{"infra_c9_m5_field",          "Boat Ride"},
		{"infra_c10_m1_npp",           "Blackrock NPP"},
		{"infra_c10_m2_reactor",       "Blackrock Reactor 1"},
		{"infra_c10_m3_roof",          "Blackrock NPP Roof"},
		{"infra_c11_ending_1",         "Mark's Tenement"},
		{"infra_c11_ending_2",         "Ending 2"},
		{"infra_c11_ending_3",         "Ending 3"},
		{"infra_ee_binary",            "Binary"},
		{"infra_ee_city_gates",        "City Gates"},
		{"infra_ee_cubes",             "Cubes"},
		{"infra_ee_hallway",           "Hallway"},
		{"infra_ee_wasteland",         "Wasteland"},
		{"main_menu",                  "Main Menu"}
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}

	Game::achievements = {
		{"CHAPTER_1_SUICIDE",                 "A Bad Case of the Mondays"},
		{"CHAPTER_1",                         "Preparations"},
		{"CHAPTER_2_FUSE",                    "First Try"},
		{"CHAPTER_2_BOXES",                   "Step and Jump"},
		{"CHAPTER_2_STASH",                   "Hartman's Stash"},
		{"CHAPTER_2_DAM",                     "Power of the Future"},
		{"CHAPTER_2_CRACK",                   "Foreseen Consequences"},
		{"CHAPTER_2",                         "Just Another Day at Work"},
		{"CHAPTER_3_ADDICTED",                "Enjoy Ice Cold Osmo Olut"},
		{"CHAPTER_3_HALLWAY",                 "???"},
		{"CHAPTER_3_DYNAMITE",                "Demolitionist"},
		{"CHAPTER_3_PUMPS",                   "Better than Before"},
		{"CHAPTER_3",                         "Forgotten World"},
		{"CHAPTER_4_CALL_DOCK",               "Lost and Found"},
		{"CHAPTER_4",                         "Heavy Industry of the Past"},
		{"CHAPTER_5_ALARMS",                  "Sneaky"},
		{"CHAPTER_5_CHEMISTRY",               "Not so Fresh Water"},
		{"CHAPTER_5_KEBAB",                   "Kebab speciale"},
		{"CHAPTER_5_USB",                     "48 61 63 6B 65 64"},
		{"CHAPTER_5_DRINKS",                  "Anarchist"},
		{"CHAPTER_5_WATERTREATMENT_REPAIRED", "Purification"},
		{"CHAPTER_5",                         "Fresh Water"},
		{"CHAPTER_6_TICKET",                  "Better Safe than Sorry"},
		{"CHAPTER_6_WATERPLANT_REPAIRED",     "Reserve"},
		{"CHAPTER_6",                         "Public Transport"},
		{"CHAPTER_7_BEER_MASTER",             "Beer Master"},
		{"CHAPTER_7_WALTER_TAPE",             "Walter's Tape"},
		{"CHAPTER_7_PLUTONIUM_CORE",          "Demon Core Experiment"},
		{"CHAPTER_7_BUNKER_SCIENTIST_WING",   "State's Accommodation"},
		{"CHAPTER_7_COFFEE_OVERDOSE",         "Long Day Ahead"},
		{"CHAPTER_7_UGU",                     "UGU"},
		{"CHAPTER_7_COFFEE_MORKO",            "Special Ingredient"},
		{"CHAPTER_7",                         "Working Overtime"},
		{"CHAPTER_8_KILL_ROBIN",              "Natural Selection"},
		{"CHAPTER_8_HELP_ROBIN",              "First Responder"},
		{"CHAPTER_8_BATTERY_STORE",           "In Case of an Emergency"},
		{"CHAPTER_8_ISLE_BERG",               "Basement Dweller"},
		{"CHAPTER_8_SHROOM_TEA",              "The Wasteland"},
		{"CHAPTER_8",                         "Late for a Meeting"},
		{"CHAPTER_9_DUCK",                    "Quack?"},
		{"CHAPTER_9_ROB",                     "Rob's Gift"},
		{"CHAPTER_9_LAB",                     "Overdose"},
		{"CHAPTER_9_LUCK",                    "Spin to Win"},
		{"CHAPTER_9_VILLA_STASH",             "The Jar of Death"},
		{"CHAPTER_9",                         "To Save a City"},
		{"CHAPTER_10_CARLA_AMIT",             "Locksmith"},
		{"CHAPTER_10_SNW_ROOM",               "Behind the Scenes"},
		{"CHAPTER_10",                        "Redemption"},
		{"PART_3_GOOD_ENDING",                "Winds of Change"},
		{"PART_3_BAD_ENDING",                 "One Man Is Not Enough"},
		{"PART_3_MELTDOWN_ENDING",            "Party Like It's 2011"},
		{"PART_1_PHOTOS_SOME",                "Photographist"},
		{"PART_1_PHOTOS_MANY",                "An Eye for Detail"},
		{"PART_1_PHOTOS_MOST",                "No Stone Unturned"},
		{"PART_1_CORRUPTION_MOST",            "The Conspiracy Unfolds"},
		{"PART_1_GEOCACHES_MOST",             "TFTC"},
		{"PART_1_REPAIR_MOST",                "Structural Analyst Extraordinaire"},
		{"PART_1_WATER_FLOW_METERS_ALL",      "Restoring the Flow"},
		{"PART_1_COMPLETED",                  "The End?"},
		{"PART_2_PHOTOS_SOME",                "Tunnel Crawler"},
		{"PART_2_PHOTOS_MANY",                "Long Exposure"},
		{"PART_2_PHOTOS_MOST",                "Photographer Extraordinaire"},
		{"PART_2_CORRUPTION_MOST",            "Sign of an Open Eye"},
		{"PART_2_GEOCACHES_MOST",             "TB OUT"},
		{"PART_2_REPAIR_MOST",                "Above and Beyond"},
		{"PART_2_COMPLETED",                  "To Be Continued"},
		{"PART_3_PHOTOS_SOME",                "Daily Photo"},
		{"PART_3_PHOTOS_MANY",                "Urban Photography"},
		{"PART_3_PHOTOS_MOST",                "Night Shift"},
		{"PART_3_CORRUPTION_MOST",            "Truthseeker"},
		{"PART_3_GEOCACHES_MOST",             "BYOP"},
		{"PART_3_REPAIR_MOST",                "Just Leave It to Me"},
		{"PART_3_SECRET_ENDING",              "Underground Justice"},
		{"EE_BLUESCREEN",                     "Error"}
	};
}
void INFRA::LoadOffsets() {
	using namespace Offsets;

	#include "Offsets/INFRA 6905.hpp"
}
const char *INFRA::Version() {
	return "INFRA (6905)";
}
const float INFRA::Tickrate() {
	return 120;
}
const char *INFRA::ModDir() {
	return "infra";
}
