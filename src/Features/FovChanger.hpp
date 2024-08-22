#pragma once

#include "Feature.hpp"

class FovChanger : public Feature {
private:
	int defaultFov;
	float viewmodelFov;

public:
	FovChanger();
	void SetFov(const int fov);
	void SetViewmodelFov(const float fov);
	void Force();
};

extern FovChanger *fovChanger;
