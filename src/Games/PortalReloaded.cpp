#include "PortalReloaded.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalReloaded::PortalReloaded() {
	this->version = SourceGame_PortalReloaded;
	Game::maps = {
		{"sp_a1_pr_map_001",    "Human Storage Vault"},
		{"sp_a1_pr_map_002",    "Time Travel"},
		{"sp_a1_pr_map_003",    "Cubes and Buttons"},
		{"sp_a1_pr_map_004",    "Portals"},
		{"sp_a1_pr_map_005",    "Time Portals"},
		{"sp_a1_pr_map_006",    "Timing Tests"},
		{"sp_a1_pr_map_007",    "Lasers"},
		{"sp_a1_pr_map_008",    "Aerial Faithplates"},
		{"sp_a1_pr_map_009",    "Light Bridges"},
		{"sp_a1_pr_map_010",    "Turrets"},
		{"sp_a1_pr_map_011",    "Excursion Funnels"},
		{"sp_a1_pr_map_012",    "Finale"},
		{"mp_coop_start",       "Lobby"},
		{"mp_coop_pr_cubes",    "Cube Logic"},
		{"mp_coop_pr_portals",  "Portal Logic"},
		{"mp_coop_pr_teamwork", "Teamwork"},
		{"mp_coop_pr_fling",    "Fling"},
		{"mp_coop_pr_loop",     "Loop"},
		{"mp_coop_pr_catapult", "Faith Plate"},
		{"mp_coop_pr_laser",    "Laser"},
		{"mp_coop_pr_bridge",   "Light Bridge"},
		{"mp_coop_pr_tbeam",    "Funnel"},
		{"mp_coop_pr_bts",      "Behind the Scenes"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}
}
void PortalReloaded::LoadOffsets() {
	using namespace Offsets;

	#include "Offsets/Portal 2 8151.hpp"
}
const char *PortalReloaded::Version() {
	return "Portal Reloaded (8151)";
}
const char *PortalReloaded::ModDir() {
	return "portalreloaded";
}
