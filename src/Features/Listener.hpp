#pragma once
#include "Feature.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Variable.hpp"

// Portal 2 Engine only
#define CGameEventManager_m_Size 16
#define CGameEventManager_m_GameEvents 124
#define CGameEventManager_m_GameEvents_name 16
#define CGameEventDescriptor_Size 24

class Listener : public IGameEventListener2, public Feature {
private:
    bool m_bRegisteredForEvents;

public:
    Listener();
    void Init();
    void Shutdown();

    virtual ~Listener();
    virtual void FireGameEvent(IGameEvent* ev);
    virtual int GetEventDebugID();
};

extern Listener* listener;

extern Variable sar_debug_listener;

extern Command sar_dump_events;
