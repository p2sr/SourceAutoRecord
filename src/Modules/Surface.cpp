#include "Surface.hpp"

#include "Command.hpp"
#include "Console.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Scheme.hpp"

#include <stdarg.h>

CON_COMMAND(sar_font_get_name, "sar_font_get_name <id> - gets the name of a font from its index\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_font_get_name.ThisPtr()->m_pszHelpString);
	}

	int id = scheme->GetDefaultFont() + atoi(args[1]);
	if (surface->IsFontValid(id)) {
		const char *name = surface->GetFontName(surface->matsurface->ThisPtr(), id);
		console->Print("%s\n", name);
	} else {
		console->Print("Invalid font index\n");
	}
}

CON_COMMAND(sar_font_list, "sar_font_list - lists all available fonts\n") {
	if (surface->m_FontAmalgams == nullptr) return;
	int fontCount = surface->m_FontAmalgams->m_Size;

	int defaultFontID = scheme->GetDefaultFont();

	for (int i = 0; i < fontCount; i++) {
		std::string output = std::to_string(i - defaultFontID) + ". ";
		if (surface->IsFontValid(i)) {
			output += surface->GetFontName(surface->matsurface->ThisPtr(), i);
			output += Utils::ssprintf(" (size %d)", surface->m_FontAmalgams->m_pElements[i].m_iMaxHeight);
		} else {
			output += "(INVALID)";
		}
		output += "\n";
		console->Print("%s", output.c_str());
	}
	
}

// using font amalgams array to determine whether font is valid.
bool Surface::IsFontValid(HFont font) {
	if (m_FontAmalgams == nullptr) return false;
	if ((unsigned)m_FontAmalgams->m_Size <= font) return false;
	return m_FontAmalgams->m_pElements[font].m_Fonts.m_Size > 0;
}

int Surface::GetFontHeight(HFont font) {
	return this->GetFontTall(this->matsurface->ThisPtr(), font);
}
int Surface::GetFontLength(HFont font, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	char data[1024];
	vsnprintf(data, sizeof(data), fmt, argptr);
	va_end(argptr);

	int length = 0;
	for (size_t i = 0; data[i]; ++i) {
		wchar_t prev = i == 0 ? 0 : data[i - 1];
		wchar_t next = data[i + 1];
		wchar_t ch = data[i];
		float wide, a, c;
		this->GetKernedCharWidth(this->matsurface->ThisPtr(), font, ch, prev, next, wide, a, c);
		length += floor(wide + 0.6);
	}
	return length;
}
void Surface::DrawTxt(HFont font, int x, int y, Color clr, const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	char data[1024];
	vsnprintf(data, sizeof(data), fmt, argptr);
	va_end(argptr);
	this->DrawColoredText(this->matsurface->ThisPtr(), font, x, y, clr.r, clr.g, clr.b, clr.a, "%s", data);
}
void Surface::DrawRect(Color clr, int x0, int y0, int x1, int y1) {
	this->DrawSetColor(this->matsurface->ThisPtr(), clr.r, clr.g, clr.b, clr.a);
	this->DrawFilledRect(this->matsurface->ThisPtr(), x0, y0, x1, y1);
}
void Surface::DrawRectAndCenterTxt(Color clr, int x0, int y0, int x1, int y1, HFont font, Color fontClr, const char *fmt, ...) {
	this->DrawRect(clr, x0, y0, x1, y1);

	va_list argptr;
	va_start(argptr, fmt);
	char data[1024];
	vsnprintf(data, sizeof(data), fmt, argptr);
	va_end(argptr);

	auto tw = this->GetFontLength(font, "%s", data);
	auto th = this->GetFontHeight(font);

	// Center of rectangle
	auto xc = x0 + ((x1 - x0) / 2);
	auto yc = y0 + ((y1 - y0) / 2);

	this->DrawTxt(font, xc - (tw / 2), yc - (th / 2), fontClr, "%s", data);
}
void Surface::DrawCircle(int x, int y, float radius, Color clr) {
	this->DrawColoredCircle(this->matsurface->ThisPtr(), x, y, radius, clr.r, clr.g, clr.b, clr.a);
}
void Surface::DrawFilledCircle(int x, int y, float radius, Color clr) {
	const int r2 = radius * radius;

	for (int cy = -radius; cy <= radius; cy++) {
		int cx = sqrtf(r2 - cy * cy) + 0.5;
		int cyy = cy + y;

		surface->DrawColoredLine(x - cx, cyy, x + cx, cyy, clr);
	}
}
void Surface::DrawColoredLine(int x0, int y0, int x1, int y1, Color clr) {
	this->DrawSetColor(this->matsurface->ThisPtr(), clr.r, clr.g, clr.b, clr.a);
	this->DrawLine(this->matsurface->ThisPtr(), x0, y0, x1, y1);
}

