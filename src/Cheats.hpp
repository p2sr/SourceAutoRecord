#pragma once
#include "Command.hpp"
#include "Variable.hpp"

class Cheats {
public:
	void Init();
	void Shutdown();

	static void PatchBhop(int slot, void *player, CUserCmd *cmd);
	static void AutoStrafe(int slot, void *player, CUserCmd *cmd);
};

extern Variable sar_autorecord;
extern Variable sar_autojump;
extern Variable sar_jumpboost;
extern Variable sar_aircontrol;
extern Variable sar_duckjump;
extern Variable sar_disable_challenge_stats_hud;
extern Variable sar_disable_steam_pause;
extern Variable sar_disable_no_focus_sleep;
extern Variable sar_disable_progress_bar_update;
extern Variable sar_prevent_mat_snapshot_recompute;
extern Variable sar_challenge_autostop;
extern Variable sar_show_entinp;
extern Variable sar_force_qc;
extern Variable sar_patch_bhop;
extern Variable sar_patch_cfg;
extern Variable sar_prevent_ehm;
extern Variable sar_disable_weapon_sway;
extern Variable sar_disable_viewmodel_shadows;

extern Variable sv_laser_cube_autoaim;
extern Variable ui_loadingscreen_transition_time;
extern Variable ui_loadingscreen_fadein_time;
extern Variable ui_loadingscreen_mintransition_time;
extern Variable ui_transition_effect;
extern Variable ui_transition_time;
extern Variable hide_gun_when_holding;
extern Variable cl_viewmodelfov;
extern Variable r_flashlightbrightness;

extern Command sar_togglewait;
