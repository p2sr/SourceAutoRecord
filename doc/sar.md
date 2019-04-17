# SAR: Cvars

|Name|Default|Flags|Description|
|---|---|---|---|
|sar_cps_result|cmd|0|Prints result of timer checkpoints.
|sar_cps_clear|cmd|0|Resets saved times of timer.
|sar_cps_add|cmd|0|Saves current time of timer.
|sar_avg_result|cmd|0|Prints result of average.
|sar_avg_stop|cmd|0|Stops average calculation.
|sar_avg_start|cmd|0|Starts calculating the average when using timer.
|sar_timer_result|cmd|0|Prints result of timer.
|sar_timer_stop|cmd|0|Stops timer.
|sar_timer_start|cmd|0|Starts timer.
|sar_tas_frame_after_for|cmd|0|Adds two command frames to the queue after waiting for specified amount of frames, the last frame will be executed after a delay.<br>Usage: sar_tas_frame_after_for <frames_to_wait> <delay> <first_command> <last_command>
|sar_tas_frame_at_for|cmd|0|Adds two command frames to the queue at specified frame, the last frame will be executed after a delay.<br>Usage: sar_tas_frame_at_for <frame> <delay> <first_command> <last_command>
|sar_tas_delay|cmd|0|Delays command queue by specified amount of frames.
|sar_tas_ss|cmd|0|Select split screen index for command buffer (0 or 1).
|sar_tas_reset|cmd|0|Stops executing commands and clears them from the queue.
|sar_tas_start|cmd|0|Starts executing queued commands.
|sar_tas_frame_offset|cmd|0|sar_tas_frame_after rely on the last sar_tas_frame_offset.<br>Usage: sar_tas_frame_offset <frame>
|sar_tas_frames_after|cmd|0|Adds command frame multiple times to the queue after waiting for specified amount of frames.<br>Usage: sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute]
|sar_tas_frame_after|cmd|0|Adds command frame to the queue after waiting for specified amount of frames.<br>Usage: sar_tas_frame_after <frames_to_wait> [command_to_execute]
|sar_tas_frames_at|cmd|0|Adds command frame multiple times to the queue at specified frame.<br>Usage: sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute]
|sar_tas_frame_at|cmd|0|Adds command frame to the queue at specified frame.<br>Usage: sar_tas_frame_at <frame> [command_to_execute]
|sar_tas_setang|cmd|0|sar_tas_setang <x> <y> [z] : Sets {x, y, z} degres to view axis.
|sar_tas_addang|cmd|0|sar_tas_addang <x> <y> [z] : Adds {x, y, z} degrees to {x, y, z} view axis.
|sar_tas_set_prop|cmd|0|sar_tas_set_prop <prop_name> : Sets value for sar_hud_player_info.
|sar_tas_aim_at_point|cmd|0|sar_tas_aim_at_point <x> <y> <z> : Aims at point {x, y, z} specified.
|sar_replay_list|cmd|0|Lists all currently imported replays.
|sar_replay_import_add|cmd|0|Imports replay file but doesn't delete already added replays.
|sar_replay_import|cmd|0|Imports replay file.
|sar_replay_export_at|cmd|0|Exports specific replay to a file.
|sar_replay_export|cmd|0|Exports replay to a file.
|sar_replay_clone_views|cmd|0|Clones view to another of a replay.
|sar_replay_merge_views|cmd|0|Merges one view to another of two replays.
|sar_replay_merge_all|cmd|0|Merges all replays into one.
|sar_replay_stop|cmd|0|Stops recording or playing user inputs.
|sar_replay_play_view|cmd|0|Plays back a specific view of a replay.
|sar_replay_play|cmd|0|Plays back a replay.
|sar_replay_record_view|cmd|0|Starts recording a specific view for a replay.
|sar_replay_record|cmd|0|Starts recording a replay.
|sar_stats_reset|cmd|0|Resets all saved stats.
|sar_stats_velocity_reset|cmd|0|Resets velocity peak.
|sar_stats_steps_reset|cmd|0|Resets total step count.
|sar_stats_jumps_reset|cmd|0|Resets total jump count and jump distance peak.
|sar_stats_velocity|cmd|0|Prints velocity stats.
|sar_stats_steps|cmd|0|Prints total amount of steps.
|sar_stats_jump|cmd|0|Prints jump stats.
|sar_speedrun_offset|cmd|0|Sets offset in ticks at which the timer should start.
|sar_speedrun_categories|cmd|0|Lists all categories.
|sar_speedrun_category|cmd|0|Sets the category for a speedrun.
|sar_speedrun_import|cmd|0|Imports speedrun data file.
|sar_speedrun_export_pb|cmd|0|Saves speedrun personal best to a csv file.
|sar_speedrun_export|cmd|0|Saves speedrun result to a csv file.
|sar_speedrun_result|cmd|0|Prints result of speedrun.
|sar_speedrun_reset|cmd|0|Resets speedrun timer.
|sar_speedrun_resume|cmd|0|Resumes speedrun timer.
|sar_speedrun_pause|cmd|0|Pauses speedrun timer.
|sar_speedrun_split|cmd|0|Splits speedrun timer.
|sar_speedrun_stop|cmd|0|Stops speedrun timer.
|sar_speedrun_start|cmd|0|Starts speedrun.
|sar_inspection_index|cmd|0|Sets entity index for inspection.
|sar_inspection_export|cmd|0|Stops recording entity data.
|sar_inspection_print|cmd|0|Prints recorded entity data.
|sar_inspection_stop|cmd|0|Stops recording entity data.
|sar_inspection_start|cmd|0|Starts recording entity data.
|sar_trace_reset|cmd|0|Resets tracer.
|sar_trace_result|cmd|0|Prints tracing result.
|sar_trace_b|cmd|0|Saves location B for tracing.
|sar_trace_a|cmd|0|Saves location A for tracing.
|sar_ihud_setpos|cmd|0|Sets automatically the position of input HUD.<br>Usage: sar_ihud_setpos <top, center or bottom> <left, center or right>
|sar_time_demos|cmd|0|Parses multiple demos and prints the total sum of them.
|sar_time_demo|cmd|0|Parses a demo and prints some information about it.
|sar_teleport_setpos|cmd|0|Saves current location for teleportation.
|sar_teleport|cmd|0|Teleports the player to the last saved location.
|sar_find_ents|cmd|0|Finds entities in the entity list by class name.
|sar_find_ent|cmd|0|Finds entity in the entity list by name.
|sar_list_ents|cmd|0|Lists entities.
|sar_workshop_list|cmd|0|Prints all workshop maps.
|sar_workshop_update|cmd|0|Updates the workshop map list.
|sar_workshop|cmd|0|Same as "map" command but lists workshop maps.
|sar_find_client_offset|cmd|0|Finds prop offset in specified client class.
|sar_find_server_offset|cmd|0|Finds prop offset in specified server class.
|sar_find_client_class|cmd|0|Finds specific client class tables and props with their offset.
|sar_find_server_class|cmd|0|Finds specific server class tables and props with their offset.
|sar_list_client_classes|cmd|0|Lists all client classes.
|sar_list_server_classes|cmd|0|Lists all server classes.
|sar_dump_client_classes|cmd|0|Dumps all client classes to a file.
|sar_dump_server_classes|cmd|0|Dumps all server classes to a file.
|sar_sum_result|cmd|0|Prints result of summary.
|sar_sum_stop|cmd|0|Stops summary counter.
|sar_sum_here|cmd|0|Starts counting total ticks of sessions.
|sar_unbind_reload|cmd|0|Unbinds current save-reload rebinder.
|sar_unbind_save|cmd|0|Unbinds current save rebinder.
|sar_bind_reload|cmd|0|Automatic save-reload rebinding when server has loaded.<br>File indexing will be synced when recording demos.<br>Usage: sar_bind_reload <key> [save_name]
|sar_bind_save|cmd|0|Automatic save rebinding when server has loaded.<br>File indexing will be synced when recording demos.<br>Usage: sar_bind_save <key> [save_name]
|sar_delete_alias_cmds|cmd|0|Deletes all alias commands.
|sar_togglewait|cmd|0|Enables or disables "wait" for the command buffer.
|sar_exit|cmd|0|Removes all function hooks, registered commands and unloads the module.
|sar_rename|cmd|0|Changes your name.
|sar_cvarlist|cmd|0|Lists all SAR cvars and unlocked engine cvars.
|sar_cvars_unlock|cmd|0|Unlocks all special cvars.
|sar_cvars_lock|cmd|0|Restores default flags of unlocked cvars.
|sar_cvars_dump|cmd|0|Dumps all cvars to a file.
|sar_cvars_load|cmd|0|Loads important SAR cvars.
|sar_cvars_save|cmd|0|Saves important SAR cvars.
|sar_about|cmd|0|Prints info about SAR plugin.
|sar_session|cmd|0|Prints the current tick of the server since it has loaded.
|sar_timer_always_running|1|4096|Timer will save current value when disconnecting.
|sar_tas_autostart|1|4096|Starts queued commands automatically on first frame after a load.
|sar_replay_autoloop|0|4096|Plays replay again when it ended.
|sar_replay_viewmode|0|4096|Fallback mode of replay system.<br>0 = Default,<br>1 = Automatically records first view and plays second view after a load,<br>2 = Automatically records second view and plays first view after a load.
|sar_replay_mode|0|4096|Mode of replay system.<br>0 = Default,<br>1 = Automatic recording after a load,<br>2 = Automatic playback after a load.
|sar_stats_auto_reset|0|4096|Resets all stats automatically.<br>0 = Default,<br>1 = Restart or disconnect only,<br>2 = Any load & sar_timer_start.<br>Note: Portal counter is not part of the "stats" feature.
|sar_stats_velocity_peak_xy|0|4096|Saves velocity peak as 2D vector.
|sar_stats_jumps_xy|0|4096|Saves jump distance as 2D vector.
|sar_speedrun_standard|1|4096|Timer automatically starts, splits and stops.
|sar_speedrun_autostop|0|4096|Stops speedrun timer automatically when going into the menu.
|sar_speedrun_autostart|0|4096|Starts speedrun timer automatically on first frame after a load.
|sar_inspection_save_every_tick|0|4096|Saves inspection data even when session tick does not increment.
|sar_hud_default_font_color|255 255 255 255|0|RGBA font color of HUD.
|sar_hud_default_font_index|0|4096|Font index of HUD.
|sar_hud_default_padding_y|2|4096|Y padding of HUD.
|sar_hud_default_padding_x|2|4096|X padding of HUD.
|sar_hud_default_spacing|4|4096|Spacing between elements of HUD.
|sar_hud_player_info|0|4096|Draws player state defined with sar_tas_set_prop.
|sar_hud_acceleration|0|4096|Draws instant acceleration.
|sar_hud_velocity_angle|0|4096|Draws velocity angles.
|sar_hud_velocity_peak|0|4096|Draws last saved velocity peak.
|sar_hud_inspection|0|4096|Draws entity inspection data.
|sar_hud_last_frame|0|4096|Draws last saved frame value.
|sar_hud_frame|0|4096|Draws current frame count.
|sar_hud_trace|0|4096|Draws distance values of tracer. 0 = Default,<br>1 = Vec3,<br>2 = Vec2.
|sar_hud_jump_peak|0|4096|Draws longest jump distance.
|sar_hud_jump|0|4096|Draws current jump distance.
|sar_hud_steps|0|4096|Draws total step count.
|sar_hud_portals|0|4096|Draws total portal count.
|sar_hud_jumps|0|4096|Draws total jump count.
|sar_hud_demo|0|4096|Draws name, tick and time of current demo.
|sar_hud_cps|0|4096|Draws latest checkpoint of timer.
|sar_hud_avg|0|4096|Draws calculated average of timer.
|sar_hud_timer|0|4096|Draws current value of timer.
|sar_hud_sum|0|4096|Draws summary value of sessions.
|sar_hud_last_session|0|4096|Draws value of latest completed session.
|sar_hud_session|0|4096|Draws current session tick.
|sar_hud_velocity|0|4096|Draws velocity of the client.<br>0 = Default,<br>1 = X/Y/Z,<br>2 = X/Y.
|sar_hud_angles|0|4096|Draws absolute view angles of the client.<br>0 = Default,<br>1 = XY,<br>2 = XYZ.
|sar_hud_position|0|4096|Draws absolute position of the client.
|sar_hud_text||0|Draws specified text when not empty.
|sar_sr_hud_font_index|70|4096|Font index of speedrun timer HUD.
|sar_sr_hud_font_color|255 255 255 255|0|RGBA font color of speedrun timer HUD.
|sar_sr_hud_y|100|4096|Y offset of speedrun timer HUD.
|sar_sr_hud_x|0|4096|X offset of speedrun timer HUD.
|sar_sr_hud|0|4096|Draws speedrun timer.
|sar_ei_hud_font_index|1|4096|Font index of entity inspection HUD.
|sar_ei_hud_font_color2|153 23 9 255|0|RGBA font color of entity inspection HUD when recording.
|sar_ei_hud_font_color|255 255 255 255|0|RGBA font color of entity inspection HUD when not recording.
|sar_ei_hud_z|0|4096|Z offset of entity inspection HUD.
|sar_ei_hud_y|0|4096|Y offset of entity inspection HUD.
|sar_ei_hud_x|0|4096|X offset of entity inspection HUD.
|sar_ei_hud|0|4096|Draws entity inspection data.
|sar_ihud_shadow_font_color|255 255 255 32|0|RGBA button shadow font color of input HUD.
|sar_ihud_shadow_color|0 0 0 32|0|RGBA button shadow color of input HUD.
|sar_ihud_shadow|1|4096|Draws button shadows of input HUD.
|sar_ihud_layout|WASDCSELRSR|0|Layout of input HUD.<br>Characters are in this order:<br>forward,<br>moveleft,<br>back,<br>moveright,<br>duck,<br>jump,<br>use,<br>attack,<br>attack2,<br>speed,<br>reload.<br>Keep it empty to disable drawing characters.
|sar_ihud_font_index|1|4096|Font index of input HUD.
|sar_ihud_font_color|255 255 255 255|0|RGBA font color of input HUD.
|sar_ihud_button_color|0 0 0 233|0|RGBA button color of input HUD.
|sar_ihud_button_size|60|4096|Button size of input HUD.
|sar_ihud_button_padding|2|4096|Padding between buttons of input HUD.
|sar_ihud_y|0|4096|Y offset of input HUD.
|sar_ihud_x|0|4096|X offset of input HUD.
|sar_ihud|0|4096|Draws keyboard events of client.<br>0 = Default,<br>1 = forward;back;moveleft;moveright,<br>2 = 1 + duck;jump;use,<br>3 = 2 + attack;attack2,<br>4 = 3 + speed;reload.
|sar_time_demo_dev|0|4096|Printing mode when using sar_time_demo.<br>0 = Default,<br>1 = Console commands,<br>2 = Console commands & packets.
|sar_debug_game_events|0|4096|Prints game event data, similar to net_showevents.
|sar_sum_during_session|1|4096|Updates the summary counter automatically during a session.
|sar_mimic|0|4096|Copies inputs to secondary split screen. Similar to ss_mimic.
|sar_save_flag|#SAVE#|0|Echo message when using sar_bind_save.<br>Default is "#SAVE#", a SourceRuns standard.<br>Keep this empty if no echo message should be binded.
|sar_disable_challenge_stats_hud|0|4096|Disables opening the challenge mode stats HUD.
|sar_duckjump|0|4096|Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.
|sar_aircontrol|0|4096|Enables more air-control on the server.
|sar_jumpboost|0|4096|Enables special game movement on the server.<br>0 = Default,<br>1 = Orange Box Engine,<br>2 = Pre-OBE.
|sar_autojump|0|4096|Enables automatic jumping on the server.
|sar_autorecord|0|4096|Enables automatic demo recording for loading a save.
|