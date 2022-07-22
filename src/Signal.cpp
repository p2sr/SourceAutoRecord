#include "Signal.hpp"


template <typename Return, typename... Args>
SignalListener<Return, Args...>::SignalListener(Signal<Return, Args...> *signal, std::function<Return(Signal<Return, Args...> *, void*, Args...)> func, int priority)
	: func(func)
	, signal(signal)
	, priority(priority)
{
	signal->AddListener(this);
}


template <typename Return, typename... Args>
Signal<Return, Args...>::Signal() {
	currentListener = listeners.begin();

	hookFunc = [&](void* thisptr, Args... args) -> Return {
		return this->Call(thisptr, args);
	};
}

template <typename Return, typename... Args>
Return Signal<Return, Args...>::CallNext(void* thisptr, Args... args) {
	++currentListener;
	return Call(thisptr, args);
}

template <typename Return, typename... Args>
Return Signal<Return, Args...>::Call(void* thisptr, Args... args) {
	Return result;

	if (currentListener == listeners.end() || registeredInterface == nullptr) {
		result = originalFunc(registeredInterface, args);
		currentListener = listeners.begin();
	} else {
		result = currentListener(this, registeredInterface, args);
	}

	return result;
}

template <typename Return, typename... Args>
Return Signal<Return, Args...>::Call(Args... args) {
	return Call(registeredInterface, args);
}

template <typename Return, typename... Args>
void Signal<Return, Args...>::Register(Interface *interf, int index) {
	interf->Hook(hookFunc, &originalFunc, index);
	currentListener = listeners.begin();

	registeredInterface = interf;
	registeredIndex = index;
}

template <typename Return, typename... Args>
void Signal<Return, Args...>::AddListener(SignalListener<Return, Args...> *listener) {
	auto i = listeners.begin();
	while (i != listeners.end()) {
		if (i->priority <= listener->priority) break;
		++i;
	}
	listeners.insert(i, listener);
}