
# Features

This file contains a detailed list of all features implemented by SourceAutoRecord up to version [1.12.8-pre6](https://github.com/p2sr/SourceAutoRecord/releases/tag/1.12.8-pre6).

## Essentials

- **Automatic demo recording**:
  - Demo sequence continuance after session change (`sar_autorecord`).
  - Configurable directory, prefix and name of the demo (`sar_record_prefix`, `sar_record_at_demo_name`).
  - Demo recording automatically starting on session start (`sar_record_at`).
  - Toggleable autorecording incrementation (`sar_record_at_increment`).
  - Back-upping the demo if attempting to overwrite it (`sar_demo_overwrite_bak`).

- **Config+**, an extension for config files and in-game console:
  - Adds a way to create advanced aliases (`sar_alias`) and command functions (`sar_function`) which can take parameters.
  - Allows the usage of nested quotation marks (`sar_expand`).
  - Adds custom variable system and basic arithmetics which can be used by aliases and functions (`svar_*`).
  - Allows conditional execution of commands (`cond` or `conds`).
  - Exposes several in-game variables for use in conditional configs.

- **Advanced console filtering** (`sar_con_filter`):
  - Allows to set more specific console filter. Multiple filters are also supported.

- **Toasts** - pop-up notifications used by other features (`sar_toast_*`):
  - Customizable color and duration, specific for tag.
  - Ability to create your own tags and toasts with it.

- **Updater** (`sar_update`):
  - Detects a new version of the plugin and updates it for you.

## Speedrun-based features

- **Speedrun timer system**:
  - Accurate in-game timing, correctly timing pauses and menus if needed.
  - Co-op integtration.
  - Category management with an ability to create custom categories and rules, which can be triggered by various level or entity-based events or special trigger zones (mtriggers).
  - Presets for most popular categories.
  - Integration with LiveSplit.
  - Speedrun timer HUD (`sar_sr_hud`) with customizable position, color and font.

- **Auto Submit**:
  - Integration with [P2SR Challenge Mode Boards](boards.portal2.sr) for automatic CM demo submission.

- **Ghosts** - an in-game ghost racers, managed by `ghost_*` console commands:
  - An ability to **play demos as ghosts** (one or multiple at once) in current session.
  - **Customization** of ghost's color and type (procedurally drawn Bendy, small circle or pyramid, in-game model).
  - **Ghost client** for joining servers and racing with other players. Along with that, **custom chat** communication (`ghost_chat`), **leaderboard** HUD  (`ghost_leaderboard_*`) and **spectator** commands (`ghost_spec_*`).

- **Challenge Mode stats HUD override** (`sar_disable_challenge_stats_hud`):
  - Prevents the Challenge Mode stats HUD from appearing at the beginning of the CM run.

- **Teleporter** (`sar_teleport`):
  - Functionality for saving and loading player and portals' position for practice purposes.

## Text-based HUDs

- **Text-based HUD element system** handled by `sar_hud_`-prefixed console commands:
  - The entire HUD block is customizable (default text color, font, position, background toggle).
  - Every plugin-defined text-based HUD can be toggled and ordered.
  - Custom HUD text support (`sar_hud_set_text`), along with color coding support (`#<hex>text`).

- Basic player-based information:
  - player's position (`position`),
  - player's view angles (`angles`),
  - player's velocity (`velocity`).

- Currently recorded/played demo name and its time (`demo`)

- Time HUDs (`timer`, `pause_timer`,  `frame`, `last_frame`, `session`, `last_session`, `sum`).

- Portal angles (`portal_angles`, `portal_angles_2`).

- Various game-based information (`eyeoffset`, `groundspeed`, `portals`, `ent_slot_serial`).

- **Funnel debug**:
  - Displays current funnel handle (`tbeam`) and funnel count (`tbeam_count`).

- **Duck state** (`duckstate`):
  - Shows basic ducked state information.
  - When `sv_cheats` is enabled, shows detailed information about ducking state, remaining duck/unduck time, forced duck and quantum crouch (with type specification).

- **Grounded frames** (`groundframes`):
  - Shows how many ticks the player has been affected by the ground friction since the last time they landed on the ground.

- **Velocity angles** (`velang`):
  - Calculates and displays angles towards which current velocity vector is pointing at.

- **FPS** (`fps`):
  - Displays the current FPS.

## User interface

- **Input HUD** (`sar_ihud`):
  - Displays the state of every in-game input in a very customizable HUD.
  - Adjustability of how each game input is displayed (text, position and size on grid, background and text color for both neutral and pressed states).
  - Key grid size, padding and position on screen.
  - Analog joystick display for both movement and view angles.
  - Simple presets for casuals and TAS makers.
  - Image display support for entire HUD and each button.

- **Strafe Quality HUD** (`sar_strafe_quality`):
  - Gives you a graph indicating groundframes and how good your strafing sync is over time.

- **Strafe Sync HUD** (`sar_strafesync`):
  - Gives you a constantly-updating percentage for how good your strafing sync is.

- **Custom crosshair** (`sar_crosshair_*` and `sar_quickhud_*`):
  - Allows user to display customized crosshair in the middle of the screen as a replacement for a default one.
  - Ability to change color, size, quickhud modes, and to set custom images for both crosshair and quickhud.

- **Velocity graph** (`sar_velocitygraph`):
  - Shows a graph displaying horizontal velocity over time.

- **Least Portals HUD** (`sar_lphud`):
  - Portal counter stylized to look like Challenge Mode HUD which accurately keeps track of placed portals in current session, trying to take save-loading into consideration.

- **Cheat Warn HUD** (`sar_cheat_hud`):
  - warms you if a setting that is forbidden in speedruns is changed.

- **Aim point HUD** (`sar_aim_point_hud`):
  - Overlays a marker with coordinates at the point you're aiming at.
  - Commands for adding and removing frozen aimpoints (`sar_aim_point_add/clear`).

- **Portal Placement HUD** (`sar_pp_hud`):
  - Displays info about portalability of the surface you're currently aiming towards.
  - Optionally overlays a portal shape which shows where the portal would be placed when shot. (`sar_pp_hud_show_blue/orange`).

- **Portal Gun HUD** (`sar_portalgun_hud`):
  - Displays linkage ID of currently possesed portal gun and portals connected to it.

- **Scroll Speed HUD** (`sar_scrollspeed`):
  - Shows a scroll speed indicator.

- **Minimap** (`sar_minimap`):
  - Displays an image HUD showing where you (and optionally your co-op partner) is on the map. Requires top-down image with proper .json configuration file that can be exported from Stitcher feature.

## Tool-Assisted Speedrunning (TAS) tools

- **Virtual TAS Controller**:
  - Accepts inputs programatically for TAS script playback.
  - Acts as the only way of injecting inputs into the game, keeping the legitimacy at the highest level.

- **Custom `.p2tas` scripting language** support.

- **TAS playback system**:
  - Feeds inputs from parsed script to the virtual controller.
  - Has ability to pause, skip, slow down, speed up and tick-advance the script playback.

- **Automation tools**:
  - A special set of tools that allows the script to autonomously react to in-game events where performed action can be automated:
    - **Auto Strafer** - finds the most optimal way to strafe on current tick, given user-defined parameters;
    - **Auto Aim** - automatically aims towards given point in 3D space, instantly or over time.
    - **Auto Jump** - automatically jumps on inpact with ground.
    - **Absolute Move Tool** - moves towards specified absolute direction.
    - **Set Angle Tool** - automatically aims towards specified view angle, instantly or over time.
    - **Decel Tool** - moves the opposite direction to your current velocity.
    - **Check Tool** - checks for player velocity or angles in given tick and restarts the playback when deviation occurs.

  - A special system for tool-based script verification - **raw scripts**, which are generated after a successful playback.

- Integration with **VS Code extension [p2tas-lang](https://github.com/RainbowwPhoenixx/p2tas-lang)**, which gives you full control over TAS playback and live debugging.

## Other major features

- **Free Camera** (`sar_cam_control`):
  - Manual control using controls similar to ones from demo's Drive mode or commands.
  - Follow camera, which follows given entity.
  - Cinematic camera (demos only):
    - Commands for defining path points and interpolation mode.
    - Possibility of exporting and importing paths using game's config system.
  - Optional ortographic projection.

- **Ruler** (`sar_ruler_*`):
  - A tool for measuring distances and angles between two points.
  - Ability to create rulers manually or using a creator (placing points where you look at).

- **Renderer** (`sar_render_*`):
  - Video renderer, fully configurable in terms of video and audio codec, quality and bitrate through console commands.
  - Frame blending.
  - Configurable autostart location and extension.

- **Stitcher** (`sar_stitcher`):
  - Custom in-game editor for capturing, stitching and exporting top-down images of the map.

- **Achievement Tracker** (`sar_achievement_tracker_*`):
  - Keeps track of achievements in current session, with ability to reset progress and isolate co-op achievements.
  - Other mods, like Portal Stories: Mel, are also supported.

- **Player Trace** (`sar_trace_*`):
  - Records and displays the path of player movement. Multiple paths are supported.
  - Colors the path depending on ground state and speedlock type.
  - Can show hitbox of the player as well as hitboxes of surrounding objects at given point on path.

## Routing/technical features

- **Placement scanner** (`sar_pp_scan_*`)
  - Overlay-based tool for checking the portalability of selected surface by generating its spark map.

- **VPhys HUD** (`sar_vphys_hud`):
  - Displays information about standing and crouched Vphys collision boxes.

- **Seamshot Finder** (`sar_seamshot_finder`):
  - An overlay tool which shows whether the corner you're currently aiming to is seamshottable.

- **FindClosestPassableSpace debug tools**.

- **Demo parsing features** (`sar_time_demo` and `sar_time_demos`):
  - Prints details and length of one or multiple demos.

- **Entity list**:
  - Commands for listing all entities or entities meeting given name/classname criteria (`sar_find_ents`, `sar_find_ent`, `sar_list_ents`).
  - Command for displaying details about given entity (`sar_ent_info`).

- **Inspection tool** (`sar_inspection`):
  - Records data of selected entity, which then can be printed in the console or saved as a .csv file.
  - Entity Inspection HUD (`sar_ei_hud`), displaying selected piece of information near inspected entity.

- **Various statistics manager**:
  - Keeps track of many session-related statistics, including retries of Challenge Mode, full game retries, total time spent in the game (SP, CM, co-op), average reset time and more.
  - Ability to export and import statistics to/from .csv file.
  - Velocity peak tracker (with text-based HUD `velocity_peak`).
  - Portals tracker (with text-based HUD `portals`).
  - Jump distance, distance peak and jump counter tracker (with text-based HUDs `jump`, `jumps` and `jump_peak`).
  - Step tracker (with text-based HUD `steps`).

- **Entity inputs debug** (`sar_show_entinp`):
  - Prints entity inputs that are happening in currently played map in the console (alternative to `developer 2`).

- **Class and datamap dumpers** (`sar_dump_*`):
  - Dumps all exposed client and server datamaps and classes into JSON files.

## Minor features
  
- **Seasonal ASCII messages**:
  - Prints seasonal message in the console on specific times of the year.

- **Cheat commands**:
  - Prevent airlock and allow air control (`sar_aircontrol`).
  - Crouch Flying Glitch on demand (`sar_give_cfg`).
  - Betsrighter (invulnerability) on demand (`sar_give_betsrighter`).
  - Force Quantum Crouch (`sar_force_qc`).
  - Crouch Flying Glitch patch (`sar_patch_cfg`).
  - Bunnyhopping patch (`sar_patch_bhop`).

- **Segmented tools**:
  - Delayed execution of commands through `hwait` command.
  - Sequential execution of commands through `seq` command.

## Tweaks and patches

- **Level loading optimisations**:
  - Eliminates majority of GUI animations which blocks the game from loading (mainly by modifying `ui_loadingscreen_`-prefixed commands).
  - Uncaps FPS and disables rendering completely on loading (using `sar_loads_uncap` and `sar_loads_norender`).
  - Several pre-defined fast loading presets to use (`sar_fast_load_preset`).

- **Demo playback tweaks**:
  - Attempts to remove broken frames at the beginning of the demo (`sar_demo_remove_broken`).
  - Attempts to interpolate player view correctly when passing through the portal (`sar_demo_portal_interp_fix`).

- **Better file-based commands autocompletion**
  - Autocompletion for commands like `playdemo`, `map` etc. works recursively in a directory.

- **`sv_cheats` patch for Portal: Reloaded** (`sar_fix_reloaded_cheats`):
  - allows a normal use of `sv_cheats` without affecting gameplay mechanics.

- **Improved co-op network communication** (`sar_always_transmit_heavy_ents`):
  - Force heavy edict transmit to prevent lag spikes

- **BINK video timing tweak** (`sar_bink_respect_host_time`):
  - Makes all BINK videos respect host time. This prevents elevator screens being sped up on renders due to them running real-time.

- **Disable weapon sway** (`sar_disable_weapon_sway`):
  - Prevents the weapon sway when the player looks around.

- **FOV Changer** (`sar_force_fov`):
  - prevents the game from resetting user-defined field of view value.

- **Prevent lag on unfocused window** (`sar_disable_no_focus_sleep`):
  - Prevents the power-saving FPS limit when the game window is unfocused.

- **Window Resizer** (`sar_allow_resizing_window`):
  - Allows to resize the game's window (experimental, Windows only).

- **Disable save indicator** (`sar_disable_save_status_hud`):
  - Hides save indicator HUD that appears when you save the game.

- **Patch Entity Handle Manipulation (EHM)** (`sar_prevent_ehm`):
  - Prevents the very rare glitch where object can lose collision (deemed forbidden in regular runs).

- **Patch CM Wrong Warp** (`sar_cm_rightwarp`):
  - Prevents Challenge Mode Wrong Warp from happening, since it can happen accidentally on NoSLA run, where it's prohibited.

- **Easier Co-op progress reset** (`sar_coop_reset_progress`):
  - Allows the player to completely reset their co-op progress.
