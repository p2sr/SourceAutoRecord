# SAR: Contributing Guide

## Overview

- [Building](#building)
  - [Windows](#windows)
  - [Linux](#linux)
- [Pull Requests](#pull-requests)
- [Coding Style](#coding-style)
- [Coding](#coding)
  - [Interfaces](#interfaces)
  - [Offsets](#offsets)
  - [Hooking](#hooking)
  - [Features](#features)
  - [Memory Utils](#memory-utils)
    - [Reading Pointer Paths](#reading-pointer-paths)
    - [Reading Function Addresses](#reading-function-addresses)
    - [External Imports](#external-imports)
    - [Access Virtual Function](#access-virtual-function)
    - [Signature-Scanning aka AOB-Scan](#signature-scanning-aka-aob-scan)
    - [Relative to Absolute Address](#relative-to-absolute-address)
  - [Console Commands](#console-commands)
    - [Variables](#variables)
    - [Commands](#commands)
    - [Buttons](#buttons)
  - [HUD](#hud)
    - [Elements](#elements)
	- [Separate](#separate)
  - [Game Support](#game-support)
    - [Versions](#versions)
    - [Unique Console Commands](#unique-console-commands)
  - [SDK](#sdk)
  - [Speedrun Timer](#speedrun-timer)
    - [Rules](#rules)
    - [Categories](#categories)

## Building

### Windows

- Visual Studio 2019
- MSVC Toolset v142
- Configure SDK version in `src/SourceAutoRecord.vcxproj` or `src/SourceAutoRecord16.vcxproj`
- Configure paths in `copy.bat`

### Linux

- g++ 8.3.0
- g++-8-multilib
- Make 4.1
- Configure paths in `makefile`

## Pull Requests

- Write a meaningful title and a short description
- Follow the [coding style](#coding-style)
- Follow the requested changes
- DO NOT stage files that you had to configure
- Use latest `master` branch

### Quick Tutorial with git

- Fork this repository on GitHub
- git clone https://github.com/<your_account>/SourceAutoRecord
- git remote add upstream https://github.com/NeKzor/SourceAutoRecord
- git fetch remotes/upstream/master
- git checkout -b feature/something remotes/upstream/master
- *Change stuff and stage files*
- git commit -m "New something"
- git push origin feature/something

### Merge existing branch (optionally)

PRs will be squashed in the end anyway.

- git checkout -b feature/something remotes/upstream/master
- git merge --squash some-branch
- *Resolve merge conflicts*
- git commit -m "New something"
- git push origin feature/something

## Coding Style

Mostly follows [Webkit Style Guide](https://webkit.org/code-style-guidelines) with some exceptions:

- PascalCaseForClassesNamespacesStructsAndFunctions
- camelCaseForPropertiesAndVariables
- _LeadingUnderscoreForTypeAliases

A `.clang-format` file is included. I'd highly recommend using an extension:

- [ClangFormat](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat) for Visual Studio
- [Clang-Format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format) for Visual Studio Code

Note: You'll sometimes see a mix with Valve's coding style.

## Coding

SAR was designed to be able to load for multiple Source Engine games on Windows and Linux.

### Interfaces

Engine interfaces can easily be obtained and hooked:

```cpp
this->g_ClientDLL = Interface::Create(this->Name(), "VClient0");
```

SAR will resolve the interface version automatically. Use `VClient0` instead of `VClient001` etc.

Note: Modules implement `Module::Name` which returns their module name. For example: `client.so` for Linux and `client.dll` for Windows. There's also the `MODULE` macro that helps you with that.

### Offsets

An offset can be the number of bytes from one address to another, the index of a virtual function or the size of an object.

Where you need them:

- Finding pointers and static functions
- Accessing properties and pointers of objects:

```cpp
struct CEntInfo {
    void* m_pEntity; // 0
    int m_SerialNumber; // 4
    CEntInfo* m_pPrev; // 8
    CEntInfo* m_pNext; // 12
};
```

- Calling and hooking virtual functions:

```cpp
class IServerPluginCallbacks {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) = 0; // 0
    virtual void Unload() = 0; // 1
    virtual void Pause() = 0; // 2
    virtual void UnPause() = 0; // 3
    ...
```

It is recommended to use offsets instead of SDK classes as they can be different for every game since SAR's philosophy is to support as many games as possible. The number of offsets to get to an object should be kept as low as possible. Why use offsets and not signatures aka patterns? Because they are much faster during initialization and development. All offsets are declared in the `Offsets` namespace. If multiple variables have the same name rename them with a prefix. For example: `C_m_vecAbsOrigin` means client-side and `S_m_vecAbsOrigin` server-side.

Read [Game Support](#game-support) for initialization.

### Hooking

Functions will be hooked through virtual method tables (VMT). SAR provides useful macros for declaration:

```cpp
// Client.hpp
DECL_DETOUR(CreateMove, float flInputSampleTime, CUserCmd* cmd)

// Client.cpp
REDECL(Client::CreateMove);

// int __cdecl Client::CreateMove_Hook(void* thisptr, float flInputSampleTime, CUserCmd* cmd)
DETOUR(Client::CreateMove, float flInputSampleTime, CUserCmd* cmd)
{
    // Always call/return original function/value unless you know what you're doing
    return Client::CreateMove(thisptr, flInputSampleTime, cmd);
}

// Somewhere in Client::Init
this->g_ClientDLL->Hook(Client::CreateMove_Hook, Client::CreateMove, Offsets::CreateMove);
```

Calling conventions will automatically be resolved using the `__rescall` macro. On Linux it will be `__cdecl` and on Windows `__thiscall`. Hooks on Windows will be declared as `__fastcall` with unused `edx` register. Use `DETOUR_T` for custom return types and `DETOUR_STD` for `__stdcall`.

### Features

Simple example for adding a new SAR feature in OOP style.

```cpp
// src/Features/MyFeature.hpp
#include "Feature.hpp"

class MyFeature : public Feature {
private:
    int state;

public:
    MyFeature();
    void ChangeState(int newState);
    int GetState();
};

extern MyFeature* myFeature;

// src/Features/MyFeature.cpp
#include "MyFeature.hpp"

MyFeature* myFeature;

MyFeature::MyFeature()
    : state(0)
{
    this->hasLoaded = true;
}
void MyFeature::ChangeState(int newState)
{
    this->state = newState;
}
int MyFeature::GetState()
{
    return this->state;
}

// src/Features.hpp
#include "Features/MyFeature.hpp"

// src/SAR.cpp
// SAR::Load
this->features->AddFeature<MyFeature>(&myFeature);
```

### Memory Utils

#### Reading Pointer Paths

```cpp
// Address of original function
auto GetButtonBits = g_Input->Original(Offsets::GetButtonBits);

// Reads in_jump pointer from address + some offset
Memory::Deref(GetButtonBits + Offsets::in_jump, &this->in_jump);
```

#### Reading Function Addresses

```cpp
// Address of original function
auto JoyStickApplyMovement = g_Input->Original(Offsets::JoyStickApplyMovement);

// Reads function address from address + some offset
Memory::Read(JoyStickApplyMovement + Offsets::KeyDown, &this->KeyDown);
Memory::Read(JoyStickApplyMovement + Offsets::KeyUp, &this->KeyUp);
```

#### External Imports

```cpp
auto tier0 = Memory::GetModuleHandleByName("libtier0.so");
auto Msg = Memory::GetSymbolAddress<_Msg>(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ");

Memory::CloseModuleHandle(tier0);
```

#### Access Virtual Function

```cpp
auto IsCommand = Memory::VMT<bool (*)(void*)>(cmd, Offsets::IsCommand));
```

#### Signature-Scanning aka AOB-Scan

Only use this in search-dumps or tests.

```cpp
uintptr_t firstResult = Memory::Scan(MODULE("engine"), "55 8B EC 0F 57 C0 81 EC ? ? ? ", 178);

std::vector<uintptr_t> allResults = Memory::MultiScan(engine->Name(), TRACE_SHUTDOWN_PATTERN, TRACE_SHUTDOWN_OFFSET1);

// Multiple patterns with different offsets
PATTERN(DATAMAP_PATTERN1, "B8 ? ? ? ? C7 05", 11, 1);
PATTERN(DATAMAP_PATTERN2, "C7 05 ? ? ? ? ? ? ? ? B8", 6, 11);

PATTERNS(DATAMAP_PATTERNS, &DATAMAP_PATTERN1, &DATAMAP_PATTERN2);

auto result = Memory::MultiScan(moduleName, &DATAMAP_PATTERNS);
```

#### Relative to Absolute Address

Only use this in tests.

```cpp
auto funcAddress = Memory::Absolute(MODULE("engine"), 0xdeadbeef);
```

### Console Commands

#### Variables

```cpp
// Boolean
Variable sar_simple_mode("sar_simple_mode", "0",
    "Useful help description.\n");
// Float
Variable sar_mode("sar_mode", "0", 0
    "Useful help description.\n");
// String
Variable sar_text("sar_text", "a string",
    "Useful help description.\n", 0);

// From the engine
auto sv_cheats = Variable("sv_cheats");
if (sv_cheats.GetBool()) {
    // Stop cheating
    sv_cheats.SetValue(0);
}
```

Note: Keep a static version of a variable if it can be accessed more than once.

#### Commands

Commands should always return a useful message if something went wrong.

```cpp
CON_COMMAND(sar_hello, "Useful help description.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("Please enter a string!\n");
    }

    console->Print("Hello %s!\n", args[1]);
}
```

#### Autocompletion

```cpp
#include "Command.hpp"

// Fastest way to declare a hidden autocompletion function
// Last argument is type of std::vector<std::string>. It is required to wrap it with ()
CON_COMMAND_COMPLETION(sar_force_fov, "Description.\n", ({ "0", "50", "60", "70", "80", "90", "100", "110", "120", "130", "140" }))
{
	// Command callback
}

// Use this macro in order to call some initialization logic
DECL_COMMAND_COMPLETION(sar_workshop)
{
	// Init some stuff
    if (workshop->maps.empty()) {
        workshop->Update();
    }

	// Basic filtering logic
    for (auto& map : workshop->maps) {
        if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
            break;
        }

        if (std::strlen(match) != std::strlen(cmd)) {
            if (std::strstr(map.c_str(), match)) {
                items.push_back(map);
            }
        } else {
            items.push_back(map);
        }
    }

    FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(sar_workshop, "Description.\n", 0, sar_workshop_CompletionFunc)
{
	// Command callback
}
```

### HUD

#### Elements

HUD elements can be declared with just a few lines of code. All elements are grouped together and start with `sar_hud_`. They also share the same settings starting with `sar_hud_default_`. The order of all elments can be customized by the user but the default order has to be declared separately.

```cpp
#include "Features/Hud/Hud.hpp"

// Called if: sar_hud_frame 1
HUD_ELEMENT(frame, "0", "Default example.\n", HudType_InGame | HudType_Paused)
{
    ctx->DrawElement("frame: %i", session->currentFrame);
}

// Called if: sar_hud_some_mode > 0
HUD_ELEMENT_MODE(some_mode, "0", 0, 5, "Mode example.\n", HudType_InGame | HudType_Paused)
{
	if (mode == 4) {
		ctx->DrawElement("mode: 4");
	} else {
		ctx->DrawElement("mode: 1-3 or 5");
	}
}

// Called if: sar_hud_some_text[0] != '\0' (not empty)
HUD_ELEMENT_STRING(some_text, "", 0, 5, "Text example.\n", HudType_InGame | HudType_Paused)
{
    ctx->DrawElement("mode: %s", text);
}

// Splitscreen support needs a 2 at the end of the macro
HUD_ELEMENT2(splitscreen, "0", "Slot example.\n", HudType_InGame | HudType_Paused)
{
	// Do something with slot
	auto slot = ctx->slot;
}

// Limit an element for a specific game
HUD_ELEMENT3(game_version, "0", "Game specific example.\n",
	HudType_InGame | HudType_Paused, // Where to draw
	false,							 // no splitscreens
	SourceGame_Portal)				 // Portal only
{
}
```

Last step is to add the element name to the ordered list. It is used for autocompletion and allows users to manually script their HUD order.

```
// Features/Hud/Hud.cpp
std::vector<std::string> elementOrder = {
	// ...
	"frame",
	"some_mode",
	"some_text",
	"splitscreen",
	"game_version"
};
```

### Separate

A more complete HUD with separate settings can be declared manually if needed.

```cpp
// Features/Hud/MyHud.hpp
#include "Hud.hpp"

#include "Variable.hpp"

class MyCustomHud : public Hud {
public:
    MyCustomHud();
    bool ShouldDraw() override;
    void Paint(int slot) override;
    bool GetCurrentSize(int& xSize, int& ySize) override;
};

extern MyCustomHud myHud;

extern Variable sar_my_hud;
extern Variable sar_my_hud_x;
extern Variable sar_my_hud_y;
extern Variable sar_my_hud_font_color;
extern Variable sar_my_hud_font_index;
```

```cpp
// Features/Hud/MyHud.cpp
#include "MyHud.hpp"

#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"

#include "Variable.hpp"

Variable sar_my_hud("sar_sr_hud", "0", 0, "Draws my HUD.\n");
Variable sar_my_hud_x("sar_sr_hud_x", "0", 0, "X offset of my HUD.\n");
Variable sar_my_hud_y("sar_sr_hud_y", "100", 0, "Y offset of my HUD.\n");
Variable sar_my_hud_font_color("sar_sr_hud_font_color", "255 255 255 255", "RGBA font color of my HUD.\n", 0);
Variable sar_my_hud_font_index("sar_sr_hud_font_index", "70", 0, "Font index of my HUD.\n");

MyHud myHud;

MyHud::MyHud()
    : Hud(HudType_InGame,         // Only when session is running (no-pauses)
		false,                    // Do not draw for splitscreen (default)
		SourceGame_Portal2Engine) // Support specific game verison (default is for every game)
{
}

// Implement a more complex drawing logic if needed
bool MyHud::ShouldDraw()
{
	// Calling the base function will resolve the HUD type condition
    return sar_my_hud.GetBool() && Hud::ShouldDraw();
}

// Will be called if ShoulDraw allows it
// The slot value is the current splitscreen index which will always
// be 0 if we do not want splitscreens or if the game does not support them
void MyHud::Paint(int slot)
{
    auto xOffset = sar_my_hud_x.GetInt();
    auto yOffset = sar_my_hud_y.GetInt();

    auto font = scheme->GetDefaultFont() + sar_my_hud_font_index.GetInt();
    auto fontColor = this->GetColor(sar_my_hud_font_color.GetString());

    surface->DrawTxt(font, xOffset, yOffset, fontColor, "%s", "hi :)");
}

// Useful for commands that need the exact position
// See Feature/Hud/InputHud.cpp
bool MyHud::GetCurrentSize(int& xSize, int& ySize)
{
	// Calc size and return value if hud is active
    return false;
}
```

#### Buttons

Portal 2 Engine only.
```cpp
#define IN_AUTOSTRAFE (1 << 31) // Make sure to use a unique flag

kbutton_t in_autostrafe;

void IN_AutoStrafeDown(const CCommand& args) { client->KeyDown(&in_autostrafe, (args.ArgC() > 1) ? args[1] : nullptr); }
void IN_AutoStrafeUp(const CCommand& args) { client->KeyUp(&in_autostrafe, (args.ArgC() > 1) ? args[1] : nullptr); }

Command startautostrafe("+autostrafe", IN_AutoStrafeDown, "Auto-strafe button.\n");
Command endautostrafe("-autostrafe", IN_AutoStrafeUp, "Auto-strafe button.\n");

// Client.cpp
// Client::GetButtonBits
client->CalcButtonBits(GET_SLOT(), bits, IN_AUTOSTRAFE, 0, &in_autostrafe, bResetState);
```

### Game Support

Since offsets can be different for every engine or platform you have to define them in game classes. SAR only supports Portal 2 engine and Half-Life 2 engine (SteamPipe and Source Unpack). Games that are based on these engines can easily derive offsets from them. Look into `src/Games` folder for examples.

#### Versions

```cpp
if (sar.game->Is(SourceGame_Portal2)) {
    // Only runs when game is Portal 2
}
```

#### Unique Console Commands

```cpp
// Only works in The Stanley Parable
sar_hello.UniqueFor(SourceGame_TheStanleyParable);
```

### SDK

A minimal Source Engine SDK can be found in `src/Utils` folder.

### Speedrun Timer

#### Rules

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

#### Categories

```cpp
#include "Features/Speedrun/TimerCategory.hpp"

SAR_CATEGORY(ApertureTag,                       // Name of game or mod
    RTA,                                        // Name of category
    _Rules({ &out_of_shower, &end_credits }));  // List of rules
```
