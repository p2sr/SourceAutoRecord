#pragma once
#include "Utils.hpp"

namespace Interfaces
{
	void* IGameMovement;
	void* IVEngineClient;
	void* IBaseClientDLL;

	void* Get(const char* filename, const char* version)
	{
		auto handle = dlopen(filename, RTLD_NOLOAD | RTLD_NOW);
		if (!handle) return nullptr;

		auto factory = dlsym(handle, "CreateInterface");
		if (!factory) return nullptr;

		typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
		return ((CreateInterfaceFn)(factory))(version, nullptr);
	}
}