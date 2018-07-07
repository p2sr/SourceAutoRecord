#pragma once
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Surface {

VMT matsurface;

using _GetFontTall = int(__func*)(void* thisptr, unsigned long font);
using _DrawColoredText = int(__cdecl*)(void* thisptr, unsigned long font, int x, int y, int r, int g, int b, int a, char* fmt, ...);
using _StartDrawing = int(__func*)(void* thisptr);
using _FinishDrawing = int(__cdecl*)();

_GetFontTall GetFontTall;
_DrawColoredText DrawColoredText;
_StartDrawing StartDrawing;
_FinishDrawing FinishDrawing;

int GetFontHeight(unsigned long font)
{
    return GetFontTall(matsurface->GetThisPtr(), font);
}
void Draw(unsigned long font, int x, int y, Color clr, char* fmt, ...)
{
    DrawColoredText(matsurface->GetThisPtr(), font, x, y, clr.r(), clr.g(), clr.b(), clr.a(), fmt);
}

void Hook()
{
    CREATE_VMT(Interfaces::ISurface, matsurface) {
        DrawColoredText = matsurface->GetOriginalFunction<_DrawColoredText>(Offsets::DrawColoredText);
        GetFontTall = matsurface->GetOriginalFunction<_GetFontTall>(Offsets::GetFontTall);
    }

    auto sdr = SAR::Find("StartDrawing");
    auto fdr = SAR::Find("FinishDrawing");
    if (sdr.Found && fdr.Found) {
        StartDrawing = reinterpret_cast<_StartDrawing>(sdr.Address);
        FinishDrawing = reinterpret_cast<_FinishDrawing>(fdr.Address);
    }
}
void Unhook()
{
    DELETE_VMT(matsurface);
}
}