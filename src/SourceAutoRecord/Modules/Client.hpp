#pragma once
#include "Engine.hpp"
#include "Surface.hpp"
#include "../Utils.hpp"

using _Paint = int(__thiscall*)(void* thisptr);

// client.dll
namespace Client
{
	namespace Original
	{
		_Paint Paint;
	}
	
	namespace Detour
	{
		int __fastcall Paint(void* thisptr, int edx)
		{
			// TODO: font size
			int font = (int)(*((uintptr_t*)thisptr + 348));

			char ticks[64];
			snprintf(ticks, sizeof(ticks), "Ticks: %i", !*Engine::LoadGame ? Engine::GetCurrentTick() : 0);
			Surface::Draw(font, 2, 63, COL_WHITE, ticks);

			return Original::Paint(thisptr);
		}
	}
}