#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#include <stdarg.h>

class Surface : public Module {
public:
	Interface *matsurface = nullptr;

	typedef unsigned long HFont;

	using _DrawSetColor = int(__rescall *)(void *thisptr, int r, int g, int b, int a);
	using _DrawFilledRect = int(__rescall *)(void *thisptr, int x0, int y0, int x1, int y1);
	using _DrawColoredCircle = int(__rescall *)(void *thisptr, int centerx, int centery, float radius, int r, int g, int b, int a);
	using _DrawLine = int(__rescall *)(void *thisptr, int x0, int y0, int x1, int y1);
	using _DrawSetTextFont = int(__rescall *)(void *thisptr, HFont font);
	using _DrawSetTextColor = int(__rescall *)(void *thisptr, Color color);
	using _GetFontTall = int(__rescall *)(void *thisptr, HFont font);
	using _DrawColoredText = int(__cdecl *)(void *thisptr, HFont font, int x, int y, int r, int g, int b, int a, char *fmt, ...);
	using _DrawTextLen = int(__cdecl *)(void *thisptr, HFont font, char *fmt, ...);
	using _GetKernedCharWidth = void(__rescall *)(void *thisptr, HFont font, wchar_t ch, wchar_t prev, wchar_t next, float &wide, float &a, float &c);
	using _StartDrawing = int(__rescall *)(void *thisptr);
	using _FinishDrawing = int(__cdecl *)();
	using _GetFontName = const char *(__rescall *)(void *thisptr, HFont font);

	using _DrawGetTextureId = int(__rescall *)(void *thisptr, char const *filename);
	using _DrawGetTextureFile = int(__rescall *)(void *thisptr, int id, char *filename, int maxlen);
	using _DrawSetTextureFile = int(__rescall *)(void *thisptr, int id, const char *filename, int hardwareFilter, bool forceReload);
	using _DrawSetTextureRGBA = int(__rescall *)(void *thisptr, int id, const unsigned char *rgba, int wide, int tall);
	using _DrawSetTexture = int(__rescall *)(void *thisptr, int id);
	using _DrawGetTextureSize = int(__rescall *)(void *thisptr, int id, int &wide, int &tall);
	using _DrawTexturedRect = int(__rescall *)(void *thisptr, int x0, int y0, int x1, int y1);
	using _IsTextureIDValid = int(__rescall *)(void *thisptr, int id);
	using _CreateNewTextureID = int(__rescall *)(void *thisptr, bool procedural);

	_GetFontTall GetFontTall = nullptr;
	_DrawSetColor DrawSetColor = nullptr;
	_DrawFilledRect DrawFilledRect = nullptr;
	_DrawColoredCircle DrawColoredCircle = nullptr;
	_DrawLine DrawLine = nullptr;
	_DrawSetTextFont DrawSetTextFont = nullptr;
	_DrawSetTextColor DrawSetTextColor = nullptr;
	_DrawColoredText DrawColoredText = nullptr;
	_DrawTextLen DrawTextLen = nullptr;
	_GetKernedCharWidth GetKernedCharWidth = nullptr;
	_StartDrawing StartDrawing = nullptr;
	_FinishDrawing FinishDrawing = nullptr;
	_GetFontName GetFontName = nullptr;

	_DrawGetTextureId DrawGetTextureId = nullptr;
	_DrawGetTextureFile DrawGetTextureFile = nullptr;
	_DrawSetTextureFile DrawSetTextureFile = nullptr;
	_DrawSetTextureRGBA DrawSetTextureRGBA = nullptr;
	_DrawSetTexture DrawSetTexture = nullptr;
	_DrawGetTextureSize DrawGetTextureSize = nullptr;
	_DrawTexturedRect DrawTexturedRect = nullptr;
	_IsTextureIDValid IsTextureIDValid = nullptr;
	_CreateNewTextureID CreateNewTextureID = nullptr;

public:
	int GetFontHeight(HFont font);
	int GetFontLength(HFont font, const char *fmt, ...);
	void DrawTxt(HFont font, int x, int y, Color clr, const char *fmt, ...);
	void DrawRect(Color clr, int x0, int y0, int x1, int y1);
	void DrawRectAndCenterTxt(Color clr, int x0, int y0, int x1, int y1, HFont font, Color fontClr, const char *fmt, ...);
	void DrawCircle(int x, int y, float radius, Color clr);
	void DrawFilledCircle(int x, int y, float radius, Color clr);
	void DrawColoredLine(int x0, int y0, int x1, int y1, Color clr);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("vguimatsurface"); }
};

extern Surface *surface;
