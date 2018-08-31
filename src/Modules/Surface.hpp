#pragma once
#include "Module.hpp"

#include <stdarg.h>

#include "Interface.hpp"
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

extern Surface* surface;
