#include "Hook.hpp"

std::vector<Hook *> Hook::hooks;

void Hook::DisableAll() {
	for (Hook *h : Hook::hooks) {
		h->Disable();
	}
}
