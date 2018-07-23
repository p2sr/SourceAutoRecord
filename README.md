[![Build Version](https://img.shields.io/badge/version-v1.7-brightgreen.svg)](https://github.com/NeKzor/SourceAutoRecord/projects/3)
[![Release Status](https://img.shields.io/github/release/NeKzor/SourceAutoRecord/all.svg)](https://github.com/NeKzor/SourceAutoRecord/releases)

**SourceAutoRecord** allows automatic demo recording, automatic binding, demo parsing, session timing and [much more](#features).

## Overview
- [Supported Games](#supported-games)
- [Features](#features)
  - [Automatic Demo Recorder](#automatic-demo-recorder)
  - [Automatic Binding](#automatic-binding)
  - [Demo Parser](#demo-parser)
  - [Session Timing](#session-timing)
    - [Summary](#summary)
  - [Timer](#timer)
    - [Average](#average)
    - [Checkpoints](#checkpoints)
  - [Speedrun](#speedrun)
  - [HUD](#hud)
    - [Optional](#optional)
  - [Input HUD](#input-hud)
  - [Speedrun HUD](#speedrun-hud)
  - [Stats](#stats)
  - [Cheats](#cheats)
    - [Movement](#movement)
    - [Routing](#routing)
    - [Unlocked](#unlocked)
  - [TAS](#tas)
    - [Command Queuer](#command-queuer)
    - [Replays](#replays)
  - [Config](#config)
- [Mapping](#mapping)
  - [Start & Stop Triggers](#start--stop-triggers)
  - [With Checkpoints](#with-checkpoints)
- [Credits](#inspired-by)

## Supported Games

- Portal 2
- Aperture Tag
- Portal Stories: Mel
- The Stanley Parable
- The Beginners Guide
- Half-Life 2
- Portal

## Features

### Automatic Demo Recorder
- `sar_autorecord <0-1>` keeps recording a demo when loading from a save

### Automatic Binding
- `sar_bind_save <key> [save_name]` binds automatically `save save_name` to the given key when loading
- `sar_save_flag [echo_message]` appends `;echo message` to the save bind
- `sar_bind_reload <key> [save_name]` binds automatically `save save_name;reload` to the given key when loading
- `sar_unbind_save` unbinds the key and stops automatic binding for `sar_bind_save`
- `sar_unbind_reload` unbinds the key and stops automatic binding for `sar_bind_reload`

Save files will be named _2, _3, etc.
File indexing will be synced automatically with the demo recorder when recording with demos.

### Demo Parser
- `sar_time_demo [demo_name]` parses a demo and prints some useful information about it
  - Passing an empty string will take the last played demo from the demo player
- `sar_time_demo_dev` prints demo's console commands and packets when using `sar_time_demo`
  - Use `con_log [file_name]` to export the extra data into a file
- `sar_time_demos [demo_name] [demo_name2] [etc.]` parses multiple demos

### Session Timing
- `sar_session` prints current tick count since the server has loaded

##### Summary
- `sar_sum_here` starts saving the total tick count of each session
- `sar_sum_stop` stops counting
- `sar_sum_reset` resets the counter
- `sar_sum_result` prints the result of all saved sessions
- `sar_sum_during_session <0-1>` counts current session too

### Timer
- `sar_timer_start` starts or restarts the timer, counting from invocation
- `sar_timer_stop` stops timer
- `sar_timer_result` prints result
- `sar_timer_always_running <0-1>` doesn't stop the timer when disconnecting from server

Mappers can use this for accurate timing, [see below](#mapping).

##### Average
- `sar_avg_start` starts calculating the average of the timer
- `sar_avg_stop` stops calculation
- `sar_avg_result` prints result

##### Checkpoints
- `sar_cps_add` saves current timer value
- `sar_cps_clear` resets all saved values
- `sar_cps_result` prints result of all checkpoints

### Speedrun
- `sar_speedrun_result` prints result of timer
- `sar_speedrun_export <file>` exports timer result to a .csv file
- `sar_speedrun_import <file>` imports timer result as personal best for comparison
- `sar_speedrun_autoexport <0-2>` exports result automatically on speedrun completion
- `sar_speedrun_autosplit <0-1>` splits automatically on map change
- `sar_speedrun_autoreset <0-1>` resets timer automatically
- `sar_speedrun_rules` prints loaded rules which the timer will follow

The timer has its own HUD, [see below](#speedrun-hud).
Here is a basic [ASL script](https://gist.github.com/NeKzor/6db7ca6a28ed55fbcce7d8af7edf0f18) for [LiveSplit](https://livesplit.github.com) which connects to this speedrun timer.

### HUD
- `sar_hud_text <0-1>` draws given string
- `sar_hud_position <0-1>` draws player's position
- `sar_hud_angles <0-2>` draws player's view angles
- `sar_hud_velocity <0-2>` draws player's velocity
- `sar_hud_session <0-1>` draws current session value
- `sar_hud_last_session <0-1>` draws value of latest completed session
- `sar_hud_sum <0-1>` draws summary value of sessions
- `sar_hud_timer <0-1>` draws timer value
- `sar_hud_avg <0-1>` draws current average of timer
- `sar_hud_cps <0-1>` draws last checkpoint value of timer
- `sar_hud_demo <0-1>` draws current name, tick and time of demo recorder or demo player
- `sar_hud_jumps <0-1>` draws total jump count
- `sar_hud_portals <0-1>` draws total portal count (Portal games only)
- `sar_hud_steps <0-1>` draws total step count
- `sar_hud_distance <0-1>` draws calculated jump distance
- `sar_hud_trace <0-2>` draws tracer result
- `sar_hud_velocity_peak <0-1>` draws velocity peak
- `sar_hud_jump <0-1>` draws last jump distance
- `sar_hud_jump_peak <0-1>` draws jump distance peak

##### Optional
- `sar_hud_default_spacing` space between HUD elements
- `sar_hud_default_padding_x` additional padding on x-axis
- `sar_hud_default_padding_y` additional padding on y-axis
- `sar_hud_default_font_index` font index
- `sar_hud_default_font_color` font color

### Input HUD
- `sar_ihud <0-4>` draws keyboard inputs of client
- `sar_ihud_x` x offset in pixels
- `sar_ihud_y` y offset in pixels
- `sar_ihud_button_padding` space between buttons
- `sar_ihud_button_size` size of button
- `sar_ihud_button_color` color of the button
- `sar_ihud_font_color` font color
- `sar_ihud_font_index` font index
- `sar_ihud_layout` characters for the buttons
- `sar_ihud_shadow <0-1>` shadow of the buttons
- `sar_ihud_shadow_color` shadow color
- `sar_ihud_shadow_font_color` shadow font color
- `sar_ihud_setpos <top, center, bottom> <left, center, right>` adjusts x and y offset automatically for the screen

### Speedrun HUD
- `sar_sr_hud <0-1>` draws speedrun timer
- `sar_sr_hud_x` x offset in pixels
- `sar_sr_hud_y` y offset in pixels
- `sar_sr_hud_size` size of timer
- `sar_sr_hud_bg_color` background color of timer
- `sar_sr_hud_font_color` font color of timer
- `sar_sr_hud_font_index` font index of timer
- `sar_sr_hud_splits <0-1>` draws timer splits
- `sar_sr_hud_splits_direction <top, left, right, bottom` draws splits on one side of the timer
- `sar_sr_hud_splits_delta` draws split delta compared to personal best
- `sar_sr_hud_pace` draws current pace compared to personal best
- `sar_sr_hud_setpos <top, center, bottom> <left, center, right>` adjusts x and y offset automatically for the screen

Variable sar_sr_hud("sar_sr_hud", "0", 0, "Draws speedrun timer.\n");
Variable sar_sr_hud_x("sar_sr_hud_x", "0", 0, "X offset of speedrun timer HUD.\n");
Variable sar_sr_hud_y("sar_sr_hud_y", "0", 0, "Y offset of speedrun timer HUD.\n");
Variable ("sar_sr_hud_size", "100", 0, "Size of speedrun timer HUD.\n");
Variable ("sar_sr_hud_bg_color", "0 0 0 233", "RGBA background color of speedrun timer HUD.\n", 0);
Variable ("sar_sr_hud_font_color", "255 255 255 255", "RGBA font color of speedrun timer HUD.\n", 0);
Variable ("sar_sr_hud_font_index", "1", 0, "Font index of speedrun timer HUD.\n");
Variable ("sar_sr_hud_splits", "0", 0, ".\n");
Variable ("sar_sr_hud_splits_direction", "default", "Splits direction for speedrun timer HUD. "
    "Usage: sar_sr_hud_splits_direction <top, left, right or bottom>. Default is bottom.\n");
Variable ("sar_sr_hud_splits_delta", "0", 0, "Draws split delta compared to personal best for speedrun timer HUD.\n");
Variable ("sar_sr_hud_pace", "0", 0, " for speedrun timer HUD.\n");

### Stats
- `sar_stats_jumps` prints jump stats
- `sar_stats_jumps_reset` resets jump stats
- `sar_stats_steps` prints step stats
- `sar_stats_steps_reset` resets step stats
- `sar_stats_velocity` prints velocity stats
- `sar_stats_velocity_reset` resets velocity stats
- `sar_stats_reset` resets all stats
- `sar_stats_jumps_xy <0-1>` saves jump distance peak as 2d-vector
- `sar_stats_velocity_peak_xy <0-1>` saves velocity peak as 2d-vector
- `sar_stats_auto_reset <0-2>` resets all stats automatically

### Cheats
- `sar_teleport` teleports player to a saved location
- `sar_teleport_setpos` saves current location for teleportation
- `sar_disable_challenge_stats_hud <0-1>` disables the challenge stats HUD in challenge mode (Portal 2)

#### Movement
- `sar_autojump <0-1>` enables tick-perfect jumping on the server
- `sar_jumpboost <0-2>` enables ABH or HL2-Bhop movement on the server (Portal 2 Engine)
- `sar_aircontrol <0-1>` enables more air-control movement on the server (Portal 2 Engine)
- `+bhop` makes the player jump (The Stanley Parable)
- `sar_anti_anti_cheat` sets sv_cheats to 1 (The Stanley Parable)

#### Routing
- `sar_trace_a` saves first location to measure a distance
- `sar_trace_b` saves second location to measure a distance
- `sar_trace_result` prints calculated distance between saved locations
- `sar_debug_entitiy_output` prints entity output data, similar to developer
- `sar_debug_game_events` prints game event data, similar to net_showevents

##### Unlocked
- `sv_bonus_challenge` (Portal 2)
- `sv_accelerate`*
- `sv_airaccelerate`*
- `sv_friction`*
- `sv_maxspeed`*
- `sv_stopspeed`*
- `sv_maxvelocity`*
- `sv_footsteps`*
- `sv_transition_fade_time` (Portal 2)*
- `sv_laser_cube_autoaim` (Portal 2)*
- `ui_loadingscreen_transition_time` (Portal 2)*
- `hide_gun_when_holding` (Portal 2)

*Flagged as cheat.

### TAS

#### Command Queuer
- `sar_tas_frame_at <frame> [command]` adds a command frame to the queue at specified frame
- `sar_tas_frame_after <delay> [command]` adds a command frame to the queue, relatively to last added frame
- `sar_tas_frames_at <frame> <interval> <last_frame> [command]` adds command frame multiple times to the queue at specified frame
- `sar_tas_frames_after <delay> <interval> <length> [command]` adds command frame multiple times to the queue, relatively to last added frame
- `sar_tas_start` starts executing queued commands
- `sar_tas_reset` stops execution and clears all queued commands
- `sar_tas_autostart <0-1>` starts playing queued commands automatically on first frame after a load

Here is a simple TAS of [Propulsion Catch](https://gist.github.com/NeKzor/5ba4fd9bafc80855a395b4a5f03f1c6e).

#### Replays
- `sar_tas_record` records client inputs
- `sar_tas_record_again` records inputs while in playback mode
- `sar_tas_play` plays recorded inputs
- `sar_tas_stop` stops recording or playing inputs
- `sar_tas_export <file>` exports replay to a .str file
- `sar_tas_import <file>` imports .str replay
- `sar_tas_autorecord <0-1>` records inputs automatically on load
- `sar_tas_autoplay <0-1>` plays inputs automatically on load

### Config
- `sar_cvars_save` saves important ConVar values to a file
- `sar_cvars_load` loads saved ConVar values

## Mapping

### Start & Stop Triggers
- Place a `point_servercommand` object anywhere in the map
- Use two `trigger_multiple` objects for start and stop
  - Add `OnStartTouch` as output
  - Target the created servercommand object
  - Select `Command` as input
  - Use `sar_timer_start` as parameter
  - Do the same for the second trigger with `sar_timer_stop`

![start.png](docs/start.png)

### With Checkpoints
- Use `trigger_multiple` object for start
  - Trigger `sar_timer_start`
  - Trigger `sar_cps_clear`
  - Enable checkpoint object
- Use `trigger_multiple` object for checkpoint
  - Trigger `sar_cps_add`
  - Disable itself
- Use `trigger_multiple` object for stop
  - Trigger `sar_timer_stop`

![cpstart.png](docs/cpstart.png)
![cp1.png](docs/cp1.png)

## Inspired By
- [SourcePauseTool](https://github.com/YaLTeR/SourcePauseTool)
- [SourceDemoRender](https://github.com/crashfort/SourceDemoRender)
- [SourceSplit](https://github.com/fatalis/SourceSplit)
- [cstrike-basehook-linux](https://github.com/aixxe/cstrike-basehook-linux)