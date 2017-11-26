#pragma once
#include "Utils.h"

#define COL_GREEN Color(17, 224, 35)

using _Msg = void(__cdecl*)(const char* pMsgFormat, ...);
using _Warning = void(__cdecl*)(const char* pMsgFormat, ...);
using _ColorMsg = void(__cdecl*)(const Color& clr, const char* pMsgFormat, ...);
using _DevMsg = void(__cdecl*)(const char* pMsgFormat, ...);
using _DevWarning = void(__cdecl*)(const char* pMsgFormat, ...);

namespace Console
{
	_Msg Msg;
	_ColorMsg ColorMsg;
	_Warning Warning;
	_DevMsg DevMsg;
	_DevWarning DevWarning;

	bool Init()
	{
		auto module = GetModuleHandleA("tier0.dll");
		if (module) {
			auto msgAddr = GetProcAddress(module, "Msg");
			auto colorMsgAddr = GetProcAddress(module, "?ConColorMsg@@YAXABVColor@@PBDZZ");
			auto warningAddr = GetProcAddress(module, "Warning");
			auto devMsgAddr = GetProcAddress(module, "?DevMsg@@YAXPBDZZ");
			auto devWarningAddr = GetProcAddress(module, "?DevWarning@@YAXPBDZZ");

			if (msgAddr && colorMsgAddr && warningAddr && devMsgAddr && devWarningAddr) {
				Msg = reinterpret_cast<_Msg>(msgAddr);
				ColorMsg = reinterpret_cast<_ColorMsg>(colorMsgAddr);
				Warning = reinterpret_cast<_Warning>(warningAddr);
				DevMsg = reinterpret_cast<_DevMsg>(devMsgAddr);
				DevWarning = reinterpret_cast<_DevWarning>(devWarningAddr);
				return true;
			}
		}
		return false;
	}
}