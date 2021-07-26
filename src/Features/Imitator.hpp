#pragma once
#include "Feature.hpp"
#include "ReplaySystem/ReplayFrame.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

class Imitator : public Feature {
private:
	ReplayFrame frame;

public:
	Imitator();
	void Save(const CUserCmd *cmd);
	void Modify(CUserCmd *cmd);
};

extern Imitator *imitator;

extern Variable sar_mimic;
