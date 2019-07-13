#pragma once
#pragma warning(suppress : 26495)
#include <cmath>
#include <cstdint>

#ifdef _WIN32
#define __funcc __thiscall
#else
#define __funcc __attribute__((__cdecl__))
#endif

struct Vector {
    float x, y, z;
    inline float Length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }
    inline float Length2D() const
    {
        return std::sqrt(x * x + y * y);
    }
    inline float Dot(const Vector& vOther) const
    {
        return Vector::DotProduct(*this, vOther);
    }
    inline Vector operator*(float fl)
    {
        Vector res;
        res.x = x * fl;
        res.y = y * fl;
        res.z = z * fl;
        return res;
    }
    inline Vector operator+(Vector vec)
    {
        Vector res;
        res.x = x + vec.x;
        res.y = y + vec.y;
        res.z = z + vec.z;
        return res;
    }
    inline float& operator[](int i)
    {
        return ((float*)this)[i];
    }
    inline float operator[](int i) const
    {
        return ((float*)this)[i];
    }
    static inline float DotProduct(const Vector& a, const Vector& b) 
    {
        return a.x*b.x + a.y*b.y + a.z*b.z; 
    }
};

struct QAngle {
    float x, y, z;
};

struct Color {
    Color()
    {
        *((int*)this) = 255;
    }
    Color(int _r, int _g, int _b)
    {
        SetColor(_r, _g, _b, 255);
    }
    Color(int _r, int _g, int _b, int _a)
    {
        SetColor(_r, _g, _b, _a);
    }
    void SetColor(int _r, int _g, int _b, int _a = 255)
    {
        _color[0] = (unsigned char)_r;
        _color[1] = (unsigned char)_g;
        _color[2] = (unsigned char)_b;
        _color[3] = (unsigned char)_a;
    }
    inline int r() const { return _color[0]; }
    inline int g() const { return _color[1]; }
    inline int b() const { return _color[2]; }
    inline int a() const { return _color[3]; }
    unsigned char _color[4] = { 0, 0, 0, 0 };
};

#define FCVAR_DEVELOPMENTONLY (1 << 1)
#define FCVAR_HIDDEN (1 << 4)
#define FCVAR_NEVER_AS_STRING (1 << 12)
#define FCVAR_CHEAT (1 << 14)

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

struct CCommand;
struct ConCommandBase;

typedef void (*FnChangeCallback_t)(void* var, const char* pOldValue, float flOldValue);

