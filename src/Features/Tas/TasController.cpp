#include "TasController.hpp"

#include "Modules/Console.hpp"

TasController* tasController;

TasController::TasController()
{
    this->hasLoaded = true;
}

TasController::~TasController()
{
}

void TasController::ControllerMove(int nSlot, float flFrametime, CUserCmd* cmd) {
    //console->Print("nSlot: %d, flFrameTime:%f\n", nSlot, flFrametime);
}
