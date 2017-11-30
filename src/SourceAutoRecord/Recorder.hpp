#pragma once
#include "Commands.hpp"
#include "Offsets.hpp"

using namespace Commands;

namespace Recorder
{
	void* RecorderPtr;

	char* DemoName;
	int* DemoNumber;
	bool* Recording;

	std::string LastDemo;

	void Set(uintptr_t recorderAddr)
	{
		RecorderPtr = **(void***)(recorderAddr);
		DemoName = (char*)reinterpret_cast<uintptr_t*>((uintptr_t)RecorderPtr + Offsets::m_szDemoBaseName);
		DemoNumber = (int*)reinterpret_cast<uintptr_t*>((uintptr_t)RecorderPtr + Offsets::m_nDemoNumber);
		Recording = (bool*)reinterpret_cast<uintptr_t*>((uintptr_t)RecorderPtr + Offsets::m_bRecording);
	}
	void SetLastDemo()
	{
		LastDemo = std::string(Recorder::DemoName);
		if (*DemoNumber > 1) {
			LastDemo += std::string("_") + std::to_string(*DemoNumber);
		}
		LastDemo += std::string(".dem");
	}
}