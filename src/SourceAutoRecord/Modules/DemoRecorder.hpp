#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Vars.hpp"

#include "Features/Rebinder.hpp"
#include "Features/Session.hpp"
#include "Features/Timer.hpp"

#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace DemoRecorder
{
	using _GetRecordingTick = int(__cdecl*)(void* thisptr);
	using _SetSignonState = void(__cdecl*)(void* thisptr, int state);
	using _StopRecording = int(__cdecl*)(void* thisptr);

	std::unique_ptr<VMTHook> s_ClientDemoRecorder;

	_GetRecordingTick GetRecordingTick;

	char* m_szDemoBaseName;
	int* m_nDemoNumber;
	bool* m_bRecording;

	std::string CurrentDemo;
	bool IsRecordingDemo;

	int GetTick()
	{
		return GetRecordingTick(s_ClientDemoRecorder->GetThisPtr());
	}

	namespace Original
	{
		_SetSignonState SetSignonState;
		_StopRecording StopRecording;
	}

	namespace Detour
	{
		void __cdecl SetSignonState(void* thisptr, int state)
		{
			//Console::PrintActive("SetSignonState = %i\n", state);
			if (state == SignonState::Full && *m_bRecording) {
				IsRecordingDemo = true;
				CurrentDemo = std::string(m_szDemoBaseName);
				if (*m_nDemoNumber > 1) {
					CurrentDemo += std::string("_") + std::to_string(*m_nDemoNumber);
				}
			}
			Original::SetSignonState(thisptr, state);
		}
		int __cdecl StopRecording(void* thisptr)
		{
			Console::PrintActive("StopRecording!\n");
			const int LastDemoNumber = *m_nDemoNumber;

			// This function does:
			//   m_bRecording = false
			//   m_nDemoNumber = 0
			auto result = Original::StopRecording(thisptr);

			if (IsRecordingDemo && sar_autorecord.GetBool()) {
				*m_nDemoNumber = LastDemoNumber;

				// Tell recorder to keep recording
				if (*Vars::m_bLoadgame) {
					*m_bRecording = true;
					(*m_nDemoNumber)++;
					Console::DevMsg("SAR: Recording!");
				}
			}
			else {
				IsRecordingDemo = false;
			}

			return result;
		}
	}
	void Hook(void* demorecorder)
	{
		if (demorecorder) {
			s_ClientDemoRecorder = std::make_unique<VMTHook>(demorecorder);
			s_ClientDemoRecorder->HookFunction((void*)Detour::SetSignonState, Offsets::SetSignonState);
			s_ClientDemoRecorder->HookFunction((void*)Detour::StopRecording, Offsets::StopRecording);
			Original::SetSignonState = s_ClientDemoRecorder->GetOriginalFunction<_SetSignonState>(Offsets::SetSignonState);
			Original::StopRecording = s_ClientDemoRecorder->GetOriginalFunction<_StopRecording>(Offsets::StopRecording);

			GetRecordingTick = s_ClientDemoRecorder->GetOriginalFunction<_GetRecordingTick>(Offsets::GetRecordingTick);
			m_szDemoBaseName = reinterpret_cast<char*>((uintptr_t)demorecorder + Offsets::m_szDemoBaseName);
			m_nDemoNumber = reinterpret_cast<int*>((uintptr_t)demorecorder + Offsets::m_nDemoNumber);
			m_bRecording = reinterpret_cast<bool*>((uintptr_t)demorecorder + Offsets::m_bRecording);
		}
	}
	void Unhook()
	{
		if (s_ClientDemoRecorder) {
			s_ClientDemoRecorder->UnhookFunction(Offsets::SetSignonState);
			s_ClientDemoRecorder->UnhookFunction(Offsets::StopRecording);
			s_ClientDemoRecorder->~VMTHook();
			s_ClientDemoRecorder.release();
			Original::SetSignonState = nullptr;
			Original::StopRecording = nullptr;

			GetRecordingTick = nullptr;
			m_szDemoBaseName = nullptr;
			m_nDemoNumber = nullptr;
			m_bRecording = nullptr;
		}
	}
}