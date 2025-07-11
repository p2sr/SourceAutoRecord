#include "VPhysics.hpp"

#include "Event.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"


// Kinda a silly way to keep track of this, but it works
// and I'm lazy
static std::vector<int *> vphysEnvironments;
static bool lastWasFast = false;

// INFRA memory leak
// The game has a "fast load" feature that skips a bunch of stuff, including
// destroying the old physics environment. This leaks ~4MB (one env) per load.
// There are other leaks, but this is the most significant.
ON_EVENT(SESSION_START) {
	if (!sar.game->Is(SourceGame_INFRA)) return;
	vphysEnvironments = {};
	for (int i = 0; ; ++i) {
		int *env = vphysics->GetActivePhysicsEnvironmentByIndex(i);
		if (!env) break;
		vphysEnvironments.push_back(env);
	}

	switch (vphysEnvironments.size()) {
		default:
			lastWasFast = false;
			break;
		case 3:
			// Fast load, destroy one unused environment
			// The first env is unused, and the second one persists through
			// fast load(s). That means on the first fast load we'll destroy
			// the first environment, and subsequent fast loads we'll destroy
			// the second environment. (since it's now using the first one)
			int *env = vphysEnvironments[lastWasFast ? 1 : 0];
			vphysics->DestroyPhysicsEnvironment(env);
			lastWasFast = true;
			break;
	}
}

int *VPhysics::GetActivePhysicsEnvironmentByIndex(int index) {
	if (!this->GetActiveEnvironmentByIndex) return nullptr;
	return this->GetActiveEnvironmentByIndex(this->g_pVphysics->ThisPtr(), index);
}

void VPhysics::DestroyPhysicsEnvironment(int *env) {
	if (!this->DestroyEnvironment) return;
	this->DestroyEnvironment(this->g_pVphysics->ThisPtr(), env);
}

bool VPhysics::Init() {
	this->g_pVphysics = Interface::Create(this->Name(), "VPhysics031", false);

	if (Offsets::GetActiveEnvironmentByIndex) {
		GetActiveEnvironmentByIndex = g_pVphysics->Original<_GetActiveEnvironmentByIndex>(Offsets::GetActiveEnvironmentByIndex);
	}
	if (Offsets::DestroyEnvironment) {
		DestroyEnvironment = g_pVphysics->Original<_DestroyEnvironment>(Offsets::DestroyEnvironment);
	}

	return this->hasLoaded = this->g_pVphysics;
}

void VPhysics::Shutdown() {
	Interface::Delete(this->g_pVphysics);
}

VPhysics *vphysics;