using _CommandCallback = void (*)(const CCommand& args);
using _CommandCompletionCallback = int (*)(const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);
using _InternalSetValue = void(__funcc*)(void* thisptr, const char* value);
using _InternalSetFloatValue = void(__funcc*)(void* thisptr, float value);
using _InternalSetIntValue = void(__funcc*)(void* thisptr, int value);
using _RegisterConCommand = void(__funcc*)(void* thisptr, ConCommandBase* pCommandBase);
using _UnregisterConCommand = void(__funcc*)(void* thisptr, ConCommandBase* pCommandBase);
using _FindCommandBase = void*(__funcc*)(void* thisptr, const char* name);
using _InstallGlobalChangeCallback = void(__funcc*)(void* thisptr, FnChangeCallback_t callback);
using _RemoveGlobalChangeCallback = void(__funcc*)(void* thisptr, FnChangeCallback_t callback);
using _AutoCompletionFunc = int(__funcc*)(void* thisptr, char const* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

class IConVar {
public:
    virtual void SetValue(const char* pValue) = 0;
    virtual void SetValue(float flValue) = 0;
    virtual void SetValue(int nValue) = 0;
    virtual void SetValue(Color value) = 0;
    virtual const char* GetName(void) const = 0;
    virtual const char* GetBaseName(void) const = 0;
    virtual bool IsFlagSet(int nFlag) const = 0;
    virtual int GetSplitScreenPlayerSlot() const = 0;
};

struct ConCommandBase {
    void* ConCommandBase_VTable; // 0
    ConCommandBase* m_pNext; // 4
    bool m_bRegistered; // 8
    const char* m_pszName; // 12
    const char* m_pszHelpString; // 16
    int m_nFlags; // 20

    ConCommandBase(const char* name, int flags, const char* helpstr)
        : ConCommandBase_VTable(nullptr)
        , m_pNext(nullptr)
        , m_bRegistered(false)
        , m_pszName(name)
        , m_pszHelpString(helpstr)
        , m_nFlags(flags)
    {
    }
};

struct CCommand {
    enum {
        COMMAND_MAX_ARGC = 64,
        COMMAND_MAX_LENGTH = 512
    };
    int m_nArgc;
    int m_nArgv0Size;
    char m_pArgSBuffer[COMMAND_MAX_LENGTH];
    char m_pArgvBuffer[COMMAND_MAX_LENGTH];
    const char* m_ppArgv[COMMAND_MAX_ARGC];

    int ArgC() const
    {
        return this->m_nArgc;
    }
    const char* Arg(int nIndex) const
    {
        return this->m_ppArgv[nIndex];
    }
    const char* operator[](int nIndex) const
    {
        return Arg(nIndex);
    }
};

struct ConCommand : ConCommandBase {
    union {
        void* m_fnCommandCallbackV1;
        _CommandCallback m_fnCommandCallback;
        void* m_pCommandCallback;
    };

    union {
        _CommandCompletionCallback m_fnCompletionCallback;
        void* m_pCommandCompletionCallback;
    };

    bool m_bHasCompletionCallback : 1;
    bool m_bUsingNewCommandCallback : 1;
    bool m_bUsingCommandCallbackInterface : 1;

    ConCommand(const char* pName, _CommandCallback callback, const char* pHelpString, int flags, _CommandCompletionCallback completionFunc)
        : ConCommandBase(pName, flags, pHelpString)
        , m_fnCommandCallback(callback)
        , m_fnCompletionCallback(completionFunc)
        , m_bHasCompletionCallback(completionFunc != nullptr)
        , m_bUsingNewCommandCallback(true)
        , m_bUsingCommandCallbackInterface(false)
    {
    }
};

struct ConVar : ConCommandBase {
    void* ConVar_VTable; // 24
    ConVar* m_pParent; // 28
    const char* m_pszDefaultValue; // 32
    char* m_pszString; // 36
    int m_StringLength; // 40
    float m_fValue; // 44
    int m_nValue; // 48
    bool m_bHasMin; // 52
    float m_fMinVal; // 56
    bool m_bHasMax; // 60
    float m_fMaxVal; // 64
    FnChangeCallback_t m_fnChangeCallback; // 68

    ConVar(const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max)
        : ConCommandBase(name, flags, helpstr)
        , ConVar_VTable(nullptr)
        , m_pParent(nullptr)
        , m_pszDefaultValue(value)
        , m_pszString(nullptr)
        , m_fValue(0.0f)
        , m_nValue(0)
        , m_bHasMin(hasmin)
        , m_fMinVal(min)
        , m_bHasMax(hasmax)
        , m_fMaxVal(max)
        , m_fnChangeCallback(nullptr)
    {
    }
};

template <class T, class I = int>
struct CUtlMemory {
    T* m_pMemory;
    int m_nAllocationCount;
    int m_nGrowSize;
};

template <class T, class A = CUtlMemory<T>>
struct CUtlVector {
    A m_Memory;
    int m_Size;
    T* m_pElements;
};

struct ConVar2 : ConCommandBase {
    void* ConVar_VTable; // 24
    ConVar2* m_pParent; // 28
    const char* m_pszDefaultValue; // 32
    char* m_pszString; // 36
    int m_StringLength; // 40
    float m_fValue; // 44
    int m_nValue; // 48
    bool m_bHasMin; // 52
    float m_fMinVal; // 56
    bool m_bHasMax; // 60
    float m_fMaxVal; // 64
    CUtlVector<FnChangeCallback_t> m_fnChangeCallback; // 68

    ConVar2(const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max)
        : ConCommandBase(name, flags, helpstr)
        , ConVar_VTable(nullptr)
        , m_pParent(nullptr)
        , m_pszDefaultValue(value)
        , m_pszString(nullptr)
        , m_fValue(0.0f)
        , m_nValue(0)
        , m_bHasMin(hasmin)
        , m_fMinVal(min)
        , m_bHasMax(hasmax)
        , m_fMaxVal(max)
        , m_fnChangeCallback()
    {
    }
};

#define SIGNONSTATE_NONE 0
#define SIGNONSTATE_CHALLENGE 1
#define SIGNONSTATE_CONNECTED 2
#define SIGNONSTATE_NEW 3
#define SIGNONSTATE_PRESPAWN 4
#define SIGNONSTATE_SPAWN 5
#define SIGNONSTATE_FULL 6
#define SIGNONSTATE_CHANGELEVEL 7

struct CUserCmd {
    void* VMT; // 0
    int command_number; // 4
    int tick_count; // 8
    QAngle viewangles; // 12, 16, 20
    float forwardmove; // 24
    float sidemove; // 28
    float upmove; // 32
    int buttons; // 36
    unsigned char impulse; // 40
    int weaponselect; // 44
    int weaponsubtype; // 48
    int random_seed; // 52
    short mousedx; // 56
    short mousedy; // 58
    bool hasbeenpredicted; // 60
};

#define GAMEMOVEMENT_JUMP_HEIGHT 21.0f

struct CMoveData {
    bool m_bFirstRunOfFunctions : 1; // 0
    bool m_bGameCodeMovedPlayer : 1; // 2
    void* m_nPlayerHandle; // 4
    int m_nImpulseCommand; // 8
    QAngle m_vecViewAngles; // 12, 16, 20
    QAngle m_vecAbsViewAngles; // 24, 28, 32
    int m_nButtons; // 36
    int m_nOldButtons; // 40
    float m_flForwardMove; // 44
    float m_flSideMove; // 48
    float m_flUpMove; // 52
    float m_flMaxSpeed; // 56
    float m_flClientMaxSpeed; // 60
    Vector m_vecVelocity; // 64, 68, 72
    QAngle m_vecAngles; // 76, 80, 84
    QAngle m_vecOldAngles; // 88, 92, 96
    float m_outStepHeight; // 100
    Vector m_outWishVel; // 104, 108, 112
    Vector m_outJumpVel; // 116, 120, 124
    Vector m_vecConstraintCenter; // 128, 132, 136
    float m_flConstraintRadius; // 140
    float m_flConstraintWidth; // 144
    float m_flConstraintSpeedFactor; // 148
    Vector m_vecAbsOrigin; // 152
};

class CHLMoveData : public CMoveData {
public:
    bool m_bIsSprinting;
};

#define IN_ATTACK (1 << 0)
#define IN_JUMP (1 << 1)
#define IN_DUCK (1 << 2)
#define IN_FORWARD (1 << 3)
#define IN_BACK (1 << 4)
#define IN_USE (1 << 5)
#define IN_MOVELEFT (1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2 (1 << 11)
#define IN_RELOAD (1 << 13)
#define IN_SPEED (1 << 17)

#define FL_ONGROUND (1 << 0)
#define FL_DUCKING (1 << 1)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)

#define WL_Feet 1
#define WL_Waist 2

#define MOVETYPE_NOCLIP 8
#define MOVETYPE_LADDER 9

typedef enum {
    HS_NEW_GAME = 0,
    HS_LOAD_GAME = 1,
    HS_CHANGE_LEVEL_SP = 2,
    HS_CHANGE_LEVEL_MP = 3,
    HS_RUN = 4,
    HS_GAME_SHUTDOWN = 5,
    HS_SHUTDOWN = 6,
    HS_RESTART = 7,

    INFRA_HS_NEW_GAME = 0,
    INFRA_HS_LOAD_GAME = 1,
    INFRA_HS_LOAD_GAME_WITHOUT_RESTART = 2,
    INFRA_HS_CHANGE_LEVEL_SP = 3,
    INFRA_HS_CHANGE_LEVEL_MP = 4,
    INFRA_HS_RUN = 5,
    INFRA_HS_GAME_SHUTDOWN = 6,
    INFRA_HS_SHUTDOWN = 7,
    INFRA_HS_RESTART = 8,
    INFRA_HS_RESTART_WITHOUT_RESTART = 9
} HOSTSTATES;

struct CHostState {
    int m_currentState; // 0
    int m_nextState; // 4
    Vector m_vecLocation; // 8, 12, 16
    QAngle m_angLocation; // 20, 24, 28
    char m_levelName[256]; // 32
    char m_landmarkName[256]; // 288
    char m_saveName[256]; // 544
    float m_flShortFrameTime; // 800
    bool m_activeGame; // 804
    bool m_bRememberLocation; // 805
    bool m_bBackgroundLevel; // 806
    bool m_bWaitingForConnection; // 807
};

#define INTERFACEVERSION_ISERVERPLUGINCALLBACKS "ISERVERPLUGINCALLBACKS002"

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
typedef void* (*InstantiateInterfaceFn)();

struct InterfaceReg {
    InstantiateInterfaceFn m_CreateFn;
    const char* m_pName;
    InterfaceReg* m_pNext;
    static InterfaceReg* s_pInterfaceRegs;

    InterfaceReg(InstantiateInterfaceFn fn, const char* pName)
        : m_pName(pName)
    {
        m_CreateFn = fn;
        m_pNext = s_pInterfaceRegs;
        s_pInterfaceRegs = this;
    }
};

class IServerPluginCallbacks {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) = 0;
    virtual void Unload() = 0;
    virtual void Pause() = 0;
    virtual void UnPause() = 0;
    virtual const char* GetPluginDescription() = 0;
    virtual void LevelInit(char const* pMapName) = 0;
    virtual void ServerActivate(void* pEdictList, int edictCount, int clientMax) = 0;
    virtual void GameFrame(bool simulating) = 0;
    virtual void LevelShutdown() = 0;
    virtual void ClientFullyConnect(void* pEdict) = 0;
    virtual void ClientActive(void* pEntity) = 0;
    virtual void ClientDisconnect(void* pEntity) = 0;
    virtual void ClientPutInServer(void* pEntity, char const* playername) = 0;
    virtual void SetCommandClient(int index) = 0;
    virtual void ClientSettingsChanged(void* pEdict) = 0;
    virtual int ClientConnect(bool* bAllowConnect, void* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) = 0;
    virtual int ClientCommand(void* pEntity, const void*& args) = 0;
    virtual int NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) = 0;
    virtual void OnQueryCvarValueFinished(int iCookie, void* pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) = 0;
    virtual void OnEdictAllocated(void* edict) = 0;
    virtual void OnEdictFreed(const void* edict) = 0;
};

