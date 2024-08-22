#include "FovChanger.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Variable.hpp"

FovChanger *fovChanger;

FovChanger::FovChanger()
	: defaultFov(0),
	viewmodelFov(0) {
	this->hasLoaded = true;
}
void FovChanger::SetFov(const int fov) {
	this->defaultFov = fov;
	this->Force();
}
void FovChanger::SetViewmodelFov(const float fov) {
	this->viewmodelFov = fov;
	this->Force();
}
void FovChanger::Force() {
	if (this->defaultFov != 0) {
		if (!sv_cheats.GetBool() && (this->defaultFov < 45 || this->defaultFov > 140)) {
			this->defaultFov = 0;
		} else {
			if (cl_fov.GetInt() != this->defaultFov) {
				cl_fov.SetValue(this->defaultFov);
			}
		}
	}
	if (this->viewmodelFov != 0 && cl_viewmodelfov.GetFloat() != this->viewmodelFov) {
		cl_viewmodelfov.SetValue(this->viewmodelFov);
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

	if (!sv_cheats.GetBool() && (fov < 45 || fov > 140)) {
		return console->Print("FOV value has to be between 45 to 140!\n");
	}

	fovChanger->SetFov(fov);
	console->Print("Enabled forcing FOV: %i\n", fov);
}

CON_COMMAND(sar_force_viewmodel_fov, "sar_force_viewmodel_fov <fov> - forces viewmodel FOV\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_force_viewmodel_fov.ThisPtr()->m_pszHelpString);
	}

	auto fov = (float)std::atof(args[1]);
	if (fov == 0) {
		fovChanger->SetViewmodelFov(fov);
		return console->Print("Disabled forcing viewmodel FOV!\n");
	}

	fovChanger->SetViewmodelFov(fov);
	console->Print("Enabled forcing viewmodel FOV: %f\n", fov);
}
