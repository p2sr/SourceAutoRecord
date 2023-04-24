#pragma once

#include "Hook.hpp"
#include "Utils/Platform.hpp"

#include <functional>
#include <vector>


#define __SIGNAL_HOOK_FUNC_ARGS_0(name, typeA, typeB, typeC, typeD, typeE, typeF) \
	(void *thisptr, int edx) { \
		return name##.Call(thisptr); \
	}

#define __SIGNAL_HOOK_FUNC_ARGS_1(name, typeA, typeB, typeC, typeD, typeE, typeF) \
	(void *thisptr, int edx, typeA a) { \
		return name##.Call(thisptr, a); \
	}

#define __SIGNAL_HOOK_FUNC_ARGS_2(name, typeA, typeB, typeC, typeD, typeE, typeF) \
	(void *thisptr, int edx, typeA a, typeB b) { \
		return name##.Call(thisptr, a, b); \
	}

#define __SIGNAL_HOOK_FUNC_ARGS_3(name, typeA, typeB, typeC, typeD, typeE, typeF) \
	(void *thisptr, int edx, typeA a, typeB b, typeC c) { \
		return name##.Call(thisptr, a, b, c); \
	}

#define __SIGNAL_HOOK_FUNC_ARGS_4(name, typeA, typeB, typeC, typeD, typeE, typeF) \
	(void *thisptr, int edx, typeA a, typeB b, typeC c, typeD d) { \
		return name##.Call(thisptr, a, b, c, d); \
	}

#define __SIGNAL_HOOK_FUNC_ARGS_5(name, typeA, typeB, typeC, typeD, typeE, typeF) \
	(void *thisptr, int edx, typeA a, typeB b, typeC c, typeD d, typeE e) { \
		return name##.Call(thisptr, a, b, c, d, e); \
	}

#define __SIGNAL_HOOK_FUNC_ARGS_6(name, typeA, typeB, typeC, typeD, typeE, typeF)     \
	(void *thisptr, int edx, typeA a, typeB b, typeC c, typeD d, typeE e, typeF f) { \
		return name##.Call(thisptr, a, b, c, d, e, f);                            \
	}

#define __SIGNAL_EXPAND_THE_FUCK_OUT_OF_THIS_MSVC_BULLSHIT(x) x

#define _SIGNAL_HOOK(name) name##_sar_signal_hook

#define __SIGNAL_HOOK_FUNC_ARGS_N(returnType, name, typeA, typeB, typeC, typeD, typeE, typeF, N, ...) \
	static returnType __fastcall _SIGNAL_HOOK(name) __SIGNAL_HOOK_FUNC_ARGS_##N##(name, typeA, typeB, typeC, typeD, typeE, typeF)

