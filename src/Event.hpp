#pragma once

#include <functional>

#define _ON_INIT1(x) \
    static void _sar_init_fn_##x(); \
    static SarInitHandler _sar_init_##x(_sar_init_fn_##x); \
    static void _sar_init_fn_##x()
#define _ON_INIT(x) _ON_INIT1(x)
#define ON_INIT _ON_INIT(__COUNTER__)

#define _ON_EVENT1(ev, x, pri) \
    static void _sar_event_fn_##x(); \
    ON_INIT { Event::RegisterCallback(Event::ev, &_sar_event_fn_##x, pri); } \
    static void _sar_event_fn_##x()
#define _ON_EVENT(ev, x, pri) _ON_EVENT1(ev, x, pri)
#define ON_EVENT(ev) _ON_EVENT(ev, __COUNTER__, 0)
#define ON_EVENT_P(ev, pri) _ON_EVENT(ev, __COUNTER__, pri)

class SarInitHandler {
public:
    SarInitHandler(std::function<void()> cb);
    std::function<void()> cb;

    static std::vector<SarInitHandler *> handlers;
    static void RunAll();
};


namespace Event {
    enum EventType {
        SESSION_START,
        SAR_UNLOAD,
        DEMO_START,
        DEMO_STOP,
        TICK,
    };

    void Trigger(Event::EventType e);
    void RegisterCallback(Event::EventType e, std::function<void()> cb, int32_t priority = 0);
}
