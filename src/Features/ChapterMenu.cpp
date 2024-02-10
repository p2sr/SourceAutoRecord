#include "ChapterMenu.hpp"

#include "Features/Stats/Stats.hpp"
#include "Game.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Utils/SDK.hpp"

#include <vector>

ChapterMenu *chapterMenu;

ChapterMenu::ChapterMenu()
	: original()
	, originalNumChapters() {
}
ChapterMenu::~ChapterMenu() {
	auto chapter = client->g_ChapterContextNames;
	if (chapter) {
		for (auto &original : this->original) {
			*chapter = original;
			++chapter;
		}
		original.clear();
	}

	if (originalNumChapters && client->nNumSPChapters) {
		*client->nNumSPChapters = originalNumChapters;
		originalNumChapters = 0;
	}
}
void ChapterMenu::LoadMaps(SourceGameVersion version) {
	auto maps = std::vector<ChapterContextData_t>();

	switch (version) {
	case SourceGame_Portal2:
		break;
	case SourceGame_PortalStoriesMel:
		maps = {
			{"st_a1_tramride", 1, 1},
			{"st_a1_mel_intro", 1, 1},
			{"st_a1_lift", 1, 1},
			{"st_a1_garden", 1, 1},
			{"st_a2_garden_de", 2, 2},
			{"st_a2_underbounce", 2, 2},
			{"st_a2_once_upon", 2, 2},
			{"st_a2_past_power", 2, 2},
			{"st_a2_ramp", 2, 2},
			{"st_a2_firestorm", 2, 2},
			{"st_a3_junkyard", 3, 2},
			{"st_a3_concepts", 3, 3},
			{"st_a3_paint_fling", 3, 3},
			{"st_a3_faith_plate", 3, 3},
			{"st_a3_transition", 3, 3},
			{"st_a4_overgrown", 4, 4},
			{"st_a4_tb_over_goo", 4, 4},
			{"st_a4_two_of_a_kind", 4, 4},
			{"st_a4_destroyed", 4, 4},
			{"st_a4_factory", 4, 4},
			{"st_a4_core_access", 5, 5},
			{"st_a4_finale", 5, 5},
			{"sp_a2_garden_de", 6, 6},
			{"sp_a2_underbounce", 6, 6},
			{"sp_a2_once_upon", 6, 6},
			{"sp_a2_past_power", 6, 6},
			{"sp_a2_ramp", 6, 6},
			{"sp_a2_firestorm", 6, 6},
			{"sp_a3_junkyard", 7, 7},
			{"sp_a3_concepts", 7, 7},
			{"sp_a3_paint_fling", 7, 7},
			{"sp_a3_faith_plate", 7, 7},
			{"sp_a3_transition", 7, 7},
			{"sp_a4_overgrown", 8, 8},
			{"sp_a4_tb_over_goo", 8, 8},
			{"sp_a4_two_of_a_kind", 8, 8},
			{"sp_a4_destroyed", 8, 8},
			{"sp_a4_factory", 8, 8},
			{"sp_a4_core_access", 9, 9},
			{"sp_a4_finale", 9, 9},
		};
		break;
	case SourceGame_ApertureTag:
		maps = {
			{"gg_intro_wakeup", 1, 1},
			{"gg_blue_only", 1, 1},
			{"gg_blue_only_2", 1, 1},
			{"gg_blue_only_3", 1, 1},
			{"gg_blue_only_2_pt2", 1, 1},
			{"gg_a1_intro4", 1, 1},
			{"gg_blue_upplatform", 1, 1},
			{"gg_red_only", 2, 2},
			{"gg_red_surf", 2, 2},
			{"gg_all_intro", 2, 2},
			{"gg_all_rotating_wall", 2, 2},
			{"gg_all_fizzler", 2, 2},
			{"gg_all_intro_2", 2, 2},
			{"gg_all_puzzle2", 3, 3},
			{"gg_a2_column_blocker", 3, 3},
			{"gg_all2_puzzle1", 3, 3},
			{"gg_all_puzzle1", 3, 3},
			{"gg_all2_escape", 3, 3},
			{"gg_stage_reveal", 4, 4},
			{"gg_stage_bridgebounce_2", 4, 4},
			{"gg_stage_redfirst", 4, 4},
			{"gg_stage_laserrelay", 4, 4},
			{"gg_stage_beamscotty", 4, 4},
			{"gg_stage_bridgebounce", 4, 4},
			{"gg_stage_roofbounce", 4, 4},
			{"gg_stage_pickbounce", 4, 4},
			{"gg_stage_theend", 4, 4},
			{"gg_tag_remix", 5, 4},
			{"gg_trailer_map", 5, 4},
		};
		break;
	default:
		break;
	}

	this->original.clear();

	auto chapter = client->g_ChapterContextNames;
	if (chapter) {
		for (auto &map : maps) {
			this->original.push_back(*chapter);
			*chapter = map;
			++chapter;
		}
	}

	if (client->nNumSPChapters) {
		this->originalNumChapters = *client->nNumSPChapters;
		*client->nNumSPChapters = maps.back().m_nChapter + 1;
	}

	console->DevMsg("Loaded %i maps!\n", maps.size());
}
