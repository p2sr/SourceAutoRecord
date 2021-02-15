# SAR: Cvars

|Name|Default|Description|
|---|---|---|
|<i title="Portal 2 Engine">+autostrafe</i>|cmd|Auto-strafe button.<br>|
|<i title="Portal 2 Engine">-autostrafe</i>|cmd|Auto-strafe button.<br>|
|<i title="The Stanley Parable">+bhop</i>|cmd|Client sends a key-down event for the in_jump state.<br>|
|cl_crosshair_t|0|Removes the top line from the crosshair :0 : normal crosshair,1 : crosshair without top.<br>|
|cl_crosshairalpha|255|Change the amount of transparency.<br>|
|cl_crosshaircolor_b|0|Changes the color of the crosshair.<br>|
|cl_crosshaircolor_g|255|Changes the color of the crosshair.<br>|
|cl_crosshaircolor_r|0|Changes the color of the crosshair.<br>|
|cl_crosshairdot|1|Decides if there is a dot in the middle of the crosshair<br>|
|cl_crosshairgap|5|Changes the distance of the crosshair lines from the center of screen.<br>|
|cl_crosshairsize|5|Changes the size of the crosshair.<br>|
|cl_crosshairthickness|0|Changes the thinkness of the crosshair lines.<br>|
|cl_quickhud_alpha|255|Change the amount of transparency.<br>|
|cl_quickhudleftcolor_b|86|Changes the color of the left quickhud.<br>|
|cl_quickhudleftcolor_g|184|Changes the color of the left quickhud.<br>|
|cl_quickhudleftcolor_r|255|Changes the color of the left quickhud.<br>|
|cl_quickhudrightcolor_b|255|Changes the color of the right quickhud.<br>|
|cl_quickhudrightcolor_g|184|Changes the color of the right quickhud.<br>|
|cl_quickhudrightcolor_r|111|Changes the color of the right quickhud.<br>|
|cond|cmd|cond [condition] [command] - runs a command only if a given condition is met.<br>|
|ghost_connect|cmd|Connect to the server : \<ip address> \<port> :<br>ex: 'localhost 53000' - '127.0.0.1 53000' - 89.10.20.20 53000'.<br>|
|ghost_delete_all|cmd|Delete all ghosts.<br>|
|ghost_delete_by_ID|cmd|ghost_delete_by_ID \<ID>. Delete the ghost selected.<br>|
|ghost_disconnect|cmd|Disconnect.<br>|
|ghost_height|16|Height of the ghosts.<br>|
|ghost_message|cmd|Send message to other players.<br>|
|ghost_name|cmd|Change your online name.<br>|
|ghost_offset|cmd|ghost_offset \<offset> \<ID>. Delay the ghost start by \<offset> frames.<br>|
|ghost_ping|cmd|Pong!<br>|
|ghost_prop_model|cmd|Set the prop model. Example : models/props/metal_box.mdl<br>|
|ghost_proximity_fade|200|Distance from ghosts at which their models fade out.<br>|
|ghost_recap|cmd|Recap all ghosts setup.<br>|
|ghost_reset|cmd|Reset ghosts.<br>|
|ghost_set_demo|cmd|ghost_set_demo \<demo> [ID]. Ghost will use this demo. If ID is specified, will create or modify the ID-�me ghost.<br>|
|ghost_set_demos|cmd|ghost_set_demos \<first_demo> [ID]. Ghost will setup a speedrun with first_demo, first_demo_2, etc.<br>If ID is specified, will create or modify the ID-th ghost.<br>|
|ghost_show_advancement|1|Show the advancement of the ghosts.<br>|
|ghost_start|cmd|Start ghosts|
|ghost_sync|0|When loading a new level, pauses the game until other players load it.<br>|
|ghost_TCP_only|0|Lathil's special command :).<br>|
|ghost_text_offset|20|Offset of the name over the ghosts.<br>|
|ghost_transparency|255|Transparency of the ghosts.<br>|
|ghost_type|cmd|ghost_type \<0/1/2>:<br>0: Ghost not recorded in demos<br>1: Ghost using props model but recorded in demos (NOT RECOMMENDED!)<br>2: Ghost has mini portalgun which is recorded in demos (NOT RECOMMENDED!)<br>|
|ghost_update_rate|50|Adjust the update rate. For people with lathil's internet.<br>|
|sar_about|cmd|Prints info about SAR plugin.<br>|
|<i title="Portal 2 Engine">sar_aircontrol</i>|0|Enables more air-control on the server.<br>|
|sar_autojump|0|Enables automatic jumping on the server.<br>|
|sar_autorecord|0|Enables automatic demo recording.<br>|
|sar_avg_result|cmd|Prints result of average.<br>|
|sar_avg_start|cmd|Starts calculating the average when using timer.<br>|
|sar_avg_stop|cmd|Stops average calculation.<br>|
|sar_bind_reload|cmd|Automatic save-reload rebinding when server has loaded.<br>File indexing will be synced when recording demos.<br>Usage: sar_bind_reload \<key> [save_name]<br>|
|sar_bind_save|cmd|Automatic save rebinding when server has loaded.<br>File indexing will be synced when recording demos.<br>Usage: sar_bind_save \<key> [save_name]<br>|
|sar_cam_control|0|sar_cam_control \<type>: Change type of camera control.<br>0 = Default (camera is controlled by game engine),<br>1 = Drive mode (camera is separated and can be controlled by user input),<br>2 = Cinematic mode (camera is controlled by predefined path).<br>|
|sar_cam_drive|1|Enables or disables camera drive mode in-game (turning it on is not required for demo player)<br>|
|sar_cam_path_getkfs|cmd|sar_cam_path_getkfs : Exports commands for recreating currently made camera path.<br>|
|sar_cam_path_remkf|cmd|sar_cam_path_remkf [frame] : Removes camera path keyframe at specified frame.<br>|
|sar_cam_path_remkfs|cmd|sar_cam_path_remkfs : Removes all camera path keyframes.<br>|
|sar_cam_path_setkf|cmd|sar_cam_path_setkf [frame] [x] [y] [z] [yaw] [pitch] [roll] [fov]: Sets the camera path keyframe.<br>|
|sar_cam_path_showkf|cmd|sar_cam_path_showkf [frame] : Display information about camera path keyframe at specified frame.<br>|
|sar_cam_path_showkfs|cmd|sar_cam_path_showkfs : Display information about all camera path keyframes.<br>|
|sar_cam_reset|cmd|sar_cam_reset: Resets camera to its default position.<br>|
|sar_cam_setang|cmd|sar_cam_setang \<pitch> \<yaw> [roll] : Sets camera angle (requires camera Drive Mode).<br>|
|sar_cam_setfov|cmd|sar_cam_setfov \<fov>: Sets camera field of view (requires camera Drive Mode).<br>|
|sar_cam_setpos|cmd|sar_cam_setpos \<x> \<y> \<z> : Sets camera position (requires camera Drive Mode).<br>|
|sar_challenge_autostop|0|Automatically stops recording demos when the leaderboard opens after a CM run.<br>|
|sar_clear_lines|cmd|Clears all active drawline overlays.<br>|
|sar_cps_add|cmd|Saves current time of timer.<br>|
|sar_cps_clear|cmd|Resets saved times of timer.<br>|
|sar_cps_result|cmd|Prints result of timer checkpoints.<br>|
|sar_crosshair_mode|0|Set the crosshair mode :<br>0 : Default crosshair<br>1 : Customizable crosshair<br>2 : Crosshair from .png<br>|
|sar_crosshair_P1|0|Use the P1 crosshair style.<br>|
|sar_crosshair_set_texture|cmd|sar_crosshair_set_texture \<filepath><br>|
|sar_cvarlist|cmd|Lists all SAR cvars and unlocked engine cvars.<br>|
|sar_cvars_dump|cmd|Dumps all cvars to a file.<br>|
|sar_cvars_dump_doc|cmd|Dumps all SAR cvars to a file.<br>|
|sar_cvars_load|cmd|Loads important SAR cvars.<br>|
|sar_cvars_lock|cmd|Restores default flags of unlocked cvars.<br>|
|sar_cvars_save|cmd|Saves important SAR cvars.<br>|
|sar_cvars_unlock|cmd|Unlocks all special cvars.<br>|
|<i title="Portal 2 Game">sar_debug_listener</i>|0|Prints event data of registered listener.<br>|
|<i title="Portal 2&#10;Portal&#10;Half-Life 2&#10;Aperture Tag&#10;Portal Stories: Mel&#10;Thinking with Time Machine&#10;Half-Life 2: Episode One/Two&#10;Half-Life: Source">sar_delete_alias_cmds</i>|cmd|Deletes all alias commands.<br>|
|<i title="Portal 2">sar_disable_challenge_stats_hud</i>|0|Disables opening the challenge mode stats HUD.<br>|
|<i title="Portal 2 Engine">sar_disable_no_focus_sleep</i>|0|Does not yield the CPU when game is not focused.<br>|
|<i title="Portal 2 Game">sar_disable_progress_bar_update</i>|0|Disables excessive usage of progress bar.<br>|
|<i title="Portal 2 Game">sar_disable_steam_pause</i>|0|Prevents pauses from steam overlay.<br>|
|<i title="Portal 2 Engine">sar_duckjump</i>|0|Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.<br>|
|sar_dump_client_classes|cmd|Dumps all client classes to a file.<br>|
|sar_dump_client_datamap|cmd|Dumps client datmap to a file.<br>|
|<i title="Portal 2 Game">sar_dump_events</i>|cmd|Dumps all registered game events of the game event manager.<br>|
|sar_dump_server_classes|cmd|Dumps all server classes to a file.<br>|
|sar_dump_server_datamap|cmd|Dumps server datamap to a file.<br>|
|sar_ei_hud|0|Draws entity inspection data.<br>|
|sar_ei_hud_font_color|255 255 255 255|RGBA font color of entity inspection HUD when not recording.<br>|
|sar_ei_hud_font_color2|153 23 9 255|RGBA font color of entity inspection HUD when recording.<br>|
|sar_ei_hud_font_index|1|Font index of entity inspection HUD.<br>|
|sar_ei_hud_x|0|X offset of entity inspection HUD.<br>|
|sar_ei_hud_y|0|Y offset of entity inspection HUD.<br>|
|sar_ei_hud_z|0|Z offset of entity inspection HUD.<br>|
|sar_exit|cmd|Removes all function hooks, registered commands and unloads the module.<br>|
|sar_export_stats|cmd|sar_export_stats [filePath]. Export the stats to the specifed path in a .csv file.<br>|
|sar_fast_load_preset|cmd|set_fast_load_preset \<preset>. Sets all loading fixes to preset values.<br>|
|sar_find_client_class|cmd|Finds specific client class tables and props with their offset.<br>Usage: sar_find_clientclass \<class_name><br>|
|sar_find_client_offset|cmd|Finds prop offset in specified client class.<br>Usage: sar_find_client_offset \<class_name> \<prop_name><br>|
|sar_find_ent|cmd|Finds entity in the entity list by name.<br>Usage: sar_find_ent \<m_iName><br>|
|sar_find_ents|cmd|Finds entities in the entity list by class name.<br>Usage: sar_find_ents \<m_iClassName><br>|
|sar_find_server_class|cmd|Finds specific server class tables and props with their offset.<br>Usage: sar_find_serverclass \<class_name><br>|
|sar_find_server_offset|cmd|Finds prop offset in specified server class.<br>Usage: sar_find_server_offset \<class_name> \<prop_name><br>|
|sar_force_fov|cmd|Forces player FOV. Usage: sar_force_fov \<fov><br>|
|sar_hud_acceleration|0|Draws instant acceleration.<br>|
|sar_hud_angles|0|Draws absolute view angles of the client.<br>0 = Default,<br>1 = XY,<br>2 = XYZ.<br>|
|sar_hud_avg|0|Draws calculated average of timer.<br>|
|sar_hud_cps|0|Draws latest checkpoint of timer.<br>|
|sar_hud_default_font_color|255 255 255 255|RGBA font color of HUD.<br>|
|sar_hud_default_font_index|0|Font index of HUD.<br>|
|sar_hud_default_order_bottom|cmd|Orders hud element to bottom : sar_hud_default_order_bottom \<name><br>|
|sar_hud_default_order_reset|cmd|Resets order of hud element.<br>|
|sar_hud_default_order_top|cmd|Orders hud element to top. Usage: sar_hud_default_order_top \<name><br>|
|sar_hud_default_padding_x|2|X padding of HUD.<br>|
|sar_hud_default_padding_y|2|Y padding of HUD.<br>|
|sar_hud_default_spacing|1|Spacing between elements of HUD.<br>|
|sar_hud_demo|0|Draws name, tick and time of current demo.<br>|
|sar_hud_frame|0|Draws current frame count.<br>|
|sar_hud_ghost_show_name|1|Display the name of the ghost over it.<br>|
|sar_hud_groundframes|0|Draws the number of ground frames since last landing.<br>|
|sar_hud_hide_text|cmd|sar_hud_hide_text \<id>. Hides the nth text value in the HUD.<br>|
|sar_hud_inspection|0|Draws entity inspection data.<br>|
|sar_hud_jump|0|Draws current jump distance.<br>|
|sar_hud_jump_peak|0|Draws longest jump distance.<br>|
|sar_hud_jumps|0|Draws total jump count.<br>|
|sar_hud_last_frame|0|Draws last saved frame value.<br>|
|sar_hud_last_session|0|Draws value of latest completed session.<br>|
|sar_hud_pause_timer|0|Draws current value of pause timer.<br>|
|sar_hud_player_info|0|Draws player state defined with sar_tas_set_prop.<br>|
|sar_hud_portals|0|Draws total portal count.<br>|
|sar_hud_position|0|Draws absolute position of the client.<br>0 = Default,<br>1 = Player position,<br>2 = Camera position.<br>|
|sar_hud_precision|3|Precision of HUD numbers.<br>|
|sar_hud_session|0|Draws current session tick.<br>|
|sar_hud_set_text|cmd|sar_hud_set_text \<id> \<text>. Sets and shows the nth text value in the HUD.<br>|
|sar_hud_show_text|cmd|sar_hud_show_text \<id>. Shows the nth text value in the HUD.<br>|
|sar_hud_steps|0|Draws total step count.<br>|
|sar_hud_strafesync_color|0 150 250 255|RGBA font color of strafesync HUD.<br>|
|sar_hud_strafesync_font_index|1|Font index of strafesync HUD.<br>|
|sar_hud_strafesync_offset_x|0|X offset of strafesync HUD.<br>|
|sar_hud_strafesync_offset_y|1000|Y offset of strafesync HUD.<br>|
|sar_hud_strafesync_split_offset_y|1050|Y offset of strafesync HUD.<br>|
|sar_hud_sum|0|Draws summary value of sessions.<br>|
|sar_hud_text||DEPRECATED: Use sar_hud_set_text.|
|sar_hud_text||DEPRECATED: Use sar_hud_set_text.|
|sar_hud_timer|0|Draws current value of timer.<br>|
|sar_hud_trace|0|Draws distance values of tracer. 0 = Default,<br>1 = Vec3,<br>2 = Vec2.<br>|
|sar_hud_velocity|0|Draws velocity of the client.<br>0 = Default,<br>1 = X/Y/Z,<br>2 = X/Y,<br>3 = X : Y : Z.<br>|
|sar_hud_velocity_angle|0|Draws velocity angles.<br>|
|sar_hud_velocity_peak|0|Draws last saved velocity peak.<br>|
|sar_ihud|0|Draws movement inputs of client.<br>0 = Default,<br>1 = forward;back;moveleft;moveright,<br>2 = 1 + duck;jump;use,<br>3 = 2 + attack;attack2,<br>4 = 3 + speed;reload.<br>|
|sar_ihud_button_color|0 0 0 255|RGBA button color of input HUD.<br>|
|sar_ihud_button_padding|2|Padding between buttons of input HUD.<br>|
|sar_ihud_button_size|60|Button size of input HUD.<br>|
|sar_ihud_font_color|255 255 255 255|RGBA font color of input HUD.<br>|
|sar_ihud_font_index|1|Font index of input HUD.<br>|
|sar_ihud_layout|WASDCSELRSR|Layout of input HUD.<br>Labels are in this order:<br>forward,<br>moveleft,<br>back,<br>moveright,<br>duck,<br>jump,<br>use,<br>attack,<br>attack2,<br>speed,<br>reload.<br>Pass an empty string to disable drawing labels completely.<br>|
|sar_ihud_setlayout|cmd|Suggests keyboard layouts for sar_ihud_layout.<br>|
|sar_ihud_setpos|cmd|Sets automatically the position of input HUD.<br>Usage: sar_ihud_setpos \<top, center or bottom> \<left, center or right><br>|
|sar_ihud_shadow|1|Draws button shadows of input HUD.<br>|
|sar_ihud_shadow_color|0 0 0 64|RGBA button shadow color of input HUD.<br>|
|sar_ihud_shadow_font_color|255 255 255 64|RGBA button shadow font color of input HUD.<br>|
|sar_ihud_x|0|X offset of input HUD.<br>|
|sar_ihud_y|0|Y offset of input HUD.<br>|
|sar_import_stats|cmd|sar_import_stats [filePath]. Import the stats from the specified .csv file.<br>|
|sar_inspection_export|cmd|Saves recorded entity data to a csv file.<br>Usage: sar_inspection_export \<file_name><br>|
|sar_inspection_index|cmd|Sets entity index for inspection.<br>|
|sar_inspection_print|cmd|Prints recorded entity data.<br>|
|sar_inspection_save_every_tick|0|Saves inspection data even when session tick does not increment.<br>|
|sar_inspection_start|cmd|Starts recording entity data.<br>|
|sar_inspection_stop|cmd|Stops recording entity data.<br>|
|<i title="Portal 2 Engine">sar_jumpboost</i>|0|Enables special game movement on the server.<br>0 = Default,<br>1 = Orange Box Engine,<br>2 = Pre-OBE.<br>|
|sar_list_client_classes|cmd|Lists all client classes.<br>|
|sar_list_ents|cmd|Lists entities.<br>|
|sar_list_server_classes|cmd|Lists all server classes.<br>|
|sar_loads_norender|0|Temporatily set mat_noredering to 1 during loads<br>|
|sar_loads_uncap|0|Temporarily set fps_max to 0 during loads<br>|
|<i title="Portal 2 Game">sar_mimic</i>|0|Copies inputs to secondary split screen. Similar to ss_mimic.<br>|
|sar_mtrigger_add|cmd|Usage 1 -> sar_mtrigger_add \<id> \<A.x> \<A.y> \<A.z> \<B.x> \<B.y> \<B.z> [angle] : add a trigger with the specified ID, position, and optional angle.<br>Usage 2 -> sar_mtrigger_add \<id> \<entity name> \<input> : add a trigger with the specified ID that will trigger at a specific entity input.<br>|
|sar_mtrigger_delete|cmd|sar_mtrigger_delete \<id> : deletes the trigger with the given ID.<br>|
|sar_mtrigger_delete_all|cmd|sar_mtrigger_delete_all : deletes every triggers.<br>|
|sar_mtrigger_draw|0|How to draw the triggers in-game. 0: do not show. 1: show outline. 2: show full box (appears through walls).<br>|
|sar_mtrigger_export_stats|cmd|sar_mtrigger_export_stats \<filename> : Export the current stats to the specified .csv file.<br>|
|sar_mtrigger_export_triggers|cmd|sar_mtrigger_export_triggers \<filename> : Export the current triggers to the specified .csv file.<br>|
|sar_mtrigger_header|Name, Times|Header of the csv file.<br>|
|sar_mtrigger_name|FrenchSaves10ticks|Name of the current player. Re-enables all triggers when changed.<br>|
|sar_mtrigger_place|cmd|sar_mtrigger_place \<id> : place a trigger with the given ID at the position being aimed at.<br>|
|sar_mtrigger_reset|cmd|Resets the state of the output and all triggers, ready for gathering stats.<br>|
|sar_mtrigger_rotate|cmd|sar_mtrigger_rotate \<id> \<angle> : changes the rotation of a trigger to the given angle, in degrees.<br>|
|sar_mtrigger_show_chat|1|Show trigger times in chat.<br>|
|sar_nextdemo|cmd|Plays the next demo in demo queue.<br>|
|sar_on_load|cmd|sar_on_load [command] [args]... - registers a command to run on level load. Will pass further args as args to the command; do not quote the command.<br>|
|sar_pause_at|-1|Pause at the specified tick. -1 to deactivate it.<br>|
|sar_pause_for|0|Pause for this amount of ticks.<br>|
|<i title="Portal 2 Game">sar_prevent_mat_snapshot_recompute</i>|0|Shortens loading times by preventing state snapshot recomputation.<br>|
|<i title="Portal 2 Game">sar_prevent_peti_materials_loading</i>|1|Fixes memory leak by blocking PeTI from loading its materials outside of map editor.<br>|
|sar_print_stats|cmd|sar_print_stats. Prints your statistics if those are loaded.<br>|
|sar_quickhud_mode|0|Set the quickhud mode :<br>0 : Default quickhud<br>1 : Customizable quickhud<br>2 : quickhud from .png<br>|
|sar_quickhud_set_texture|cmd|sar_quickhud_set_texture \<filepath>. Enter the base name, it will search for \<filepath>1.png, \<filepath>2.png, \<filepath>3.png and \<filepath>4.png<br>ex: sar_quickhud_set_texture "E:\Steam\steamapps\common\Portal 2\portal2\krzyhau"<br>|
|sar_quickhud_size|15|Size of the custom quickhud.<br>|
|sar_quickhud_x|45|Horizontal distance of the custom quickhud.<br>|
|sar_quickhud_y|0|Vertical distance of the custom quickhud.<br>|
|sar_record_at|-1|Start recording a demo at the tick specified. Will use sar_record_at_demo_name.<br>|
|sar_record_at_demo_name|chamber|Name of the demo automatically recorded.<br>|
|sar_record_at_increment|0|Increment automatically the demo name.<br>|
|sar_rename|cmd|Changes your name. Usage: sar_rename \<name><br>|
|sar_replay_autoloop|0|Plays replay again when it ended.<br>|
|sar_replay_clone_views|cmd|Clones view to another of a replay.<br>Usage: sar_replay_clone_views \<replay_index> \<view_index><br>|
|sar_replay_export|cmd|Exports replay to a file.<br>Usage: sar_replay_export \<file><br>|
|sar_replay_export_at|cmd|Exports specific replay to a file.<br>Usage: sar_replay_export_at \<index> \<file><br>|
|sar_replay_import|cmd|Imports replay file.<br>Usage: sar_replay_import \<file><br>|
|sar_replay_import_add|cmd|Imports replay file but doesn't delete already added replays.<br>Usage: sar_replay_import_add \<file><br>|
|sar_replay_list|cmd|Lists all currently imported replays.<br>|
|sar_replay_merge_all|cmd|Merges all replays into one.<br>|
|sar_replay_merge_views|cmd|Merges one view to another of two replays.<br>Usage: sar_replay_merge_views \<replay_index1> \<replay_index2> \<view_index1> \<view_index2><br>|
|sar_replay_mode|0|Mode of replay system.<br>0 = Default,<br>1 = Automatic recording after a load,<br>2 = Automatic playback after a load.<br>|
|sar_replay_play|cmd|Plays back a replay.<br>|
|sar_replay_play_view|cmd|Plays back a specific view of a replay.<br>Usage: sar_replay_play_view \<view_index><br>|
|sar_replay_record|cmd|Starts recording a replay.<br>|
|sar_replay_record_view|cmd|Starts recording a specific view for a replay.<br>Usage: sar_replay_record_view \<view_index><br>|
|sar_replay_stop|cmd|Stops recording or playing user inputs.<br>|
|<i title="Portal 2 Game">sar_replay_viewmode</i>|0|Fallback mode of replay system.<br>0 = Default,<br>1 = Automatically records first view and plays second view after a load,<br>2 = Automatically records second view and plays first view after a load.<br>|
|sar_save_flag|#SAVE#|Echo message when using sar_bind_save.<br>Default is "#SAVE#", a SourceRuns standard.<br>Keep this empty if no echo message should be binded.<br>|
|sar_seamshot_finder|0|Enables or disables seamshot finder overlay.<br>|
|sar_seamshot_helper|0|Enables or disables seamshot helper overlay.<br>|
|sar_session|cmd|Prints the current tick of the server since it has loaded.<br>|
|sar_skiptodemo|cmd|sar_skiptodemo \<demoname>. Skip demos in demo queue to this demo.<br>|
|<i title="Portal Game">sar_speedrun_autostop</i>|0|Stops speedrun timer automatically when going into the menu.<br>|
|<i title="Portal Game">sar_speedrun_category</i>|cmd|Sets the category for a speedrun.<br>|
|sar_speedrun_category_create|cmd|sar_speedrun_category_create \<name> \<map> \<entity> \<input> \<map> \<entity> \<input> - creates a custom category with the given name and start/end rules.<br>Entity names may begin with ! to specify a classname rather than a targetname.<br>|
|sar_speedrun_category_remove|cmd|sar_speedrun_category_remove \<name> - removes a custom category.<br>|
|sar_speedrun_do_split_with_time|cmd|sar_speedrun_do_split_with_time [ticks] - perform a split whose (non-cumulative) time is precisely the number of ticks specified. Any time in this session so far is added to the next split.<br>|
|<i title="Portal Game">sar_speedrun_export</i>|cmd|Saves speedrun result to a csv file.<br>Usage: sar_speedrun_export \<file_name><br>|
|<i title="Portal Game">sar_speedrun_export_pb</i>|cmd|Saves speedrun personal best to a csv file.<br>Usage: sar_speedrun_export_pb \<file_name><br>|
|<i title="Portal Game">sar_speedrun_import</i>|cmd|Imports speedrun data file.<br>Usage: sar_speedrun_import \<file_name><br>|
|<i title="Portal Game">sar_speedrun_offset</i>|cmd|Sets offset in ticks at which the timer should start.<br>|
|<i title="Portal Game">sar_speedrun_pause</i>|cmd|Pauses speedrun timer manually.<br>|
|<i title="Portal Game">sar_speedrun_reset</i>|cmd|Resets speedrun timer.<br>|
|<i title="Portal Game">sar_speedrun_result</i>|cmd|Prints result of speedrun.<br>|
|<i title="Portal Game">sar_speedrun_resume</i>|cmd|Resumes speedrun timer manually.<br>|
|<i title="Portal Game">sar_speedrun_smartsplit</i>|1|Timer interface only splits once per level change.<br>|
|<i title="Portal Game">sar_speedrun_split</i>|cmd|Splits speedrun timer manually.<br>|
|<i title="Portal Game">sar_speedrun_standard</i>|1|Timer automatically starts, splits and stops.<br>|
|<i title="Portal Game">sar_speedrun_start</i>|cmd|Starts speedrun timer manually.<br>|
|<i title="Portal Game">sar_speedrun_start_on_load</i>|0|Starts speedrun timer automatically on first frame after a load.<br>|
|<i title="Portal Game">sar_speedrun_stop</i>|cmd|Stops speedrun timer manually.<br>|
|<i title="Portal Game">sar_speedrun_time_pauses</i>|1|Timer automatically adds non-simulated ticks when server pauses.<br>|
|<i title="Portal Game">sar_sr_hud</i>|0|Draws speedrun timer.<br>|
|<i title="Portal Game">sar_sr_hud_font_color</i>|255 255 255 255|RGBA font color of speedrun timer HUD.<br>|
|<i title="Portal Game">sar_sr_hud_font_index</i>|70|Font index of speedrun timer HUD.<br>|
|<i title="Portal Game">sar_sr_hud_x</i>|0|X offset of speedrun timer HUD.<br>|
|<i title="Portal Game">sar_sr_hud_y</i>|100|Y offset of speedrun timer HUD.<br>|
|sar_startdemos|cmd|Improved version of startdemos. 'sar_startdemos \<demoname>' Use 'stopdemo' to stop playing demos.<br>|
|sar_startdemosfolder|cmd|sar_startdemosfolder \<folder name>. Plays all the demos in the specified folder by alphabetic order.<br>|
|sar_statcounter_filePath|Stats/phunkpaiDWPS.csv|Path to the statcounter .csv file.<br>|
|sar_stats_auto_reset|0|Resets all stats automatically.<br>0 = Default,<br>1 = Restart or disconnect only,<br>2 = Any load & sar_timer_start.<br>Note: Portal counter is not part of the "stats" feature.<br>|
|sar_stats_jump|cmd|Prints jump stats.<br>|
|sar_stats_jumps_reset|cmd|Resets total jump count and jump distance peak.<br>|
|sar_stats_jumps_xy|0|Saves jump distance as 2D vector.<br>|
|sar_stats_reset|cmd|Resets all saved stats.<br>|
|sar_stats_steps|cmd|Prints total amount of steps.<br>|
|sar_stats_steps_reset|cmd|Resets total step count.<br>|
|sar_stats_velocity|cmd|Prints velocity stats.<br>|
|sar_stats_velocity_peak_xy|0|Saves velocity peak as 2D vector.<br>|
|sar_stats_velocity_reset|cmd|Resets velocity peak.<br>|
|sar_strafesync|0|Shows strafe sync stats.<br>|
|sar_strafesync_noground|0|0: Always run.<br>1: Do not run when on ground.<br>|
|sar_strafesync_pause|cmd|Pause strafe sync session.<br>|
|sar_strafesync_reset|cmd|Reset strafe sync session.<br>|
|sar_strafesync_resume|cmd|Resume strafe sync session.<br>|
|sar_strafesync_session_time|0|In seconds. How much time should pass until session is reset.<br>If 0, you'll have to reset the session manually.<br>|
|sar_strafesync_split|cmd|Makes a new split.<br>|
|sar_sum_during_session|1|Updates the summary counter automatically during a session.<br>|
|sar_sum_here|cmd|Starts counting total ticks of sessions.<br>|
|sar_sum_result|cmd|Prints result of summary.<br>|
|sar_sum_stop|cmd|Stops summary counter.<br>|
|sar_tas_addang|cmd|sar_tas_addang \<x> \<y> [z] : Adds {x, y, z} degrees to {x, y, z} view axis.<br>|
|sar_tas_aim_at_point|cmd|sar_tas_aim_at_point \<x> \<y> \<z> [speed] : Aims at point {x, y, z} specified.<br>Setting the [speed] parameter will make a time interpolation between current player angles and the targeted point.<br>|
|sar_tas_autostart|1|Starts queued commands automatically on first frame after a load.<br>|
|sar_tas_delay|cmd|Delays command queue by specified amount of frames.<br>Usage: sar_tas_delay \<frames_to_wait><br>|
|sar_tas_frame_after|cmd|Adds command frame to the queue after waiting for specified amount of frames.<br>Usage: sar_tas_frame_after \<frames_to_wait> [command_to_execute]<br>|
|sar_tas_frame_after_for|cmd|Adds two command frames to the queue after waiting for specified amount of frames, the last frame will be executed after a delay.<br>Usage: sar_tas_frame_after_for \<frames_to_wait> \<delay> \<first_command> \<last_command><br>|
|sar_tas_frame_at|cmd|Adds command frame to the queue at specified frame.<br>Usage: sar_tas_frame_at \<frame> [command_to_execute]<br>|
|sar_tas_frame_at_for|cmd|Adds two command frames to the queue at specified frame, the last frame will be executed after a delay.<br>Usage: sar_tas_frame_at_for \<frame> \<delay> \<first_command> \<last_command><br>|
|sar_tas_frame_next|cmd|Adds command frame to the queue after waiting for specified amount of frames.<br>Usage: sar_tas_frame_next \<frames_to_wait> [command_to_execute]<br>|
|sar_tas_frame_offset|cmd|sar_tas_frame_after rely on the last sar_tas_frame_offset.<br>Usage: sar_tas_frame_offset \<frame><br>|
|sar_tas_frames_after|cmd|Adds command frame multiple times to the queue after waiting for specified amount of frames.<br>Usage: sar_tas_frames_after \<frames_to_wait> \<interval> \<length> [command_to_execute]<br>|
|sar_tas_frames_at|cmd|Adds command frame multiple times to the queue at specified frame.<br>Usage: sar_tas_frames_at \<frame> \<interval> \<last_frame> [command_to_execute]<br>|
|sar_tas_reset|cmd|Stops executing commands and clears them from the queue.<br>|
|sar_tas_set_prop|cmd|sar_tas_set_prop \<prop_name> : Sets value for sar_hud_player_info.<br>|
|sar_tas_setang|cmd|sar_tas_setang \<x> \<y> [z] [speed] : Sets {x, y, z} degrees to view axis.<br>Setting the [speed] parameter will make a time interpolation between current player angles and the targeted angles.<br>|
|<i title="Portal 2 Game">sar_tas_ss</i>|cmd|Select split screen index for command buffer (0 or 1).<br>Usage: sar_tas_ss \<index><br>|
|<i title="Portal 2 Game">sar_tas_ss_forceuser</i>|0|Forces engine to calculate movement for every splitescreen client.<br>|
|sar_tas_start|cmd|Starts executing queued commands.<br>|
|<i title="Portal 2 Engine">sar_tas_strafe</i>|cmd|sar_tas_strafe \<type> \<direction> : Automatic strafing.<br>Type: 0 = off, 1 = straight, 2 = turning and keeping velocity, 3 = turning with velocity gain.<br>Direction: -1 = left, 1 = right.<br>|
|<i title="Portal 2 Engine">sar_tas_strafe_vectorial</i>|cmd|sar_tas_strafe_vectorial \<type>: Change type of vectorial strafing.<br>0 = Auto-strafer calculates perfect viewangle,<br>1 = Auto-strafer calculates perfect forward-side movement,<br>2 = Auto-strafer calculates perfect forward-side movement, while setting the viewangle toward current velocity, to make strafing visually visible.<br>|
|sar_teleport|cmd|Teleports the player to the last saved location.<br>|
|sar_teleport_setpos|cmd|Saves current location for teleportation.<br>|
|sar_time_demo|cmd|Parses a demo and prints some information about it.<br>Usage: sar_time_demo \<demo_name><br>|
|sar_time_demo_dev|0|Printing mode when using sar_time_demo.<br>0 = Default,<br>1 = Console commands,<br>2 = Console commands & packets.<br>|
|sar_time_demos|cmd|Parses multiple demos and prints the total sum of them.<br>Usage: sar_time_demos \<demo_name> \<demo_name2> \<etc.><br>|
|sar_timer_always_running|1|Timer will save current value when disconnecting.<br>|
|sar_timer_result|cmd|Prints result of timer.<br>|
|sar_timer_start|cmd|Starts timer.<br>|
|sar_timer_stop|cmd|Stops timer.<br>|
|sar_timer_time_pauses|1|Timer adds non-simulated ticks when server pauses.<br>|
|<i title="Portal 2&#10;Aperture Tag&#10;Portal Stories: Mel&#10;INFRA&#10;Thinking with Time Machine">sar_togglewait</i>|cmd|Enables or disables "wait" for the command buffer.<br>|
|sar_trace_a|cmd|Saves location A for tracing.<br>|
|sar_trace_b|cmd|Saves location B for tracing.<br>|
|sar_trace_reset|cmd|Resets tracer.<br>|
|sar_trace_result|cmd|Prints tracing result.<br>|
|sar_unbind_reload|cmd|Unbinds current save-reload rebinder.<br>|
|sar_unbind_save|cmd|Unbinds current save rebinder.<br>|
|sar_vphys_hud|0|Enables or disables the vphys HUD.<br>|
|sar_vphys_hud_x|0|The x position of the vphys HUD.<br>|
|sar_vphys_hud_y|0|The y position of the vphys HUD.<br>|
|sar_vphys_setangle|cmd|sar_vphys_setangle \<hitbox> \<angle>: Sets rotation angle to either standing (0) or crouching (1) havok collision shadow.<br>|
|sar_vphys_setgravity|cmd|sar_vphys_setgravity \<hitbox> \<enabled>: Sets gravity flag state to either standing (0) or crouching (1) havok collision shadow.<br>|
|sar_vphys_setspin|cmd|sar_vphys_setspin \<hitbox> \<angvel>: Sets rotation speed to either standing (0) or crouching (1) havok collision shadow.<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop</i>|cmd|Same as "map" command but lists workshop maps.<br>Usage: sar_workshop \<file> [ss/changelevel]<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop_list</i>|cmd|Prints all workshop maps.<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop_update</i>|cmd|Updates the workshop map list.<br>|
|seq|cmd|seq [command]... - runs a sequence of commands one tick after one another.<br>|
|wait|cmd|wait \<tick> \<commands>. Wait for the amount of tick specified.<br>|
|wait_mode|0|When the pending commands should be executed. 0 is absolute, 1 is relative to when you entered the wait command.<br>|
|wait_persist_across_loads|0|Whether pending commands should be carried across loads (1) or just be dropped (0).<br>|