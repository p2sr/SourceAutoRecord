#pragma once
#include "Utils.hpp"

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
		auto tier0 = GetModuleHandleA("tier0.dll");
		if (tier0) {
			auto msgAddr = GetProcAddress(tier0, "Msg");
			auto colorMsgAddr = GetProcAddress(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ");
			auto warningAddr = GetProcAddress(tier0, "Warning");
			auto devMsgAddr = GetProcAddress(tier0, "?DevMsg@@YAXPBDZZ");
			auto devWarningAddr = GetProcAddress(tier0, "?DevWarning@@YAXPBDZZ");

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