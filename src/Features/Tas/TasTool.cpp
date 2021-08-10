#include "TasTool.hpp"

#include "TasPlayer.hpp"
#include <algorithm>

std::list<TasTool *> &TasTool::GetList() {
	static std::list<TasTool *> list;
	return list;
}

TasTool::TasTool(const char *name)
	: name(name) {
	this->GetList().push_back(this);
}

TasTool::~TasTool() {
}

const char *TasTool::GetName() {
	return this->name;
}

void TasTool::SetParams(std::shared_ptr<TasToolParams> params) {
	this->params = params;
	this->updated = true;

	// the tool has been updated. prioritize it by moving it to the end of the global list
	// mlugg please don't kill me
	GetList().splice(GetList().end(), GetList(), std::find(GetList().begin(), GetList().end(), this));
}

void TasTool::Reset() {
	params = std::make_shared<TasToolParams>();
}

std::shared_ptr<TasToolParams> TasTool::GetCurrentParams() {
	return params;
}