bool Surface::Init() {
	this->matsurface = Interface::Create(this->Name(), "VGUI_Surface031", false);
	if (this->matsurface) {
		this->DrawSetColor = matsurface->Original<_DrawSetColor>(Offsets::DrawSetColor);
		this->DrawFilledRect = matsurface->Original<_DrawFilledRect>(Offsets::DrawFilledRect);
		this->DrawColoredCircle = matsurface->Original<_DrawColoredCircle>(Offsets::DrawColoredCircle);
		this->DrawLine = matsurface->Original<_DrawLine>(Offsets::DrawLine);
		this->DrawSetTextFont = matsurface->Original<_DrawSetTextFont>(Offsets::DrawSetTextFont);
		this->DrawSetTextColor = matsurface->Original<_DrawSetTextColor>(Offsets::DrawSetTextColor);
		this->GetFontTall = matsurface->Original<_GetFontTall>(Offsets::GetFontTall);
		this->DrawColoredText = matsurface->Original<_DrawColoredText>(Offsets::DrawColoredText);
		this->DrawTextLen = matsurface->Original<_DrawTextLen>(Offsets::DrawTextLen);
		this->GetKernedCharWidth = matsurface->Original<_GetKernedCharWidth>(Offsets::GetKernedCharWidth);
		this->GetFontName = matsurface->Original<_GetFontName>(Offsets::GetFontName);

		this->DrawSetTextureFile = matsurface->Original<_DrawSetTextureFile>(Offsets::DrawSetTextureFile);
		this->DrawSetTextureRGBA = matsurface->Original<_DrawSetTextureRGBA>(Offsets::DrawSetTextureRGBA);
		this->DrawSetTexture = matsurface->Original<_DrawSetTexture>(Offsets::DrawSetTexture);
		this->DrawGetTextureSize = matsurface->Original<_DrawGetTextureSize>(Offsets::DrawGetTextureSize);
		this->DrawTexturedRect = matsurface->Original<_DrawTexturedRect>(Offsets::DrawTexturedRect);
		this->IsTextureIDValid = matsurface->Original<_IsTextureIDValid>(Offsets::IsTextureIDValid);
		this->CreateNewTextureID = matsurface->Original<_CreateNewTextureID>(Offsets::CreateNewTextureID);

		auto PaintTraverseEx = matsurface->Original(Offsets::PaintTraverseEx);
		this->StartDrawing = Memory::Read<_StartDrawing>(PaintTraverseEx + Offsets::StartDrawing);
		this->FinishDrawing = Memory::Read<_FinishDrawing>(PaintTraverseEx + Offsets::FinishDrawing);

		// finding m_FontAmalgams pointer from CMatSystemSurface::GetFontName
		using _FontManager = void*(*)();
		_FontManager FontManager = Memory::Read<_FontManager>((uintptr_t)this->GetFontName + Offsets::FontManager);
		m_FontAmalgams = ((CUtlVector<CFontAmalgam> *)FontManager());
	}

	return this->hasLoaded = this->matsurface;
}
void Surface::Shutdown() {
	Interface::Delete(this->matsurface);
}

Surface *surface;
