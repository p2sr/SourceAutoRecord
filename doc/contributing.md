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
  - [Game Support](#game-support)
    - [Versions](#versions)
    - [Unique Console Commands](#unique-console-commands)
  - [SDK](#sdk)
  - [Speedrun Timer](#speedrun-timer)
    - [Rules](#rules)
    - [Categories](#categories)

## Building

### Windows

- Visual Studio 2017 or 2019
- MSVC Toolset v141
- Configure SDK version in `src/SourceAutoRecord.vcxproj` or `src/SourceAutoRecord16.vcxproj`
- Configure paths in `copy.bat`

### Linux

- g++ 5.4.0
- g++-multilib
- Make 4.1
- Configure paths in `makefile`

## Pull Requests

- Write a meaningful title and a short description
- Follow the coding style
- Follow my requested changes
- Don't stage files that you had to configure
- Use latest `master` branch

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

All offsets are declared in the `Offsets` namespace. If multiple variables have the same name rename them with a prefix. For example: `C_m_vecAbsOrigin` means client-side and `S_m_vecAbsOrigin` server-side.

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
this->g_ClientDLL->Hook(Client::HudUpdate_Hook, Client::HudUpdate, Offsets::HudUpdate);
```

Macros resolve the calling convention automatically: `__cdecl` for Linux and `__thiscall` as `__fastcall` with unused `edx` register for Windows. Use `DETOUR_T` for custom return types and `DETOUR_STD` for `__stdcall`.

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

#### Buttons

Portal 2 Engine only.
```cpp
#define IN_AUTOSTRAFE (1 << 30) // Use a unique flag

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
