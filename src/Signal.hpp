#pragma once

#include "Interface.hpp"

#include <functional>
#include <vector>

#define DECL_SIGNAL(returnType, name, ...) \
	using name##Signal_t = Signal<returnType, __VA_ARGS__>; \
	using name##Listener_t = SignalListener<returnType, __VA_ARGS__>; \
	using name##Return_t = returnType; \
	static name##Signal_t name;
#define REGISTER_SIGNAL(name, interf, offset) name##->Register(interf, offset);

#define _SIGNAL_LISTENER(x, priority, signalname, ...) \
	static signalname##Return_t _sar_signal_listener_##x##_func(signalname##Signal_t *signal, void *thisptr, __VA_ARGS__); \
	signalname##Listener_t _sar_signal_listener_##x##(&signalname, _sar_signal_listener_##x##_func, priority); \
	static signalname##Return_t _sar_signal_listener_##x##_func(signalname##Signal_t *signal, void *thisptr, __VA_ARGS__)

#define SIGNAL_LISTENER(priority, signal, ...) _SIGNAL_LISTENER(__COUNTER__, priority, signal, __VA_ARGS__)


template <typename Return, typename... Args>
class Signal;

template <typename Return, typename... Args>
class SignalListener {
private:
	Signal<Return, Args...> *signal;
	std::function<Return(Signal *, void *, Args...)> func;
	int priority;
public:
	SignalListener(Signal<Return, Args...> *signal, std::function<Return(Signal<Return, Args...> *, void *, Args...)> func, int priority);
	Return operator()(void *thisptr, Args... args) { func(signal, thisptr, args); }
};

template <typename Return, typename... Args>
class Signal {
private:
	using SignalListenersDict = std::vector<SignalListener<Return, Args...>*>;
	SignalListenersDict listeners;
	SignalListenersDict::iterator currentListener;

	using SignalFunc = Return(__rescall*)(void*, Args...);
	SignalFunc originalFunc;
	SignalFunc hookFunc;

	Interface *registeredInterface;
	int registeredIndex;
public:
	Signal();
	Return CallNext(void *thisptr, Args... args);
	Return Call(void *thisptr, Args... args);
	Return Call(Args... args);
	void Register(Interface *interf, int index);
	void AddListener(SignalListener<Return, Args...> *listener);

	Return Original(Args... args) { return originalFunc(args); }
	Return operator(Args... args) { return Call(args); }
};