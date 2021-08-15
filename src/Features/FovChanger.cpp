#include "FovChanger.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"

FovChanger *fovChanger;

FovChanger::FovChanger()
	: defaultFov(0) {
	this->hasLoaded = true;
}
void FovChanger::SetFov(const int fov) {
	this->defaultFov = fov;
	this->Force();
}
void FovChanger::Force() {
	if (this->defaultFov != 0) {
		cl_fov.SetValue(this->defaultFov);
	}
}

ON_EVENT(SESSION_START) {
	fovChanger->needToUpdate = true;
}

ON_EVENT(PRE_TICK) {
	if (engine->demoplayer->IsPlaying()) return;
	if (fovChanger->needToUpdate) {
		fovChanger->Force();
	}
}

// Commands

CON_COMMAND_COMPLETION(sar_force_fov, "sar_force_fov <fov> - forces player FOV\n", ({"0", "50", "60", "70", "80", "90", "100", "110", "120", "130", "140"})) {
	if (args.ArgC() != 2) {
		return console->Print(sar_force_fov.ThisPtr()->m_pszHelpString);
	}

	auto fov = std::atoi(args[1]);
	if (fov == 0) {
		fovChanger->SetFov(fov);
		return console->Print("Disabled forcing FOV!\n");
	}

	if (fov < 45 || fov > 140) {
		return console->Print("FOV value has to be between 45 to 140!\n");
	}

	fovChanger->SetFov(fov);
	console->Print("Enabled forcing FOV: %i\n", fov);
}
