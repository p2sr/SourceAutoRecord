#pragma once
#include "Module.hpp"

#include "Cheats.hpp"
#include "Interface.hpp"
#include "Utils.hpp"

class Client : public Module {
private:
    Interface* g_ClientDLL;
    Interface* g_pClientMode;
    Interface* g_HUDChallengeStats;
    Interface* s_EntityList;

public:
    using _GetClientEntity = void*(__func*)(void* thisptr, int entnum);
    using _KeyDown = int(__cdecl*)(void* b, const char* c);
    using _KeyUp = int(__cdecl*)(void* b, const char* c);

    _GetClientEntity GetClientEntity;
    _KeyDown KeyDown;
    _KeyUp KeyUp;

    void* in_jump;

public:
    void* GetPlayer();
    Vector GetAbsOrigin();
    QAngle GetAbsAngles();
    Vector GetLocalVelocity();
    int GetFlags();

    // CHLClient::HudUpdate
    DECL_DETOUR(HudUpdate, unsigned int a2)

    // ClientModeShared::CreateMove
    DECL_DETOUR(CreateMove, float flInputSampleTime, CUserCmd* cmd)

    // CHud::GetName
    DECL_DETOUR_T(const char*, GetName)

    bool Init() override;
    void Shutdown() override;
};

extern Client* client;
