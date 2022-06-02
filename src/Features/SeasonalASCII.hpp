#pragma once
#include "Utils/SDK.hpp"
#include <vector>
#include <thread>

struct SeasonalASCIIArt {
	std::vector<Color> colors;
	const char *message;

	int year;
	int month;
	int day;
	
	bool IsItTimeForIt();
	void Display();
};

namespace SeasonalASCII {
	void Init();
	SeasonalASCIIArt *GetCurrentArt();
}
