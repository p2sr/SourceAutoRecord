#pragma once
#include "Module.hpp"

#include "Cheats.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#ifdef _WIN32
#define AirMove_Mid_Offset 679
#define AirMove_Signature "F3 0F 10 50 40"
#define AirMove_Continue_Offset 5
#define AirMove_Skip_Offset 142
#endif

class Server : public Module {
public:
    Interface* g_GameMovement;
    Interface* g_ServerGameDLL;

    using _UTIL_PlayerByIndex = void*(__cdecl*)(int index);
    _UTIL_PlayerByIndex UTIL_PlayerByIndex;

    CGlobalVars* gpGlobals;
    bool* g_InRestore;
    CEventQueue* g_EventQueue;

public:
    void* GetPlayer();
    int GetPortals();

    bool jumpedLastTime = false;
    bool callFromCheckJumpButton = false;

    // CGameMovement::CheckJumpButton
    DECL_DETOUR_T(bool, CheckJumpButton)

    // CGameMovement::PlayerMove
    DECL_DETOUR(PlayerMove)

    // CGameMovement::FinishGravity
    DECL_DETOUR(FinishGravity)

    // CGameMovement::AirMove
    DECL_DETOUR_B(AirMove)

#ifdef _WIN32
    // CGameMovement::AirMove
    static uintptr_t AirMove_Skip;
    static uintptr_t AirMove_Continue;
    DECL_DETOUR_MID_MH(AirMove_Mid)
#endif

// CServerGameDLL::GameFrame
#ifdef _WIN32
    DECL_DETOUR_STD(GameFrame, bool simulating)
#else
    DECL_DETOUR(GameFrame, bool simulating)
#endif

    bool Init() override;
    void Shutdown() override;
};

extern Server* server;
