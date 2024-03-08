# SAR: Cvars

|Name|Default|Description|
|---|---|---|
|cl_crosshair_t|0|Removes the top line from the crosshair :0: normal crosshair,1: crosshair without top.<br>|
|cl_crosshairalpha|255|Change the amount of transparency.<br>|
|cl_crosshaircolor_b|255|Changes the color of the crosshair.<br>|
|cl_crosshaircolor_g|255|Changes the color of the crosshair.<br>|
|cl_crosshaircolor_r|255|Changes the color of the crosshair.<br>|
|cl_crosshairdot|1|Decides if there is a dot in the middle of the crosshair<br>|
|cl_crosshairgap|9|Changes the distance of the crosshair lines from the center of screen.<br>|
|cl_crosshairsize|1|Changes the size of the crosshair.<br>|
|cl_crosshairthickness|0|Changes the thinkness of the crosshair lines.<br>|
|cl_quickhud_alpha|255|Change the amount of transparency.<br>|
|cl_quickhudleftcolor_b|86|Changes the color of the left quickhud.<br>|
|cl_quickhudleftcolor_g|184|Changes the color of the left quickhud.<br>|
|cl_quickhudleftcolor_r|255|Changes the color of the left quickhud.<br>|
|cl_quickhudrightcolor_b|255|Changes the color of the right quickhud.<br>|
|cl_quickhudrightcolor_g|184|Changes the color of the right quickhud.<br>|
|cl_quickhudrightcolor_r|111|Changes the color of the right quickhud.<br>|
|cond|cmd|cond \<condition> \<command> [args]... - runs a command only if a given condition is met<br>|
|conds|cmd|conds [\<condition> \<command>]... [else] - runs the first command which has a satisfied condition<br>|
|ghost_chat|cmd|ghost_chat - open the chat HUD for messaging other players<br>|
|ghost_connect|cmd|ghost_connect \<ip address> \<port> - connect to the server<br>ex: 'localhost 53000' - '127.0.0.1 53000' - '89.10.20.20 53000'.<br>|
|ghost_debug|cmd|ghost_debug - output a fuckton of debug info about network ghosts<br>|
|ghost_delete_all|cmd|ghost_delete_all - delete all ghosts<br>|
|ghost_delete_by_ID|cmd|ghost_delete_by_ID \<ID> - delete the ghost selected<br>|
|ghost_demo_color|cmd|ghost_demo_color \<color> \<ID>  - sets the color of ghost<br>|
|ghost_disconnect|cmd|ghost_disconnect - disconnect<br>|
|ghost_draw_through_walls|0|Whether to draw ghosts through walls. 0 = none, 1 = names, 2 = names and ghosts.<br>|
|ghost_height|16|Height of the ghosts. (For prop models, only affects their position).<br>|
|+ghost_leaderboard|cmd|+ghost_leaderboard - enable the ghost leaderboard HUD<br>|
|-ghost_leaderboard|cmd|-ghost_leaderboard - disable the ghost leaderboard HUD<br>|
|ghost_leaderboard_font|68|The font to use for the ghost leaderboard.<br>|
|ghost_leaderboard_max_display|10|The maximum number of names to display on the leaderboard.<br>|
|ghost_leaderboard_mode|0|The mode for the leaderboard. 0 = CM, 1 = race<br>|
|ghost_leaderboard_reset|cmd|ghost_leaderboard_reset - reset all leaderboard entries to "no time".<br>|
|ghost_leaderboard_x|10|The x position of the leaderboard.<br>|
|ghost_leaderboard_y|10|The x position of the leaderboard.<br>|
|+ghost_list|cmd|+ghost_list - enable the ghost list HUD<br>|
|-ghost_list|cmd|-ghost_list - disable the ghost list HUD<br>|
|ghost_list|cmd|ghost_list - list all players in the current ghost server<br>|
|ghost_list_font|0|Font index for ghost list HUD.<br>|
|ghost_list_mode|0|Mode for ghost list HUD. 0 = all players, 1 = current map<br>|
|ghost_list_show_map|0|Show the map name in the ghost list HUD.<br>|
|ghost_list_x|2|X position of ghost list HUD.<br>|
|ghost_list_y|-2|Y position of ghost list HUD.<br>|
|ghost_message|cmd|ghost_message - send message to other players<br>|
|ghost_name|cmd|ghost_name - change your online name<br>|
|ghost_name_font_size|5.0|The size to render ghost names at.<br>|
|ghost_net_dump|0|Dump all ghost network activity to a file for debugging.<br>|
|ghost_net_dump_mark|cmd|Mark a point of interest in the ghost network activity dump.<br>|
|ghost_offset|cmd|ghost_offset \<offset> \<ID> - delay the ghost start by \<offset> frames<br>|
|ghost_opacity|255|Opacity of the ghosts.<br>|
|ghost_ping|cmd|Pong!<br>|
|ghost_prop_model|cmd|ghost_prop_model \<filepath> - set the prop model. Example: models/props/metal_box.mdl<br>|
|ghost_proximity_fade|100|Distance from ghosts at which their models fade out.<br>|
|ghost_recap|cmd|ghost_recap - recap all ghosts setup<br>|
|ghost_reset|cmd|ghost_reset - reset ghosts<br>|
|ghost_set_color|cmd|ghost_set_color \<hex code> - sets the ghost color to the specified sRGB color code<br>|
|ghost_set_demo|cmd|ghost_set_demo \<demo> [ID] - ghost will use this demo. If ID is specified, will create or modify the ID-th ghost<br>|
|ghost_set_demos|cmd|ghost_set_demos \<first_demo> [first_id] [ID] - ghost will setup a speedrun with first_demo, first_demo_2, etc.<br>If first_id is specified as e.g. 5, will instead start from first_demo_5, then first_demo_6, etc. Specifying first_id as 1 will use first_demo, first_demo_2 etc as normal.<br>If ID is specified, will create or modify the ID-th ghost.<br>|
|ghost_shading|1|Enable simple light level based shading for overlaid ghosts.<br>|
|ghost_show_advancement|3|Show the advancement of the ghosts. 1 = show finished runs on the current map, 2 = show all finished runs, 3 = show all finished runs and map changes<br>|
|ghost_show_names|1|Whether to show names above ghosts.<br>|
|ghost_show_spec_chat|1|Show chat messages from spectators when not spectating.<br>|
|ghost_spec_connect|cmd|ghost_spec_connect \<ip address> \<port> - connect to the server as a spectator<br>ex: 'localhost 53000' - '127.0.0.1 53000' - '89.10.20.20 53000'.<br>|
|ghost_spec_next|cmd|ghost_spec_next [samemap] - spectate the next ghost<br>|
|ghost_spec_pov|cmd|ghost_spec_pov \<name\|none> - spectate the specified ghost<br>|
|ghost_spec_prev|cmd|ghost_spec_prev - spectate the previous ghost<br>|
|ghost_spec_see_spectators|0|Whether to see other spectators while spectating.<br>|
|ghost_spec_thirdperson|0|Whether to spectate ghost from a third-person perspective.<br>|
|ghost_spec_thirdperson_dist|300|The maximum distance from which to spectate in third-person.<br>|
|ghost_start|cmd|ghost_start - start ghosts<br>|
|ghost_sync|0|When loading a new level, pauses the game until other players load it.<br>|
|ghost_sync_countdown|3|The number of seconds of countdown to show at the start of every synced map. 0 to disable.<br>|
|ghost_TCP_only|0|Uses only TCP for ghost servers. For people with unreliable internet.<br>|
|ghost_text_offset|7|Offset of the name over the ghosts.<br>|
|ghost_type|cmd|ghost_type \<0/1/2/3/4>:<br>0: Colored circle<br>1: Colored pyramid<br>2: Colored pyramid with portal gun (RECORDED IN DEMOS)<br>3: Prop model (RECORDED IN DEMOS)<br>4: Bendy<br>|
|ghost_update_rate|50|Milliseconds between ghost updates. For people with slow/metered internet.<br>|
|hwait|cmd|hwait \<tick> \<command> [args...] - run a command after the given number of host ticks<br>|
|nop|cmd|nop [args]... - nop ignores all its arguments and does nothing<br>|
|sar_about|cmd|sar_about - prints info about SAR plugin<br>|
|sar_achievement_tracker_ignore_coop|0|When set, achievement tracker ignores coop-specific achievements.<br>|
|sar_achievement_tracker_reset|cmd|sar_achievement_tracker_reset - resets the status of achievement tracker.<br>|
|sar_achievement_tracker_show|0|Enables achievement tracker toasts (using tag "achievements").<br>|
|sar_achievement_tracker_status|cmd|sar_achievement_tracker_status - shows achievement completion status.<br>|
|sar_aim_point_add|cmd|sar_aim_point_add - add frozen aimpoint at current position<br>|
|sar_aim_point_clear|cmd|sar_aim_point_clear - clear all frozen aimpoints<br>|
|sar_aim_point_hud|0|Overlays a marker with coordinates at the point you're aiming at<br>|
|sar_aircontrol|0|Enables more air-control on the server.<br>|
|sar_alias|cmd|sar_alias \<name> [command] [args]... - create an alias, similar to the 'alias' command but not requiring quoting. If no command is specified, prints the given alias<br>|
|sar_alias_run|cmd|sar_alias_run \<name> [args]... - run a SAR alias, passing on any additional arguments<br>|
|sar_allow_resizing_window|0|EXPERIMENTAL! Forces resizing on game's window.<br>|
|sar_always_transmit_heavy_ents|0|Always transmit large but seldom changing edicts to clients to prevent lag spikes.<br>|
|sar_autojump|0|Enables automatic jumping on the server.<br>|
|sar_autorecord|0|Enables or disables automatic demo recording.<br>|
|sar_autostrafe|0|Automatically strafes in your current wishdir.<br>|
|sar_avg_result|cmd|sar_avg_result - prints result of average<br>|
|sar_avg_start|cmd|sar_avg_start - starts calculating the average when using timer<br>|
|sar_avg_stop|cmd|sar_avg_stop - stops average calculation<br>|
|sar_bink_respect_host_time|1|Make BINK video playback respect host time.<br>|
|sar_cam_control|0|sar_cam_control \<type>: Change type of camera control.<br>0 = Default (camera is controlled by game engine),<br>1 = Drive mode (camera is separated and can be controlled by user input),<br>2 = Cinematic mode (camera is controlled by predefined path).<br>3 = Follow mode (Camera is following the player but not rotating, useful when strafing on gel).<br>|
|sar_cam_drive|1|Enables or disables camera drive mode in-game (turning it on is not required for demo player)<br>|
|sar_cam_force_eye_pos|0|Forces camera to be placed exactly on the player's eye position<br>|
|sar_cam_ortho|0|Enables or disables camera orthographic projection.<br>|
|sar_cam_ortho_nearz|1|Changes the near Z plane of orthographic projection.<br>|
|sar_cam_ortho_scale|1|Changes the scale of orthographic projection (how many units per pixel).<br>|
|sar_cam_path_draw|0|Draws a representation of the camera path in the world. Disabled in cinematic mode.<br>|
|sar_cam_path_export|cmd|sar_cam_path_export \<filename> [format] [framerate] - exports current camera path to a given file in given format.<br>Available formats:<br>kf - default, exports commands that can be used to recreate camera path. Does not use rate parameter.<br>raw - exports a dump of raw camera position for each frame in given framerate (60 by default).<br>davinci - exports a script for DaVinci Resolve's Camera 3D Fusion component based on raw camera dump.<br>|
|sar_cam_path_getkfs|cmd|sar_cam_path_getkfs - exports commands for recreating currently made camera path<br>|
|sar_cam_path_goto|cmd|sar_cam_path_goto \<frame> [skipto] - sends the camera to a specified frame of the camera path. If skipto is 1, goto the tick in the demo.<br>|
|sar_cam_path_interp|2|Sets interpolation type between keyframes for cinematic camera.<br>0 = Linear interpolation<br>1 = Cubic spline<br>2 = Piecewise Cubic Hermite Interpolating Polynomial (PCHIP)<br>|
|sar_cam_path_remkf|cmd|sar_cam_path_remkf \<frame> - removes camera path keyframe at specified frame<br>|
|sar_cam_path_remkfs|cmd|sar_cam_path_remkfs - removes all camera path keyframes<br>|
|sar_cam_path_setkf|cmd|sar_cam_path_setkf [frame] [x] [y] [z] [pitch] [yaw] [roll] [fov] - sets the camera path keyframe<br>|
|sar_cam_path_showkf|cmd|sar_cam_path_showkf \<frame> - display information about camera path keyframe at specified frame<br>|
|sar_cam_reset|cmd|sar_cam_reset - resets camera to its default position<br>|
|sar_cam_setang|cmd|sar_cam_setang \<pitch> \<yaw> [roll] - sets camera angle (requires camera Drive Mode)<br>|
|sar_cam_setfov|cmd|sar_cam_setfov \<fov> - sets camera field of view (requires camera Drive Mode)<br>|
|sar_cam_setpos|cmd|sar_cam_setpos \<x> \<y> \<z> - sets camera position (requires camera Drive Mode)<br>|
|sar_challenge_autostop|0|Automatically stops recording demos when the leaderboard opens after a CM run. If 2, automatically appends the run time to the demo name.<br>|
|sar_challenge_autosubmit_reload_api_key|cmd|sar_challenge_autosubmit_reload_api_key - reload the board.portal2.sr API key from its file.<br>|
|sar_chat|cmd|sar_chat - open the chat HUD<br>|
|sar_cheat_hud|1|Display a warning in the HUD when cheats are active. 0 = disable, 1 = display if sv_cheats off, 2 = display always<br>|
|sar_cheat_hud_x|-4|X position of the cheat warning HUD.<br>|
|sar_cheat_hud_y|4|Y position of the cheat warning HUD.<br>|
|sar_check_update|cmd|sar_check_update [release\|pre] - check whether the latest version of SAR is being used<br>|
|sar_clear_lines|cmd|sar_clear_lines - clears all active drawline overlays<br>|
|sar_cm_rightwarp|0|Fix CM wrongwarp.<br>|
|sar_con_filter|0|Enable the console filter<br>|
|sar_con_filter_allow|cmd|sar_con_filter_allow \<string> [end] - add an allow rule to the console filter, allowing until 'end' is matched<br>|
|sar_con_filter_block|cmd|sar_con_filter_block \<string> [end] - add a disallow rule to the console filter, blocking until 'end' is matched<br>|
|sar_con_filter_debug|cmd|sar_con_filter_debug - print the console filter rule list<br>|
|sar_con_filter_default|0|Whether to allow text through the console filter by default<br>|
|sar_con_filter_reset|cmd|sar_con_filter_reset - clear the console filter rule list<br>|
|sar_con_filter_suppress_blank_lines|0|Whether to suppress blank lines in console<br>|
|sar_coop_reset_progress|cmd|sar_coop_reset_progress - resets all coop progress<br>|
|sar_cps_add|cmd|sar_cps_add - saves current time of timer<br>|
|sar_cps_clear|cmd|sar_cps_clear - resets saved times of timer<br>|
|sar_cps_result|cmd|sar_cps_result - prints result of timer checkpoints<br>|
|sar_crosshair_mode|0|Set the crosshair mode :<br>0: Default crosshair<br>1: Customizable crosshair<br>2: Crosshair from .png<br>|
|sar_crosshair_P1|0|Use the P1 crosshair style.<br>|
|sar_crosshair_set_texture|cmd|sar_crosshair_set_texture \<filepath><br>|
|sar_cvarlist|cmd|sar_cvarlist - lists all SAR cvars and unlocked engine cvars<br>|
|sar_cvars_dump|cmd|sar_cvars_dump - dumps all cvars to a file<br>|
|sar_cvars_dump_doc|cmd|sar_cvars_dump_doc - dumps all SAR cvars to a file<br>|
|sar_cvars_lock|cmd|sar_cvars_lock - restores default flags of unlocked cvars<br>|
|sar_cvars_unlock|cmd|sar_cvars_unlock - unlocks all special cvars<br>|
|sar_debug_listener|0|Prints event data of registered listener.<br>|
|sar_delete_alias_cmds|cmd|sar_delete_alias_cmds - deletes all alias commands<br>|
|sar_demo_blacklist|0|Stop a set of commands from being run by demo playback.<br>|
|sar_demo_blacklist_addcmd|cmd|sar_demo_blacklist_addcmd \<command> - add a command to the demo blacklist<br>|
|sar_demo_blacklist_all|0|Stop all commands from being run by demo playback.<br>|
|sar_demo_overwrite_bak|0|Rename demos to (name)_bak if they would be overwritten by recording<br>|
|sar_demo_portal_interp_fix|1|Fix eye interpolation through portals in demo playback.<br>|
|sar_demo_remove_broken|1|Whether to remove broken frames from demo playback<br>|
|sar_demo_replay|cmd|sar_demo_replay - play the last recorded or played demo<br>|
|<i title="Portal 2">sar_disable_challenge_stats_hud</i>|0|Disables opening the challenge mode stats HUD.<br>|
|sar_disable_coop_score_hud|0|Disables the coop score HUD which appears in demo playback.<br>|
|sar_disable_no_focus_sleep|0|Does not yield the CPU when game is not focused.<br>|
|sar_disable_progress_bar_update|0|Disables excessive usage of progress bar.<br>|
|sar_disable_save_status_hud|0|Disables the saving/saved HUD which appears when you make a save.<br>|
|sar_disable_steam_pause|0|Prevents pauses from steam overlay.<br>|
|<i title="Portal 2&#10;Portal Stories: Mel&#10;Portal Reloaded">sar_disable_viewmodel_shadows</i>|0|Disables the shadows on the viewmodel.<br>|
|<i title="Portal 2">sar_disable_weapon_sway</i>|0|Disables the viewmodel lagging behind.<br>|
|sar_dpi_scale|1|Fraction to scale mouse DPI down by.<br>|
|sar_drawline|cmd|sar_drawline \<x> \<y> \<z> \<x> \<y> \<z> [r] [g] [b] - overlay a line in the world<br>|
|sar_drawline_clear|cmd|sar_drawline_clear - clear all active sar_drawlines<br>|
|sar_duckjump|0|Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.<br>|
|sar_dump_client_classes|cmd|sar_dump_client_classes - dumps all client classes to a file<br>|
|sar_dump_client_datamap|cmd|sar_dump_client_datamap - dumps client datmap to a file<br>|
|sar_dump_events|cmd|sar_dump_events - dumps all registered game events of the game event manager<br>|
|sar_dump_server_classes|cmd|sar_dump_server_classes - dumps all server classes to a file<br>|
|sar_dump_server_datamap|cmd|sar_dump_server_datamap - dumps server datamap to a file<br>|
|sar_echo|cmd|sar_echo \<color> \<string...> - echo a string to console with a given color<br>|
|sar_echo_nolf|cmd|sar_echo_nolf \<color> \<string...> - echo a string to console with a given color and no trailing line feed<br>|
|sar_ei_hud|0|Draws entity inspection data.<br>|
|sar_ei_hud_font_color|255 255 255 255|RGBA font color of entity inspection HUD when not recording.<br>|
|sar_ei_hud_font_color2|153 23 9 255|RGBA font color of entity inspection HUD when recording.<br>|
|sar_ei_hud_font_index|1|Font index of entity inspection HUD.<br>|
|sar_ei_hud_x|0|X offset of entity inspection HUD.<br>|
|sar_ei_hud_y|0|Y offset of entity inspection HUD.<br>|
|sar_ei_hud_z|0|Z offset of entity inspection HUD.<br>|
|sar_ent_info|cmd|sar_ent_info [selector] - show info about the entity under the crosshair or with the given name<br>|
|sar_ent_slot_serial|cmd|sar_ent_slot_serial \<id> [value] - prints entity slot serial number, or sets it if additional parameter is specified.<br>Banned in most categories, check with the rules before use!<br>|
|sar_exit|cmd|sar_exit - removes all function hooks, registered commands and unloads the module<br>|
|sar_expand|cmd|sar_expand [cmd]... - run a command after expanding svar substitutions<br>|
|sar_export_stats|cmd|sar_export_stats \<filepath> -  export the stats to the specifed path in a .csv file<br>|
|sar_fast_load_preset|cmd|sar_fast_load_preset \<preset> - sets all loading fixes to preset values<br>|
|sar_fcps_anim_start|cmd|sar_fcps_anim_start \<id> - start animating the ID'th FCPS call.<br>|
|sar_fcps_anim_step|cmd|sar_fcps_anim_step - step the FCPS animation forward.<br>|
|sar_fcps_autostep|0|Time between automatic steps of FCPS animation. 0 to disable automatic stepping.<br>|
|sar_fcps_clear|cmd|sar_fcps_clear - clear the FCPS event history.<br>|
|sar_fcps_override|0|Override FCPS for tracing results.<br>|
|sar_find_client_class|cmd|sar_find_client_class \<class_name> - finds specific client class tables and props with their offset<br>|
|sar_find_ents|cmd|sar_find_ents \<selector> - finds entities in the entity list by class name<br>|
|sar_find_server_class|cmd|sar_find_server_class \<class_name> - finds specific server class tables and props with their offset<br>|
|<i title="Portal Reloaded">sar_fix_reloaded_cheats</i>|1|Overrides map execution of specific console commands in Reloaded in order to separate map usage from player usage for these commands.<br>|
|sar_font_get_name|cmd|sar_font_get_name \<id> - gets the name of a font from its index<br>|
|sar_font_list|cmd|sar_font_list - lists all available fonts<br>|
|sar_force_fov|cmd|sar_force_fov \<fov> - forces player FOV<br>|
|sar_force_qc|0|When ducking, forces view offset to always be at standing height. Requires sv_cheats to work.<br>|
|sar_force_viewmodel_fov|cmd|sar_force_viewmodel_fov \<fov> - forces viewmodel FOV<br>|
|sar_frametime_debug|0|Output debugging information to the console related to frametime.<br>|
|sar_frametime_uncap|0|EXPERIMENTAL - USE AT OWN RISK. Removes the 10-1000 FPS cap on frametime. More info https://wiki.portal2.sr/Frametime<br>|
|sar_function|cmd|sar_function \<name> [command] [args]... - create a function, replacing $1, $2 etc in the command string with the respective argument, and more. If no command is specified, prints the given function<br>|
|sar_function_run|cmd|sar_function_run \<name> [args]... - run a function with the given arguments<br>|
|sar_get_partner_id|cmd|sar_get_partner_id - Prints your coop partner's steam id<br>|
|sar_geteyepos|cmd|sar_geteyepos [slot] - get the view position (portal shooting origin) and view angles of a certain player.<br>|
|sar_getpos|cmd|sar_getpos [slot] [server\|client] - get the absolute origin and angles of a particular player from either the server or client. Defaults to slot 0 and server.<br>|
|sar_give_betsrighter|cmd|sar_give_betsrighter [n] - gives the player in slot n (0 by default) betsrighter.<br>|
|sar_give_fly|cmd|sar_give_fly [n] - gives the player in slot n (0 by default) preserved crouchfly.<br>|
|sar_groundframes_reset|cmd|sar_groundframes_reset - reset recorded groundframe statistics.<br>|
|sar_groundframes_total|cmd|sar_groundframes_total [slot] - output a summary of groundframe counts for the given player slot.<br>|
|sar_hud_angles|0|Draws absolute view angles of the client.<br>0 = Default,<br>1 = XY,<br>2 = XYZ.<br>|
|sar_hud_avg|0|Draws calculated average of timer.<br>|
|sar_hud_bg|0|Enable the SAR HUD background.<br>|
|sar_hud_cps|0|Draws latest checkpoint of timer.<br>|
|sar_hud_demo|0|Draws name, tick and time of current demo.<br>|
|sar_hud_duckstate|0|Draw the state of player ducking.<br>1 - shows either ducked or standing state<br>2 - shows detailed report (requires sv_cheats)<br>|
|sar_hud_ent_slot_serial|0|Draw the serial number of given entity slot.<br>|
|sar_hud_eyeoffset|0|Draws player's eye offset.<br>0 = Default,<br>1 = eye offset including precision error,<br>2 = raw eye offset.<br>|
|sar_hud_font_color|255 255 255 255|RGBA font color of HUD.<br>|
|sar_hud_font_index|0|Font index of HUD.<br>|
|sar_hud_fps|0|Show fps (frames per second) on the SAR hud.<br>1 - Show fps<br>2 - Show fps with fps cap<br>|
|sar_hud_frame|0|Draws current frame count.<br>|
|sar_hud_ghost_spec|0|Show the name of the ghost you're currently spectating.<br>|
|sar_hud_grounded|0|Draws the state of player being on ground.<br>|
|sar_hud_groundframes|0|Draws the number of ground frames since last landing. Setting it to 2 preserves the value.<br>|
|sar_hud_groundspeed|0|Draw the speed of the player upon leaving the ground.<br>|
|sar_hud_hide_text|cmd|sar_hud_hide_text \<id\|all> - hides the nth text value in the HUD<br>|
|sar_hud_inspection|0|Draws entity inspection data.<br>|
|sar_hud_jump|0|Draws current jump distance.<br>|
|sar_hud_jump_peak|0|Draws longest jump distance.<br>|
|sar_hud_jumps|0|Draws total jump count.<br>|
|sar_hud_last_frame|0|Draws last saved frame value.<br>|
|sar_hud_last_session|0|Draws value of latest completed session.<br>|
|sar_hud_orange_only|0|Only display the SAR HUD for orange, for solo coop (fullscreen PIP).<br>|
|sar_hud_order_bottom|cmd|sar_hud_order_bottom \<name> - orders hud element to bottom<br>|
|sar_hud_order_reset|cmd|sar_hud_order_reset - resets order of hud elements<br>|
|sar_hud_order_top|cmd|sar_hud_order_top \<name> - orders hud element to top<br>|
|sar_hud_pause_timer|0|Draws current value of pause timer.<br>|
|sar_hud_portal_angles|0|Draw the camera angles of the last primary portal shot.<br>|
|sar_hud_portal_angles_2|0|Draw the camera angles of the last secondary portal shot.<br>|
|sar_hud_portals|0|Draws total portal count.<br>|
|sar_hud_position|0|Draws absolute position of the client.<br>0 = Default,<br>1 = Player position,<br>2 = Camera (shoot) position.<br>|
|sar_hud_precision|3|Precision of HUD numbers.<br>|
|sar_hud_rainbow|-1|Enables the rainbow HUD mode. -1 = default, 0 = disable, 1 = enable.<br>|
|sar_hud_session|0|Draws current session tick.<br>|
|sar_hud_set_text|cmd|sar_hud_set_text \<id> \<text>... - sets and shows the nth text value in the HUD<br>|
|sar_hud_set_text_color|cmd|sar_hud_set_text_color \<id> [color] - sets the color of the nth text value in the HUD. Reset by not giving color.<br>|
|sar_hud_show_text|cmd|sar_hud_show_text \<id\|all> - shows the nth text value in the HUD<br>|
|sar_hud_spacing|1|Spacing between elements of HUD.<br>|
|sar_hud_steps|0|Draws total step count.<br>|
|sar_hud_strafesync_color|0 150 250 255|RGBA font color of strafesync HUD.<br>|
|sar_hud_strafesync_font_index|1|Font index of strafesync HUD.<br>|
|sar_hud_strafesync_offset_x|0|X offset of strafesync HUD.<br>|
|sar_hud_strafesync_offset_y|1000|Y offset of strafesync HUD.<br>|
|sar_hud_strafesync_split_offset_y|1050|Y offset of strafesync HUD.<br>|
|sar_hud_sum|0|Draws summary value of sessions.<br>|
|sar_hud_tastick|0|Draws current TAS playback tick.<br>|
|sar_hud_tbeam|0|Draw the name of the funnel player is currently in (requires sv_cheats).<br>|
|sar_hud_tbeam_count|0|Draw the player's funnel count (requires sv_cheats).<br>|
|sar_hud_timer|0|Draws current value of timer.<br>|
|sar_hud_toggle_text|cmd|sar_hud_toggle_text \<id> - toggles the nth text value in the HUD<br>|
|sar_hud_trace|0|Draws info about current trace bbox tick.<br>|
|sar_hud_velang|0|Draw the angle of the player's horizontal velocity vector.<br>|
|sar_hud_velocity|0|Draws velocity of the client.<br>0 = Default,<br>1 = X, Y, Z<br>2 = X:Y<br>3 = X:Y, Z<br>4 = X:Y:Z<br>5 = X, Y, X:Y, Z<br>|
|sar_hud_velocity_peak|0|Draws last saved velocity peak.<br>|
|sar_hud_velocity_precision|2|Precision of velocity HUD numbers.<br>|
|sar_hud_x|2|X padding of HUD.<br>|
|sar_hud_y|2|Y padding of HUD.<br>|
|sar_ihud|0|Enables or disables movement inputs HUD of client.<br>|
|sar_ihud_add_key|cmd|sar_ihud_add_key \<key><br>|
|sar_ihud_analog_image_scale|0.6|Scale of analog input images against max extent.<br>|
|sar_ihud_analog_view_deshake|0|Try to eliminate small fluctuations in the movement analog.<br>|
|sar_ihud_clear_background|cmd|sar_ihud_clear_background<br>|
|sar_ihud_grid_padding|2|Padding between grid squares of input HUD.<br>|
|sar_ihud_grid_size|60|Grid square size of input HUD.<br>|
|sar_ihud_modify|cmd|sar_ihud_modify \<element\|all> [param=value]... - modifies parameters in given element.<br>Params: enabled, text, pos, x, y, width, height, font, background, highlight, textcolor, texthighlight, image, highlightimage, minhold.<br>|
|sar_ihud_preset|cmd|sar_ihud_preset \<preset> - modifies input hud based on given preset<br>|
|sar_ihud_set_background|cmd|sar_ihud_set_background \<path> \<grid x> \<grid y> \<grid w> \<grid h><br>|
|sar_ihud_setpos|cmd|sar_ihud_setpos \<top\|center\|bottom\|y\|y%> \<left\|center\|right\|x\|x%> - automatically sets the position of input HUD.<br>|
|sar_ihud_x|2|X position of input HUD.<br>|
|sar_ihud_y|2|Y position of input HUD.<br>|
|sar_import_stats|cmd|sar_import_stats \<filePath> - import the stats from the specified .csv file<br>|
|sar_inspection_export|cmd|sar_inspection_export \<file_name> - saves recorded entity data to a csv file<br>|
|sar_inspection_index|cmd|sar_inspection_index - sets entity index for inspection<br>|
|sar_inspection_print|cmd|sar_inspection_print - prints recorded entity data<br>|
|sar_inspection_save_every_tick|0|Saves inspection data even when session tick does not increment.<br>|
|sar_inspection_start|cmd|sar_inspection_start - starts recording entity data<br>|
|sar_inspection_stop|cmd|sar_inspection_stop - stops recording entity data<br>|
|sar_jumpboost|0|Enables special game movement on the server.<br>0 = Default,<br>1 = Orange Box Engine,<br>2 = Pre-OBE.<br>|
|sar_list_client_classes|cmd|sar_list_client_classes - lists all client classes<br>|
|sar_list_ents|cmd|sar_list_ents - lists entities<br>|
|sar_list_server_classes|cmd|sar_list_server_classes - lists all server classes<br>|
|sar_load_delay|0|Delay for this number of milliseconds at the end of a load.<br>|
|sar_loads_norender|0|Temporarily set mat_norendering to 1 during loads<br>|
|sar_loads_uncap|0|Temporarily set fps_max to 0 during loads<br>|
|sar_lphud|0|Enables or disables the portals display on screen.<br>|
|sar_lphud_font|92|Change font of portal counter.<br>|
|sar_lphud_reset|cmd|sar_lphud_reset - resets lp counter<br>|
|sar_lphud_reset_on_changelevel|0|Reset the lp counter on any changelevel or restart_level. Useful for ILs.<br>|
|sar_lphud_set|cmd|sar_lphud_set \<number> - sets lp counter to given number<br>|
|sar_lphud_setpos|cmd|sar_lphud_setpos \<top\|center\|bottom\|y\|y%> \<left\|center\|right\|x\|x%> - automatically sets the position of least portals HUD.<br>|
|sar_lphud_x|-10|x pos of lp counter.<br>|
|sar_lphud_y|-10|y pos of lp counter.<br>|
|sar_minimap_load|cmd|sar_minimap_load \<filename> - load a minimap from a JSON file.<br>|
|sar_minimap_max_height|1000|The maximum height of the minimap.<br>|
|sar_minimap_max_width|500|The maximum width of the minimap.<br>|
|sar_minimap_player_color|255 0 0 255|The color of the circle representing the player on the minimap.<br>|
|sar_minimap_x|-10|The X position of the minimap.<br>|
|sar_minimap_y|10|The Y position of the minimap.<br>|
|sar_mtrigger_legacy|0|<br>|
|sar_mtrigger_legacy_format|!seg -> !tt (!st)|Formatting of the text that is displayed in the chat (!map - for map name, !seg - for segment name, !tt - for total time, !st - for split time).<br>|
|sar_mtrigger_legacy_textcolor|255 176 0|The color of the text that is displayed in the chat.<br>|
|sar_nextdemo|cmd|sar_nextdemo - plays the next demo in demo queue<br>|
|sar_on_config_exec|cmd|sar_on_config_exec \<command> [args]... - registers a command to be run on config.cfg exec<br>|
|sar_on_config_exec_clear|cmd|sar_on_config_exec_clear - clears commands registered on event "config_exec"<br>|
|sar_on_config_exec_list|cmd|sar_on_config_exec_list - lists commands registered on event "config_exec"<br>|
|sar_on_coop_reset_done|cmd|sar_on_coop_reset_done \<command> [args]... - registers a command to be run when coop reset is completed<br>|
|sar_on_coop_reset_done_clear|cmd|sar_on_coop_reset_done_clear - clears commands registered on event "coop_reset_done"<br>|
|sar_on_coop_reset_done_list|cmd|sar_on_coop_reset_done_list - lists commands registered on event "coop_reset_done"<br>|
|sar_on_coop_reset_remote|cmd|sar_on_coop_reset_remote \<command> [args]... - registers a command to be run when coop reset run remotely<br>|
|sar_on_coop_reset_remote_clear|cmd|sar_on_coop_reset_remote_clear - clears commands registered on event "coop_reset_remote"<br>|
|sar_on_coop_reset_remote_list|cmd|sar_on_coop_reset_remote_list - lists commands registered on event "coop_reset_remote"<br>|
|sar_on_coop_spawn|cmd|sar_on_coop_spawn \<command> [args]... - registers a command to be run on coop spawn<br>|
|sar_on_coop_spawn_clear|cmd|sar_on_coop_spawn_clear - clears commands registered on event "coop_spawn"<br>|
|sar_on_coop_spawn_list|cmd|sar_on_coop_spawn_list - lists commands registered on event "coop_spawn"<br>|
|sar_on_demo_start|cmd|sar_on_demo_start \<command> [args]... - registers a command to be run when demo playback starts<br>|
|sar_on_demo_start_clear|cmd|sar_on_demo_start_clear - clears commands registered on event "demo_start"<br>|
|sar_on_demo_start_list|cmd|sar_on_demo_start_list - lists commands registered on event "demo_start"<br>|
|sar_on_demo_stop|cmd|sar_on_demo_stop \<command> [args]... - registers a command to be run when demo playback stops<br>|
|sar_on_demo_stop_clear|cmd|sar_on_demo_stop_clear - clears commands registered on event "demo_stop"<br>|
|sar_on_demo_stop_list|cmd|sar_on_demo_stop_list - lists commands registered on event "demo_stop"<br>|
|sar_on_exit|cmd|sar_on_exit \<command> [args]... - registers a command to be run on game exit<br>|
|sar_on_exit_clear|cmd|sar_on_exit_clear - clears commands registered on event "exit"<br>|
|sar_on_exit_list|cmd|sar_on_exit_list - lists commands registered on event "exit"<br>|
|sar_on_flags|cmd|sar_on_flags \<command> [args]... - registers a command to be run when CM flags are hit<br>|
|sar_on_flags_clear|cmd|sar_on_flags_clear - clears commands registered on event "flags"<br>|
|sar_on_flags_list|cmd|sar_on_flags_list - lists commands registered on event "flags"<br>|
|sar_on_load|cmd|sar_on_load \<command> [args]... - registers a command to be run on session start<br>|
|sar_on_load_clear|cmd|sar_on_load_clear - clears commands registered on event "load"<br>|
|sar_on_load_list|cmd|sar_on_load_list - lists commands registered on event "load"<br>|
|sar_on_not_pb|cmd|sar_on_not_pb \<command> [args]... - registers a command to be run when auto-submitter detects not PB<br>|
|sar_on_not_pb_clear|cmd|sar_on_not_pb_clear - clears commands registered on event "not_pb"<br>|
|sar_on_not_pb_list|cmd|sar_on_not_pb_list - lists commands registered on event "not_pb"<br>|
|sar_on_pb|cmd|sar_on_pb \<command> [args]... - registers a command to be run when auto-submitter detects PB<br>|
|sar_on_pb_clear|cmd|sar_on_pb_clear - clears commands registered on event "pb"<br>|
|sar_on_pb_list|cmd|sar_on_pb_list - lists commands registered on event "pb"<br>|
|sar_on_session_end|cmd|sar_on_session_end \<command> [args]... - registers a command to be run on session end<br>|
|sar_on_session_end_clear|cmd|sar_on_session_end_clear - clears commands registered on event "session_end"<br>|
|sar_on_session_end_list|cmd|sar_on_session_end_list - lists commands registered on event "session_end"<br>|
|sar_on_tas_end|cmd|sar_on_tas_end \<command> [args]... - registers a command to be run when TAS script playback ends<br>|
|sar_on_tas_end_clear|cmd|sar_on_tas_end_clear - clears commands registered on event "tas_end"<br>|
|sar_on_tas_end_list|cmd|sar_on_tas_end_list - lists commands registered on event "tas_end"<br>|
|sar_on_tas_start|cmd|sar_on_tas_start \<command> [args]... - registers a command to be run when TAS script playback starts<br>|
|sar_on_tas_start_clear|cmd|sar_on_tas_start_clear - clears commands registered on event "tas_start"<br>|
|sar_on_tas_start_list|cmd|sar_on_tas_start_list - lists commands registered on event "tas_start"<br>|
|sar_paint_reseed|cmd|sar_paint_reseed \<seed> - re-seed all paint sprayers in the map to the given value (-9999 to 9999 inclusive)<br>|
|sar_patch_bhop|0|Patches bhop by limiting wish direction if your velocity is too high.<br>|
|sar_patch_cfg|0|Patches Crouch Flying Glitch.<br>|
|sar_pause_at|-1|Pause at the specified tick. -1 to deactivate it.<br>|
|sar_pause_for|0|Pause for this amount of ticks.<br>|
|sar_pip_align|cmd|sar_pip_align \<top\|center\|bottom> \<left\|center\|right> - aligns the remote view.<br>|
|sar_placement_helper_hud|0|Visually displays all portal placement helpers (requires sv_cheats).<br>|
|sar_portalgun_hud|0|Enables the portalgun HUD.<br>|
|sar_portalgun_hud_x|5|The x position of the portalgun HUD.<br>|
|sar_portalgun_hud_y|5|The y position of the portalgun HUD.<br>|
|sar_portals_thru_portals|0|Allow firing portals through portals.<br>|
|sar_pp_hud|0|Enables or disables the portals placement HUD.<br>|
|sar_pp_hud_font|0|Change font of portal placement hud.<br>|
|sar_pp_hud_opacity|100|Opacity of portal previews.<br>|
|sar_pp_hud_show_blue|0|Enables or disables blue portal preview.<br>|
|sar_pp_hud_show_orange|0|Enables or disables orange portal preview.<br>|
|sar_pp_hud_x|5|x pos of portal placement hud.<br>|
|sar_pp_hud_y|5|y pos of portal placement hud.<br>|
|sar_pp_scan_reset|cmd|sar_pp_scan_reset - reset ppscan.<br>|
|sar_pp_scan_set|cmd|sar_pp_scan_set - set the ppscan point where you're aiming.<br>|
|sar_prevent_ehm|0|Prevents Entity Handle Misinterpretation (EHM) from happening.<br>|
|sar_prevent_mat_snapshot_recompute|0|Shortens loading times by preventing state snapshot recomputation.<br>|
|sar_print_stats|cmd|sar_print_stats - prints your statistics if those are loaded<br>|
|sar_quickhud_mode|0|Set the quickhud mode :<br>0: Default quickhud<br>1: Customizable quickhud<br>2: quickhud from .png<br>|
|sar_quickhud_set_texture|cmd|sar_quickhud_set_texture \<filepath> - enter the base name, it will search for \<filepath>1.png, \<filepath>2.png, \<filepath>3.png and \<filepath>4.png<br>ex: sar_quickhud_set_texture "E:\\Steam\\steamapps\\common\\Portal 2\\portal2\\krzyhau"<br>|
|sar_record_at|-1|Start recording a demo at the tick specified. Will use sar_record_at_demo_name.<br>|
|sar_record_at_demo_name|chamber|Name of the demo automatically recorded.<br>|
|sar_record_at_increment|0|Increment automatically the demo name.<br>|
|sar_record_mkdir|1|Automatically create directories for demos if necessary.<br>|
|sar_record_prefix||A string to prepend to recorded demo names. Can include strftime format codes.<br>|
|sar_rename|cmd|sar_rename \<name> - changes your name<br>|
|sar_render_abitrate|160|Audio bitrate used in renders (kbit/s)<br>|
|sar_render_acodec|aac|Audio codec used in renders (aac, ac3, vorbis, opus, flac)<br>|
|sar_render_autostart|0|Whether to automatically start when demo playback begins<br>|
|sar_render_autostart_extension|mp4|The file extension (and hence container format) to use for automatically started renders.<br>|
|sar_render_autostop|1|Whether to automatically stop when __END__ is seen in demo playback<br>|
|sar_render_blend|0|How many frames to blend for each output frame; 1 = do not blend, 0 = automatically determine based on host_framerate<br>|
|sar_render_blend_mode|1|What type of frameblending to use. 0 = linear, 1 = Gaussian<br>|
|sar_render_finish|cmd|sar_render_finish - stop rendering frames<br>|
|sar_render_fps|60|Render output FPS<br>|
|sar_render_merge|0|When set, merge all the renders until sar_render_finish is entered<br>|
|sar_render_quality|35|Render output quality, higher is better (50=lossless)<br>|
|sar_render_sample_rate|44100|Audio output sample rate<br>|
|sar_render_shutter_angle|360|The shutter angle to use for rendering in degrees.<br>|
|sar_render_skip_coop_videos|1|When set, don't include coop loading time in renders<br>|
|sar_render_start|cmd|sar_render_start \<file> - start rendering frames to the given video file<br>|
|sar_render_vbitrate|40000|Video bitrate used in renders (kbit/s)<br>|
|sar_render_vcodec|h264|Video codec used in renders (h264, hevc, vp8, vp9, dnxhd)<br>|
|sar_rng_load|cmd|sar_rng_load \<filename> - load RNG seed data on next session start<br>|
|sar_rng_save|cmd|sar_rng_save \<filename> - save RNG seed data to the specified file<br>|
|sar_ruler_add|cmd|sar_ruler_add \<x> \<y> \<z> \<x> \<y> \<z> - adds a ruler to a set of currently drawn rulers.<br>|
|sar_ruler_clear|cmd|sar_ruler_clear - clear all created rulers<br>|
|sar_ruler_creator|0|Enables or disables ruler creator<br>0 = Ruler creator disabled<br>1 = Point-to-point ruler creator<br>2 = Player-to-point ruler creator<br>|
|sar_ruler_creator_set|cmd|sar_ruler_creator_set - sets the point, progressing the ruler creation process.<br>|
|sar_ruler_draw|1|Sets the drawing mode of the ruler<br>0 = rulers are not drawn<br>1 = lines, length and angles are drawn (default)<br>2 = only lines and length are drawn<br>3 = only lines are drawn<br>4 = lines, deltas, angles and point origins are drawn<br>|
|sar_ruler_grid_align|1|Aligns ruler creation point to the grid of specified size.<br>|
|sar_ruler_max_trace_dist|16384|Sets maximum trace distance for placing ruler points.<br>|
|sar_scrollspeed|0|Show a HUD indicating your scroll speed.<br>|
|sar_seamshot_finder|0|Enables or disables seamshot finder overlay.<br>|
|sar_session|cmd|sar_session - prints the current tick of the server since it has loaded<br>|
|sar_show_entinp|0|Print all entity inputs to console.<br>|
|sar_skiptodemo|cmd|sar_skiptodemo \<demoname> - skip demos in demo queue to this demo<br>|
|sar_speedrun_autoreset_clear|cmd|sar_speedrun_autoreset_clear - stop using the autoreset file<br>|
|sar_speedrun_autoreset_load|cmd|sar_speedrun_autoreset_load \<file> - load the given file of autoreset timestamps and use it while the speedrun timer is active<br>|
|sar_speedrun_autostop|0|Automatically stop recording demos when a speedrun finishes. If 2, automatically append the run time to the demo name.<br>|
|sar_speedrun_category|cmd|sar_speedrun_category [category] - get or set the speedrun category<br>|
|sar_speedrun_category_add_rule|cmd|sar_speedrun_category_add_rule \<category> \<rule> - add a rule to a speedrun category<br>|
|sar_speedrun_category_create|cmd|sar_speedrun_category_create \<category> - create a new speedrun category with the given name<br>|
|sar_speedrun_category_remove|cmd|sar_speedrun_category_remove \<category> - delete the given speedrun category<br>|
|sar_speedrun_category_remove_rule|cmd|sar_speedrun_category_remove_rule \<category> \<rule> - remove a rule from a speedrun category<br>|
|sar_speedrun_cc_finish|cmd|sar_speedrun_cc_finish - finish the category creator<br>|
|sar_speedrun_cc_place|cmd|sar_speedrun_cc_place - place a trigger-ey rule in the world<br>|
|sar_speedrun_cc_place_start|cmd|sar_speedrun_cc_place_start \<rule name> \<rule type> [options]... - start placing a trigger-ey rule in the world<br>|
|sar_speedrun_cc_rule|cmd|sar_speedrun_cc_rule \<rule name> \<rule type> [options]... - add a rule to the category currently being created<br>|
|sar_speedrun_cc_start|cmd|sar_speedrun_cc_start \<category name> [default options]... - start the category creator<br>|
|sar_speedrun_draw_triggers|0|Draw the triggers associated with speedrun rules in the world.<br>|
|sar_speedrun_export|cmd|sar_speedrun_export \<filename> - export the speedrun result to the specified CSV file<br>|
|sar_speedrun_export_all|cmd|sar_speedrun_export_all \<filename> - export the results of many speedruns to the specified CSV file<br>|
|sar_speedrun_offset|0|Start speedruns with this time on the timer.<br>|
|sar_speedrun_pause|cmd|sar_speedrun_pause - pause the speedrun timer<br>|
|sar_speedrun_recover|cmd|sar_speedrun_recover \<ticks\|time> - recover a crashed run by resuming the timer at the given time on next load<br>|
|sar_speedrun_reset|cmd|sar_speedrun_reset - reset the speedrun timer<br>|
|sar_speedrun_reset_categories|cmd|sar_speedrun_reset_categories - delete all custom categories and rules, reverting to the builtin ones<br>|
|sar_speedrun_reset_export|cmd|sar_speedrun_reset_export - reset the log of complete and incomplete runs to be exported<br>|
|sar_speedrun_result|cmd|sar_speedrun_result - print the speedrun result<br>|
|sar_speedrun_resume|cmd|sar_speedrun_resume - resume the speedrun timer<br>|
|sar_speedrun_rule|cmd|sar_speedrun_rule [rule] - show information about speedrun rules<br>|
|sar_speedrun_rule_create|cmd|sar_speedrun_rule_create \<name> \<type> [option=value]... - create a speedrun rule with the given name, type, and options<br>|
|sar_speedrun_rule_remove|cmd|sar_speedrun_rule_remove \<rule> - delete the given speedrun rule<br>|
|sar_speedrun_skip_cutscenes|0|Skip Tube Ride and Long Fall in Portal 2.<br>|
|sar_speedrun_smartsplit|1|Only split the speedrun timer a maximum of once per map.<br>|
|sar_speedrun_split|cmd|sar_speedrun_split - perform a split on the speedrun timer<br>|
|sar_speedrun_start|cmd|sar_speedrun_start - start the speedrun timer<br>|
|sar_speedrun_start_on_load|0|Automatically start the speedrun timer when a map is loaded. 2 = restart if active.<br>|
|sar_speedrun_stop|cmd|sar_speedrun_stop - stop the speedrun timer<br>|
|sar_speedrun_stop_in_menu|0|Automatically stop the speedrun timer when the menu is loaded.<br>|
|sar_speedrun_time_pauses|0|Include time spent paused in the speedrun timer.<br>|
|sar_sr_hud|0|Draws speedrun timer.<br>|
|sar_sr_hud_font_color|255 255 255 255|RGBA font color of speedrun timer HUD.<br>|
|sar_sr_hud_font_index|70|Font index of speedrun timer HUD.<br>|
|sar_sr_hud_x|0|X offset of speedrun timer HUD.<br>|
|sar_sr_hud_y|100|Y offset of speedrun timer HUD.<br>|
|sar_startdemos|cmd|sar_startdemos \<demoname> - improved version of startdemos. Use 'stopdemo' to stop playing demos<br>|
|sar_startdemosfolder|cmd|sar_startdemosfolder \<folder name> - plays all the demos in the specified folder by alphabetic order<br>|
|sar_statcounter_filePath|Stats/phunkpaiDWPS.csv|Path to the statcounter .csv file.<br>|
|sar_stats_auto_reset|0|Resets all stats automatically.<br>0 = Default,<br>1 = Restart or disconnect only,<br>2 = Any load & sar_timer_start.<br>Note: Portal counter is not part of the "stats" feature.<br>|
|sar_stats_jump|cmd|sar_stats_jump - prints jump stats<br>|
|sar_stats_jumps_reset|cmd|sar_stats_jumps_reset - resets total jump count and jump distance peak<br>|
|sar_stats_jumps_xy|0|Saves jump distance as 2D vector.<br>|
|sar_stats_reset|cmd|sar_stats_reset - resets all saved stats<br>|
|sar_stats_steps|cmd|sar_stats_steps - prints total amount of steps<br>|
|sar_stats_steps_reset|cmd|sar_stats_steps_reset - resets total step count<br>|
|sar_stats_velocity|cmd|sar_stats_velocity - prints velocity stats<br>|
|sar_stats_velocity_peak_xy|0|Saves velocity peak as 2D vector.<br>|
|sar_stats_velocity_reset|cmd|sar_stats_velocity_reset - resets velocity peak<br>|
|sar_stitcher|0|Enable the image stitcher.<br>|
|sar_stitcher_reset|cmd|sar_stitcher_reset - reset the stitcher.<br>|
|sar_stop|cmd|sar_stop \<name> - stop recording the current demo and rename it to 'name' (not considering sar_record_prefix)<br>|
|sar_strafe_quality|0|Enables or disables the strafe quality HUD.<br>|
|sar_strafe_quality_height|50|The height of the strafe quality HUD.<br>|
|sar_strafe_quality_ticks|40|The number of ticks to average over for the strafe quality HUD.<br>|
|sar_strafe_quality_width|300|The width of the strafe quality HUD.<br>|
|sar_strafehud|0|Enables or disables strafe hud.<br>|
|sar_strafehud_avg_sample_count|60|How many samples to use for average counter.<br>|
|sar_strafehud_detail_scale|4|The detail scale for the lines of hud.<br>|
|sar_strafehud_font|13|Font used by strafe hud.<br>|
|sar_strafehud_lock_mode|1|Lock mode used by strafe hud:<br>0 - view direction<br>1 - velocity direction<br>2 - absolute angles<br>|
|sar_strafehud_match_accel_scale|0|Match the scales for minimum and maximum deceleration.<br>|
|sar_strafehud_size|256|The width and height of the strafe hud.<br>|
|sar_strafehud_use_friction|0|Use ground friction when calculating acceleration.<br>|
|sar_strafehud_x|-10|The X position of the strafe hud.<br>|
|sar_strafehud_y|10|The Y position of the strafe hud.<br>|
|sar_strafesync|0|Shows strafe sync stats.<br>|
|sar_strafesync_noground|1|0: Always run.<br>1: Do not run when on ground.<br>|
|sar_strafesync_pause|cmd|sar_strafesync_pause - pause strafe sync session<br>|
|sar_strafesync_reset|cmd|sar_strafesync_reset - reset strafe sync session<br>|
|sar_strafesync_resume|cmd|sar_strafesync_resume - resume strafe sync session<br>|
|sar_strafesync_session_time|0|In seconds. How much time should pass until session is reset.<br>If 0, you'll have to reset the session manually.<br>|
|sar_strafesync_split|cmd|sar_strafesync_split - makes a new split<br>|
|sar_sum_during_session|1|Updates the summary counter automatically during a session.<br>|
|sar_sum_here|cmd|sar_sum_here - starts counting total ticks of sessions<br>|
|sar_sum_result|cmd|sar_sum_result - prints result of summary<br>|
|sar_sum_stop|cmd|sar_sum_stop - stops summary counter<br>|
|sar_tas_advance|cmd|sar_tas_advance - advances TAS playback by one tick<br>|
|sar_tas_autosave_raw|1|Enables automatic saving of raw, processed TAS scripts.<br>|
|sar_tas_check_disable|0|Globally disable the 'check' TAS tool.<br>|
|sar_tas_check_max_replays|15|Maximum replays for the 'check' TAS tool until it gives up.<br>|
|sar_tas_debug|0|Debug TAS informations. 0 - none, 1 - basic, 2 - all.<br>|
|sar_tas_dump_player_info|0|Dump player info for each tick of TAS playback to a file.<br>|
|sar_tas_dump_usercmd|0|Dump TAS-generated usercmds to a file.<br>|
|sar_tas_interpolate|0|Preserve client interpolation in TAS playback.<br>|
|sar_tas_pause|cmd|sar_tas_pause - pauses TAS playback<br>|
|sar_tas_pauseat|0|Pauses the TAS playback on specified tick.<br>|
|sar_tas_play|cmd|sar_tas_play \<filename> [filename2] - plays a TAS script with given name. If two script names are given, play coop<br>|
|sar_tas_play_single|cmd|sar_tas_play_single \<filename> [slot] - plays a single coop TAS script, giving the player control of the other slot.<br>|
|sar_tas_playback_rate|1.0|The rate at which to play back TAS scripts.<br>|
|sar_tas_real_controller_debug|0|Debugs controller.<br>|
|sar_tas_replay|cmd|sar_tas_replay - replays the last played TAS<br>|
|sar_tas_restore_fps|1|Restore fps_max and host_framerate after TAS playback.<br>|
|sar_tas_resume|cmd|sar_tas_resume - resumes TAS playback<br>|
|sar_tas_save_raw|cmd|sar_tas_save_raw - saves a processed version of just processed script<br>|
|sar_tas_server|0|Enable the remote TAS server. Setting this value to something higher than one will bind the server on that port.<br>|
|sar_tas_skipto|0|Fast-forwards the TAS playback until given playback tick.<br>|
|sar_tas_stop|cmd|sar_tas_stop - stop TAS playing<br>|
|sar_tas_tools_enabled|1|Enables tool processing for TAS script making.<br>|
|sar_tas_tools_force|0|Force tool playback for TAS scripts; primarily for debugging.<br>|
|sar_teleport|cmd|sar_teleport [noportals] - teleports the player to the last saved location<br>|
|sar_teleport_setpos|cmd|sar_teleport_setpos - saves current location for teleportation<br>|
|sar_tick_debug|0|Output debugging information to the console related to ticks and frames.<br>|
|sar_time_demo|cmd|sar_time_demo \<demo_name> - parses a demo and prints some information about it<br>|
|sar_time_demo_dev|0|Printing mode when using sar_time_demo.<br>0 = Default,<br>1 = Console commands,<br>2 = Console commands & packets.<br>|
|sar_time_demos|cmd|sar_time_demos \<demo_name> [demo_name2]... - parses multiple demos and prints the total sum of them<br>|
|sar_timer_always_running|1|Timer will save current value when disconnecting.<br>|
|sar_timer_result|cmd|sar_timer_result - prints result of timer<br>|
|sar_timer_start|cmd|sar_timer_start - starts timer<br>|
|sar_timer_stop|cmd|sar_timer_stop - stops timer<br>|
|sar_timer_time_pauses|1|Timer adds non-simulated ticks when server pauses.<br>|
|sar_toast_align|0|The side to align toasts to horizontally. 0 = left, 1 = center, 2 = right.<br>|
|sar_toast_anchor|1|Where to put new toasts. 0 = bottom, 1 = top.<br>|
|sar_toast_background|1|Sets the background highlight for toasts. 0 = no background, 1 = text width only, 2 = full width.<br>|
|sar_toast_compact|0|Enables a compact form of the toasts HUD.<br>|
|sar_toast_create|cmd|sar_toast_create \<tag> \<text> - create a toast<br>|
|sar_toast_disable|0|Disable all toasts from showing.<br>|
|sar_toast_dismiss_all|cmd|sar_toast_dismiss_all - dismiss all active toasts<br>|
|sar_toast_font|6|The font index to use for toasts.<br>|
|sar_toast_net_create|cmd|sar_toast_net_create \<tag> \<text> - create a toast, also sending it to your coop partner<br>|
|sar_toast_net_disable|0|Disable network toasts.<br>|
|sar_toast_setpos|cmd|sar_toast_setpos \<bottom\|top> \<left\|center\|right> - set the position of the toasts HUD<br>|
|sar_toast_tag_dismiss_all|cmd|sar_toast_tag_dismiss_all \<tag> - dismiss all active toasts with the given tag<br>|
|sar_toast_tag_set_color|cmd|sar_toast_tag_set_color \<tag> \<color> - set the color of the specified toast tag to an sRGB color<br>|
|sar_toast_tag_set_duration|cmd|sar_toast_tag_set_duration \<tag> \<duration> - set the duration of the specified toast tag in seconds. The duration may be given as 'forever'<br>|
|sar_toast_width|250|The maximum width for toasts.<br>|
|sar_toast_x|10|The horizontal position of the toasts HUD.<br>|
|sar_toast_y|10|The vertical position of the toasts HUD.<br>|
|sar_togglewait|cmd|sar_togglewait - enables or disables "wait" for the command buffer<br>|
|sar_trace_autoclear|1|Automatically clear the trace on session start<br>|
|sar_trace_bbox_at|-1|Display a player-sized bbox at the given tick.<br>|
|sar_trace_bbox_ent_dist|200|Distance from which to capture entity hitboxes.<br>|
|sar_trace_bbox_ent_draw|1|Draw hitboxes of nearby entities in the trace.<br>|
|sar_trace_bbox_ent_record|1|Record hitboxes of nearby entities in the trace. You may want to disable this if memory consumption gets too high.<br>|
|sar_trace_bbox_use_hover|0|Move trace bbox to hovered trace point tick on given trace.<br>|
|sar_trace_clear|cmd|sar_trace_clear \<name> - Clear player trace with a given name<br>|
|sar_trace_clear_all|cmd|sar_trace_clear_all - Clear all the traces<br>|
|sar_trace_draw|0|Display the recorded player trace. Requires cheats<br>|
|sar_trace_draw_speed_deltas|0|Display the speed deltas. Requires sar_trace_draw<br>|
|sar_trace_draw_through_walls|1|Display the player trace through walls. Requires sar_trace_draw<br>|
|sar_trace_draw_time|3|Display tick above trace hover info<br>0 = hide tick info<br>1 = ticks since trace recording started<br>2 = session timer<br>3 = TAS timer (if no TAS was played, uses 1 instead)<br>|
|sar_trace_dump|cmd|sar_trace_dump \<tick> [player slot] [trace name] - dump the player state from the given trace tick on the given trace ID (defaults to 1) in the given slot (defaults to 0).<br>|
|sar_trace_export|cmd|sar_trace_export \<filename> [trace name] - Export trace data into a csv file.<br>|
|sar_trace_font_size|3.0|The size of text overlaid on recorded traces.<br>|
|sar_trace_override|1|Clears old trace when you start recording to it instead of recording on top of it.<br>|
|sar_trace_portal_opacity|100|Opacity of trace portal previews.<br>|
|sar_trace_portal_oval|0|Draw trace portals as ovals rather than rectangles.<br>|
|sar_trace_portal_record|1|Record portal locations.<br>|
|sar_trace_record|0|Record the trace to a slot. Set to 0 for not recording<br>|
|sar_trace_teleport_at|cmd|sar_trace_teleport_at \<tick> [player slot] [trace name] - teleports the player at the given trace tick on the given trace ID (defaults to hovered one or the first one ever made) in the given slot (defaults to 0).<br>|
|sar_trace_teleport_eye|cmd|sar_trace_teleport_eye \<tick> [player slot] [trace name] - teleports the player to the eye position at the given trace tick on the given trace (defaults to hovered one or the first one ever made) in the given slot (defaults to 0).<br>|
|sar_trace_use_shot_eyeoffset|1|Uses eye offset and angles accurate for portal shooting.<br>|
|sar_transition_timer|0|Output how slow your dialogue fade was.|
|sar_update|cmd|sar_update [release\|pre] [exit] [force] - update SAR to the latest version. If exit is given, exit the game upon successful update; if force is given, always re-install, even if it may be a downgrade<br>|
|sar_velocitygraph|0|Shows velocity graph.<br>|
|sar_velocitygraph_background|0|Background of velocity graph.<br>|
|sar_velocitygraph_font_index|21|Font index of velocity graph.<br>|
|sar_velocitygraph_rainbow|0|Rainbow mode of velocity graph text.<br>|
|sar_velocitygraph_show_speed_on_graph|1|Show speed between jumps.<br>|
|sar_vphys_hud|0|Enables or disables the vphys HUD.<br>|
|sar_vphys_hud_font|1|Sets font of the vphys HUD.<br>|
|sar_vphys_hud_precision|3|Sets decimal precision of the vphys HUD.<br>|
|sar_vphys_hud_show_hitboxes|2|Sets visibility of hitboxes when vphys hud is active.<br>0 = hitboxes are not drawn<br>1 = only active vphys hitbox is drawn<br>2 = active vphys and player's bounding box are drawn<br>3 = both vphys hitboxes and player's bounding box are drawn<br>|
|sar_vphys_hud_x|0|The x position of the vphys HUD.<br>|
|sar_vphys_hud_y|0|The y position of the vphys HUD.<br>|
|sar_vphys_setangle|cmd|sar_vphys_setangle \<hitbox> \<angle> - sets rotation angle to either standing (0) or crouching (1) havok collision shadow<br>|
|sar_vphys_setasleep|cmd|sar_vphys_setasleep \<hitbox> \<asleep> - sets whether your standing (0) or crouching (1) havok collision shadow is asleep<br>|
|sar_vphys_setgravity|cmd|sar_vphys_setgravity \<hitbox> \<enabled> - sets gravity flag state to either standing (0) or crouching (1) havok collision shadow<br>|
|sar_vphys_setspin|cmd|sar_vphys_setspin \<hitbox> \<angvel> - sets rotation speed to either standing (0) or crouching (1) havok collision shadow<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop</i>|cmd|sar_workshop \<file> [ss/changelevel] - same as "map" command but lists workshop maps<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop_list</i>|cmd|sar_workshop_list - prints all workshop maps<br>|
|sar_workshop_skip|cmd|sar_workshop_skip - Skips to the next level in workshop<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop_update</i>|cmd|sar_workshop_update - updates the workshop map list<br>|
|seq|cmd|seq \<commands>... - runs a sequence of commands one tick after one another<br>|
|svar_abs|cmd|svar_abs \<variable> - perform the given operation on an svar<br>|
|svar_add|cmd|svar_add \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_capture|cmd|svar_capture \<variable> \<command> [args]... - capture a command's output and place it into an svar, removing newlines<br>|
|svar_ceil|cmd|svar_ceil \<variable> - perform the given operation on an svar<br>|
|svar_count|cmd|svar_count - prints a count of all the defined svars<br>|
|svar_div|cmd|svar_div \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_fadd|cmd|svar_fadd \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_fdiv|cmd|svar_fdiv \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_floor|cmd|svar_floor \<variable> - perform the given operation on an svar<br>|
|svar_fmod|cmd|svar_fmod \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_fmul|cmd|svar_fmul \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_from_cvar|cmd|svar_from_cvar \<variable> \<cvar> - capture a cvar's value and place it into an svar, removing newlines<br>|
|svar_fsub|cmd|svar_fsub \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_get|cmd|svar_get \<variable> - get the value of a svar<br>|
|svar_mod|cmd|svar_mod \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_mul|cmd|svar_mul \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_no_persist|cmd|svar_no_persist \<variable> - unmark an svar as persistent<br>|
|svar_persist|cmd|svar_persist \<variable> - mark an svar as persistent<br>|
|svar_round|cmd|svar_round \<variable> - perform the given operation on an svar<br>|
|svar_set|cmd|svar_set \<variable> \<value> - set a svar (SAR variable) to a given value<br>|
|svar_sub|cmd|svar_sub \<variable> \<variable\|value> - perform the given operation on an svar<br>|
|svar_substr|cmd|svar_substr \<variable> \<from> [len] - sets a svar to its substring.<br>|
|wait|cmd|wait \<tick> \<commands> - wait for the amount of ticks specified<br>|
|wait_for|cmd|wait_for \<tick> \<commands> - wait for the amount of ticks specified<br>|
|wait_mode|0|When the pending commands should be executed. 0 is absolute, 1 is relative to when you entered the wait command.<br>|
|wait_persist_across_loads|0|Whether pending commands should be carried across loads (1) or just be dropped (0).<br>|
|wait_to|cmd|wait_to \<tick> \<commands> - run this command on the specified session tick<br>|