struct CPlugin {
    char m_szName[128]; // 0
    bool m_bDisable; // 128
    IServerPluginCallbacks* m_pPlugin; // 132
    int m_iPluginInterfaceVersion; // 136
    void* m_pPluginModule; // 140
};

#define EXPOSE_INTERFACE_FN(functionName, interfaceName, versionName) \
    static InterfaceReg __g_Create##interfaceName##_reg(functionName, versionName);

#define EXPOSE_INTERFACE(className, interfaceName, versionName)                                           \
    static void* __Create##className##_interface() { return static_cast<interfaceName*>(new className); } \
    static InterfaceReg __g_Create##className##_reg(__Create##className##_interface, versionName);

#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName)                           \
    static void* __Create##className##interfaceName##_interface() { return static_cast<interfaceName*>(&globalVarName); } \
    static InterfaceReg __g_Create##className##interfaceName##_reg(__Create##className##interfaceName##_interface, versionName);

#define EXPOSE_SINGLE_INTERFACE(className, interfaceName, versionName) \
    static className __g_##className##_singleton;                      \
    EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)

struct CEventAction {
    const char* m_iTarget; // 0
    const char* m_iTargetInput; // 4
    const char* m_iParameter; // 8
    float m_flDelay; // 12
    int m_nTimesToFire; // 16
    int m_iIDStamp; // 20
    CEventAction* m_pNext; // 24
};

