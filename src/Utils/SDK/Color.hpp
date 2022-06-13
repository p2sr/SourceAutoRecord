#pragma once

#include <cstdint>
#include <cstring>

struct Color {
	Color()
		: Color(0, 0, 0) {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	uint8_t r, g, b, a;
	inline bool operator==(const Color col) const {
		return !memcmp(this, &col, sizeof col);
	}
	inline bool operator!=(const Color col) const {
		return memcmp(this, &col, sizeof col);
	}
};

enum class TextColor {
	PLAYERNAME = 3,
	GREEN = 4,
	LIGHT_GREEN = 5,
	ORANGE = 6,
};
