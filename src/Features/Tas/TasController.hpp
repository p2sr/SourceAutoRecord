#pragma once

#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Utils/SDK.hpp"
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

static const char *g_TasControllerDigitalActions[] = {
	"+jump", "+duck", "+use", "+zoom", "+attack", "+attack2"};
static const int g_TasControllerInGameButtons[] = {
	IN_JUMP, IN_DUCK, IN_USE, 0, IN_ATTACK, IN_ATTACK2};

struct TasControllerButton {
	bool active = false;
	bool state = false;
	const char *command;
};

class TasController : public Feature {
private:
	Vector moveAnalog = {0, 0};
	Vector viewAnalog = {0, 0};
	TasControllerButton buttons[TAS_CONTROLLER_INPUT_COUNT] = {};
	std::vector<std::string> commandQueue;

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

	void ResetDigitalInputs();

	void AddCommandToQueue(std::string c);

	bool GetButtonState(TasControllerInput i);
	void SetButtonState(TasControllerInput i, bool state);

	void ControllerMove(int nSlot, float flFrametime, CUserCmd *cmd);
};

extern TasController *tasController;

extern Variable sar_tas_real_controller_debug;

extern Variable cl_pitchdown;
extern Variable cl_pitchup;