struct EventQueuePrioritizedEvent_t {
    float m_flFireTime; // 0
    char* m_iTarget; // 4
    char* m_iTargetInput; // 8
    int m_pActivator; // 12
    int m_pCaller; // 16
    int m_iOutputID; // 20
    int m_pEntTarget; // 24
    char m_VariantValue[20]; // 28
    EventQueuePrioritizedEvent_t* m_pNext; // 48
    EventQueuePrioritizedEvent_t* m_pPrev; // 52
};

struct CEventQueue {
    EventQueuePrioritizedEvent_t m_Events; // 0
    int m_iListCount; // 56
};

struct CEntInfo {
    void* m_pEntity; // 0
    int m_SerialNumber; // 4
    CEntInfo* m_pPrev; // 8
    CEntInfo* m_pNext; // 12
};

struct CEntInfo2 : CEntInfo {
    void* unk1; // 16
    void* unk2; // 20
};

typedef enum {
    DPT_Int = 0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,
    DPT_String,
    DPT_Array,
    DPT_DataTable,
    DPT_Int64,
    DPT_NUMSendPropTypes
} SendPropType;

struct SendProp;
struct RecvProp;
struct SendTable;

typedef void (*RecvVarProxyFn)(const void* pData, void* pStruct, void* pOut);
typedef void (*ArrayLengthRecvProxyFn)(void* pStruct, int objectID, int currentArrayLength);
typedef void (*DataTableRecvVarProxyFn)(const RecvProp* pProp, void** pOut, void* pData, int objectID);
typedef void (*SendVarProxyFn)(const SendProp* pProp, const void* pStructBase, const void* pData, void* pOut, int iElement, int objectID);
typedef int (*ArrayLengthSendProxyFn)(const void* pStruct, int objectID);
typedef void* (*SendTableProxyFn)(const SendProp* pProp, const void* pStructBase, const void* pData, void* pRecipients, int objectID);

