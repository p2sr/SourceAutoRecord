#pragma once
#include "Modules/Console.hpp"

#include "Patterns.hpp"
#include "Utils.hpp"

#define SAR_VERSION "1.7-linux"
#define SAR_BUILD __TIME__ " " __DATE__
#define SAR_COLOR Color(247, 235, 69)
#define COL_ACTIVE Color(110, 247, 76)
#define COL_DEFAULT Color(255, 255, 255, 255)

namespace SAR
{
	ScanResult Find(const char* pattern)
	{
		auto result = Scan(Patterns::Get(pattern));
		if (result.Found) {
			Console::DevMsg("%s\n", result.Message);
		}
		else {
			Console::DevWarning("%s\n", result.Message);
		}
		return result;
	}
}