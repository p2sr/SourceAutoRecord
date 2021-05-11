#include "FovChanger.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Session.hpp"

#include "Command.hpp"
#include "Variable.hpp"
#include "Event.hpp"

FovChanger* fovChanger;

FovChanger::FovChanger()
    : defaultFov(0)
{
    this->hasLoaded = true;
}
void FovChanger::SetFov(const int fov)
{
    this->defaultFov = fov;
    this->Force();
}
void FovChanger::Force()
{
    if (this->defaultFov != 0) {
        cl_fov.SetValue(this->defaultFov);
    }
}

ON_EVENT(TICK) {
    if (engine->demoplayer->IsPlaying()) return;
    if ((session->isRunning && session->GetTick() == 16) || fovChanger->needToUpdate) {
        fovChanger->Force();
    }
}

// Commands

CON_COMMAND_COMPLETION(sar_force_fov, "Forces player FOV. Usage: sar_force_fov <fov>\n", ({ "0", "50", "60", "70", "80", "90", "100", "110", "120", "130", "140" }))
{
    if (args.ArgC() != 2) {
        return console->Print("sar_force_fov <fov> : Forces player FOV.\n");
    }

    auto fov = std::atoi(args[1]);
    if (fov == 0) {
        fovChanger->SetFov(fov);
        return console->Print("Disabled forcing FOV!\n");
    }

    if (fov < 75 || fov > 150) {
        return console->Print("FOV value has to be between 75 to 150!\n");
    }

    fovChanger->SetFov(fov);
    console->Print("Enabled forcing FOV: %i\n", fov);
}
