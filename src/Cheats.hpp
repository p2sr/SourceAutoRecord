#pragma once
#include "Command.hpp"
#include "Variable.hpp"

class Cheats {
public:
    void Init();
    void Shutdown();
};

extern Variable sar_autorecord;
extern Variable sar_autojump;
extern Variable sar_jumpboost;
extern Variable sar_aircontrol;
extern Variable sar_duckjump;
extern Variable sar_disable_challenge_stats_hud;
extern Variable sar_disable_steam_pause;
extern Variable sar_disable_no_focus_sleep;

extern Variable sv_laser_cube_autoaim;
extern Variable ui_loadingscreen_transition_time;
extern Variable ui_loadingscreen_fadein_time;
extern Variable ui_loadingscreen_mintransition_time;
extern Variable hide_gun_when_holding;

extern Command startbhop;
extern Command endbhop;
extern Command sar_anti_anti_cheat;
extern Command sar_togglewait;

extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(map);
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(changelevel);
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2);
