[![Build Version](https://img.shields.io/badge/version-v1.5--win-brightgreen.svg)](https://github.com/NeKzor/SourceAutoRecord/projects/2)
[![Build Version](https://img.shields.io/badge/version-v1.6--linux-brightgreen.svg)](https://github.com/NeKzor/SourceAutoRecord/projects/3)
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
  - [HUD](#hud)
    - [Optional](#optional)
  - [Stats](#stats)
  - [Cheats](#cheats)
    - [Unlocked](#unlocked)
  - [Config](#config)
- [Mapping](#mapping)
  - [Start & Stop Triggers](#start--stop-triggers)
  - [With Checkpoints](#with-checkpoints)
- [Credits](#inspired-by)

## Supported Games
- Portal 2
- Aperture Tag
- Portal Stories: Mel
- Portal

## Features

### Automatic Demo Recorder
- `sar_autorecord` keeps recording a demo when loading from a save

### Automatic Binding
- `sar_bind_save <key> [save_name]` binds automatically `save save_name` to the given key when loading
- `sar_save_flag [echo_message]` appends `;echo message` to the save bind
- `sar_bind_reload <key> [save_name]` binds automatically `save save_name;reload` to the given key when loading
- `sar_unbind_save` unbinds the key and stops automatic binding for `sar_bind_save`
- `sar_unbind_reload` unbinds the key and stops automatic binding for `sar_bind_reload`
- Save files will be named _2, _3, etc.
- File indexing will be synced automatically with the demo recorder when recording with demos

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
- `sar_sum_during_session` counts current session too

### Timer
- `sar_timer_start` starts or restarts the timer, counting from invocation
- `sar_timer_stop` stops timer
- `sar_timer_result` prints result
- `sar_timer_always_running` doesn't stop the timer when disconnecting from server
- Mappers can use this for accurate timing, [see below](#mapping)

##### Average
- `sar_avg_start` starts calculating the average of the timer
- `sar_avg_stop` stops calculation
- `sar_avg_result` prints result

##### Checkpoints
- `sar_cps_add` saves current timer value
- `sar_cps_clear` resets all saved values
- `sar_cps_result` prints result of all checkpoints

### HUD
- `sar_hud_text` draws given string
- `sar_hud_position` draws player's position
- `sar_hud_angles` draws player's view angles
- `sar_hud_velocity` draws player's velocity
- `sar_hud_session` draws current session value
- `sar_hud_last_session` draws value of latest completed session
- `sar_hud_sum` draws summary value of sessions
- `sar_hud_timer` draws timer value
- `sar_hud_avg` draws current average of timer
- `sar_hud_cps` draws last checkpoint value of timer
- `sar_hud_demo` draws current name, tick and time of demo recorder or demo player
- `sar_hud_jumps` draws total jump count
- `sar_hud_portals` draws total portal count
- `sar_hud_steps` draws total step count
- `sar_hud_distance` draws calculated jump distance

##### Optional
- `sar_hud_default_spacing` space between HUD elements
- `sar_hud_default_padding_x` additional padding on x-axis
- `sar_hud_default_padding_y` additional padding on y-axis
- `sar_hud_default_font_index` font index
- `sar_hud_default_font_color` font color

### Stats
- `sar_stats_auto_reset` resets all stats automatically
- `sar_stats_reset_jumps` resets jump count
- `sar_stats_reset_steps` resets step count

### Cheats
- `sar_autojump` enables tick-perfect jumping on the server
- `sar_teleport` teleports player to a saved location
- `sar_teleport_setpos` saves current location for teleportation

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