#include "Events.hpp"

#include <map>

struct EventReg {
    std::function<void()> cb;
    int32_t priority;
};

static std::map<Event, std::vector<EventReg>> _g_callbacks;

void Events::Trigger(Event e) {
    auto it = _g_callbacks.find(e);
    if (it != _g_callbacks.end()) {
        for (auto &cb : it->second) {
            cb.cb();
        }
    }
}

void Events::RegisterCallback(Event e, std::function<void()> cb, int32_t priority) {
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
