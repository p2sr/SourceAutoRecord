# P2TAS Documentation

## Introduction

P2TAS is a scripting language used for defining a sequence of time-stamped actions to be executed by a virtual controller within Portal 2 (and other Portal 2-based games supported by SAR), for the purpose of creating tool-assisted speedruns.

The following document will explain the P2TAS syntax, as well as technical details of how P2TAS files are processed and executed by the plugin.

## TAS Virtual Controller specifications

SAR uses its own virtual controller to execute actions defined by P2TAS files. It was implemented by injecting custom code into a Steam Controller's input processing function.

The controller allows the following input methods:

- digital inputs (jumping, crouching, "use" action, zooming, primary/secondary attack),
- floating-point precision analog inputs (movement and camera view),
- console command execution.

All of inputs are executed with tick-based precision, even despite the alternate-ticks mechanism.

The following features of the virtual controller has been proven possible to replicate by physical hardware.

## File structure

Script written in P2TAS is stored as a text file with `.p2tas` extension, which is then interpreted by SourceAutoRecord plugin when a playback of said file is requested.

The file is expected to start with a header, and then be followed by at least one tickbulk.

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

Additionally, as shown above, tickbulk parts are optional. If persistent parts are left empty, they will preserve the behaviour of the same part in last defined framebulk. This also means that pipelines are optional as well, and the following framebulk is valid:

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

## Automation Tools

To make certain actions easier to perform, SourceAutoRecord includes automation tools which can be controlled with tool commands in a tickbulk. These tools fetch game data in real time to produce appropriate controller inputs.

Tools work every tick, meaning that you only have to set its parameters once with tool commands for a continuous behaviour. Consequently, if a tool has a continuous behaviour that never ends by itself, it has to be turned off by sending an appropriate tool command.

Below is a complete list of automation tools supported by SourceAutoRecord.

### `strafe` tool

```cs
strafe {vec/ang/veccam/off} {max/keep/<number>ups} {forward/forwardvel/left/right/<number>deg} {nopitchlock} {letspeedlock}
```

A tool for automated strafing. It will attempt to gain the biggest acceleration in given tick, while trying to change movement direction towards the specified one.

Autostrafe tool will always prioritize accelerating first, unless there's no longer need for accelerating. Since there are almost always two ideal movement directions for the perfect acceleration, it will choose direction based on currently set target direction. If that direction has been reached, the autostrafer will toggle into a "line mode", where it will attempt its best to follow a straight line towards set target direction.

Strafe tool accepts any number of parameters. Here's a list of valid parameters:

- Switching strafe modes:
  - `vec` (default) - vectorial strafing - using analog movement input to control the direction of movement, while view angles remains mostly unaffected.
  - `ang` - angular strafing - using view angle input to control the direction of movement, while the movement is oscillated between left and right. Useful for strafing on propulsion gel.
  - `veccam` - same as vectorial strafing, but the view angles will be modified to point along current horizontal velocity.
  - `off` - disables autostrafer.