struct RecvTable {
    RecvProp* m_pProps;
    int m_nProps;
    void* m_pDecoder;
    char* m_pNetTableName;
    bool m_bInitialized;
    bool m_bInMainList;
};

struct RecvProp {
    char* m_pVarName;
    SendPropType m_RecvType;
    int m_Flags;
    int m_StringBufferSize;
    bool m_bInsideArray;
    const void* m_pExtraData;
    RecvProp* m_pArrayProp;
    ArrayLengthRecvProxyFn m_ArrayLengthProxy;
    RecvVarProxyFn m_ProxyFn;
    DataTableRecvVarProxyFn m_DataTableProxyFn;
    RecvTable* m_pDataTable;
    int m_Offset;
    int m_ElementStride;
    int m_nElements;
    const char* m_pParentArrayPropName;
};

struct SendProp {
    void* VMT; // 0
    RecvProp* m_pMatchingRecvProp; // 4
    SendPropType m_Type; // 8
    int m_nBits; // 12
    float m_fLowValue; // 16
    float m_fHighValue; // 20
    SendProp* m_pArrayProp; // 24
    ArrayLengthSendProxyFn m_ArrayLengthProxy; // 28
    int m_nElements; // 32
    int m_ElementStride; // 36
    char* m_pExcludeDTName; // 40
    char* m_pParentArrayPropName; // 44
    char* m_pVarName; // 48
    float m_fHighLowMul; // 52
    int m_Flags; // 56
    SendVarProxyFn m_ProxyFn; // 60
    SendTableProxyFn m_DataTableProxyFn; // 64
    SendTable* m_pDataTable; // 68
    int m_Offset; // 72
    const void* m_pExtraData; // 76
};

