#include "ScrollSpeed.hpp"

#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

#define CONSECUTIVE_END 5
#define TOTAL_THRESHOLD 5

Variable sar_scrollspeed("sar_scrollspeed", "0", "Show a HUD indicating your scroll speed.");

int g_total[2];
int g_scrolls[2];
int g_lastScroll[2];

float g_lastSps[2];

bool ScrollSpeedHud::ShouldDraw() {
	return sar_scrollspeed.GetBool();
}

void ScrollSpeedHud::Paint(int slot) {
	float sps = g_lastSps[slot];

	int screenWidth, screenHeight;
	engine->GetScreenSize(nullptr, screenWidth, screenHeight);

	int x = 150;
	int y = screenHeight / 2 - 100;

	surface->DrawRect(Color{75, 75, 75, 255}, x - 1, y - 1, x + 51, y + 201);
	surface->DrawRect(Color{255, 100, 100, 255}, x, y, x + 50, y + 100);
	surface->DrawRect(Color{100, 255, 100, 255}, x, y + 100, x + 50, y + 200);

	int height = 200 - (sps * 200.0f / 60.0f);

	surface->DrawRect(Color{255, 255, 255, 255}, x, y + height - 1, x + 50, y + height + 2);
}

ON_EVENT(SESSION_START) {
	for (int slot = 0; slot < 2; ++slot) {
		g_total[slot] = 0;
		g_scrolls[slot] = 0;
		g_lastScroll[slot] = -100;
		g_lastSps[slot] = 0;
	}
}

ON_EVENT(POST_TICK) {
	for (int slot = 0; slot < 2; ++slot) {
		// Cache the sps iff total > TOTAL_THRESHOLD
		if (g_total[slot] > TOTAL_THRESHOLD) g_lastSps[slot] = (float)g_scrolls[slot] / g_total[slot] * 60.0f;
	}
}

void ScrollSpeedHud::OnJump(int slot) {
	int tick = session->GetTick();

	// Reset if it's been long enough
	if (tick < g_lastScroll[slot] || tick > g_lastScroll[slot] + CONSECUTIVE_END) {
		g_total[slot] = 1;
		g_scrolls[slot] = 0;
	} else {
		g_total[slot] += tick - g_lastScroll[slot];
	}

	// Count the scroll
	g_lastScroll[slot] = tick;
	++g_scrolls[slot];
}

ScrollSpeedHud scrollSpeedHud;
