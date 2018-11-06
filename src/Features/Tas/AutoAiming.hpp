#pragma once
#include "Features/Feature.hpp"

class AutoAiming : public Feature {
public:
    AutoAiming();
    void AimAtPoint(float x, float y, float z);
};

extern AutoAiming* autoAiming;
