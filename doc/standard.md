# SAR: Speedrun Standard (S3)

A new way to time your speedrun!

## Features

- Tick-perfect timing with rule system
- Support for categories
- Automatic start, stop and split
- Menu timing
- Pause timing (1.9)
- LiveSplit support

## Support¹

Game|Windows|Linux
---|:-:|:-:
[Portal 2](https://store.steampowered.com/app/620)²|✔|✔
[Aperture Tag](https://store.steampowered.com/app/280740)|✔|✔
[Portal Stories: Mel](https://store.steampowered.com/app/317400)|✔|✔
[The Stanley Parable](https://store.steampowered.com/app/221910)|✖|✖
[The Beginners Guide](https://store.steampowered.com/app/303210)|✖|✖
[Half-Life 2](https://store.steampowered.com/app/220)|✖|✖
[Portal](https://store.steampowered.com/app/400)|✔|✔

¹ Latest Steam version.<br>
² Cooperative game not supported.

## Interface

SAR provides an interface for accessing the timer externally which can be used with [LiveSplit](https://livesplit.github.com).

### Examples

- [SAR.asl](https://raw.githubusercontent.com/NeKzor/SourceAutoRecord/livesplit/SAR.asl)

## Console Commands

### Commands

- sar_speedrun_start
- sar_speedrun_stop
- sar_speedrun_split
- sar_speedrun_pause
- sar_speedrun_resume
- sar_speedrun_reset
- sar_speedrun_result
- sar_speedrun_export
- sar_speedrun_export_pb
- sar_speedrun_import
- sar_speedrun_category
- sar_speedrun_categories
- sar_speedrun_offset

### Variables

- sar_speedrun_autostart
- sar_speedrun_autostop
- sar_speedrun_standard
- sar_speedrun_time_pauses (1.9)

### HUD

- sar_sr_hud
- sar_sr_hud_x
- sar_sr_hud_y
- sar_sr_hud_font_color
- sar_sr_hud_font_index
