#include "VPhysics.hpp"

#include "Event.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"


// Kinda a silly way to keep track of this, but it works
// and I'm lazy
static std::vector<int *> vphysEnvironments;
static std::vector<int *> vphysEnvironmentsLast;

// INFRA memory leak
// The game has a "fast load" feature that skips a bunch of stuff, including
// destroying the old physics environment. This leaks ~4MB (one env) per load.
// Slow loads destroy the two environments that are in use, and create a new
// two. We detect this and destroy the old environments that are no longer in
// use. There are other leaks, but this is the most significant.
ON_EVENT(SESSION_START) {
	if (!sar.game->Is(SourceGame_INFRA)) return;
	vphysEnvironmentsLast = vphysEnvironments;
	vphysEnvironments = {};
	for (int i = 0; ; ++i) {
		int *env = vphysics->GetActivePhysicsEnvironmentByIndex(i);
		if (!env) break;
		vphysEnvironments.push_back(env);
	}

	// console->Print("%d active physics environment(s)\n", (int)vphysEnvironments.size());
	// int i = 0;
	// for (const auto &env : vphysEnvironmentsLast) {
	// 	if (std::find(vphysEnvironments.begin(), vphysEnvironments.end(), env) == vphysEnvironments.end()) {
	// 		console->ColorMsg(Color(255,96,64), "- %d %p\n", i, env);
	// 	}
	// 	i++;
	// }
	// i = 0;
	// for (const auto &env : vphysEnvironments) {
	// 	if (std::find(vphysEnvironmentsLast.begin(), vphysEnvironmentsLast.end(), env) == vphysEnvironmentsLast.end()) {
	// 		console->ColorMsg(Color(96,255,64), "+ %d %p\n", i, env);
	// 	} else {
	// 		console->Print("  %d %p\n", i, env);
	// 	}
	// 	i++;
	// }

	int newEnvs = 0;
	for (const auto &env : vphysEnvironments) {
		if (std::find(vphysEnvironmentsLast.begin(), vphysEnvironmentsLast.end(), env) == vphysEnvironmentsLast.end()) {
			newEnvs++;
		}
	}
	if (newEnvs == 2) {
		for (const auto &env : vphysEnvironments) {
			if (std::find(vphysEnvironmentsLast.begin(), vphysEnvironmentsLast.end(), env) != vphysEnvironmentsLast.end()) {
				vphysics->DestroyPhysicsEnvironment(env);
			}
		}
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
