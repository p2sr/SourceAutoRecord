#pragma once

#include "Feature.hpp"

class FovChanger : public Feature {
private:
	int defaultFov;
	int viewmodelFov;

public:
	FovChanger();
	void SetFov(const int fov);
	void SetViewmodelFov(const int fov);
	void Force();

	bool needToUpdate = false;
};

extern FovChanger *fovChanger;
