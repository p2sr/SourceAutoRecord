#include "Imitator.hpp"

#include "Utils/SDK.hpp"
#include "Variable.hpp"

Variable sar_mimic("sar_mimic", "0", "Copies inputs to secondary split screen. Similar to ss_mimic.\n");

Imitator *imitator;

Imitator::Imitator()
	: frame() {
	this->hasLoaded = true;
}
void Imitator::Save(const CUserCmd *cmd) {
	this->frame.viewangles = cmd->viewangles;
	this->frame.forwardmove = cmd->forwardmove;
	this->frame.sidemove = cmd->sidemove;
	this->frame.upmove = cmd->upmove;
	this->frame.buttons = cmd->buttons;
	this->frame.impulse = cmd->impulse;
	this->frame.mousedx = cmd->mousedx;
	this->frame.mousedy = cmd->mousedy;
}
void Imitator::Modify(CUserCmd *cmd) {
	cmd->viewangles = this->frame.viewangles;
	cmd->forwardmove = this->frame.forwardmove;
	cmd->sidemove = this->frame.sidemove;
	cmd->upmove = this->frame.upmove;
	cmd->buttons = this->frame.buttons;
	cmd->impulse = this->frame.impulse;
	cmd->mousedx = this->frame.mousedx;
	cmd->mousedy = this->frame.mousedy;
}
