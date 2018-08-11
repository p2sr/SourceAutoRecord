#pragma once
#include <stdarg.h>

#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

class Surface : public Module {
public:
    Interface* matsurface;

    typedef unsigned long HFont;

    using _DrawSetColor = int(__func*)(void* thisptr, int r, int g, int b, int a);
    using _DrawFilledRect = int(__func*)(void* thisptr, int x0, int y0, int x1, int y1);
    using _DrawLine = int(__func*)(void* thisptr, int x0, int y0, int x1, int y1);
    using _DrawSetTextFont = int(__func*)(void* thisptr, HFont font);
    using _DrawSetTextColor = int(__func*)(void* thisptr, Color color);
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
    _DrawLine DrawLine;
    _DrawSetTextFont DrawSetTextFont;
    _DrawSetTextColor DrawSetTextColor;
    _DrawColoredText DrawColoredText;
    _DrawTextLen DrawTextLen;
    _StartDrawing StartDrawing;
    _FinishDrawing FinishDrawing;

public:
    int GetFontHeight(HFont font);
    int GetFontLength(HFont font, const char* fmt, ...);
    void DrawTxt(HFont font, int x, int y, Color clr, const char* fmt, ...);
    void DrawRect(Color clr, int x0, int y0, int x1, int y1);
    void DrawRectAndCenterTxt(Color clr, int x0, int y0, int x1, int y1, HFont font, Color fontClr, const char* fmt, ...);
    bool Init() override;
    void Shutdown() override;
};

int Surface::GetFontHeight(HFont font)
{
#ifdef _WIN32
    return this->GetFontTall(font);
#else
    return this->GetFontTall(this->matsurface->ThisPtr(), font);
#endif
}
int Surface::GetFontLength(HFont font, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);
    return this->DrawTextLen(this->matsurface->ThisPtr(), font, data);
}
void Surface::DrawTxt(HFont font, int x, int y, Color clr, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);
    this->DrawColoredText(this->matsurface->ThisPtr(), font, x, y, clr.r(), clr.g(), clr.b(), clr.a(), data);
}
void Surface::DrawRect(Color clr, int x0, int y0, int x1, int y1)
{
    this->DrawSetColor(this->matsurface->ThisPtr(), clr.r(), clr.g(), clr.b(), clr.a());
    this->DrawFilledRect(this->matsurface->ThisPtr(), x0, y0, x1, y1);
}
void Surface::DrawRectAndCenterTxt(Color clr, int x0, int y0, int x1, int y1, HFont font, Color fontClr, const char* fmt, ...)
{
    this->DrawRect(clr, x0, y0, x1, y1);

    va_list argptr;
    va_start(argptr, fmt);
    char data[1024];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);

    auto tw = this->GetFontLength(font, data);
    auto th = this->GetFontHeight(font);

    // Center of rectangle
    auto xc = x0 + ((x1 - x0) / 2);
    auto yc = y0 + ((y1 - y0) / 2);

    this->DrawTxt(font, xc - (tw / 2), yc - (th / 2), fontClr, data);
}
bool Surface::Init()
{
    this->matsurface = Interface::Create(MODULE("vguimatsurface"), "VGUI_Surface0", false);
    if (this->matsurface) {
        this->DrawSetColor = matsurface->Original<_DrawSetColor>(Offsets::DrawSetColor);
        this->DrawFilledRect = matsurface->Original<_DrawFilledRect>(Offsets::DrawFilledRect);
        this->DrawLine = matsurface->Original<_DrawLine>(Offsets::DrawLine);
        this->DrawSetTextFont = matsurface->Original<_DrawSetTextFont>(Offsets::DrawSetTextFont);
        this->DrawSetTextColor = matsurface->Original<_DrawSetTextColor>(Offsets::DrawSetTextColor);
        this->GetFontTall = matsurface->Original<_GetFontTall>(Offsets::GetFontTall);
        this->DrawColoredText = matsurface->Original<_DrawColoredText>(Offsets::DrawColoredText);
        this->DrawTextLen = matsurface->Original<_DrawTextLen>(Offsets::DrawTextLen);

        auto PaintTraverseEx = matsurface->Original(Offsets::PaintTraverseEx);
        this->StartDrawing = Memory::Read<_StartDrawing>(PaintTraverseEx + Offsets::StartDrawing);
        this->FinishDrawing = Memory::Read<_FinishDrawing>(PaintTraverseEx + Offsets::FinishDrawing);
    }

    return this->hasLoaded = this->matsurface;
}
void Surface::Shutdown()
{
    Interface::Delete(this->matsurface);
}

Surface* surface;
extern Surface* surface;
