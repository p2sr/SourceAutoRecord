#pragma once

#include <functional>

#define _ON_INIT1(x)                                    \
	static void _sar_init_fn_##x();                        \
	static SarInitHandler _sar_init_##x(_sar_init_fn_##x); \
	static void _sar_init_fn_##x()
#define _ON_INIT(x) _ON_INIT1(x)
#define ON_INIT _ON_INIT(__COUNTER__)

#define _ON_EVENT1(ev, x, pri)                                            \
	static void _sar_event_fn_##x(Event::EventData<Event::ev> event);        \
	ON_INIT { Event::RegisterCallback<Event::ev>(&_sar_event_fn_##x, pri); } \
	static void _sar_event_fn_##x(Event::EventData<Event::ev> event)
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
		SESSION_END,
		SAR_UNLOAD,
		DEMO_START,
		DEMO_STOP,
		PRE_TICK,
		POST_TICK,
		CM_FLAGS,
		PROCESS_MOVEMENT,
		COOP_RESET_DONE,
		COOP_RESET_REMOTE,
	};

	template <EventType E>
	struct EventData {};
	template <>
	struct EventData<PRE_TICK> {
		bool simulating;
		int tick;
	};
	template <>
	struct EventData<POST_TICK> {
		bool simulating;
		int tick;
	};
	template <>
	struct EventData<CM_FLAGS> {
		int slot;
		float time;
		bool end;
	};
	template <>
	struct EventData<PROCESS_MOVEMENT> {
		int slot;
		bool server;
	};

	template <EventType E>
	struct _EventReg {
		std::function<void(EventData<E>)> cb;
		int32_t priority;
	};

	template <EventType E>
	std::vector<_EventReg<E>> _g_eventCallbacks;

	template <EventType E>
	void Trigger(EventData<E> data) {
		for (auto &cb : _g_eventCallbacks<E>) {
			cb.cb(data);
		}
	}

	template <EventType E>
	void RegisterCallback(std::function<void(EventData<E>)> cb, int32_t priority = 0) {
		_EventReg<E> reg{cb, priority};

		auto &vec = _g_eventCallbacks<E>;

		auto vit = vec.begin();
		while (vit != vec.end()) {
			if (vit->priority <= priority) break;
			++vit;
		}

		vec.insert(vit, reg);
	}
}  // namespace Event
