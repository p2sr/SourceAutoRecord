#pragma once

#include "Feature.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

class ReloadedFix : public Feature {
public:
	bool inMapCheatsEnabled = false;
public:
	ReloadedFix();
	void OverrideInput(const char *className, const char *inputName, variant_t* parameter);
private:
	void CustomSetPortalID();
};

extern ReloadedFix *reloadedFix;


extern Variable sar_fix_reloaded_cheats;
