#pragma once
#include "Offsets.hpp"
#include "Utils.hpp"

using _DrawColoredText = int(__cdecl*)(void* thisptr, unsigned long font, int x, int y, int r, int g, int b, int a, char *fmt, ...);

namespace Surface
{
	void* Ptr;
	_DrawColoredText DrawColoredText;

	void Set(uintptr_t mssPtr)
	{
		Ptr = **(void***)(mssPtr);
		DrawColoredText = (_DrawColoredText)GetVirtualFunctionByIndex(Ptr, Offsets::DrawColoredText);
	}
	void Draw(unsigned long font, int x, int y, Color clr, char *fmt, ...)
	{
		DrawColoredText(Ptr, font, x, y, clr.Colors[0], clr.Colors[1], clr.Colors[2], clr.Colors[3], fmt);
	}
}