struct SendProp2 {
    void* VMT; // 0
    RecvProp* m_pMatchingRecvProp; // 4
    SendPropType m_Type; // 8
    int m_nBits; // 12
    float m_fLowValue; // 16
    float m_fHighValue; // 20
    SendProp2* m_pArrayProp; // 24
    ArrayLengthSendProxyFn m_ArrayLengthProxy; // 28
    int m_nElements; // 32
    int m_ElementStride; // 36
    char* m_pExcludeDTName; // 40
    char* m_pParentArrayPropName; // 44
    char* m_pVarName; // 48
    float m_fHighLowMul; // 52
    char m_priority; // 56
    int m_Flags; // 60
    SendVarProxyFn m_ProxyFn; // 64
    SendTableProxyFn m_DataTableProxyFn; // 68
    SendTable* m_pDataTable; // 72
    int m_Offset; // 76
    const void* m_pExtraData; // 80
};

struct SendTable {
    SendProp* m_pProps;
    int m_nProps;
    char* m_pNetTableName;
    void* m_pPrecalc;
    bool m_bInitialized : 1;
    bool m_bHasBeenWritten : 1;
    bool m_bHasPropsEncodedAgainstCurrentTickCount : 1;
};

typedef void* (*CreateClientClassFn)(int entnum, int serialNum);
typedef void* (*CreateEventFn)();

struct ClientClass {
    CreateClientClassFn m_pCreateFn;
    CreateEventFn m_pCreateEventFn;
    char* m_pNetworkName;
    RecvTable* m_pRecvTable;
    ClientClass* m_pNext;
    int m_ClassID;
};

struct ServerClass {
    char* m_pNetworkName;
    SendTable* m_pTable;
    ServerClass* m_pNext;
    int m_ClassID;
    int m_InstanceBaselineIndex;
};

typedef enum _fieldtypes {
    FIELD_VOID = 0,
    FIELD_FLOAT,
    FIELD_STRING,
    FIELD_VECTOR,
    FIELD_QUATERNION,
    FIELD_INTEGER,
    FIELD_BOOLEAN,
    FIELD_SHORT,
    FIELD_CHARACTER,
    FIELD_COLOR32,
    FIELD_EMBEDDED,
    FIELD_CUSTOM,
    FIELD_CLASSPTR,
    FIELD_EHANDLE,
    FIELD_EDICT,
    FIELD_POSITION_VECTOR,
    FIELD_TIME,
    FIELD_TICK,
    FIELD_MODELNAME,
    FIELD_SOUNDNAME,
    FIELD_INPUT,
    FIELD_FUNCTION,
    FIELD_VMATRIX,
    FIELD_VMATRIX_WORLDSPACE,
    FIELD_MATRIX3X4_WORLDSPACE,
    FIELD_INTERVAL,
    FIELD_MODELINDEX,
    FIELD_MATERIALINDEX,
    FIELD_VECTOR2D,
    FIELD_TYPECOUNT
} fieldtype_t;

enum {
    TD_OFFSET_NORMAL = 0,
    TD_OFFSET_PACKED,
    TD_OFFSET_COUNT
};

struct inputdata_t;
typedef void (*inputfunc_t)(inputdata_t& data);