#define _SIGNAL_HOOK_FUNC(returnType, name, ...) \
	__SIGNAL_EXPAND_THE_FUCK_OUT_OF_THIS_MSVC_BULLSHIT(__SIGNAL_HOOK_FUNC_ARGS_N(returnType, name, ##__VA_ARGS__##, 6, 5, 4, 3, 2, 1, 0))

#define DECL_SIGNAL_T(returnType, name, ...)                         \
	using name##Signal_t = Signal<returnType, ##__VA_ARGS__##>;           \
	using name##Listener_t = SignalListener<returnType, ##__VA_ARGS__##>; \
	using name##Return_t = returnType;                                \
	_SIGNAL_HOOK_FUNC(returnType, name, ##__VA_ARGS__##) \
	static name##Signal_t name;

#define DECL_SIGNAL(name, ...) DECL_SIGNAL_T(int, name, ##__VA_ARGS__##)

#define REGISTER_SIGNAL(name, instance, offset) \
	name##.Register(_SIGNAL_HOOK(name), instance, offset);

#define __SIGNAL_LISTENER(x, priority, signalname, ...) \
	static signalname##Return_t _sar_signal_listener_##x##_func(signalname##Signal_t *signal, void *thisptr, ##__VA_ARGS__##); \
	signalname##Listener_t _sar_signal_listener_##x##(&signalname, _sar_signal_listener_##x##_func, priority); \
	static signalname##Return_t _sar_signal_listener_##x##_func(signalname##Signal_t *signal, void *thisptr, ##__VA_ARGS__##)

#define _SIGNAL_LISTENER(x, priority, signalname, ...) __SIGNAL_LISTENER(x, priority, signalname, ##__VA_ARGS__##)

// for whatever reason, __COUNTER__ doesn't actually increment, so I'm just appending __LINE__ to it.
// this cursed sequence of macros is the only way that have worked for me and I'm not feeling like
// trying to figure out why. We're getting unique function names, that's what matters.
#define _SIGNAL_LISTENER_COUNTER() __COUNTER__
#define _SIGNAL_LISTENER_LINE() __LINE__
#define _SIGNAL_LISTENER_ID() _SIGNAL_LISTENER_COUNTER()##_SIGNAL_LISTENER_LINE()

#define SIGNAL_LISTENER(priority, signal, ...) _SIGNAL_LISTENER(_SIGNAL_LISTENER_ID(), priority, signal, ##__VA_ARGS__##)

struct SignalException : public std::exception {
	std::string msg;
	SignalException(std::string msg)
		: msg(msg) {
	}
	~SignalException() throw() {}
	const char *what() const throw() { return msg.c_str(); }
};


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
	Return operator()(void *thisptr, Args... args) { return func(signal, thisptr, args...); }
};

template <typename Return, typename... Args>
class Signal {
private:
	using SignalListenersDict = std::vector<SignalListener<Return, Args...>*>;
	SignalListenersDict listeners;
	typename SignalListenersDict::iterator currentListener;

	using SignalFunc = Return(__rescall *)(void *, Args...);
	SignalFunc originalFunc;

	Hook* hook;

	void *registeredInstance;
	int registeredIndex;
	bool registered;

public:
	Signal() {
		currentListener = listeners.begin();
		registered = false;
	}

	~Signal() {
		if (!registered) return;
		hook->Disable();
	}

	Return CallNext(void *thisptr, Args... args) {
		++currentListener;
		return Call(thisptr, args...);
	}

	Return Call(void *thisptr, Args... args) {
		if (!registered) {
			throw SignalException("Attempted to call an unregistered signal");
		}

		if (currentListener == listeners.end() || thisptr == nullptr) {
			currentListener = listeners.begin();
			return Original(thisptr, args...);
		} else {
			return (**currentListener)(thisptr, args...);
		}
	}
	Return Call(Args... args) {
		return Call(registeredInstance, args...);
	}

	Return Original(void *thisptr, Args... args) {
		if (!registered) {
			throw SignalException("Attempted to call original function of an unregistered signal");
		}
		hook->Disable();
		auto result = originalFunc(thisptr, args...);
		hook->Enable();
		return result;
	}
	Return Original(Args... args) { 
		return Original(registeredInstance, args...); 
	}

	void Register(void *hookFunc, void *instancePtr, int index) {
		this->hook = new Hook(hookFunc);
		this->originalFunc = Memory::VMT<SignalFunc>(instancePtr, index);
		hook->SetFunc(this->originalFunc);
		hook->Enable();

		currentListener = listeners.begin();

		registeredInstance = instancePtr;
		registeredIndex = index;
		registered = true;
	}

	void AddListener(SignalListener<Return, Args...> *listener) {
		auto i = listeners.begin();
		while (i != listeners.end()) {
			if ((*i)->priority <= listener->priority) break;
			++i;
		}
		listeners.insert(i, listener);
	}

	bool IsRegistered() { return registered; }

	Return operator()(Args... args) { return Call(args...); }
	Return operator()(void* thisptr, Args... args) { return Call(thisptr, args...); }
};