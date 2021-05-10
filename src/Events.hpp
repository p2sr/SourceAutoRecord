#pragma once

#include <functional>

enum class Event {
    SESSION_START,
};

namespace Events {
    void Trigger(Event e);
    void RegisterCallback(Event e, std::function<void()> cb, int32_t priority = 0);
}
