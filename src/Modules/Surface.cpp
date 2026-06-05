#include "Surface.hpp"

#include "Command.hpp"
#include "Console.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Scheme.hpp"
#include "InputSystem.hpp"
#include "VGui.hpp"
#include "Engine.hpp"

#include <stdarg.h>

REDECL(Surface::LockCursor);

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
		length += floor(wide + 0.6f);
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

void Surface::DrawTxt(HFont font, const Vector2<int> &center, Color clr, const std::string& text) {
	this->DrawTxt(font, center.x, center.y, clr, text.c_str());
}

void Surface::DrawRect(Color clr, int x0, int y0, int x1, int y1) {
	this->DrawSetColor(this->matsurface->ThisPtr(), clr.r, clr.g, clr.b, clr.a);
	this->DrawFilledRect(this->matsurface->ThisPtr(), x0, y0, x1, y1);
}

void Surface::DrawRect(Color clr, const Vector2<int> &v0, const Vector2<int> &v1) {
	this->DrawRect(clr, v0.x, v0.y, v1.x, v1.y);
}

void Surface::DrawRect(Color clr, const Bounds<int> &bounds) {
	this->DrawRect(clr, bounds.vBegin.x, bounds.vBegin.y, bounds.vEnd.x, bounds.vEnd.y);
}

struct TriangleTexture {
  std::vector<unsigned char> pixels;
  int width;
  int height;
};

TriangleTexture GenerateTriangleTexture(
  Color clr,
  Vector2<float> a,
  Vector2<float> b,
  Vector2<float> c) {
  float minXf = std::floor(std::min({a.x, b.x, c.x}));
  float minYf = std::floor(std::min({a.y, b.y, c.y}));

  float maxXf = std::ceil(std::max({a.x, b.x, c.x}));
  float maxYf = std::ceil(std::max({a.y, b.y, c.y}));

  int w = std::max(1, (int)(maxXf - minXf));
  int h = std::max(1, (int)(maxYf - minYf));

  a.x -= minXf;
  a.y -= minYf;

  b.x -= minXf;
  b.y -= minYf;

  c.x -= minXf;
  c.y -= minYf;

  std::vector<unsigned char> pixels(w * h * 4, 0);

  auto Edge = [](const Vector2<float>& a, const Vector2<float>& b, const Vector2<float> p) {
    return (p.x - a.x) * (b.y - a.y) -
           (p.y - a.y) * (b.x - a.x);
  };

  float area = Edge(a, b, c);

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      Vector2<float> p = Vector2<float>{
        x + 0.5f,
        y + 0.5f
      };

      float w0 = Edge(b, c, p);
      float w1 = Edge(c, a, p);
      float w2 = Edge(a, b, p);

      bool inside;

      if (area > 0.0f) {
        inside = w0 >= 0.0f &&
                 w1 >= 0.0f &&
                 w2 >= 0.0f;
      } else {
        inside = w0 <= 0.0f &&
                 w1 <= 0.0f &&
                 w2 <= 0.0f;
      }

      if (inside) {
        int i = (y * w + x) * 4;
        pixels[i + 0] = clr.r;
        pixels[i + 1] = clr.g;
        pixels[i + 2] = clr.b;
        pixels[i + 3] = clr.a;
      }
    }
  }

  return {
    std::move(pixels),
    w,
    h
  };
}

void Surface::DrawTriangle(Color clr, Vector2<float> v0, Vector2<float> v1, Vector2<float> v2) {
  this->DrawSetColor(
    this->matsurface->ThisPtr(),
    clr.r,
    clr.g,
    clr.b,
    clr.a
  );

  if (v1.y < v0.y) std::swap(v0, v1);
  if (v2.y < v0.y) std::swap(v0, v2);
  if (v2.y < v1.y) std::swap(v1, v2);

  auto interpX = [](
    const Vector2<float>& a,
    const Vector2<float>& b,
    float y
  ) {
    if (std::abs(b.y - a.y) < 0.0001f) return a.x;

    return a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y);
  };

  int yStart = (int)std::ceil(v0.y);
  int yMid = (int)std::ceil(v1.y);
  int yEnd = (int)std::ceil(v2.y);

  for (int y = yStart; y < yMid; y++) {
    float x1 = interpX(v0, v2, (float)y);
    float x2 = interpX(v0, v1, (float)y);

    if (x1 > x2) std::swap(x1, x2);

    this->DrawColoredLine(
      (int)std::round(x1),
      y,
      (int)std::round(x2),
      y,
      clr
    );
  }

  for (int y = yMid; y < yEnd; y++) {
    float x1 = interpX(v0, v2, (float)y);
    float x2 = interpX(v1, v2, (float)y);

    if (x1 > x2) std::swap(x1, x2);

    this->DrawColoredLine(
      (int)std::round(x1),
      y,
      (int)std::round(x2),
      y,
      clr
    );
  }
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
void Surface::DrawRectAndCenterTxt(Color clr, const Vector2<int> &v0, const Vector2<int> &v1, HFont font, Color fontClr, const std::string &text) {
	this->DrawRectAndCenterTxt(clr, v0.x, v0.y, v1.x, v1.y, font, fontClr, text.c_str());
}

