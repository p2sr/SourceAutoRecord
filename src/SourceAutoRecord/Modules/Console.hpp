#pragma once
#include "Utils.hpp"

#define Print(...) ColorMsg(SAR_COLOR, __VA_ARGS__)
#define PrintActive(...) ColorMsg(COL_ACTIVE, __VA_ARGS__)

namespace Console
{
	using _Msg = void(__cdecl*)(const char* pMsgFormat, ...);
	using _Warning = void(__cdecl*)(const char* pMsgFormat, ...);
	using _ColorMsg = void(__cdecl*)(const Color& clr, const char* pMsgFormat, ...);
	using _DevMsg = void(__cdecl*)(const char* pMsgFormat, ...);
	using _DevWarning = void(__cdecl*)(const char* pMsgFormat, ...);

	_Msg Msg;
	_ColorMsg ColorMsg;
	_Warning Warning;
	_DevMsg DevMsg;
	_DevWarning DevWarning;

	bool Init()
	{
		auto module = MODULEINFO();
		if (GetModuleInformation("libtier0.so", &module)) {
			auto tier0 = dlopen(module.modulePath, RTLD_NOLOAD | RTLD_NOW);

			if (tier0) {
				auto msgAddr = dlsym(tier0, "Msg");
				auto colorMsgAddr = dlsym(tier0, "_Z11ConColorMsgRK5ColorPKcz");
				auto warningAddr = dlsym(tier0, "Warning");
				auto devMsgAddr = dlsym(tier0, "_Z6DevMsgPKcz");
				auto devWarningAddr = dlsym(tier0, "_Z10DevWarningPKcz");

				if (msgAddr && colorMsgAddr && warningAddr && devMsgAddr && devWarningAddr) {
					Msg = reinterpret_cast<_Msg>(msgAddr);
					ColorMsg = reinterpret_cast<_ColorMsg>(colorMsgAddr);
					Warning = reinterpret_cast<_Warning>(warningAddr);
					DevMsg = reinterpret_cast<_DevMsg>(devMsgAddr);
					DevWarning = reinterpret_cast<_DevWarning>(devWarningAddr);
					return true;
				}
			}
		}
		return false;
	}
}