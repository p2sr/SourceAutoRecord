#pragma once
#include <cstdint>

#include "Module.hpp"

#include "Interface.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

class Client : public Module {
private:
    Interface* g_ClientDLL = nullptr;
    Interface* g_pClientMode = nullptr;
    Interface* g_HUDChallengeStats = nullptr;
    Interface* s_EntityList = nullptr;

public:
    using _GetClientEntity = void*(__func*)(void* thisptr, int entnum);
    using _KeyDown = int(__cdecl*)(void* b, const char* c);
    using _KeyUp = int(__cdecl*)(void* b, const char* c);
    using _GetAllClasses = ClientClass* (*)();

    _GetClientEntity GetClientEntity = nullptr;
    _KeyDown KeyDown = nullptr;
    _KeyUp KeyUp = nullptr;
    _GetAllClasses GetAllClasses = nullptr;

    void* in_jump = nullptr;

public:
    void* GetPlayer();
    Vector GetAbsOrigin();
    QAngle GetAbsAngles();
    Vector GetLocalVelocity();
    Vector GetViewOffset();
    void GetOffset(const char* className, const char* propName, int& offset);

private:
    int16_t FindOffset(RecvTable* table, const char* propName);

public:
    // CHLClient::HudUpdate
    DECL_DETOUR(HudUpdate, unsigned int a2)

    // ClientModeShared::CreateMove
    DECL_DETOUR(CreateMove, float flInputSampleTime, CUserCmd* cmd)

    // CHud::GetName
    DECL_DETOUR_T(const char*, GetName)

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("client"); }
};

extern Client* client;

extern Variable cl_showpos;
extern Variable ui_loadingscreen_transition_time;
extern Variable hide_gun_when_holding;
