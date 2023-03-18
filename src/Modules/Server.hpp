#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"
#include "Command.hpp"

#ifdef _WIN32
#	define AirMove_Mid_Offset 679
#	define AirMove_Signature "F3 0F 10 50 40"
#	define AirMove_Continue_Offset 5
#	define AirMove_Skip_Offset 142
#endif

class Server : public Module {
public:
	Interface *g_GameMovement = nullptr;
	Interface *g_ServerGameDLL = nullptr;
	Interface *gEntList = nullptr;

	using _UTIL_PlayerByIndex = ServerEnt *(__cdecl *)(int index);
	using _GetAllServerClasses = ServerClass *(*)();
	using _IsRestoring = bool (*)();
	using _TraceFirePortal = int(__rescall *)(uintptr_t pgunptr, Vector &vOrigin, Vector &Direction, bool portalToPlace, int ePlacedBy, TracePortalPlacementInfo_t &placementInfo);
	using _FindPortal = uintptr_t (*)(char linkage, bool secondaryPortal, bool createIfNotFound);

#ifdef _WIN32
	using _AcceptInput = bool(__rescall *)(void *thisptr, const char *inputName, void *activator, void *caller, variant_t value, int outputID);
#else
	using _AcceptInput = bool(__rescall *)(void *thisptr, const char *inputName, void *activator, void *caller, variant_t value, int outputID);
#endif

	using _CreateEntityByName = void *(__rescall *)(void *, const char *);
	using _DispatchSpawn = void(__rescall *)(void *, void *);
	using _SetKeyValueChar = bool(__rescall *)(void *, void *, const char *, const char *);
	using _SetKeyValueFloat = bool(__rescall *)(void *, void *, const char *, float);
	using _SetKeyValueVector = bool(__rescall *)(void *, void *, const char *, const Vector &);

	_UTIL_PlayerByIndex UTIL_PlayerByIndex = nullptr;
	_GetAllServerClasses GetAllServerClasses = nullptr;
	_IsRestoring IsRestoring = nullptr;
	_CreateEntityByName CreateEntityByName = nullptr;
	_DispatchSpawn DispatchSpawn = nullptr;
	_SetKeyValueChar SetKeyValueChar = nullptr;
	_SetKeyValueFloat SetKeyValueFloat = nullptr;
	_SetKeyValueVector SetKeyValueVector = nullptr;
	_AcceptInput AcceptInput = nullptr;
	_TraceFirePortal TraceFirePortal = nullptr;
	_FindPortal FindPortal = nullptr;

	CGlobalVars *gpGlobals = nullptr;
	CEntInfo *m_EntPtrArray = nullptr;

	int tickCount = 0;

private:
	bool jumpedLastTime = false;
	float savedVerticalVelocity = 0.0f;
	bool callFromCheckJumpButton = false;

public:
	DECL_M(GetPortals, int);
	DECL_M(GetAbsOrigin, Vector);
	DECL_M(GetAbsAngles, QAngle);
	DECL_M(GetLocalVelocity, Vector);
	DECL_M(GetFlags, int);
	DECL_M(GetEFlags, int);
	DECL_M(GetMaxSpeed, float);
	DECL_M(GetGravity, float);
	DECL_M(GetViewOffset, Vector);
	DECL_M(GetPortalLocal, CPortalPlayerLocalData);
	DECL_M(GetEntityName, char *);
	DECL_M(GetEntityClassName, char *);
	DECL_M(GetPlayerState, CPlayerState);

	ServerEnt *GetPlayer(int index);
	bool IsPlayer(void *entity);
	bool AllowsMovementChanges();
	int GetSplitScreenPlayerSlot(void *entity);
	void KillEntity(void *entity);
	float GetCMTimer();

public:
	// CPortal_Player::PlayerRunCommand
	DECL_DETOUR(PlayerRunCommand, CUserCmd *cmd, void *moveHelper);

	// CBasePlayer::ViewPunch
	DECL_DETOUR_T(void, ViewPunch, const QAngle &offset);

	// CServerNetworkProperty::IsInPVS
	DECL_DETOUR_T(bool, IsInPVS, void *info);

	// CGameMovement::ProcessMovement
	DECL_DETOUR(ProcessMovement, void *pPlayer, CMoveData *pMove);

	// CGameMovement::ProcessMovement
	DECL_DETOUR_T(Vector *, GetPlayerViewOffset, bool ducked);

	DECL_DETOUR(StartTouchChallengeNode, void *entity);

	// CGameMovement::CheckJumpButton
	DECL_DETOUR_T(bool, CheckJumpButton);

	static _CheckJumpButton CheckJumpButtonBase;

	// CGameMovement::PlayerMove
	DECL_DETOUR(PlayerMove);

	// CGameMovement::FinishGravity
	DECL_DETOUR(FinishGravity);

	// CGameMovement::AirMove
	DECL_DETOUR_B(AirMove);

	float *aircontrol_fling_speed_addr;

	// CServerGameDLL::GameFrame
	DECL_DETOUR(GameFrame, bool simulating);

	// CServerGameDLL::ApplyGameSettings
	DECL_DETOUR(ApplyGameSettings, KeyValues *pKV);

	// CGlobalEntityList::OnRemoveEntity
	DECL_DETOUR_T(void, OnRemoveEntity, IHandleEntity *ent, CBaseHandle handle);

	DECL_DETOUR_COMMAND(say);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("server"); }
};

extern Server *server;

extern Variable sv_cheats;
extern Variable sv_footsteps;
extern Variable sv_alternateticks;
extern Variable sv_bonus_challenge;
extern Variable sv_accelerate;
extern Variable sv_airaccelerate;
extern Variable sv_paintairacceleration;
extern Variable sv_friction;
extern Variable sv_maxspeed;
extern Variable sv_stopspeed;
extern Variable sv_maxvelocity;
extern Variable sv_gravity;