struct datamap_t;
struct typedescription_t {
    fieldtype_t fieldType; // 0
    const char* fieldName; // 4
    int fieldOffset[TD_OFFSET_COUNT]; // 8
    unsigned short fieldSize; // 16
    short flags; // 18
    const char* externalName; // 20
    void* pSaveRestoreOps; // 24
#ifndef _WIN32
    void* unk; // 28
#endif
    inputfunc_t inputFunc; // 28/32
    datamap_t* td; // 32/36
    int fieldSizeInBytes; // 36/40
    struct typedescription_t* override_field; // 40/44
    int override_count; // 44/48
    float fieldTolerance; // 48/52
};

struct datamap_t2;
struct typedescription_t2 {
    fieldtype_t fieldType; // 0
    const char* fieldName; // 4
    int fieldOffset; // 8
    unsigned short fieldSize; // 12
    short flags; // 14
    const char* externalName; // 16
    void* pSaveRestoreOps; // 20
#ifndef _WIN32
    void* unk1; // 24
#endif
    inputfunc_t inputFunc; // 24/28
    datamap_t2* td; // 28/32
    int fieldSizeInBytes; // 32/36
    struct typedescription_t2* override_field; // 36/40
    int override_count; // 40/44
    float fieldTolerance; // 44/48
    int flatOffset[TD_OFFSET_COUNT]; // 48/52
    unsigned short flatGroup; // 56/60
};

struct datamap_t {
    typedescription_t* dataDesc; // 0
    int dataNumFields; // 4
    char const* dataClassName; // 8
    datamap_t* baseMap; // 12
    bool chains_validated; // 16
    bool packed_offsets_computed; // 20
    int packed_size; // 24
};

struct datamap_t2 {
    typedescription_t2* dataDesc; // 0
    int dataNumFields; // 4
    char const* dataClassName; // 8
    datamap_t2* baseMap; // 12
    int m_nPackedSize; // 16
    void* m_pOptimizedDataMap; // 20
};

enum MapLoadType_t {
    MapLoad_NewGame = 0,
    MapLoad_LoadGame = 1,
    MapLoad_Transition = 2,
    MapLoad_Background = 3
};

#define FL_EDICT_FREE (1 << 1)

struct CBaseEdict {
    int m_fStateFlags; // 0
    int m_NetworkSerialNumber; // 4
    void* m_pNetworkable; // 8
    void* m_pUnk; // 12

    inline bool IsFree() const
    {
        return (m_fStateFlags & FL_EDICT_FREE) != 0;
    }
};

struct edict_t : CBaseEdict {
};

int ENTINDEX(edict_t* pEdict);
edict_t* INDEXENT(int iEdictNum);

struct CGlobalVarsBase {
    float realtime; // 0
    int framecount; // 4
    float absoluteframetime; // 8
    float curtime; // 12
    float frametime; // 16
    int maxClients; // 20
    int tickcount; // 24
    float interval_per_tick; // 28
    float interpolation_amount; // 32
    int simTicksThisFrame; // 36
    int network_protocol; // 40
    void* pSaveData; // 44
    bool m_bClient; // 48
    int nTimestampNetworkingBase; // 52
    int nTimestampRandomizeWindow; // 56
};

struct CGlobalVars : CGlobalVarsBase {
    char* mapname; // 60
    int mapversion; // 64
    char* startspot; // 68
    MapLoadType_t eLoadType; // 72
    char bMapLoadFailed; // 76
    char deathmatch; // 77
    char coop; // 78
    char teamplay; // 79
    int maxEntities; // 80
    int serverCount; // 84
    edict_t* pEdicts; // 88
};

enum JoystickAxis_t {
    JOY_AXIS_X = 0,
    JOY_AXIS_Y,
    JOY_AXIS_Z,
    JOY_AXIS_R,
    JOY_AXIS_U,
    JOY_AXIS_V,
    MAX_JOYSTICK_AXES,
};

typedef struct {
    unsigned int AxisFlags; // 0
    unsigned int AxisMap; // 4
    unsigned int ControlMap; // 8
} joy_axis_t;

