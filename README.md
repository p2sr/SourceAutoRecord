# SourceAutoRecord

## Main Features
- Automatic demo recording
- Automatic binding
- Demo parsing
- Session timing
- Small engine bug fixes

## Supported Games
- Portal 2

## List of Commands

| Name | Description |
| --- | --- |
| sar_bind_save | Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_save <key> [save_name] |
| sar_bind_reload | Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_reload <key> [save_name] |
| sar_unbind_save | Unbinds current save rebinder. |
| sar_unbind_reload | Unbinds current save-reload rebinder. |
| sar_save_flag | Echo message when using sar_binder_save. Default is \"#SAVE#\", a [SourceRuns standard](https://wiki.sourceruns.org/wiki/Demo_Recording). Keep this empty if no echo message should be binded. |
| sar_time_demo | Parses a demo and prints some information about it. |
| sar_time_demos | Parses multiple demos and prints the total sum of them. |
| sar_session_tick | Prints the current tick of the server since it has loaded. |
| sar_about | Prints info about this plugin. |
| sar_sum_here | Starts counting total ticks of sessions. |
| sar_sum_reset | Stops current running summary counter and resets. |
| sar_sum_result | Prints result of summary. |
| sar_autojump | Enables automatic jumping on the server. |
| sv_bonus_challenge | - |
| sv_accelerate | - |
| sv_airaccelerate | - |
| sv_friction | - |
| sv_maxspeed | - |
| sv_stopspeed | - |

## Inspired By
- [SourcePauseTool](https://github.com/YaLTeR/SourcePauseTool)
- [SourceDemoRender](https://github.com/crashfort/SourceDemoRender)