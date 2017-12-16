#pragma once
#include "Offsets.hpp"
#include "Utils.hpp"

using _GetRecordingTick = int(__thiscall*)(void* thisptr);

namespace DemoRecorder
{
	void* Ptr;

	_GetRecordingTick GetRecordingTick;

	char* DemoName;
	int* DemoNumber;
	bool* Recording;

	std::string CurrentDemo;
	std::string LastDemo;
	int LastDemoTick;

	void Set(uintptr_t recorderAddr)
	{
		Ptr = **(void***)(recorderAddr);
		GetRecordingTick = (_GetRecordingTick)GetVirtualFunctionByIndex(Ptr, Offsets::GetRecordingTick);
		DemoName = (char*)reinterpret_cast<uintptr_t*>((uintptr_t)Ptr + Offsets::m_szDemoBaseName);
		DemoNumber = (int*)reinterpret_cast<uintptr_t*>((uintptr_t)Ptr + Offsets::m_nDemoNumber);
		Recording = (bool*)reinterpret_cast<uintptr_t*>((uintptr_t)Ptr + Offsets::m_bRecording);
	}
	int GetTick()
	{
		return GetRecordingTick(Ptr);
	}
	void SetCurrentDemo()
	{
		CurrentDemo = std::string(DemoRecorder::DemoName);
		if (*DemoNumber > 1) CurrentDemo += "_" + std::to_string(*DemoNumber);
	}
	void SetLastDemo(int tick)
	{
		LastDemo = std::string(DemoRecorder::DemoName);
		if (*DemoNumber > 1) LastDemo += "_" + std::to_string(*DemoNumber);
		LastDemoTick = tick;
	}
}