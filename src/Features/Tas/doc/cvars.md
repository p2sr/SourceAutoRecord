# SAR: Cvars

|Name|Default|Description|
|---|---|---|
|<i title="Portal 2 Engine">+autostrafe</i>|cmd|Auto-strafe button.<br>|
|<i title="Portal 2 Engine">-autostrafe</i>|cmd|Auto-strafe button.<br>|
|<i title="The Stanley Parable">+bhop</i>|cmd|Client sends a key-down event for the in_jump state.<br>|
|<i title="The Stanley Parable">-bhop</i>|cmd|Client sends a key-up event for the in_jump state.<br>|
|sar_about|cmd|Prints info about SAR plugin.<br>|
|<i title="Portal 2 Engine">sar_aircontrol</i>|0|Enables more air-control on the server.<br>|
|<i title="The Stanley Parable">sar_anti_anti_cheat</i>|cmd|Sets sv_cheats to 1.<br>|
|sar_autojump|0|Enables automatic jumping on the server.<br>|
|sar_autorecord|0|Enables automatic demo recording.<br>|
|sar_avg_result|cmd|Prints result of average.<br>|
|sar_avg_start|cmd|Starts calculating the average when using timer.<br>|
|sar_avg_stop|cmd|Stops average calculation.<br>|
|sar_bind_reload|cmd|Automatic save-reload rebinding when server has loaded.<br>File indexing will be synced when recording demos.<br>Usage: sar_bind_reload \<key> [save_name]<br>|
|sar_bind_save|cmd|Automatic save rebinding when server has loaded.<br>File indexing will be synced when recording demos.<br>Usage: sar_bind_save \<key> [save_name]<br>|
|sar_cps_add|cmd|Saves current time of timer.<br>|
|sar_cps_clear|cmd|Resets saved times of timer.<br>|
|sar_cps_result|cmd|Prints result of timer checkpoints.<br>|
|sar_cvarlist|cmd|Lists all SAR cvars and unlocked engine cvars.<br>|
|sar_cvars_dump|cmd|Dumps all cvars to a file.<br>|
|sar_cvars_dump_doc|cmd|Dumps all SAR cvars to a file.<br>|
|sar_cvars_load|cmd|Loads important SAR cvars.<br>|
|sar_cvars_lock|cmd|Restores default flags of unlocked cvars.<br>|
|sar_cvars_save|cmd|Saves important SAR cvars.<br>|
|sar_cvars_unlock|cmd|Unlocks all special cvars.<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_debug_listener</i>|0|Prints event data of registered listener.<br>|
|<i title="Portal 2&#10;Portal&#10;Half-Life 2&#10;Aperture Tag&#10;Portal Stories: Mel">sar_delete_alias_cmds</i>|cmd|Deletes all alias commands.<br>|
|<i title="Portal 2">sar_disable_challenge_stats_hud</i>|0|Disables opening the challenge mode stats HUD.<br>|
|<i title="Portal 2 Engine">sar_disable_no_focus_sleep</i>|0|Does not yield the CPU when game is not focused.<br>|
|<i title="Portal 2 Game">sar_disable_steam_pause</i>|0|Prevents pauses from steam overlay.<br>|
|<i title="Portal 2 Engine">sar_duckjump</i>|0|Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.<br>|
|sar_dump_client_classes|cmd|Dumps all client classes to a file.<br>|
|sar_dump_client_datamap|cmd|Dumps client datmap to a file.<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_dump_events</i>|cmd|Dumps all registered game events of the game event manager.<br>|
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
|sar_find_client_class|cmd|Finds specific client class tables and props with their offset.<br>Usage: sar_find_clientclass \<class_name><br>|
|sar_find_client_offset|cmd|Finds prop offset in specified client class.<br>Usage: sar_find_client_offset \<class_name> \<prop_name><br>|
|sar_find_ent|cmd|Finds entity in the entity list by name.<br>Usage: sar_find_ent \<m_iName><br>|
|sar_find_ents|cmd|Finds entities in the entity list by class name.<br>Usage: sar_find_ents \<m_iClassName><br>|
|sar_find_server_class|cmd|Finds specific server class tables and props with their offset.<br>Usage: sar_find_serverclass \<class_name><br>|
|sar_find_server_offset|cmd|Finds prop offset in specified server class.<br>Usage: sar_find_server_offset \<class_name> \<prop_name><br>|
|sar_hud_acceleration|0|Draws instant acceleration.<br>|
|sar_hud_angles|0|Draws absolute view angles of the client.<br>0 = Default,<br>1 = XY,<br>2 = XYZ.<br>|
|sar_hud_avg|0|Draws calculated average of timer.<br>|
|sar_hud_cps|0|Draws latest checkpoint of timer.<br>|
|sar_hud_default_font_color|255 255 255 255|RGBA font color of HUD.<br>|
|sar_hud_default_font_index|0|Font index of HUD.<br>|
|sar_hud_default_padding_x|2|X padding of HUD.<br>|
|sar_hud_default_padding_y|2|Y padding of HUD.<br>|
|sar_hud_default_spacing|1|Spacing between elements of HUD.<br>|
|sar_hud_demo|0|Draws name, tick and time of current demo.<br>|
|sar_hud_frame|0|Draws current frame count.<br>|
|sar_hud_inspection|0|Draws entity inspection data.<br>|
|sar_hud_jump|0|Draws current jump distance.<br>|
|sar_hud_jump_peak|0|Draws longest jump distance.<br>|
|sar_hud_jumps|0|Draws total jump count.<br>|
|sar_hud_last_frame|0|Draws last saved frame value.<br>|
|sar_hud_last_session|0|Draws value of latest completed session.<br>|
|<i title="Portal Game">sar_hud_pause_timer</i>|0|Draws current value of pause timer.<br>|
|sar_hud_player_info|0|Draws player state defined with sar_tas_set_prop.<br>|
|<i title="Portal Game">sar_hud_portals</i>|0|Draws total portal count.<br>|
|sar_hud_position|0|Draws absolute position of the client.<br>0 = Default,<br>1 = Player position,<br>2 = Camera position.<br>|
|sar_hud_session|0|Draws current session tick.<br>|
|sar_hud_steps|0|Draws total step count.<br>|
|sar_hud_sum|0|Draws summary value of sessions.<br>|
|sar_hud_text||Draws specified text when not empty.<br>|
|sar_hud_timer|0|Draws current value of timer.<br>|
|sar_hud_trace|0|Draws distance values of tracer. 0 = Default,<br>1 = Vec3,<br>2 = Vec2.<br>|
|sar_hud_velocity|0|Draws velocity of the client.<br>0 = Default,<br>1 = X/Y/Z,<br>2 = X/Y,<br>3 = X : Y : Z.<br>|
|sar_hud_velocity_angle|0|Draws velocity angles.<br>|
|sar_hud_velocity_peak|0|Draws last saved velocity peak.<br>|
|sar_ihud|0|Draws movement inputs of client.<br>0 = Default,<br>1 = forward;back;moveleft;moveright,<br>2 = 1 + duck;jump;use,<br>3 = 2 + attack;attack2,<br>4 = 3 + speed;reload.<br>|
|sar_ihud_button_color|0 0 0 233|RGBA button color of input HUD.<br>|
|sar_ihud_button_padding|2|Padding between buttons of input HUD.<br>|
|sar_ihud_button_size|60|Button size of input HUD.<br>|
|sar_ihud_font_color|255 255 255 255|RGBA font color of input HUD.<br>|
|sar_ihud_font_index|1|Font index of input HUD.<br>|
|sar_ihud_layout|WASDCSELRSR|Layout of input HUD.<br>Labels are in this order:<br>forward,<br>moveleft,<br>back,<br>moveright,<br>duck,<br>jump,<br>use,<br>attack,<br>attack2,<br>speed,<br>reload.<br>Pass an empty string to disable drawing labels completely.<br>|
|sar_ihud_setlayout|cmd|Suggests keyboard layouts for sar_ihud_layout.<br>|
|sar_ihud_setpos|cmd|Sets automatically the position of input HUD.<br>Usage: sar_ihud_setpos \<top, center or bottom> \<left, center or right><br>|
|sar_ihud_shadow|1|Draws button shadows of input HUD.<br>|
|sar_ihud_shadow_color|0 0 0 32|RGBA button shadow color of input HUD.<br>|
|sar_ihud_shadow_font_color|255 255 255 32|RGBA button shadow font color of input HUD.<br>|
|sar_ihud_x|0|X offset of input HUD.<br>|
|sar_ihud_y|0|Y offset of input HUD.<br>|
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
|<i title="Portal 2&#10;Aperture Tag">sar_mimic</i>|0|Copies inputs to secondary split screen. Similar to ss_mimic.<br>|
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
|<i title="Portal 2&#10;Aperture Tag">sar_replay_viewmode</i>|0|Fallback mode of replay system.<br>0 = Default,<br>1 = Automatically records first view and plays second view after a load,<br>2 = Automatically records second view and plays first view after a load.<br>|
|sar_save_flag|#SAVE#|Echo message when using sar_bind_save.<br>Default is "#SAVE#", a SourceRuns standard.<br>Keep this empty if no echo message should be binded.<br>|
|sar_session|cmd|Prints the current tick of the server since it has loaded.<br>|
|<i title="Portal Game">sar_speedrun_autostart</i>|0|Starts speedrun timer automatically on first frame after a load.<br>|
|<i title="Portal Game">sar_speedrun_autostop</i>|0|Stops speedrun timer automatically when going into the menu.<br>|
|<i title="Portal Game">sar_speedrun_category</i>|cmd|Sets the category for a speedrun.<br>|
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
|<i title="Portal Game">sar_speedrun_stop</i>|cmd|Stops speedrun timer manually.<br>|
|<i title="Portal Game">sar_speedrun_time_pauses</i>|1|Timer automatically adds non-simulated ticks when server pauses.<br>|
|<i title="Portal Game">sar_sr_hud</i>|0|Draws speedrun timer.<br>|
|<i title="Portal Game">sar_sr_hud_font_color</i>|255 255 255 255|RGBA font color of speedrun timer HUD.<br>|
|<i title="Portal Game">sar_sr_hud_font_index</i>|70|Font index of speedrun timer HUD.<br>|
|<i title="Portal Game">sar_sr_hud_x</i>|0|X offset of speedrun timer HUD.<br>|
|<i title="Portal Game">sar_sr_hud_y</i>|100|Y offset of speedrun timer HUD.<br>|
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
|<i title="Portal 2&#10;Aperture Tag">sar_tas_ss</i>|cmd|Select split screen index for command buffer (0 or 1).<br>Usage: sar_tas_ss \<index><br>|
|<i title="Portal 2&#10;Aperture Tag">sar_tas_ss_forceuser</i>|0|Forces engine to calculate movement for every splitescreen client.<br>|
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
|<i title="Portal 2&#10;Aperture Tag&#10;Portal Stories: Mel&#10;INFRA">sar_togglewait</i>|cmd|Enables or disables "wait" for the command buffer.<br>|
|sar_trace_a|cmd|Saves location A for tracing.<br>|
|sar_trace_b|cmd|Saves location B for tracing.<br>|
|sar_trace_reset|cmd|Resets tracer.<br>|
|sar_trace_result|cmd|Prints tracing result.<br>|
|sar_unbind_reload|cmd|Unbinds current save-reload rebinder.<br>|
|sar_unbind_save|cmd|Unbinds current save rebinder.<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop</i>|cmd|Same as "map" command but lists workshop maps.<br>Usage: sar_workshop \<file><br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop_list</i>|cmd|Prints all workshop maps.<br>|
|<i title="Portal 2&#10;Aperture Tag">sar_workshop_update</i>|cmd|Updates the workshop map list.<br>|