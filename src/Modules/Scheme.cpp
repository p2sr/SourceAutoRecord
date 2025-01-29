#include "Scheme.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Surface.hpp"

unsigned long Scheme::GetFontByID(int id) {
	unsigned long defaultFont = GetDefaultFont();
	unsigned long font = defaultFont + id;
	return surface->IsFontValid(font) ? font : defaultFont;
}

// Find a font by name and size. If you can't find the exact size, return the nearest one.
// If you can't find the font at all, return the default font.
unsigned long Scheme::FindFont(const char *fontName, int size) {
	if (surface->m_FontAmalgams == nullptr) return GetDefaultFont();
	int fontCount = surface->m_FontAmalgams->m_Size;

	int nearestID = -1;
	int nearestSize = -1;
	for (int i = 0; i < fontCount; i++) {
		if (surface->IsFontValid(i)) {
			const char *name = surface->GetFontName(surface->matsurface->ThisPtr(), i);
			if (strcmp(name, fontName) == 0) {
				int curSize = surface->m_FontAmalgams->m_pElements[i].m_iMaxHeight;
				if (curSize == size) {
					return i;
				} else if (nearestSize == -1 || (abs(curSize - size) < abs(nearestSize - size))) {
					nearestID = i;
					nearestSize = curSize;
				}
			}
		}
	}

	if (nearestID != -1) {
		return nearestID;
	}

	return GetDefaultFont();
}

unsigned long Scheme::GetDefaultFont() {
	return this->GetFont(this->g_pScheme->ThisPtr(), "DefaultFixedOutline", 0);
}
bool Scheme::Init() {
	if (auto g_pVGuiSchemeManager = Interface::Create(this->Name(), "VGUI_Scheme010", false)) {
		using _GetIScheme = void *(__rescall *)(void *thisptr, unsigned long scheme);
		auto GetIScheme = g_pVGuiSchemeManager->Original<_GetIScheme>(Offsets::GetIScheme);

		// Default scheme is 1
		this->g_pScheme = Interface::Create(GetIScheme(g_pVGuiSchemeManager->ThisPtr(), 1));
		if (this->g_pScheme) {
			this->GetFont = this->g_pScheme->Original<_GetFont>(Offsets::GetFont);
		}
		Interface::Delete(g_pVGuiSchemeManager);
	}

	return this->hasLoaded = this->g_pScheme;
}
void Scheme::Shutdown() {
	Interface::Delete(this->g_pScheme);
}

Scheme *scheme;
