#pragma once
#include "Feature.hpp"

#include "Utils/SDK.hpp"

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
    void DumpGameEvents();
};

extern Listener* listener;
