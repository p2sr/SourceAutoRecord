#pragma once

#include "Feature.hpp"

class FovChanger : public Feature {
private:
    int defaultFov;

public:
    FovChanger();
    void SetFov(const int fov);
    void Force();
};

extern FovChanger* fovChanger;
