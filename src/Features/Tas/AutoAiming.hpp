#pragma once

#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"



class AutoAiming : public Feature
{

public:

	void AimAtPoint(float x, float y, float z);

};

extern AutoAiming *autoAiming;