struct CameraThirdData_t {
    float m_flPitch; // 0
    float m_flYaw; // 4
    float m_flDist; // 8
    float m_flLag; // 12
    Vector m_vecHullMin; // 16, 20, 24
    Vector m_vecHullMax; // 28, 32, 36
};

typedef unsigned long CRC32_t;

class CVerifiedUserCmd {
public:
    CUserCmd m_cmd;
    CRC32_t m_crc;
};

struct PerUserInput_t {
    float m_flAccumulatedMouseXMovement; // ?
    float m_flAccumulatedMouseYMovement; // ?
    float m_flPreviousMouseXPosition; // ?
    float m_flPreviousMouseYPosition; // ?
    float m_flRemainingJoystickSampleTime; // ?
    float m_flKeyboardSampleTime; // 12
    float m_flSpinFrameTime; // ?
    float m_flSpinRate; // ?
    float m_flLastYawAngle; // ?
    joy_axis_t m_rgAxes[MAX_JOYSTICK_AXES]; // ???
    bool m_fCameraInterceptingMouse; // ?
    bool m_fCameraInThirdPerson; // ?
    bool m_fCameraMovingWithMouse; // ?
    Vector m_vecCameraOffset; // 104, 108, 112
    bool m_fCameraDistanceMove; // 116
    int m_nCameraOldX; // 120
    int m_nCameraOldY; // 124
    int m_nCameraX; // 128
    int m_nCameraY; // 132
    bool m_CameraIsOrthographic; // 136
    QAngle m_angPreviousViewAngles; // 140, 144, 148
    QAngle m_angPreviousViewAnglesTilt; // 152, 156, 160
    float m_flLastForwardMove; // 164
    int m_nClearInputState; // 168
    CUserCmd* m_pCommands; // 172
    CVerifiedUserCmd* m_pVerifiedCommands; // 176
    unsigned long m_hSelectedWeapon; // 180 CHandle<C_BaseCombatWeapon>
    CameraThirdData_t* m_pCameraThirdData; // 184
    int m_nCamCommand; // 188
};

struct kbutton_t {
    struct Split_t {
        int down[2];
        int state;
    };

    Split_t& GetPerUser(int nSlot = -1);
    Split_t m_PerUser[2];
};

enum TOGGLE_STATE {
    TS_AT_TOP,
    TS_AT_BOTTOM,
    TS_GOING_UP,
    TS_GOING_DOWN
};

typedef enum {
    USE_OFF = 0,
    USE_ON = 1,
    USE_SET = 2,
    USE_TOGGLE = 3
} USE_TYPE;

class IGameEvent {
public:
    virtual ~IGameEvent() = default;
    virtual const char* GetName() const = 0;
    virtual bool IsReliable() const = 0;
    virtual bool IsLocal() const = 0;
    virtual bool IsEmpty(const char* key = 0) = 0;
    virtual bool GetBool(const char* key = 0, bool default_value = false) = 0;
    virtual int GetInt(const char* key = 0, int default_value = 0) = 0;
    virtual float GetFloat(const char* key = 0, float default_value = 0.0f) = 0;
    virtual const char* GetString(const char* key = 0, const char* default_value = "") = 0;
    virtual void SetBool(const char* key, bool value) = 0;
    virtual void SetInt(const char* key, int value) = 0;
    virtual void SetFloat(const char* key, float value) = 0;
    virtual void SetString(const char* key, const char* value) = 0;
};

class IGameEventListener2 {
public:
    virtual ~IGameEventListener2() = default;
    virtual void FireGameEvent(IGameEvent* event) = 0;
    virtual int GetEventDebugID() = 0;
};

static const char* EVENTS[] = {
    "player_spawn_blue",
    "player_spawn_orange"
};

struct cmdalias_t {
    cmdalias_t* next;
    char name[32];
    char* value;
};

struct GameOverlayActivated_t {
    uint8_t m_bActive;
};