- Manipulating target speed:
  - `max` (default) - always accelerates as much as possible
  - `keep` - attempts to keep horizontal speed from the moment of sending the tool command.
  - `<number>ups` - attempts to reach given speed, and then keep it. It will either accelerate as efficiently as possible, or decelerate by finding the most efficient rotation towards the target direction as possible (doesn't decelerate as fast as possible. For that, use `decel`).
- Manipulating target direction:
  - `forward` - strafes towards the currently faced direction.
  - `forwardvel` (default) - strafes towards current velocity direction.
  - `left` - forces autostrafer to always strafe left.
  - `right` - forces autostrafer to always strafe right.
  - `<number>deg` - sets target direction based on given angle. It corresponds to a look direction for given yaw angle.
- Additional behavior flags:
  - `nopitchlock` - prevents the autostrafer from locking view angle pitch in a range between -30 and 30 degrees, which is the range where pitch doesn't affect player's movement. Using this flag and going past this range makes strafing sub-optimal, but allows for greater aim range.
  - `letspeedlock` - allows the strafer to become soft-speedlocked when strafing above 300ups midair. This sacrifices speed, but allows slightly better turning towards a diagonal direction when affected by a speedlock.

Parameters marked as default define the default behavior of the tool (as in, the behavior which occurs when no other parameters in given category are given).

Examples of usage:

- `strafe max right` - vectorial-strafes with the greatest possible acceleration clockwise.
- `strafe ang forwardvel` - angular-strafes with the greatest possible acceleration towards the current velocity direction.
- `strafe 30deg max nopitchlock` - vectorial-strafes with the greatest possible acceleration towards 30 degrees without locking pitch angle between -30 and 30 degrees.

### `autojump` tool

```cs
autojump {on/ducked/duck/off}
```

Autojump tool, as the name suggests, will automatically manipulate the jump input in order to perform tick-perfect jumps from the ground, avoiding any ground friction. It's normally used in combination with the `strafe` tool to achieve perfect bunnyhopping.

This tool accepts one of the following parameters:

- `on` - enables autojumping.
- `ducked` or `duck` - enables ducked autojumping, which holds crouch button when jumping, giving a small boost in the jump height.
- `off` - disables autojumping.

### `absmov` tool

```cs
absmov {<angle>deg/off} [scale]
```

This tool allows to move in an absolute direction defined by an angle in degrees (which corresponds to a look direction for a given yaw angle).

It takes at most two parameters. First parameter is expected to be a number defining an angle of movement as described above. The second parameter is optional, and defines a scalar value of movement analog vector (as in, how much to move in a given direction) in a range from 0.0 to 1.0.

The tool can be disabled by passing `off` as a parameter.

### `setang` tool

```cs
setang {<pitch> <yaw>/ahead} [time] [easing_type]
```

Similarly to a `setang` console command, this tool can set view angles to a desired pitch and yaw values. You can also use `ahead` as a parameter to set the view angles to look the direction you're currently moving. In addition, this tool allows a smooth interpolation from current view angles to desired ones.

The tool takes at least one parameter when looking `ahead`, or two which describe the desired pitch and yaw angle, in this order. Optional next parameter describes a time in ticks for how long the view angle should be interpolated to desired angles before reaching them. Optional final parameter describes an easing function of interpolation:

- `linear` - interpolates linearly. Default if no interpolation method is given.
- `sine` or `sin` - interpolates using a sine easing.
- `cubic` - interpolates using a cubic easing.
- `exp` or `exponential` - interpolates using an exponential easing.

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
Again, similarly to `setang` tool, it takes interpolation time and easing type as optional parameters.
The tool can be disabled by passing `off` as a first parameter.

Example of usage:

- `autoaim 128 0 512 10 sine` - smoothly interpolates to looking towards world point X:128 Y:0 Z:512 for 10 ticks, then locks on that point until the autoaim tool is disabled.
- `autoaim ent prop_weighted_cube[2]` - autoaims towards *third* weighted cube existing within the map.
- `autoaim off` - disables the autoaim tool.

### `decel` tool

```cs
decel {<speed>/off}
```

This tool decelerates the player as fast as possible towards the speed, in units per second, specified by a first parameter. It can be disabled any time by passing `off` as a parameter instead of speed number.

### `check` tool

```cs
check {pos <x> <y> <z>} {posepsilon <number>} {ang <pitch> <yaw>} {angepsilon <number>}
```

This tool can be used to perform a check on player's position and view angles. If position and view angles differ by more than given epsilon, the TAS script is restarted. The tool will restart until the number of automatic restarts surpasses the one defined by a `sar_tas_check_max_replays` console variable.

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

A tool-based alternative to console command part of a tickbulk. It will simply execute the command given as a parameter (whitespaces allowed).

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
stop
```

Stops all tools that were activated in the past tickbulks.

## Raw Scripts

Automation tools fetch data from the game in real-time in order to produce their inputs. However, Portal 2 uses an alternate ticks system, which results in user inputs being fetched twice before using them to process two ticks at once. This means that, when creating user inputs for a second tick in pair, we will have an outdated information about the game's state, preventing automation tools from creating accurate inputs.

The solution to this problem we've decided upon is to allow less legitimate input injection method for automation tools. However, in order to keep the legitimacy of our system, we've created a system for generating raw scripts.

Raw scripts are P2TAS scripts which do not contain any automation tools. When P2TAS script with automation tools is played back, a raw script is generated based on inputs from automation tools.

Raw scripts are usually identified by a `_raw` suffix in the script's file name. When it's present, TAS script player will ensure that no automation tools are being executed.

## RNG Manipulation

Portal 2 TASing is famously annoying due to randomness - both game-based and physics based. There are currently efforts of artificially manipulating RNG to resolve script playback into a single possibility. So far, there's a limited amount of RNG manipulation available, mostly related to gel spread.

SAR can generate files storing details of RNG state using console command `sar_rng_save`, which then can be used in a P2TAS script by referencing this file in the header of the script to reproduce exact same RNG state.

## Version History

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
