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

	char* DemoName;
	int* DemoNumber;
	bool* Recording;

	std::string CurrentDemo;

	int GetTick()
	{
		return GetRecordingTick(s_ClientDemoRecorder->GetThisPtr());
	}
	void SetCurrentDemo()
	{
		CurrentDemo = std::string(DemoRecorder::DemoName);
		if (*DemoNumber > 1) CurrentDemo += "_" + std::to_string(*DemoNumber);
	}

	namespace Original
	{
		_SetSignonState SetSignonState;
		_StopRecording StopRecording;
	}

	namespace Detour
	{
		bool IsRecordingDemo;

		void __cdecl SetSignonState(void* thisptr, int state)
		{
			Console::PrintActive("SetSignonState = %i\n", state);
			if (state == SignonState::Prespawn) {
				if (Rebinder::IsSaveBinding || Rebinder::IsReloadBinding) {
					Rebinder::LastIndexNumber = (IsRecordingDemo)
						? *DemoRecorder::DemoNumber
						: Rebinder::LastIndexNumber + 1;

					Rebinder::RebindSave();
					Rebinder::RebindReload();
				}
			}
			// Demo recorder starts syncing from this tick
			else if (state == SignonState::Full) {
				Session::Rebase(Vars::gpGlobals->interval_per_tick);
				Timer::Rebase(Vars::gpGlobals->interval_per_tick);

				if (*DemoRecorder::Recording) {
					IsRecordingDemo = true;
					DemoRecorder::SetCurrentDemo();
				}
			}
			Original::SetSignonState(thisptr, state);
		}
		int __cdecl StopRecording(void* thisptr)
		{
			Console::PrintActive("StopRecording!\n");
			const int LastDemoNumber = *DemoRecorder::DemoNumber;

			// This function does:
			// m_bRecording = false
			// m_nDemoNumber = 0
			int result = Original::StopRecording(thisptr);

			if (IsRecordingDemo && sar_autorecord.GetBool()) {
				*DemoRecorder::DemoNumber = LastDemoNumber;

				// Tell recorder to keep recording
				if (*Vars::LoadGame) {
					*DemoRecorder::Recording = true;
					(*DemoRecorder::DemoNumber)++;
				}
			}
			else {
				IsRecordingDemo = false;
			}

			return result;
		}
	}
	void Hook()
	{
		auto recorder = SAR::Find("demorecorder");
		if (recorder.Found) {
			auto ptr = **(void***)recorder.Address;

			s_ClientDemoRecorder = std::make_unique<VMTHook>(ptr);
			s_ClientDemoRecorder->HookFunction((void*)Detour::SetSignonState, Offsets::SetSignonState);
			s_ClientDemoRecorder->HookFunction((void*)Detour::StopRecording, Offsets::StopRecording);
			Original::SetSignonState = s_ClientDemoRecorder->GetOriginalFunction<_SetSignonState>(Offsets::SetSignonState);
			Original::StopRecording = s_ClientDemoRecorder->GetOriginalFunction<_StopRecording>(Offsets::StopRecording);

			GetRecordingTick = s_ClientDemoRecorder->GetOriginalFunction<_GetRecordingTick>(Offsets::GetRecordingTick);
			DemoName = reinterpret_cast<char*>((uintptr_t)ptr + Offsets::m_szDemoBaseName);
			DemoNumber = reinterpret_cast<int*>((uintptr_t)ptr + Offsets::m_nDemoNumber);
			Recording = reinterpret_cast<bool*>((uintptr_t)ptr + Offsets::m_bRecording);
		}
	}
}