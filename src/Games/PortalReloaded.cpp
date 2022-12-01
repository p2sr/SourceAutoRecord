#include "PortalReloaded.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalReloaded::PortalReloaded() {
	this->version = SourceGame_PortalReloaded;
	Game::mapNames = {
		"sp_a1_pr_map_001",
		"sp_a1_pr_map_002",
		"sp_a1_pr_map_003",
		"sp_a1_pr_map_004",
		"sp_a1_pr_map_005",
		"sp_a1_pr_map_006",
		"sp_a1_pr_map_007",
		"sp_a1_pr_map_008",
		"sp_a1_pr_map_009",
		"sp_a1_pr_map_010",
		"sp_a1_pr_map_011",
		"sp_a1_pr_map_012",
	};
}
void PortalReloaded::LoadOffsets() {
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
const char *PortalReloaded::Version() {
	return "Portal Reloaded (8151)";
}
const char *PortalReloaded::ModDir() {
	return "portalreloaded";
}
