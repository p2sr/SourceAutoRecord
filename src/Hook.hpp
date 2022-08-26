#pragma once

#include "Utils/Memory.hpp"
#include "Utils/Platform.hpp"

#include <cstring>

class Hook {
public:
	template <typename T = void *>
	Hook(T hook)
		: func(nullptr)
		, hook((void *)hook)
		, enabled(false) {
		Hook::GetHooks().push_back(this);
	}

	~Hook() {}

	template <typename T = void *>
	void SetFunc(T func, bool enable = true) {
		this->func = (void *)func;
		Memory::UnProtect(this->func, 5);
		if (enable) this->Enable();
	}

	void Enable() {
		if (this->locked) return;
		if (this->enabled) return;
		if (!this->func || !this->hook) return;
		memcpy(this->origCode, this->func, sizeof this->origCode);
		uint8_t *ptr = (uint8_t *)this->func;
		ptr[0] = 0xE9;  // JMP
		*(uint32_t *)(ptr + 1) = (uintptr_t)this->hook - ((uintptr_t)ptr + 5);
		this->enabled = true;
	}

	void Disable(bool lock = false) {
		if (lock) this->locked = true;
		if (!this->enabled) return;
		if (!this->func || !this->hook) return;
		memcpy(this->func, this->origCode, sizeof this->origCode);
		this->enabled = false;
	}

	static void DisableAll() {
		for (Hook *h : Hook::GetHooks()) {
			h->Disable(true);
		}
	}

	static std::vector<Hook*> &GetHooks() {
		static std::vector<Hook *> hooks;
		return hooks;
	}

private:
	void *func;
	void *hook;
	bool enabled;
	bool locked;
	uint8_t origCode[5];
};
