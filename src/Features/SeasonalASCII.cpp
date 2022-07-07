#include "SeasonalASCII.hpp"

#include "Variable.hpp"
#include "Modules/Console.hpp"

#include <ctime>

// defining some ascii arts

#define RED Color{255,0,0}
#define GREEN Color{25,160,0}
#define BLUE Color{0,10,170}
#define ORANGE Color{255,155,0}
#define PURPLE Color{160,0,150}
#define YELLOW Color{230,255,0}
#define BROWN Color{70,40,30}
#define WHITE Color{255,255,255}
#define BLACK Color{0,0,0}
#define PINK Color{255,160,220}
#define AQUA Color{120,220,255}

static std::vector<SeasonalASCIIArt> g_seasonalAsciiArts = {

// Pride month
{{SAR_PRINT_COLOR, RED, BLACK, BROWN, ORANGE, AQUA, PINK, YELLOW, GREEN, BLUE, PURPLE, WHITE},
"      \n"
"     $c{1}########################################\n"
"     $c{2}###$c{1}#####################################\n"
"     $c{3}###$c{2}###$c{4}##################################\n"
"     $c{5}###$c{3}###$c{2}###$c{4}###############################\n"
"     $c{6}###$c{5}###$c{3}###$c{2}###$c{7}############################\n"
"     $c{11}###$c{6}###$c{5}###$c{3}###$c{2}###$c{7}#########################\n"
"     $c{11}###$c{6}###$c{5}###$c{3}###$c{2}###$c{8}#########################\n"
"     $c{6}###$c{5}###$c{3}###$c{2}###$c{8}############################\n"
"     $c{5}###$c{3}###$c{2}###$c{9}###############################\n"
"     $c{3}###$c{2}###$c{9}##################################\n"
"     $c{2}###$c{10}#####################################\n"
"     $c{10}########################################\n"
"      \n" 
"$c{0}Happy pride month to all members of the $c{1}L$c{4}G$c{7}B$c{8}T$c{9}Q$c{10}+ $c{0}community!\n"
, 0, 6, 0},

// put more ascii arts here

};

#undef RED
#undef GREEN
#undef BLUE
#undef ORANGE
#undef PURPLE
#undef YELLOW
#undef BROWN
#undef WHITE
#undef BLACK
#undef PINK
#undef AQUA


static std::thread g_prideThread;

// is it time?!
bool SeasonalASCIIArt::IsItTimeForIt() {
	time_t t = time(NULL);
	struct tm *ltime = localtime(&t);

	if (year > 0 && ltime->tm_year + 1900 != year) return false;
	if (month > 0 && ltime->tm_mon + 1 != month) return false;
	if (day > 0) {
		if (month > 0 && ltime->tm_mday != day) return false;
		if (month <= 0 && ltime->tm_yday + 1 != day) return false;
    }

	// it is time!
	return true;
}


void SeasonalASCII::Init() {
	auto asciiArt = GetCurrentArt();
	if (asciiArt != nullptr) {
		g_prideThread = std::thread([](SeasonalASCIIArt* art) {
			GO_THE_FUCK_TO_SLEEP(3000);
			art->Display();
		}, asciiArt);
		g_prideThread.detach();
	}
}

SeasonalASCIIArt *SeasonalASCII::GetCurrentArt() {
	for (auto &art : g_seasonalAsciiArts) {
		if (art.IsItTimeForIt()) {
			return &art;
		}
	}
    return nullptr;
}

void SeasonalASCIIArt::Display() {
	std::string art(message);

	Color currentColor{255, 255, 255};
	unsigned messageBegin = 0;

	std::vector<std::pair<Color, std::string>> convertedMessages;

	for (unsigned i = 0; i < art.size(); i++) {
		if (art[i] != '$' || i >= art.size() - 4 || art[i + 1] != 'c' || art[i + 2] != '{') continue;

		unsigned numStart = i + 3;
		unsigned numSize = 0;
		while (numStart + numSize < art.size() && art[numStart + numSize] != '}') numSize++;
		if (art[numStart + numSize] != '}' || numSize==0) continue;

		// new color code! store previous message if needed
		if (i > messageBegin) {
			std::string printMessage = art.substr(messageBegin, i - messageBegin);
			convertedMessages.push_back({currentColor, printMessage});
		}
		i += numSize + 3;
		messageBegin = i+1;

		// save new color code
		int colorNum = std::stoi(art.substr(numStart, numSize));
		if (colorNum < 0 || (unsigned)colorNum >= colors.size()) colorNum = 0;
		currentColor = colors[colorNum];
	}

	// store last message
	std::string lastMessage = art.substr(messageBegin, art.size() - messageBegin);
	convertedMessages.push_back({currentColor, lastMessage});

	// print all messages
	for (auto const &msg : convertedMessages) {
		THREAD_PRINTCOLOR(msg.first, msg.second.c_str());
	}
}
