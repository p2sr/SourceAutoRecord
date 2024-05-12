#include "ChallengeMode.hpp"

#include "Cheats.hpp"
#include "Event.hpp"
#include "Features/EntityList.hpp"
#include "Features/Stats/Stats.hpp"
#include "Game.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <cmath>
#include <cstring>
#include <vector>

ChallengeMode *cm;

void ChallengeMode::CreateNode() {
	if (!sv_bonus_challenge.GetBool() || engine->demoplayer->IsPlaying()) {
		return;
	}

	for (auto &node : ChallengeMode::customNodes) {
		if (!std::strcmp(node.m_szMapName, engine->hoststate->m_levelName)) {
			server->Create("challenge_mode_end_node", node.m_vecNodeOrigin, node.m_vecNodeAngles, nullptr);
			console->DevMsg("Created challenge node!\n");
			return;
		}
	}
}
void ChallengeMode::LoadNodes(SourceGameVersion version) {
	switch (version) {
	case SourceGame_PortalStoriesMel:
		ChallengeMode::customNodes = {
			{"st_a1_tramride", {-5885, 12050, 125}, {0, 270, 0}},
			{"st_a1_mel_intro", {600, 4720, 12335}, {0, 0, 0}},
			{"st_a1_lift", {190, 1195, -9030}, {270, 270, 0}},
			{"st_a1_garden", {3755, 1840, -610}, {0, 180, 0}},
			{"st_a2_garden_de", {-6080, 4448, -610}, {0, 0, 0}},
			{"st_a2_underbounce", {1050, 784, 445}, {0, 0, 0}},
			{"st_a2_once_upon", {1640, -2070, 2475}, {0, 270, 0}},
			{"st_a2_past_power", {-4068, 5010, -145}, {0, 270, 0}},
			{"st_a2_ramp", {-630, -1664, 15}, {0, 0, 0}},
			{"st_a2_firestorm", {6768, 1878, 1456}, {0, -180, 0}},
			{"st_a3_junkyard", {0, 770, 60}, {0, 180, 0}},
			{"st_a3_concepts", {1632, 190, 65}, {0, 90, 0}},
			{"st_a3_paint_fling", {-512, 2145, 510}, {0, 270, 0}},
			{"st_a3_faith_plate", {3216, 4960, 574}, {0, 180, 0}},
			{"st_a3_transition", {2032, 2285, 345}, {0, 270, 0}},
			{"st_a4_overgrown", {2336, 1373, 350}, {0, 270, 0}},
			{"st_a4_tb_over_goo", {320, -4990, 590}, {0, 90, 0}},
			{"st_a4_two_of_a_kind", {-672, 3500, 705}, {0, 270, 0}},
			{"st_a4_destroyed", {96, 1435, 880}, {0, 270, 0}},
			{"st_a4_factory", {10560, -1968, 580}, {0, 180, 0}},
			{"st_a4_core_access", {0, 1995, 845}, {0, 270, 0}},
			{"st_a4_finale", {16, 570, 380}, {0, 270, 0}},
			{"sp_a1_tramride", {-5885, 12050, 125}, {0, 270, 0}},
			{"sp_a1_mel_intro", {600, 4720, 12335}, {0, 0, 0}},
			{"sp_a1_lift", {190, 1195, -9030}, {270, 270, 0}},
			{"sp_a1_garden", {3755, 1840, -610}, {0, 180, 0}},
			{"sp_a2_garden_de", {-6080, 4448, -610}, {0, 0, 0}},
			{"sp_a2_underbounce", {1050, 784, 445}, {0, 0, 0}},
			{"sp_a2_once_upon", {1640, -2070, 2475}, {0, 270, 0}},
			{"sp_a2_past_power", {-4068, 5010, -145}, {0, 270, 0}},
			{"sp_a2_ramp", {-630, -1664, 15}, {0, 0, 0}},
			{"sp_a2_firestorm", {6768, 1878, 1456}, {0, -180, 0}},
			{"sp_a3_junkyard", {0, 770, 60}, {0, 180, 0}},
			{"sp_a3_concepts", {1632, 190, 65}, {0, 90, 0}},
			{"sp_a3_paint_fling", {-512, 2145, 510}, {0, 270, 0}},
			{"st_a3_faith_plate", {3216, 4960, 574}, {0, 180, 0}},
			{"sp_a3_transition", {2032, 2285, 345}, {0, 270, 0}},
			{"sp_a4_overgrown", {2336, 1373, 350}, {0, 270, 0}},
			{"sp_a4_tb_over_goo", {320, -4990, 590}, {0, 90, 0}},
			{"sp_a4_two_of_a_kind", {-672, 3500, 705}, {0, 270, 0}},
			{"sp_a4_destroyed", {96, 1435, 880}, {0, 270, 0}},
			{"sp_a4_factory", {10560, -1968, 580}, {0, 180, 0}},
			{"sp_a4_core_access", {0, 1995, 845}, {0, 270, 0}},
			{"sp_a4_finale", {16, 570, 380}, {0, 270, 0}},
		};
		break;
	case SourceGame_ApertureTag:
		ChallengeMode::customNodes = {
			// TODO
		};
		break;
	case SourceGame_ThinkingWithTimeMachine:
		ChallengeMode::customNodes = {
			// TODO
		};
		break;
	case SourceGame_PortalReloaded:
		ChallengeMode::customNodes = {
			// TODO
		};
		break;
	default:
		break;
	}

	console->DevMsg("Loaded %i nodes!\n", ChallengeMode::customNodes.size());
}

std::vector<ChallengeNodeData_t> ChallengeMode::customNodes;

ON_EVENT(SESSION_START) {
	if (cm) {
		cm->CreateNode();
	}
}
