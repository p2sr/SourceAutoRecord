#include "Portal2.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

#define OFFSET_DEFAULT(name, win, linux) name = win;
#define OFFSET_EMPTY(name)

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
