#include "TasTool.hpp"

#include "TasPlayer.hpp"
#include <algorithm>

std::list<TasTool *> &TasTool::GetList(int slot) {
	static std::list<TasTool *> list[2];
	return list[slot];
}

TasTool *TasTool::GetInstanceByName(int slot, std::string name) {
	for (TasTool *tool : TasTool::GetList(slot)) {
		if (tool->GetName() == name) return tool;
	}
	return nullptr;
}

std::vector<std::string> TasTool::priorityList = {
	"check",
	"cmd",
	"stop",
	"use",
	"duck",
	"zoom",
	"shoot",
	"setang",
	"autoaim",
	"look",
	"autojump",
	"absmov",
	"move",
	"strafe",
	"decel",
};

TasTool::TasTool(const char *name, int slot)
	: name(name)
	, slot(slot) {
	this->GetList(slot).push_back(this);
	
	// in case the tool is not defined in the priority list, put it in the back of it
	std::string nameStr = name;
	if (std::find(priorityList.begin(), priorityList.end(), nameStr) == priorityList.end()) {
		priorityList.push_back(name);
	}
}

TasTool::~TasTool() {
}

const char *TasTool::GetName() {
	return this->name;
}

void TasTool::SetParams(std::shared_ptr<TasToolParams> params) {
	this->paramsPtr = params;
	this->updated = true;

	// legacy behaviour for version 2 or older
	FOR_TAS_SCRIPT_VERSIONS_UNTIL(2) {
		// the tool has been updated. prioritize it by moving it to the end of the global list
		// mlugg please don't kill me
		GetList(slot).splice(GetList(slot).end(), GetList(slot), std::find(GetList(slot).begin(), GetList(slot).end(), this));
	}
}


void TasTool::Reset() {
	paramsPtr = std::make_shared<TasToolParams>();
}
