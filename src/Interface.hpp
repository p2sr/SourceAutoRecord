#pragma once
#include "Utils/Memory.hpp"

class Interface {
public:
	uintptr_t **baseclass;
	uintptr_t *vtable;
	int vtableSize;

private:
	bool isHooked;
	std::unique_ptr<uintptr_t[]> copy;

public:
	Interface();
	Interface(void *baseclass, bool copyVtable = true, bool autoHook = true);
	~Interface();

	void CopyVtable();
	void EnableHooks();
	void DisableHooks();

	template <typename T = uintptr_t>
	T Original(int index, bool readJmp = false) {
		if (readJmp) {
			auto source = this->vtable[index] + 1;
			auto rel = *reinterpret_cast<uintptr_t *>(source);
			return (T)(source + rel + sizeof(rel));
		}
		return (T)this->vtable[index];
	}
	template <typename T = uintptr_t>
	T Hooked(int index) {
		return (T)this->copy[index + 1];
	}
	template <typename T = uintptr_t>
	T Current(int index) {
		return (T)(*this->baseclass)[index];
	}
	template <typename T = uintptr_t, typename U = void *>
	bool Hook(T detour, U &original, int index) {
		if (index >= 0 && index < this->vtableSize) {
			this->copy[index + 1] = reinterpret_cast<uintptr_t>(detour);
			original = this->Original<U>(index);
			return true;
		}
		return false;
	}

	bool Unhook(int index);

	inline void *ThisPtr() {
		return reinterpret_cast<void *>(this->baseclass);
	}

	static Interface *Create(void *ptr, bool copyVtable = true, bool autoHook = true);
	static Interface *Create(const char *filename, const char *interfaceSymbol, bool copyVtable = true, bool autoHook = true);
	static void Delete(Interface *ptr);
	static void *GetPtr(const char *filename, const char *interfaceSymbol);

	template <typename T = void *>
	static T Get(const char *filename, const char *interfaceSymbol) {
		return (T)Interface::GetPtr(filename, interfaceSymbol);
	}
};
