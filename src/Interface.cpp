#include "Interface.hpp"

#include "Modules/Console.hpp"
#include "Utils/Memory.hpp"
#include "Utils/SDK.hpp"

#include <cstring>

Interface::Interface()
	: baseclass(nullptr)
	, vtable(nullptr)
	, vtableSize(0)
	, isHooked(false)
	, copy(nullptr) {
}
Interface::Interface(void *baseclass, bool copyVtable, bool autoHook)
	: Interface() {
	this->baseclass = reinterpret_cast<uintptr_t **>(baseclass);
	this->vtable = *this->baseclass;

	while (this->vtable[this->vtableSize]) {
		++this->vtableSize;
	}

	if (copyVtable) {
		this->CopyVtable();
		if (autoHook) {
			this->EnableHooks();
		}
	}
}
Interface::~Interface() {
	this->DisableHooks();
	if (this->copy) {
		this->copy.reset();
	}
}
void Interface::CopyVtable() {
	if (!this->copy) {
		this->copy = std::make_unique<uintptr_t[]>(this->vtableSize + 1);
		std::memcpy(this->copy.get(), this->vtable - 1, sizeof(uintptr_t) + this->vtableSize * sizeof(uintptr_t));
	}
}
void Interface::EnableHooks() {
	if (!this->isHooked) {
		*this->baseclass = this->copy.get() + 1;
		this->isHooked = true;
	}
}
void Interface::DisableHooks() {
	if (this->isHooked) {
		*this->baseclass = this->vtable;
		this->isHooked = false;
	}
}
bool Interface::Unhook(int index) {
	if (index >= 0 && index < this->vtableSize) {
		this->copy[index + 1] = this->vtable[index];
		return true;
	}
	return false;
}
Interface *Interface::Create(void *ptr, bool copyVtable, bool autoHook) {
	return (ptr) ? new Interface(ptr, copyVtable, autoHook) : nullptr;
}
Interface *Interface::Create(const char *filename, const char *interfaceSymbol, bool copyVtable, bool autoHook) {
	auto ptr = Interface::GetPtr(filename, interfaceSymbol);
	return (ptr) ? new Interface(ptr, copyVtable, autoHook) : nullptr;
}
void Interface::Delete(Interface *ptr) {
	if (ptr) {
		delete ptr;
		ptr = nullptr;
	}
}
void *Interface::GetPtr(const char *filename, const char *interfaceSymbol) {
	auto handle = Memory::GetModuleHandleByName(filename);
	if (!handle) {
		console->DevWarning("SAR: Failed to open module %s!\n", filename);
		return nullptr;
	}

	auto CreateInterface = Memory::GetSymbolAddress<CreateInterfaceFn>(handle, "CreateInterface");
	Memory::CloseModuleHandle(handle);

	if (!CreateInterface) {
		console->DevWarning("SAR: Failed to find symbol CreateInterface for %s!\n", filename);
		return nullptr;
	}

	int ret;
	void *fn = CreateInterface(interfaceSymbol, &ret);

	if (ret) {
		console->DevWarning("SAR: Failed to find interface with symbol %s in %s!\n", interfaceSymbol, filename);
		return nullptr;
	}
	return fn;
}
