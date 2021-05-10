#include "Event.hpp"

#include <map>

std::vector<SarInitHandler *> SarInitHandler::handlers;

SarInitHandler::SarInitHandler(std::function<void()> cb)
    : cb(cb)
    { handlers.push_back(this); }

void SarInitHandler::RunAll() {
    for (auto h : SarInitHandler::handlers) {
        h->cb();
    }
}

struct EventReg {
    std::function<void()> cb;
    int32_t priority;
};

static std::map<Event::EventType, std::vector<EventReg>> _g_callbacks;

void Event::Trigger(Event::EventType e) {
    auto it = _g_callbacks.find(e);
    if (it != _g_callbacks.end()) {
        for (auto &cb : it->second) {
            cb.cb();
        }
    }
}

void Event::RegisterCallback(Event::EventType e, std::function<void()> cb, int32_t priority) {
    EventReg reg{ cb, priority };

    auto mit = _g_callbacks.find(e);
    if (mit == _g_callbacks.end()) {
        _g_callbacks[e] = { reg };
        return;
    }

    auto vec = mit->second;

    auto vit = vec.begin();
    while (vit != vec.end()) {
        if (vit->priority <= priority) break;
        ++vit;
    }

    vec.insert(vit, reg);
}
