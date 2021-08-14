#include "CategoriesPreset.hpp"

#include "SAR.hpp"

void InitSpeedrunCategoriesTo(std::map<std::string, SpeedrunCategory> *cats, std::map<std::string, SpeedrunRule> *rules, std::string *defaultCat) {
	if (sar.game->Is(SourceGame_PortalStoriesMel)) {
		// PS:M {{{
		*defaultCat = "RTA";
		*cats = {
			{"RTA", {{"Story - Start", "Advanced - Start", "Story - End", "Advanced - End"}}},
		};
		*rules = {
			{
				"Story - Start",
				SpeedrunRule(
					RuleAction::START,
					"st_a1_tramride",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"Intro_Viewcontroller",
						"",
						"Disable",
						"",
					}),
			},
			{
				"Advanced - Start",
				SpeedrunRule(
					RuleAction::START,
					"sp_a1_tramride",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"Intro_Viewcontroller",
						"",
						"Disable",
						"",
					}),
			},
			{
				"Story - End",
				SpeedrunRule(
					RuleAction::STOP,
					"st_a4_finale",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"soundscape",
						"",
						"Kill",
						"",
					}),
			},
			{
				"Advanced - End",
				SpeedrunRule(
					RuleAction::STOP,
					"sp_a4_finale",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"soundscape",
						"",
						"Kill",
						"",
					}),
			},
		};
		// }}}
	} else if (sar.game->Is(SourceGame_ApertureTag)) {
		// ApTag {{{
		*defaultCat = "RTA";
		*cats = {
			{"RTA", {{"Start", "Good Ending", "Bad Ending"}}},
		};
		*rules = {
			{
				"Start",
				SpeedrunRule(
					RuleAction::START,
					"gg_intro_wakeup",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"tele_out_shower",
						"",
						"Enable",
						"",
					}),
			},
			{
				"Good Ending",
				SpeedrunRule(
					RuleAction::STOP,
					"gg_stage_theend",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"ele_exit_door",
						"",
						"Close",
						"",
					}),
			},
			{
				"Bad Ending",
				SpeedrunRule(
					RuleAction::STOP,
					"gg_stage_theend",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"credits_video",
						"",
						"PlayMovie",
						"",
					}),
			},
		};
		// }}}
	} else if (sar.game->Is(SourceGame_ThinkingWithTimeMachine)) {
		// TWTM {{{
		*defaultCat = "RTA";
		*cats = {
			{"RTA", {{"Start", "Finish"}}},
		};
		*rules = {
			{
				"Start",
				SpeedrunRule(
					RuleAction::START,
					"tm_intro_01",
					EntityInputRule{
						ENTRULE_TARGETNAME | ENTRULE_PARAMETER,
						"wall_fall",
						"",
						"SetAnimation",
						"fall2",
					}),
			},
			{
				"Finish",
				SpeedrunRule(
					RuleAction::STOP,
					"tm_map_final",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"player_br",
						"",
						"Enable",
						"",
					}),
			},
		};
		// }}}
	} else if (sar.game->Is(SourceGame_PortalReloaded)) {
		// Portal Reloaded {{{
		*defaultCat = "Chambers RTA";
		*cats = {
			{"Chambers RTA", {{"Tube Start", "Portal Save Start", "Tube Ending", "Escape Ending", "02", "03", "04", "06", "07", "09", "11", "13", "14", "16", "17", "19", "21", "23"}}},
			{"RTA", {{"Tube Start", "Portal Save Start", "Tube Ending", "Escape Ending"}}},
		};
		*rules = {
			{
				"Tube Start",
				SpeedrunRule(
					RuleAction::START,
					"sp_a1_pr_map_002",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"announcer1",
						"",
						"Trigger",
						"",
					}),
			},
			{
				"Portal Save Start",
				SpeedrunRule(
					RuleAction::START,
					"sp_a1_pr_map_002",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"@wportal1",
						"",
						"Open",
						"",
					}),
			},
			{
				"Tube Ending",
				SpeedrunRule(
					RuleAction::STOP,
					"sp_a1_pr_map_012",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"finale-finale_vc",
						"",
						"Enable",
						"",
					}),
			},
			{
				"Escape Ending",
				SpeedrunRule(
					RuleAction::STOP,
					"sp_a1_pr_map_012",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"finale-escape_ending",
						"",
						"EnableRefire",
						"",
					}),
			},
			{"02", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_003", ZoneTriggerRule{{336, 432, 100}, {150, 150, 200}, 0})},
			{"03", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_003", ZoneTriggerRule{{-2400, -2080, 100}, {150, 150, 200}, 0})},
			{"04", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_003", ZoneTriggerRule{{-2400, 416, 200}, {150, 150, 200}, 0})},
			{"06", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_004", ZoneTriggerRule{{1952, 208, 100}, {150, 150, 200}, 0})},
			{"07", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_004", ZoneTriggerRule{{4864, 6784, 100}, {150, 150, 200}, 0})},
			{"09", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_005", ZoneTriggerRule{{160, -1472, 100}, {150, 150, 200}, 0})},
			{"11", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_006", ZoneTriggerRule{{-2816, -288, 100}, {150, 150, 200}, 0})},
			{"13", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_007", ZoneTriggerRule{{544, -320, 100}, {150, 150, 200}, 0})},
			{"14", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_007", ZoneTriggerRule{{-1472, -1312, 100}, {150, 150, 200}, 0})},
			{"16", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_008", ZoneTriggerRule{{-608, 1024, 676}, {150, 150, 200}, 0})},
			{"17", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_008", ZoneTriggerRule{{4096, 6528, 676}, {150, 150, 200}, 0})},
			{"19", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_009", ZoneTriggerRule{{-2048, -3488, 548}, {150, 150, 200}, 0})},
			{"21", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_010", ZoneTriggerRule{{1344, 288, 260}, {150, 150, 200}, 0})},
			{"23", SpeedrunRule(RuleAction::SPLIT, "sp_a1_pr_map_011", ZoneTriggerRule{{-2336, -2944, 484}, {150, 150, 200}, 0})},
		};
		// }}}
	} else {
		// Portal 2 {{{
		*defaultCat = "Singleplayer";
		*cats = {
			{"Singleplayer", {{"Container Ride Start", "Vault Start", "Moon Shot"}}},
			{"Coop", {{"Coop Start", "Coop Course 5 End"}}},
			{"Coop AC", {{"Coop Start", "Coop Course 6 End"}}},
		};
		*rules = {
			{
				"Container Ride Start",
				SpeedrunRule(
					RuleAction::START,
					"sp_a1_intro1",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"camera_intro",
						"",
						"TeleportToView",
						"",
					}),
			},
			{
				"Vault Start",
				SpeedrunRule(
					RuleAction::START,
					"sp_a1_intro1",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"camera_1",
						"",
						"TeleportPlayerToProxy",
						"",
					}),
			},
			{
				"Moon Shot",
				SpeedrunRule(
					RuleAction::STOP,
					"sp_a4_finale4",
					EntityInputRule{
						ENTRULE_TARGETNAME | ENTRULE_PARAMETER,
						"@glados",
						"",
						"RunScriptCode",
						"BBPortalPlaced()",
					}),
			},
			{
				"Coop Start",
				SpeedrunRule(
					RuleAction::START,
					"mp_coop_start",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"teleport_start",
						"",
						"Enable",
						"",
					}),
			},
			{
				"Coop Course 5 End",
				SpeedrunRule(
					RuleAction::STOP,
					"mp_coop_paint_longjump_intro",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"vault-movie_outro",
						"",
						"PlayMovieForAllPlayers",
						"",
					}),
			},
			{
				"Coop Course 6 End",
				SpeedrunRule(
					RuleAction::STOP,
					"mp_coop_paint_crazy_box",
					EntityInputRule{
						ENTRULE_TARGETNAME,
						"movie_outro",
						"",
						"PlayMovieForAllPlayers",
						"",
					}),
			},
		};
		// }}}
	}
}