void Surface::DrawRectAndCenterTxt(Color clr, const Bounds<int> &bounds, HFont font, Color fontClr, const std::string &text) {
	this->DrawRectAndCenterTxt(clr, bounds.vBegin.x, bounds.vBegin.y, bounds.vEnd.x, bounds.vEnd.y, font, fontClr, text.c_str());
}

void Surface::DrawCircle(int x, int y, float radius, Color clr) {
	this->DrawColoredCircle(this->matsurface->ThisPtr(), x, y, radius, clr.r, clr.g, clr.b, clr.a);
}
void Surface::DrawCircle(const Vector2<int> &center, float radius, Color clr) {
	this->DrawCircle(center.x, center.y, radius, clr);
}

void Surface::DrawFilledCircle(int x, int y, float radius, Color clr) {
	const int r2 = radius * radius;

	for (int cy = -radius; cy <= radius; cy++) {
		int cx = sqrtf(r2 - cy * cy) + 0.5;
		int cyy = cy + y;

		surface->DrawColoredLine(x - cx, cyy, x + cx, cyy, clr);
	}
}
void Surface::DrawFilledCircle(const Vector2<int> &center, float radius, Color clr) {
	this->DrawFilledCircle(center.x, center.y, radius, clr);
}
void Surface::DrawColoredLine(int x0, int y0, int x1, int y1, Color clr) {
	this->DrawSetColor(this->matsurface->ThisPtr(), clr.r, clr.g, clr.b, clr.a);
	this->DrawLine(this->matsurface->ThisPtr(), x0, y0, x1, y1);
}
void Surface::DrawColoredLine(const Vector2<int> &v0, const Vector2<int> &v1, Color clr) {
	this->DrawColoredLine(v0.x, v0.y, v1.x, v1.y, clr);
}

int __rescall Surface::StartDrawingFallback(void *thisptr) {
	return 0;
}
int __cdecl Surface::FinishDrawingFallback() {
	return 0;
}

// THIS IS THE HOOK!!!!
DETOUR_T(void, Surface::LockCursor) {

  // static void* inputCtx = engine->GetInputContext(engine->engineClient->ThisPtr(), 0);
  // console->Print("%p\n", inputCtx);

  // inputSystem->SetCursorVisible(inputSystem->g_InputStackSystem->ThisPtr(), inputCtx, true);

  if (g_drawImgui) {
    surface->UnlockCursor();
    // console->Print("OMG??!!\n");

    return;
  }

  return LockCursor(thisptr);
}

bool Surface::Init() {
	this->matsurface = Interface::Create(this->Name(), "VGUI_Surface031", true);
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
    this->DrawTexturedPolygon = matsurface->Original<_DrawTexturedPolygon>(Offsets::DrawTexturedPolygon);

		this->DrawSetTextureFile = matsurface->Original<_DrawSetTextureFile>(Offsets::DrawSetTextureFile);
		this->DrawSetTextureRGBA = matsurface->Original<_DrawSetTextureRGBA>(Offsets::DrawSetTextureRGBA);
		this->DrawSetTexture = matsurface->Original<_DrawSetTexture>(Offsets::DrawSetTexture);
		this->DrawGetTextureSize = matsurface->Original<_DrawGetTextureSize>(Offsets::DrawGetTextureSize);
		this->DrawTexturedRect = matsurface->Original<_DrawTexturedRect>(Offsets::DrawTexturedRect);
		this->IsTextureIDValid = matsurface->Original<_IsTextureIDValid>(Offsets::IsTextureIDValid);
		this->CreateNewTextureID = matsurface->Original<_CreateNewTextureID>(Offsets::CreateNewTextureID);
    this->UnlockCursor = matsurface->Original<_UnlockCursor>(64);

		auto PaintTraverseEx = matsurface->Original(Offsets::PaintTraverseEx);
		this->StartDrawing = Memory::Read<_StartDrawing>(PaintTraverseEx + Offsets::StartDrawing);
		this->FinishDrawing = Memory::Read<_FinishDrawing>(PaintTraverseEx + Offsets::FinishDrawing);
		if (!Offsets::StartDrawing) {
			this->StartDrawing = Surface::StartDrawingFallback;
		}
		if (!Offsets::FinishDrawing) {
			this->FinishDrawing = Surface::FinishDrawingFallback;
		}

    // I DONT UNDERSTAND THIS SHIT!!!!
    surface->matsurface->Hook(
      LockCursor_Hook,
      LockCursor,
      65
    );

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
