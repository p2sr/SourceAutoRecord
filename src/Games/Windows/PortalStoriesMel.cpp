#include "PortalStoriesMel.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalStoriesMel::PortalStoriesMel() {
	this->version = SourceGame_PortalStoriesMel;
	Game::mapNames = {
		"sp_a1_tramride",
		"sp_a1_mel_intro",
		"sp_a1_lift",
		"sp_a1_garden",
		"sp_a2_garden_de",
		"sp_a2_underbounce",
		"sp_a2_once_upon",
		"sp_a2_past_power",
		"sp_a2_ramp",
		"sp_a2_firestorm",
		"sp_a3_junkyard",
		"sp_a3_concepts",
		"sp_a3_paint_fling",
		"sp_a3_faith_plate",
		"sp_a3_transition",
		"sp_a4_overgrown",
		"sp_a4_tb_over_goo",
		"sp_a4_two_of_a_kind",
		"sp_a4_destroyed",
		"sp_a4_factory",
		"sp_a4_core_access",
		"sp_a4_finale"};
}
void PortalStoriesMel::LoadOffsets() {
	Portal2::LoadOffsets();
}
const char *PortalStoriesMel::Version() {
	return "Portal Stories: Mel (7054)";
}
const char *PortalStoriesMel::ModDir() {
	return "portal_stories";
}
