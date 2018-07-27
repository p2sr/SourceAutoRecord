#pragma once
#include <stdarg.h>

#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

namespace Surface {

VMT matsurface;

typedef unsigned long HFont;

using _DrawSetColor = int(__func*)(void* thisptr, int r, int g, int b, int a);
using _DrawFilledRect = int(__func*)(void* thisptr, int x0, int y0, int x1, int y1);
#ifdef _WIN32
using _GetFontTall = int(__stdcall*)(HFont font);
#else
using _GetFontTall = int(__cdecl*)(void* thisptr, HFont font);
#endif
using _DrawColoredText = int(__cdecl*)(void* thisptr, HFont font, int x, int y, int r, int g, int b, int a, char* fmt, ...);
using _DrawTextLen = int(__cdecl*)(void* thisptr, HFont font, char* fmt, ...);
using _StartDrawing = int(__func*)(void* thisptr);
using _FinishDrawing = int(__cdecl*)();

_GetFontTall GetFontTall;
_DrawSetColor DrawSetColor;
_DrawFilledRect DrawFilledRect;
_DrawColoredText DrawColoredText;
_DrawTextLen DrawTextLen;
_StartDrawing StartDrawing;
_FinishDrawing FinishDrawing;

int GetFontHeight(HFont font)
{
#ifdef _WIN32
    return GetFontTall(font);
#else
    return GetFontTall(matsurface->GetThisPtr(), font);
#endif
}
int GetFontLength(HFont font, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);
    return DrawTextLen(matsurface->GetThisPtr(), font, data);
}
void DrawTxt(HFont font, int x, int y, Color clr, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);
    DrawColoredText(matsurface->GetThisPtr(), font, x, y, clr.r(), clr.g(), clr.b(), clr.a(), data);
}
void DrawRect(Color clr, int x0, int y0, int x1, int y1)
{
    DrawSetColor(matsurface->GetThisPtr(), clr.r(), clr.g(), clr.b(), clr.a());
    DrawFilledRect(matsurface->GetThisPtr(), x0, y0, x1, y1);
}
void DrawRectAndCenterTxt(Color clr, int x0, int y0, int x1, int y1, HFont font, Color fontClr, const char* fmt, ...)
{
    DrawRect(clr, x0, y0, x1, y1);

    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);

    auto tw = GetFontLength(font, data);
    auto th = GetFontHeight(font);

    // Center of rectangle
    auto xc = x0 + ((x1 - x0) / 2);
    auto yc = y0 + ((y1 - y0) / 2);

    DrawTxt(font, xc - (tw / 2), yc - (th / 2), fontClr, data);
}

void Hook()
{
    CREATE_VMT(Interfaces::ISurface, matsurface)
    {
        DrawSetColor = matsurface->GetOriginalFunction<_DrawSetColor>(Offsets::DrawSetColor);
        DrawFilledRect = matsurface->GetOriginalFunction<_DrawFilledRect>(Offsets::DrawFilledRect);
        GetFontTall = matsurface->GetOriginalFunction<_GetFontTall>(Offsets::GetFontTall);
        DrawColoredText = matsurface->GetOriginalFunction<_DrawColoredText>(Offsets::DrawColoredText);
        DrawTextLen = matsurface->GetOriginalFunction<_DrawTextLen>(Offsets::DrawTextLen);
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
