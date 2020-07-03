#pragma once

#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Variable.hpp"


#define TAS_CONTROLLER_INPUT_COUNT 6

enum TasControllerInput {
    Jump,
    Crouch,
    Use,
    Zoom,
    FireBlue,
    FireOrange
};

static const char* g_TasControllerDigitalActions[] = {
    "+jump", "+duck", "+use", "+zoom", "+attack", "+attack2"
};

struct TasControllerButton {
    bool active = false;
    bool state = false;
    const char* command;
};

class TasController : public Feature {
private:
    Vector moveAnalog;
    Vector viewAnalog;
    TasControllerButton buttons[TAS_CONTROLLER_INPUT_COUNT];
    std::vector<char*> commandQueue;

    bool enabled;
public:
    TasController();
    ~TasController();

    Vector GetMoveAnalog();
    Vector GetViewAnalog();

    void SetMoveAnalog(float x, float y);
    void SetViewAnalog(float x, float y);

    bool isEnabled();
    void Enable();
    void Disable();

    void AddCommandToQueue(char* c);

    bool GetButtonState(TasControllerInput i);
    void SetButtonState(TasControllerInput i, bool state);

    void ControllerMove(int nSlot, float flFrametime, CUserCmd* cmd);
};

extern TasController* tasController;

extern Variable cl_pitchdown;
extern Variable cl_pitchup;
extern Variable sensitivity;