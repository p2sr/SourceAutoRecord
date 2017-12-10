#pragma once
#include "Offsets.hpp"
#include "Utils.hpp"

using _IsPlayingBack = bool(__thiscall*)(void* thisptr);

namespace DemoPlayer
{
	void* Ptr;
	char* DemoName;

	_IsPlayingBack IsPlayingBack;

	void Set(uintptr_t demoPlayerPtr)
	{
		Ptr = **(void***)(demoPlayerPtr);
		IsPlayingBack = (_IsPlayingBack)GetVirtualFunctionByIndex(Ptr, Offsets::IsPlayingBack);
		DemoName = (char*)reinterpret_cast<uintptr_t*>((uintptr_t)Ptr + Offsets::m_szFileName);
	}
	bool IsPlaying()
	{
		return IsPlayingBack(Ptr);
	}
}