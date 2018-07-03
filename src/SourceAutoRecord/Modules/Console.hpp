#pragma once
#include "Utils.hpp"

#define Print(...) ColorMsg(SAR_COLOR, __VA_ARGS__)
#define PrintActive(...) ColorMsg(COL_ACTIVE, __VA_ARGS__)

namespace Console {

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
    auto tier0 = Memory::GetModuleHandleByName("tier0.dll");
    if (tier0) {
        auto msgAddr = Memory::GetSymbolAddress(tier0, "Msg");
        auto colorMsgAddr = Memory::GetSymbolAddress(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ");
        auto warningAddr = Memory::GetSymbolAddress(tier0, "Warning");
        auto devMsgAddr = Memory::GetSymbolAddress(tier0, "?DevMsg@@YAXPBDZZ");
        auto devWarningAddr = Memory::GetSymbolAddress(tier0, "?DevWarning@@YAXPBDZZ");

        Memory::CloseModuleHandle(tier0);

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