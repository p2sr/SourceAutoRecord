# SAR: Speedrun Standard (S3)

A new way to time your speedrun!

## Features

- Tick-perfect timing with rule system
- Support for categories
- Automatic start, stop and split
- Menu timing
- Pause timing
- LiveSplit support

## Support

Game|Windows|Linux
---|:-:|:-:
[Portal 2](https://store.steampowered.com/app/620)¹|✔|✔
[Aperture Tag](https://store.steampowered.com/app/280740)|✔|✔
[Portal Stories: Mel](https://store.steampowered.com/app/317400)|✔|✔
[Thinking with Time Machine](https://store.steampowered.com/app/286080)|✔|✔
[The Stanley Parable](https://store.steampowered.com/app/221910)|✖|✖
[The Beginners Guide](https://store.steampowered.com/app/303210)|✖|✖
[Half-Life 2](https://store.steampowered.com/app/220)|✖|✖
[Portal](https://store.steampowered.com/app/400)|✔|✔
[INFRA](https://store.steampowered.com/app/251110)|✖|➖

¹ Cooperative game not fully supported.

## Interface

SAR provides an interface for accessing the timer externally which can be used with [LiveSplit](https://livesplit.github.com).

### Examples

- [SAR.asl](https://raw.githubusercontent.com/NeKzor/SourceAutoRecord/livesplit/SAR.asl)

## Console Commands & Variables

|Name|Default|Description|
|---|:-:|---|
|sar_speedrun_standard|1|Speedrun timer automatically starts, splits and stops.
|sar_speedrun_reset|cmd|Resets speedrun timer.
|sar_speedrun_result|cmd|Prints result of speedrun.
|sar_speedrun_autostart|0|Starts speedrun timer automatically on first frame after a load.
|sar_speedrun_autostop|0|Stops speedrun timer automatically when going into the menu.
|sar_speedrun_smartsplit|1|[Timer interface](#interface) only splits once per level change.
|sar_speedrun_categories|cmd|Lists all categories.
|sar_speedrun_category|cmd|Sets the category for a speedrun.
|sar_speedrun_offset|cmd|Sets offset in ticks at which the timer should start.
|sar_speedrun_time_pauses|1|Timer automatically counts non-simulated ticks every time the server pauses.

### HUD

Use sar_sr_hud 1 to draw the current time to the screen.

|Name|Default|Description|
|---|:-:|---|
|sar_sr_hud_x|0|X offset of speedrun timer HUD.
|sar_sr_hud_y|100|Y offset of speedrun timer HUD.
|sar_sr_hud_font_color|255 255 255 255|RGBA font color of speedrun timer HUD.
|sar_sr_hud_font_index|70|Font index of speedrun timer HUD.

#### Manual Mode

|Name|Default|Description|
|---|:-:|---|
|sar_speedrun_pause|cmd|Pauses speedrun timer.
|sar_speedrun_resume|cmd|Resumes speedrun timer.
|sar_speedrun_split|cmd|Splits speedrun timer.
|sar_speedrun_start|cmd|Starts speedrun timer.
|sar_speedrun_stop|cmd|Stops speedrun timer.

### Data File

|Name|Default|Description|
|---|:-:|---|
|sar_speedrun_export_pb|cmd|Saves speedrun personal best to a csv file.<br>Usage: sar_speedrun_export_pb <file_name>
|sar_speedrun_export|cmd|Saves speedrun result to a csv file.<br>Usage: sar_speedrun_export <file_name>
|sar_speedrun_import|cmd|Imports speedrun data file.<br>Usage: sar_speedrun_import <file_name>
