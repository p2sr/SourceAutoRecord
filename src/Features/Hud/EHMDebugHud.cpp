#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Features/EntityList.hpp"
#include "Variable.hpp"
#include "Command.hpp"
#include "Event.hpp"
#include "Hud.hpp"

Variable sar_ehm_hud("sar_ehm_hud", "0", "Enables EHM debug HUD.\n");

Variable sar_ehm_hud_x("sar_ehm_hud_x", "-10", "The X position of the EHM debug HUD.\n", 0);
Variable sar_ehm_hud_y("sar_ehm_hud_y", "10", "The Y position of the EHM debug HUD.\n", 0);
Variable sar_ehm_hud_font("sar_ehm_hud_font", "2", "Font used by the EHM debug HUD.\n", 0);

Variable sar_ehm_hud_list_length("sar_ehm_hud_list_length", "20", "How many slots to show in the EHM debug HUD.\n", 0);

Variable sar_ehm_hud_autofill("sar_ehm_hud_autofill", "1", 
	"Whether to listen for changed slot and use it to replace the oldest one in EHM debug HUD.\n");

struct SlotInfoRecord {
	int slot;
	float lifetime;
	const char *lastClassname;
	bool isActive;
};

std::vector<SlotInfoRecord> g_slotStates;
std::vector<int> g_hudSlots;

static bool isSlotDisplayed(int slot) {
	for (int i = 0; i < g_hudSlots.size(); i++) {
		if (g_hudSlots[i] == slot) {
			return true;
		}
	}
	return false;
}

static void pushHudSlotOnTop(int slot) {
	if (isSlotDisplayed(slot)) return;

	g_hudSlots.insert(g_hudSlots.begin(), slot);
	if (g_hudSlots.size() >= sar_ehm_hud_list_length.GetInt()) {
		g_hudSlots.pop_back();
	}
}

static void swapOldestSlotWith(int slot) {
	if (isSlotDisplayed(slot)) return;

	if (g_hudSlots.size() < sar_ehm_hud_list_length.GetInt()) {
		g_hudSlots.insert(g_hudSlots.begin(), slot);
		return;
	}

	int oldest_index = -1;
	float oldest_lifetime = -1.0f;

	for (int i = 0; i < g_hudSlots.size(); i++) {
		float lifetime = g_slotStates[g_hudSlots[i]].lifetime;
		if (lifetime > oldest_lifetime) {
			oldest_index = i;
			oldest_lifetime = lifetime;
		}
	}

	if (oldest_index != -1) {
		g_hudSlots[oldest_index] = slot;
	}
}

static void handleSlotRecords() {
	bool fresh = false;
	if (g_slotStates.size() != Offsets::NUM_ENT_ENTRIES) {
		g_slotStates.resize(Offsets::NUM_ENT_ENTRIES);
		fresh = true;
	}

	for (int i = Offsets::NUM_ENT_ENTRIES - 1; i >= 0; i--) {
		SlotInfoRecord *record = &g_slotStates[i];
		
		auto info = entityList->GetEntityInfoByIndex(i);

		bool active = info->m_pEntity != nullptr;
		bool slotChanged = record->slot != info->m_SerialNumber;
		bool activeStateChanged = record->isActive != active;

		if (fresh || slotChanged || activeStateChanged) {
			record->slot = info->m_SerialNumber;
			record->lifetime = 0.0f;
			record->isActive = active;
			record->lastClassname = active
				? server->GetEntityClassName(info->m_pEntity)
				: g_slotStates[i].lastClassname;

			if (!fresh && sar_ehm_hud_autofill.GetBool()) {
				swapOldestSlotWith(i);
			}

			continue;
		}

		g_slotStates[i].lifetime += 1.0f / 60.0f;
	}
}

static void clearSlotRecords() {
	if (g_slotStates.size() > 0) {
		g_slotStates.clear();
	}
}

static void handleHudSlotsResizing() {
	while (g_hudSlots.size() < sar_ehm_hud_list_length.GetInt()) {
		int slotToAssign = (g_hudSlots.size() > 0) ? (g_hudSlots[g_hudSlots.size() - 1] + 1) : 0;
		slotToAssign %= Offsets::NUM_ENT_ENTRIES;
		g_hudSlots.push_back(slotToAssign);
	}
	if (g_hudSlots.size() > sar_ehm_hud_list_length.GetInt()) {
		g_hudSlots.resize(sar_ehm_hud_list_length.GetInt());
	}
}

ON_EVENT(POST_TICK) {
	if (!sar_ehm_hud.GetBool() || !sv_cheats.GetBool()) {
		clearSlotRecords();
		return;
	}

	handleSlotRecords();
	handleHudSlotsResizing();
}

