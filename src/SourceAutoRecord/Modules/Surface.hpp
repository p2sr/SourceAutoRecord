#pragma once
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Surface
{
	using _DrawColoredText = int(__cdecl*)(void* thisptr, unsigned long font, int x, int y, int r, int g, int b, int a, char *fmt, ...);
	using _StartDrawing = int(__cdecl*)(void* thisptr);
	using _FinishDrawing = int(__cdecl*)();

	std::unique_ptr<VMTHook> matsurface;

	_DrawColoredText DrawColoredText;
	_StartDrawing StartDrawing;
	_FinishDrawing FinishDrawing;

	void Draw(unsigned long font, int x, int y, Color clr, char *fmt, ...)
	{
		DrawColoredText(matsurface->GetThisPtr(), font, x, y, clr.r(), clr.g(), clr.b(), clr.a(), fmt);
	}

	void Hook()
	{
		if (Interfaces::ISurface) {
			matsurface = std::make_unique<VMTHook>(Interfaces::ISurface);
			DrawColoredText = matsurface->GetOriginalFunction<_DrawColoredText>(Offsets::DrawColoredText);
		}

		auto sdr = SAR::Find("StartDrawing");
		auto fdr = SAR::Find("FinishDrawing");
		if (sdr.Found && fdr.Found) {
			StartDrawing = reinterpret_cast<_StartDrawing>(sdr.Address);
			FinishDrawing = reinterpret_cast<_FinishDrawing>(fdr.Address);
		}
	}
}