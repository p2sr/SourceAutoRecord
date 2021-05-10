#pragma once

#include <functional>

#define _ON_INIT1(x) \
    static void _sar_init_fn_##x(); \
    static SarInitHandler _sar_init_##x(_sar_init_fn_##x); \
    static void _sar_init_fn_##x()
#define _ON_INIT(x) _ON_INIT1(x)
#define ON_INIT _ON_INIT(__COUNTER__)

class SarInitHandler {
public:
    SarInitHandler(std::function<void()> cb);
    std::function<void()> cb;

    static std::vector<SarInitHandler *> handlers;
    static void RunAll();
};

enum class Event {
    SESSION_START,
};

namespace Events {
    void Trigger(Event e);
    void RegisterCallback(Event e, std::function<void()> cb, int32_t priority = 0);
}
