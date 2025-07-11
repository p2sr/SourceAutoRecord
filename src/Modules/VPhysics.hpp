#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"

class VPhysics : public Module {
public:
	Interface *g_pVphysics = nullptr;

	using _GetActiveEnvironmentByIndex = int *(__rescall *)(void *thisptr, int index);
	_GetActiveEnvironmentByIndex GetActiveEnvironmentByIndex = nullptr;

	using _DestroyEnvironment = void(__rescall *)(void *thisptr, void *env);
	_DestroyEnvironment DestroyEnvironment = nullptr;

public:
	int *GetActivePhysicsEnvironmentByIndex(int index);
	void DestroyPhysicsEnvironment(int *env);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("vphysics"); }
};

extern VPhysics *vphysics;
