#pragma once
#include "Offsets.hpp"
#include "Utils.hpp"

using _DrawColoredText = int(__cdecl*)(void* thisptr, unsigned long font, int x, int y, int r, int g, int b, int a, char *fmt, ...);

namespace Surface
{
	void* MatSystemSurfacePtr;
	_DrawColoredText DrawColoredText;

	void Set(uintptr_t mssPtr)
	{
		MatSystemSurfacePtr = **(void***)(mssPtr);
		DrawColoredText = (_DrawColoredText)GetVirtualFunctionByIndex(MatSystemSurfacePtr, Offsets::DrawColoredText);
	}
	void Draw(unsigned long font, int x, int y, Color clr, char *fmt, ...)
	{
		DrawColoredText(MatSystemSurfacePtr, font, x, y, clr.r(), clr.g(), clr.b(), clr.a(), fmt);
	}
}