CON_COMMAND(sar_ehm_hud_push, "sar_ehm_hud_push <slot> - push slot on top of the EHM debug HUD.\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_ehm_hud_push.ThisPtr()->m_pszHelpString);
	}

	if (!sv_cheats.GetBool()) {
		return console->Print("sv_cheats must be enabled to use this command.\n");
	}

	int slot = std::atoi(args[1]);
	pushHudSlotOnTop(slot);
}

CON_COMMAND(sar_ehm_hud_refill, "sar_ehm_hud_refill [slot] - fills EHM debug HUD with slots starting from a given one.\n") {
	if (!sv_cheats.GetBool()) {
		return console->Print("sv_cheats must be enabled to use this command.\n");
	}

	int slot = args.ArgC() >= 2 ? std::atoi(args[1]) : 0;
	g_hudSlots.clear();
	for (int i = 0; i < sar_ehm_hud_list_length.GetInt(); i++) {
		g_hudSlots.push_back(slot + i);
	}
}


class EHMDebugHud : public Hud {
public:
	EHMDebugHud()
		: Hud(HudType_InGame, true) {}
	const int paddingX = 20;
	const int paddingY = 4;
	const int paddingBorder = 10;
	const char *slotHeader = "Slot ID";
	const char *serialHeader = "Serial Number";
	const char *classnameHeader = "Slot Entity Classname       ";

	bool GetCurrentSize(int &w, int &h) {
		auto font = scheme->GetFontByID(sar_ehm_hud_font.GetInt());

		int lineHeight = surface->GetFontHeight(font);
		h = (lineHeight + paddingY) * (sar_ehm_hud_list_length.GetInt() + 1) + paddingBorder * 2;
		w = paddingBorder * 2
			+ surface->GetFontLength(font, slotHeader) + paddingX
			+ surface->GetFontLength(font, serialHeader) + paddingX
			+ surface->GetFontLength(font, classnameHeader);

		return true;
	}

	void Paint(int slot) override {
		if (!sar_ehm_hud.GetBool() || !sv_cheats.GetBool()) return;

		auto font = scheme->GetFontByID(sar_ehm_hud_font.GetInt());

		int hudX = PositionFromString(sar_ehm_hud_x.GetString(), true);
		int hudY = PositionFromString(sar_ehm_hud_y.GetString(), false);

		int width, height;
		GetCurrentSize(width, height);

		surface->DrawRect(Color(0, 0, 0, 192), hudX, hudY, hudX + width, hudY + height);

		int lineHeight = surface->GetFontHeight(font) + paddingY;
		int slotHeaderX = paddingBorder;
		int serialHeaderX = slotHeaderX + surface->GetFontLength(font, slotHeader) + paddingX;
		int classnameHeaderX = serialHeaderX + surface->GetFontLength(font, serialHeader) + paddingX;

		surface->DrawTxt(font, hudX + slotHeaderX, hudY + paddingBorder, Color(255, 255, 255, 255), slotHeader);
		surface->DrawTxt(font, hudX + serialHeaderX, hudY + paddingBorder, Color(255, 255, 255, 255), serialHeader);
		surface->DrawTxt(font, hudX + classnameHeaderX, hudY + paddingBorder, Color(255, 255, 255, 255), classnameHeader);

		for (int i = 0; i < g_hudSlots.size(); ++i) {
			int slot = g_hudSlots[i];
			auto slotRecord = g_slotStates[slot];
			auto info = entityList->GetEntityInfoByIndex(slot);

			int recordPosY = hudY + paddingBorder + paddingY + lineHeight * (i + 1);

			float factor = std::fminf(1.0f, slotRecord.lifetime);

			Color serialColor = slotRecord.isActive 
				? Color(255, 255, 255, 128) 
				: Color(255, 255, 255 * factor, 255 - 127 * factor);

			Color classnameColor = slotRecord.isActive
				? Color(128 + 127 * factor, 255, 255 * factor, 128)
				: Color(255, 128 + 127 * factor, 255 * factor, 32);

			surface->DrawTxt(font, hudX + slotHeaderX, recordPosY, Color(255, 255, 255, 128), "%d", slot);
			surface->DrawTxt(font, hudX + serialHeaderX, recordPosY, serialColor, "%d", info->m_SerialNumber);
			surface->DrawTxt(font, hudX + classnameHeaderX, recordPosY, classnameColor, "%s", slotRecord.lastClassname);
		}
	}
};

static EHMDebugHud ehmDebugHud;
