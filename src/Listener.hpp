#pragma once
#include "Modules/Engine.hpp"

#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Timer/Timer.hpp"

#include "Utils.hpp"

// Portal 2 Engine only
#define CGameEventManager_m_Size 16
#define CGameEventManager_m_GameEvents 124
#define CGameEventManager_m_GameEvents_name 16
#define CGameEventDescriptor_Size 24

class Listener : public IGameEventListener2 {
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

Listener::Listener()
    : m_bRegisteredForEvents(false)
{
}
void Listener::Init()
{
    if (engine->AddListener) {
        for (const auto& event : EVENTS) {
            auto result = engine->AddListener(engine->s_GameEventManager->ThisPtr(), this, event, true);
            if (result) {
                //console->DevMsg("SAR: Added event listener for %s!\n", event);
            } else {
                console->DevWarning("SAR: Failed to add event listener for %s!\n", event);
            }
        }
    }
}
void Listener::Shutdown()
{
    if (engine->RemoveListener) {
        engine->RemoveListener(engine->s_GameEventManager->ThisPtr(), this);
    }
}
Listener::~Listener()
{
    this->Shutdown();
}
void Listener::FireGameEvent(IGameEvent* ev)
{
    if (!ev)
        return;

    if (sar_debug_game_events.GetBool()) {
        console->Print("[%i] Event fired: %s\n", engine->GetSessionTick(), ev->GetName());
        if (engine->ConPrintEvent) {
#ifdef _WIN32
            engine->ConPrintEvent(ev);
#else
            engine->ConPrintEvent(engine->s_GameEventManager->ThisPtr(), ev);
#endif
        }
    }

    if (engine->GetMaxClients() >= 2) {
        // TODO: Start when orange spawns?
        if (!std::strcmp(ev->GetName(), "player_spawn_orange")) {
            console->Print("Detected cooperative spawn!\n");
            session->Rebase(*engine->tickcount);
            timer->Rebase(*engine->tickcount);
            speedrun->Unpause(engine->tickcount);
        }
    }
}
int Listener::GetEventDebugID()
{
    return 42;
}
void Listener::DumpGameEvents()
{
    if (!engine->s_GameEventManager) {
        return;
    }

    auto s_GameEventManager = reinterpret_cast<uintptr_t>(engine->s_GameEventManager->ThisPtr());
    auto m_Size = *reinterpret_cast<int*>(s_GameEventManager + CGameEventManager_m_Size);
    console->Print("m_Size = %i\n", m_Size);
    if (m_Size > 0) {
        auto m_GameEvents = *(uintptr_t*)(s_GameEventManager + CGameEventManager_m_GameEvents);
        for (int i = 0; i < m_Size; i++) {
            auto name = *(char**)(m_GameEvents + CGameEventDescriptor_Size * i + CGameEventManager_m_GameEvents);
            console->Print("%s\n", name);
        }
    }
}

Listener* listener;
extern Listener* listener;
