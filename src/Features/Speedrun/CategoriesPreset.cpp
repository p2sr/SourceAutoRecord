#include "CategoriesPreset.hpp"
#include "SAR.hpp"

void InitSpeedrunCategoriesTo(std::map<std::string, SpeedrunCategory> *cats, std::map<std::string, SpeedrunRule> *rules, std::string *defaultCat)
{
    if (sar.game->Is(SourceGame_PortalStoriesMel)) {
        // PS:M {{{
        *defaultCat = "RTA";
        *cats = {
            { "RTA", { { "Story - Start", "Advanced - Start", "Story - End", "Advanced - End" } } },
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
                    }
                ),
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
                    }
                ),
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
                    }
                ),
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
                    }
                ),
            },
        };
        // }}}
    } else if (sar.game->Is(SourceGame_ApertureTag)) {
        // ApTag {{{
        *defaultCat = "RTA";
        *cats = {
            { "RTA", { { "Start", "Good Ending", "Bad Ending" } } },
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
                    }
                ),
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
                    }
                ),
            },
            {
                "Bad Ending",
                SpeedrunRule(
                    RuleAction::STOP,
                    "gg_stage_theend",
                    EntityInputRule{
                        ENTRULE_TARGETNAME | ENTRULE_PARAMETER,
                        "die_ending_math_final",
                        "",
                        "Add",
                        "1",
                    }
                ),
            },
        };
        // }}}
    } else if (sar.game->Is(SourceGame_ThinkingWithTimeMachine)) {
        // TWTM {{{
         *defaultCat = "RTA";
        *cats = {
            { "RTA", { { "Start", "Finish" } } },
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
                    }
                ),
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
                    }
                ),
            },
        };
        // }}}
    } else {
        // Portal 2 {{{
        *defaultCat = "Singleplayer";
        *cats = {
            { "Singleplayer", { { "Container Ride Start", "Vault Start", "Moon Shot" } } },
            { "Coop", { { "Coop Start", "Coop Course 5 End" } } },
            { "Coop AC", { { "Coop Start", "Coop Course 6 End" } } },
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
                    }
                ),
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
                    }
                ),
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
                    }
                ),
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
                    }
                ),
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
                    }
                ),
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
                    }
                ),
            },
        };
        // }}}
    }
}
