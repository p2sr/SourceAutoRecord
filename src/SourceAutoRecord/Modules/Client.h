#pragma once
#include "SourceAutoRecord.h"
#include "Surface.h"

#define COL_WHITE Color(255, 255, 255, 255)

using _Paint = int(__thiscall*)(void* thisptr);

// client.dll
namespace Client
{
	_Paint Original_Paint;
	
	int __fastcall Detour_Paint(void* thisptr, int edx)
	{
		auto font = (int)(*((uintptr_t*)thisptr + 348));
		Surface::Draw(font, 2, 62, COL_WHITE, "The ting goes skrrrahh");
		return Original_Paint(thisptr);
	}
}