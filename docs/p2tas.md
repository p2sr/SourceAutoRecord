# P2TAS Documentation

## Table of Contents
1. [Introduction](#introduction)
2. [SAR's TAS environment specification](#sars-tas-environment-specification)
3. [`.p2tas` file structure](#p2tas-file-structure)
4. [Automation tools](#automation-tools)
8. [Version history](#version-history)


## Introduction

P2TAS is a scripting language and a set of tools used for defining and playing back a sequence of time-stamped actions to be executed within Portal 2 (and other Portal 2-based games supported by SAR), for the purpose of creating tool-assisted speedruns.

The following document serves as a documentation of SAR's behaviour for the purpose of TAS playback, as well as P2TAS scripting language and features available through it.

## SAR's TAS environment specification

### Script playback
Tool-assisted speedrunning in Portal 2 is done through a playback of pre-programmed sequence of inputs declared by our custom P2TAS scripting language. SAR is handling both interpretation of these scripts and their playback, which includes injection of player inputs and their proper timing. Although SAR is filled with other features, some of which make TAS creation process easier, they're not required for a successful TAS script playback.

### The virtual controller
SAR implements a virtual controller by injecting code into the Steam Controller's input fetching function. In raw script playback, this is the only injection point required for TAS script playback, ensuring minimal interference with game logic.

This controller supports:

- **Digital inputs**: jump, crouch, use, zoom, primary/secondary attack (blue and orange portal)
- **Analog inputs**: 2D movement and view angle controls with floating-point precision
- **Console commands**: Arbitrary command execution at specific ticks (as a way to replicate key binds)

The virtual controller has effectively instant response time, allowing different inputs on consecutive ticks despite the existence of `sv_alternateticks` - something confirmed to be replicable with physical hardware (Steam Controller with ~1ms response time).

### Automation tools and raw playback
SAR's TAS environment includes a set of automation tools (like autojump, autoaim or autostrafer) that calculate complex input sequences based on real-time state of the game. In order for them to work, SAR injects into additional game functions to read player's state properly, as they're not yet available at the input fetching stage.

Because of that, two distinct playback modes exist: 
- **Tools playback** -  scripts are processed with all tools specified in the script. This is a default playback mode and should be used in a creation process of a TAS script. It is not guaranteed to procude fully legitimate gameplay. By default, tools playback should produce a raw script, automatically saved as a file with `_raw.p2tas` suffix.

- **Raw playback** - only pre-calculated inputs are interpreted with no tools processing. Every script with `_raw.p2tas` suffix is expected to be a raw script and will be played as such. Raw scripts are guaranteed to produce legitimate gameplay, as they're not using any other extra input injection methods other than the virtual controller, so they should be used for final verification and demo recording.

### RNG Manipulation

Portal 2 TASing is famously annoying due to randomness - both game-based and physics based - which can lead to inconsistent results from a playback of a single script. In the past, there were several attempts of artificially manipulating RNG to resolve script playback into a single possibility. So far, there's a limited amount of RNG manipulation available, mostly related to simple gameplay RNG like gel spread, catapulted prop torque and view punch.

SAR can generate files storing details of RNG state using console command `sar_rng_save`, which then can be used in a P2TAS script by referencing this file in the header of the script to reproduce exact same RNG state. While this solution is not ideal in terms of legitimacy, it is not discouraged, as a final result is still based on state of the game that once existed.

## `.p2tas` file structure

Script written in P2TAS is stored as a text file with `.p2tas` extension. The file is expected to start with a header, and then be followed by at least one tickbulk.

```
version <versionnumber>
start {next} {save/next/cm/now} <name>
[rngmanip [path]]

<tickbulk>
[tickbulk]
...
```

The parser is case-sensitive. All keywords are lowercase.

### Header

The file should begin with a set of properties, defined in this exact order:

- `version <number>` - required, defines a version of the script file, which affects how the script is processed and executed. Currently, the newest version is 7. For more details, see [Version History](#version-history).
- `start {next} {save/next/cm/now} <name>` - required, defines a starting point for the script file playback:
  - `next` - when used, playback doesn't start immediately after the given starting operation, but after the next game load. Usually used in full-game runs to run scripts right after level transition.
  - `save` - starts from a save with a given `name`.
  - `map` - starts from a map with a given `name`.
  - `cm` - same as `map`, but starts it in Challenge Mode.
- `rngmanip <path>` - optional, defines a path to `.p2rng` file. For more details, see [RNG Manipulation](#rng-manipulation)

### Tickbulks

A tickbulk is a line defining a state of the virtual controller and automation tools at given moment of time. A normal tickbulk consists of a number defining a tick-based timestamp representing a moment of time when it should apply its actions, followed by a set of tickbulk parts representing the actions to apply.

All non-empty lines past header are expected to be tickbulks, ordered by an ascending timestamp.

Tickbulks follow the specified structure:

```cs
tick>movement|camera|buttons|commands|tools
```

...where each tickbulk part separated by a pipeline represents the following:

- `movement` - a 2D vector defined by two numbers separated by a whitespace, defining horizontal and vertical (in that order) value of movement analog input, in a range from -1 to 1 for both axes. The vector's length corresponds to the force of the movement, with 1 being the maximum.
- `camera` - a 2D vector defined by two numbers separated by a whitespace, defining horizontal and vertical (in that order) value of movement analog input, in a range from -180 to 180 for both axes. The values correspond to the number of degrees the camera will rotate in given tick.
- `buttons` - a sequence of digital input states. Each digital input is assigned to a letter. Passing uppercase letter activates the digital input, and lowercase deactivates it. Optionally, a number immediately following the uppercase letter indicates how long the input should be activated for. These digital inputs are supported:
  - `J`/`j` - jump,
  - `D`/`d` - duck/crouching,
  - `U`/`u` - use/interacting,
  - `Z`/`z` - zooming,
  - `B`/`b` - blue portal/primary attack,
  - `O`/`o` - orange portal/secondary attack.
- `commands` - a set of semicolon-separated console commands to execute at given timestamp.
- `tools` - a set of semicolon-separated automation tool commands to execute at given timestamp. Each command starts with an identifier of a tool, and is followed by a space-separated list of parameters for it. For more information, see [Automation Tools](#automation-tools).

An example of a tickbulk using all tickbulk parts:

```cs
// moving forward, jumping for one tick, holding duck,
// using command to enable trace and starting autoaim automation tool.
69>0 1|0 0|J1Duzbo|sar_trace_record 1|autoaim ent prop_weighted_cube
```

Everything except console commands and tool commands is persistent, meaning that once set, the values will persist for the following ticks.

```cs
10>0 1|1 0|D|| // began moving forward, rotating right and ducking.
20>|0 0||| // stopped rotating, but still moving and ducking.
```

Additionally, as shown above, tickbulk parts are optional. If persistent parts are left empty, they will preserve the behaviour of the same part in last defined tickbulk. This also means that pipelines are optional as well, and the following tickbulk is valid:

```cs
69> // yup, that's valid
```

### Relative timestamp

Timestamp in a tickbulk can be made relative to the last written one by adding a `+` character prefix to it. This is helpful when an action has to be performed a certain amount of time after another.

```cs
120>0 1 // start moving
+10>||J1 // jump 10 ticks later (tick 130)
+35>||O1 // shoot orange portal 35 ticks after jumping (tick 165)
```

### Tool tickbulk

It is often needed to only use tickbulk part responsible for automation tools commands. For this purpose, a special tickbulk structure is allowed:

```cs
tick>>tools
```

This allows to avoid typing multiple pipelines to reach the automation tools tickbulk part.

```cs
60>||||autojump on // inferior pipeline quartet 
+200>>autojump off // superior chevron duet
```

### `repeat` blocks

`repeat` block is a parser-level extension of the P2TAS language. Everything within the `repeat` block is repeated given amount of times. They can be nested.

A `repeat` block is defined by a set of lines which begins with a `repeat` line containing a number of iterations to perform, and ends with `end` line:

```cs
repeat <number>
// repeated content
end
```

As an example, this `repeat` block would cause the `+use` button to be pressed 10 times every 2 ticks:

```cs
repeat 10
+2>||U1
end
```

### Comments

P2TAS supports single-line comments, starting with `//` and running until the end of the line. It also allows multi-block comments, starting with `/*` and ending with `*/`. Everything that is a comment is ignored by a parser.

### Whitespaces

Whitespaces are used to separate tokens and are useless beyond this point. This means you can put as many spaces, newlines and indents as you want, anywhere you want, as long as it doesn't actually affect the tokens themselves.

## Automation tools

To make certain actions easier to perform, SourceAutoRecord includes automation tools which can be controlled with tool commands in a tickbulk. These tools fetch game data in real time to produce appropriate controller inputs.

Tools can have a continuous behaviour, affecting player inputs every tick, meaning that you only have to set its parameters once with tool commands for it to work. Consequently, if a tool has a continuous behaviour that never ends by itself, it has to be turned off by sending an appropriate tool command.

Below is a complete list of automation tools supported by SourceAutoRecord.

### `strafe` tool

```cs
strafe <parameters>
```

A tool for automated strafing. It's the most complicated automation tool in P2TAS. It will attempt to find the most optimal movement direction which satisfy criteria specified by parameter (whether it's greatest acceleration towards specified direction or something else) and apply it to player's inputs.

Strafe tool accepts any non-zero number of parameters in unspecified order. Here's a list of behaviours which can be manipulated with those parameters:

- **Input type** - The goal of autostrafer is to find a movement direction. However, this direction can be achieved by the player in multiple ways. These paramaters can control the input mode:
  - `vec` (default) - vectorial strafing - using analog movement input to control the direction of movement, while view angles remains mostly unaffected.
  - `ang` - angular strafing - using view angle input to control the direction of movement, while the movement is oscillated between left and right. Useful for strafing on propulsion gel, as moving on it causes your side movement acceleration to be scaled down.
  - `veccam` - same as vectorial strafing, but the view angles will be modified to point along current horizontal velocity. Occasionally useful for better visual presentation.

- **Target velocity** - By default, the tool will attempt to maximize acceleration for every tick. Same behaviour occurs when target velocity is set to be higher than a current one. If they're equal, the tool will attempt to strafe towards target direction while keeping the same velocity, taking ground friction into consideration. If target velocity is lower, it will try to maximize velocity direction change in every tick, effectively making velocity slightly lower (for better deceleration, consider using `decel` tool.). These paramaters can control target velocity:
  - `max` (default) - always accelerates as much as possible.
  - `min` - sets target horizontal velocity to 0 units per second.
  - `keep` - attempts to keep horizontal speed from the moment of sending the tool command.
  - `<number>ups` - sets target horizontal velocity to given value, in units per second.

- **Target direction** - Because of the very nature of strafing, there are two possible strafe directions. Every tick, the tool will decide between those two directions, preferring the one that brings player's velocity direction closer to target direction given through parameters. After the target is reached, the tool will define a straight line it will try to follow until external factors (like colliding with something) will cause it to deviate too far from it, in which case, it'll fall back to default mode. These parameters can control target direction:
  - `forward` - strafes towards the currently faced direction.
  - `forwardvel` (default) - strafes towards current velocity direction.
  - `left` - forces autostrafer to always strafe left.
  - `right` - forces autostrafer to always strafe right.
  - `<number>deg` - sets target direction based on given angle in degrees. It corresponds to a look direction for given yaw angle (second angle number in `cl_showpos`)

- **Extra behaviour** - Autostrafer has several internal mechanisms which work around some game's limitations in order for a tool to function properly. These behaviours can be tweaked with these parameters:
  - `nopitchlock` - prevents the autostrafer from locking view pitch angle in a range between -30 and 30 degrees, which is the range where pitch angle doesn't affect player's movement. Using this flag and going past this range makes strafing sub-optimal, but allows for greater aim range.
  - `letspeedlock` - allows the strafer to become soft-speedlocked when strafing above 300ups midair. This sacrifices speed, but allows slightly better turning towards a diagonal direction when affected by a speedlock.

**Movement input manipulation** - For more advanced usage, it is sometimes necessary to make a tiny adjustment of the movement input vector, even if it means sacrificing strafe accuracy. These parameters can be used for that:
  - `<number>usdeg`, `<number>osdeg` - sets an offset, in degrees, for a calculated movement direction. `usdeg` will understrafe, bringing movement vector closer to velocity vector and causing weaker turn, while `osdeg` will overstrafe, bringing movement vector further away from velocity vector and causing bigger turn.
  - `<number>us`, `<number>os` - controls understrafing and overstrafing, similarly to `usdeg` and `osdeg`, however, these accept scalar values, where unit value for both (`1us` and `1os`) are equivalent of bringing movement direction yielding most acceleration to the closest direction yielding no acceleration in both ways.
  - `<number>` - using suffix-less number parameter is treated as an input strenght - movement input vector will be scaled by that number.


Additional notes:
- Parameters marked as default define the default behavior of the tool (as in, the behavior which occurs when no other parameters in given category are given).
- The tool is continuous, meaning it has to be disabled with `off` parameter. It can also be enabled with `on` parameter, which will use default settings.
- When the tool is about to reach both target velocity and target direction, it will calculate perfect movement input to reach them and then silently stop, but will continue once any of these change.

Examples of usage:

- `strafe on` - enables autostrafer with default settings (vectorial strafing with the greatest acceleration towards current looking direction)
- `strafe right` - vectorial-strafes with the greatest possible acceleration clockwise.
- `strafe ang forwardvel` - angular-strafes with the greatest possible acceleration towards the current velocity direction.
- `strafe 30deg max nopitchlock` - vectorial-strafes with the greatest possible acceleration towards 30 degrees without locking pitch angle between -30 and 30 degrees.
- `strafe 255ups 0.1os` - vectorial-strafes to reach velocity of 255ups

### `autojump` tool

```cs
autojump {on/unducked/ducked/duck/off}
```

Autojump tool, as the name suggests, will automatically manipulate the jump input in order to perform tick-perfect jumps from the ground, avoiding any ground friction. It's normally used in combination with the `strafe` tool to achieve perfect bunnyhopping.

This tool accepts one of the following parameters:

- `on` or `unducked` - enables autojumping.
- `ducked` or `duck` - enables ducked autojumping, which holds crouch button when jumping, giving a small boost in the jump height.
- `off` - disables autojumping.

### `absmov` tool

```cs
absmov {<angle>deg/off} [scale]
```

This tool allows to move in an absolute direction defined by an angle in degrees (which corresponds to a look direction for a given yaw angle).

It takes at most two parameters. First parameter is expected to be a number defining an angle of movement as described above. The second parameter is an optional input strenght, between 0.0 and 1.0 (default).

The tool is continuous can be disabled by passing `off` as a parameter.

### `setang` tool

```cs
setang {<pitch> <yaw>/ahead} [time] [easing_type]
```

Similarly to a `setang` console command, this tool can set view angles to a desired pitch and yaw values. You can also use `ahead` as a parameter to set the view angles to look the direction you're currently moving. In addition, this tool allows a smooth interpolation from current view angles to desired ones.

The tool takes at least one parameter when looking `ahead`, or two which describe the desired pitch and yaw angle, in this order. The following optional parameter describes a time in ticks for how long the view angle should be interpolated to desired angles before reaching them. The interpolation takes external factors into consideration (like passing through portals) and will always end up on given absolute angles. Optional final parameter describes an easing function of interpolation:

- `linear` - interpolates linearly. Default if no interpolation method is given.
- `sine` or `sin` - interpolates using a sine easing.
- `cubic` - interpolates using a cubic easing.
- `exp` or `exponential` - interpolates using an exponential easing.

When time parameter is used, this tool is continuous. It will disable itself once the angle has been reached, but it can be interrupted by using `off` parameter or with another use of `setang` command.

Examples of usage:

- `setang ahead` - looks the current direction of movement instantly.
- `setang ahead 10 sine` - smoothly interpolates to looking the current direction of movement over 10 ticks.
- `setang -30 90` - looks at the pitch -30 and yaw 90 instantly.
- `setang -30 90 20` - linearly interpolates to looking at the pitch -30 and yaw 90 over 20 ticks.
- `setang -30 90 30 exp` - exponentially interpolates to looking at the pitch -30 and yaw 90 over 30 ticks.

### `autoaim` tool

```cs
autoaim {ent <entity_selector>/<x> <y> <z>/off} [time] [easing_type]
```

Works similar to `setang` tool, except the tool is continuously aiming towards given entity or point until the tool is disabled.
If first parameter is not `ent`, it expects the first three parameters to be X, Y and Z coordinates of a point to aim towards. Otherwise, it expects a second parameter to be a string representing an entity selector (an entity classname or targetname, optionally followed by a square-bracket array accessor describing which entity in a list of ones matching the selector to use for this tool).
Similarly to `setang` tool, it takes interpolation time and easing type as optional parameters.

This tool is always continuous, even after finishing interpolation. It can be disabled by passing `off` as a first parameter.

Example of usage:

- `autoaim 128 0 512 10 sine` - smoothly interpolates to looking towards world point X:128 Y:0 Z:512 for 10 ticks, then locks on that point until the autoaim tool is disabled.
- `autoaim ent prop_weighted_cube[2]` - autoaims towards *third* weighted cube existing within the map.
- `autoaim off` - disables the autoaim tool.

### `decel` tool

```cs
decel {<speed>/off}
```

This tool decelerates the player as fast as possible towards the desired horizontal velocity, in units per second. It's a continuous tool that disables itself once target velocity has been reached, but it can be disabled any time by passing `off` as a parameter.

### `check` tool

```cs
check <conditions>
```

This tool can be used to perform a check on player's properties at the tick of activation to ensure specific outcome of a TAS script (as a way to work around inconsistency issues). When given condition fails, it automatically restarts the script until condition is passed or it reaches maximum number of replayes (which can be tweaked with `sar_tas_check_max_replays` console variable.)

Here's a list of possible conditions:
- `pos <x> <y> <z>` - checks if player is at given world coordinates.
- `posepsilon <epsilon>` - defines maximum allowed distance from player to given coordinates in units (0.5 by default).
- `ang <pitch> <yaw>` - checks if the player aims towards given angles (in degrees).
- `angepsilon <epsilon>` - defines maximum allowed distance from current player angles to given ones, in degrees (0.2 by default).
- `vel <x/y/z/xy/xz/yz/xyz> <speed>` - checks if player's accumulated velocity from given components is equal to given value.
- `velepsilon <epsilon>` - defines maximum allowed difference between player's accumulated velocity from components given in `vel` parameter and the speed value defined in that parameter (1.0 by default).
- `holding [entity_selector]` - checks if player is holding a prop. Optionally, accepts a selector to check for a specific entity. Behaviour of the selector is similar to the one from [`autoaim` tool](#autoaim-tool).

### `duck` tool

```cs
duck {on/<time>/off}
```

A tool-based alternative to duck digital input.

### `use` tool

```cs
use [{spam/off}]
```

A tool-based alternative to use digital input. Passing no parameters will press the `+use` key once. Passing `spam` as a parameter will spam the `+use` input every other tick until disabled with `off` parameter.

### `zoom` tool

```cs
zoom {in/out/toggle}
```

A tool-based alternative to zoom digital input. Passing `in` or `out` will zoom in or out respectively, unless the desired zooming state is already active, in which case it does nothing. Passing `toggle` will toggle between zooming in and out.

### `shoot` tool

```cs
shoot {blue/orange/off} {hold/spam}
```

A tool-based alternative to digital inputs for shooting blue and orange portal. Passing `hold` as a parameter will hold the input until disabled with `off` parameter. Passing `spam` as a parameter will spam the shoot input with accordance to the fire delay until disabled with `off` parameter.

### `cmd` tool

```cs
cmd <command>
```

A tool-based alternative to console command part of a tickbulk. It will simply execute the command given as a parameter, whitespace allowed. It does not allow multiple commands, as command separator (`;`) is also used as a command separator for TAS tools. Use multiple `cmd` commands or a command tickbulk part instead.

### `move` tool

```cs
move {off/stop/forward/back/backward/left/right/<value>deg/<x> <y>} [scale]
```

A tool-based alternative to movement part of a tickbulk. It will move the player in the given direction until it's disabled with the `off` parameter. Similarly to `absmov`, it can be scaled by an optional parameter. In addition, the tool accepts special keywords for movement direction. On top of that, these keywords can be used in combination. As an example: `forward left` will move the player towards the forward-left direction.

### `look` tool

```cs
look {off/stop/up/down/left/right/<value>deg} [time]
```

A tool-based alternative for look direction part of a tickbulk. It will change the view angle based on given parameters. It accepts different combinations of parameters:

- if two values of degrees are given, they're used as pitch and yaw delta for each tick.
- if only one value is given along with one or multiple direction identifiers (up, down, left or right), it will use the value as scalar for given directions.

Additionally, time parameter in ticks can be given, which will disable the tool after given amount of ticks.

### `stop` tool

```cs
stop [types]
```

Stops all tools that were activated in the past tickbulks. Optionally, it accepts any number of parameters specifying types of tools to disable. Here's a list of available types:
- `movement` (or `moving`, `move`) - disables tools related to movement
- `viewangles` (or `angles`, `ang`, `looking`, `look`) - disables tools related to changing your angle (this does NOT include `strafe` tool)
- `buttons` (or `inputs`, `pressing`, `press`) - disables tools related to pressing digital inputs
- `all` (or `everything`) - default behaviour, disables every tool.


## Version history

- Version 1:
  - Initial release.
- Version 2:
  - Fix a bug where strafe tool is suboptimal with manual jump inputs (e.g. `>||J1`).
- Version 3:
  - Introduce static tool execution order.
  - Fix some Windows <-> Linux inconsistencies regarding `abs` overloads.
  - Fix `forwardmove`/`sidemove` calculation for `strafe` tool.
  - Disable `veccam` strafing when line is reached.
- Version 4:
  - Add anti-speedlock functionality to autostrafer (disable with `letspeedlock`).
  - Fix more Windows <-> Linux inconsistencies (`sin`, `cos`, `atan2`, and `pow`).
- Version 5:
  - Fix anti-speedlock on the ground.
- Version 6:
  - handle `veccam`+`setang` correctly.
  - Fix autostrafer line-following problems.
  - Fix autostrafer behaviour at velocity length 0.
  - Pitchlock to the correct sign.
- Version 7:
  - Fix autostrafer air control limit with `sar_aircontrol` set.
- Version 8:
  - Improve autostrafer to lock onto target speed and direction instead of wiggling.
  - Improve autostrafer to handle soft speedcap better.
  - Improve autostrafer to respect pitch while keeping velocity in `nopitchlock` mode.
  - Fix `decel` tool not respecting jump-based tools.
- Version 9:
  - Fix `use` tool by processing some tools during input fetching.

Last updated during version 9 release.
