#pragma once
#include "vmthook/vmthook.h"

#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

#define BUTTON_CODE_INVALID -1
#define KEY_ESCAPE 70

namespace InputSystem
{
	using _StringToButtonCode = int(__cdecl*)(void* thisptr, const char* pString);
	using _KeySetBinding = void(__cdecl*)(int keynum, const char* pBinding);

	 std::unique_ptr<VMTHook> g_InputSystem;
	
	_StringToButtonCode StringToButtonCode;
	_KeySetBinding KeySetBinding;

	int GetButton(const char* pString)
	{
		return StringToButtonCode(g_InputSystem->GetThisPtr(), pString);
	}
	void Hook()
	{
		if (Interfaces::IInputSystem) {
			g_InputSystem = std::make_unique<VMTHook>(Interfaces::IInputSystem);
			StringToButtonCode = g_InputSystem->GetOriginalFunction<_StringToButtonCode>(Offsets::StringToButtonCode);
		}

		auto ksb = SAR::Find("Key_SetBinding");
		if (ksb.Found) {
			KeySetBinding = reinterpret_cast<_KeySetBinding>(ksb.Address);
		}
	}
	void Unhook()
	{
		if (g_InputSystem) {
			g_InputSystem->~VMTHook();
			g_InputSystem.release();
			StringToButtonCode = nullptr;
		}

		if (KeySetBinding) {
			KeySetBinding = nullptr;
		}
	}
}