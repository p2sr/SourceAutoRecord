# SAR: Speedrun Standard (S3)

A new way to time your speedrun!

## Features

- Tick-perfect timing with rule system
- Support for categories
- Automatic start, stop and split
- Menu timing
- LiveSplit support

## Support

Game|Windows|Linux
---|:-:|:-:
Portal 2|✔|✔
Aperture Tag|✔|✔
Portal Stories: Mel|✔|✔
The Stanley Parable|✖|✖
The Beginners Guide|✖|✖
Half-Life 2|✖|✖
Portal|✔|✔

## Adding Game Support

### Rules

```cpp
#include "Features/Speedrun/TimerRule.hpp"

SAR_RULE3(moon_shot,        // Name of the rule
    "sp_a4_finale4",        // Name of the map
    "moon_portal_detector", // Name of the entity
    SearchMode::Names)      // Search in entity list by name
{
    // Access property
    auto portalCount = reinterpret_cast<int*>((uintptr_t)entity + 1337);

    if (*portalCount != 0) {
        return TimerAction::End; // Timer ends on this tick
    }

    return TimerAction::DoNothing; // Continue running
}
```

Note: Pointers of entities will be cached when the server has loaded. Make sure that the entity lives long enough to get any valid states. This also means that entities which get created at a later time cannot be accessed.

### Categories

```cpp
#include "Features/Speedrun/TimerCategory.hpp"

SAR_CATEGORY(ApertureTag,                       // Name of game or mod
    RTA,                                        // Name of category
    _Rules({ &out_of_shower, &end_credits }));  // List of rules
```

## Interface

SAR provides an interface for accessing the timer externally which can be used with [LiveSplit](https://livesplit.github.com).
Here's a generic [ASL Script](https://gist.github.com/NeKzor/6db7ca6a28ed55fbcce7d8af7edf0f18) as an example.

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

### HUD

- sar_sr_hud
- sar_sr_hud_x
- sar_sr_hud_y
- sar_sr_hud_font_color
- sar_sr_hud_font_index
