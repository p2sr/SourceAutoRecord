# SourceAutoRecord

<div align="center">

![](docs/img/sar_logo.webp)

</div>

<div align="center">

[![CI](https://github.com/p2sr/SourceAutoRecord/workflows/CI/badge.svg)](https://github.com/p2sr/SourceAutoRecord/actions?query=workflow%3ACI+branch%3Amaster)
[![CD](https://github.com/p2sr/SourceAutoRecord/workflows/CD/badge.svg)](https://github.com/p2sr/SourceAutoRecord/actions?query=workflow%3ACD+branch%3Amaster)
[![Latest release](https://img.shields.io/github/v/release/p2sr/SourceAutoRecord?label=latest%20release)](https://github.com/p2sr/SourceAutoRecord/releases/latest)
[![Latest prerelease](https://img.shields.io/github/v/release/p2sr/SourceAutoRecord?label=latest%20pre-release&include_prereleases)](https://github.com/p2sr/SourceAutoRecord/releases)
[![License](https://img.shields.io/github/license/p2sr/SourceAutoRecord)](https://github.com/p2sr/SourceAutoRecord/blob/master/LICENSE)

</div>

<div align="center">

### [Overview](#overview) | [Features](#features) | [Installation](#installation) | [Support](#support) | [Documentation](#documentation)

</div>

# Overview

**SourceAutoRecord** (**SAR**) is a plugin for Portal 2 and mods based on it, bringing lots of features and modifications 
into the game for speedrunners and people alike.

# Features

### General:
- Advanced automatic demo recording.
- Reduced loading times.
- Configurable in-game input display.
- Lots of useful text HUDs (position, view angles, velocity, ground frames etc.), as well as custom ones.
- Config+, an extension of the in-game console which introduces variables, functions and more.

### Speedrunning:
- Accurate session timing and LiveSplit integration.
- Built-in timer and autosplitter with complex custom rule system.
- Autosubmission of Challenge Mode runs to the community leaderboards.
- Ghosts of other players from demos or in a [ghost server](https://github.com/Blenderiste09/GhostServer).
- Tools for easier segmented run creation.

### TASing:
- Custom `.p2tas` scripting language
- Legitimate TAS script playback, controllable with tick advancing, skipping and pausing.
- Automation tools (autostrafer, autojump, autoaim etc.)
- Integration with [Visual Studio Code](https://code.visualstudio.com/) 
through the [p2tas-lang](https://github.com/RainbowwPhoenixx/p2tas-lang) plugin.
- Player trace, for recording and showing the path of the player's movement.

### Miscellaneous:
- Freecam and cinematic camera.
- Configurable renderer with video encoding and frame-blending.
- In-game overhead image stitcher, which can be used to create top-down minimaps of the level.
- Movement modifications like [ABH](https://wiki.sourceruns.org/wiki/Accelerated_Back_Hopping), air control and others.
- Other fun cheats.

# Installation


Download the latest released binary for your operating system (`.dll` for Windows, `.so` for Linux). Place it into the 
game folder, eg. `steamapps/common/Portal 2` (where your game's executable is located).  Once the game is launched, 
open developer console and enter `plugin_load sar`.

If you're planning on speedrunning, it is recommended to install [srconfigs](https://github.com/p2sr/srconfigs/) as well.

In order to update the plugin, you can simply use the in-game SAR command `sar_update`.

# Support

| Game                                                                    | Windows | Linux |
|-------------------------------------------------------------------------|---------|-------|
| [Portal 2](https://store.steampowered.com/app/620)                      | ✔       | ✔     |
| [Aperture Tag](https://store.steampowered.com/app/280740)               | ✔       | ✔     |
| [Portal Stories: Mel](https://store.steampowered.com/app/317400)        | ✔       | ✔     |
| [Thinking with Time Machine](https://store.steampowered.com/app/286080) | ✔       | ✔     |
| [Portal Reloaded](https://store.steampowered.com/app/1255980)           | ✔       | ✔     |

# Documentation
- [Contributing](docs/contributing.md)
- [Console Commands & Variables](docs/cvars.md)
