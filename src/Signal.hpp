#pragma once

#include "Interface.hpp"
#include "Utils/Platform.hpp"

#include <functional>
#include <vector>

#define DECL_SIGNAL(returnType, name, ...) \
	using name##Signal_t = Signal<returnType, __VA_ARGS__>; \
	using name##Listener_t = SignalListener<returnType, __VA_ARGS__>; \
	using name##Return_t = returnType; \
	static name##Signal_t name;
#define REGISTER_SIGNAL(name, interf, offset) name##->Register(interf, offset);

#define __SIGNAL_LISTENER(x, priority, signalname, ...) \
	static signalname##Return_t _sar_signal_listener_##x##_func(signalname##Signal_t *signal, void *thisptr, __VA_ARGS__); \
	signalname##Listener_t _sar_signal_listener_##x##(&signalname, _sar_signal_listener_##x##_func, priority); \
	static signalname##Return_t _sar_signal_listener_##x##_func(signalname##Signal_t *signal, void *thisptr, __VA_ARGS__)

#define _SIGNAL_LISTENER(x, priority, signalname, ...) __SIGNAL_LISTENER(x, priority, signalname, __VA_ARGS__)

#define SIGNAL_LISTENER(priority, signal, ...) _SIGNAL_LISTENER(##__COUNTER__##, priority, signal, __VA_ARGS__)


template <typename Return, typename... Args>
class Signal;

template <typename Return, typename... Args>
class SignalListener {
private:
	using SignalListenerFunc = Return(Signal<Return, Args...> *, void *, Args...);

	Signal<Return, Args...> *signal;
	std::function<SignalListenerFunc> func;
	int priority;

private:
	friend class Signal<Return, Args...>;

public:
	SignalListener(Signal<Return, Args...> *signal, std::function<SignalListenerFunc> func, int priority)
		: func(func)
		, signal(signal)
		, priority(priority) {
		signal->AddListener(this);
	}
	Return operator()(void *thisptr, Args... args) { func(signal, thisptr, args...); }
};

template <typename Return, typename... Args>
class Signal {
private:
	using SignalListenersDict = std::vector<SignalListener<Return, Args...>*>;
	SignalListenersDict listeners;
	typename SignalListenersDict::iterator currentListener;

	Return(__rescall *originalFunc)(void *, Args...);
	std::function<Return(void *, Args...)> hookFunc;

	Interface *registeredInterface;
	int registeredIndex;

public:
	Signal() {
		currentListener = listeners.begin();

		hookFunc = [&](void *thisptr, Args... args) -> Return {
			return this->Call(thisptr, args...);
		};
	}
	Return CallNext(void *thisptr, Args... args) {
		++currentListener;
		return Call(thisptr, args...);
	}

	Return Call(void *thisptr, Args... args) {
		if (currentListener == listeners.end() || thisptr == nullptr) {
			currentListener = listeners.begin();
			return originalFunc(thisptr, args...);
		} else {
			SignalListener<Return, Args...> *listener = *currentListener;
			// listener(thisptr, args...);  -- bro why my epic function operator overload no work
			return listener->func(this, thisptr, args...);
		}
	}
	Return Call(Args... args) {
		return Call(registeredInterface, args...);
	}
	void Register(Interface *interf, int index) {
		interf->Hook(hookFunc, originalFunc, index);
		currentListener = listeners.begin();

		registeredInterface = interf;
		registeredIndex = index;
	}
	void AddListener(SignalListener<Return, Args...> *listener) {
		auto i = listeners.begin();
		while (i != listeners.end()) {
			if ((*i)->priority <= listener->priority) break;
			++i;
		}
		listeners.insert(i, listener);
	}

	Return Original(Args... args) { return originalFunc(args...); }
	Return operator()(Args... args) { return Call(args...); }
};