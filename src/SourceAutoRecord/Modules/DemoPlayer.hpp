#pragma once
#include "Offsets.hpp"
#include "Utils.hpp"

using _IsPlayingBack = bool(__thiscall*)(void* thisptr);
using _GetPlaybackTick = int(__thiscall*)(void* thisptr);

namespace DemoPlayer
{
	void* Ptr;
	char* DemoName;

	_IsPlayingBack IsPlayingBack;
	_GetPlaybackTick GetPlaybackTick;

	void Set(uintptr_t demoPlayerPtr)
	{
		Ptr = **(void***)(demoPlayerPtr);
		GetPlaybackTick = (_GetPlaybackTick)GetVirtualFunctionByIndex(Ptr, Offsets::GetPlaybackTick);
		IsPlayingBack = (_IsPlayingBack)GetVirtualFunctionByIndex(Ptr, Offsets::IsPlayingBack);
		DemoName = (char*)reinterpret_cast<uintptr_t*>((uintptr_t)Ptr + Offsets::m_szFileName);
	}
	int GetTick()
	{
		return GetPlaybackTick(Ptr);
	}
	bool IsPlaying()
	{
		return IsPlayingBack(Ptr);
	}
}