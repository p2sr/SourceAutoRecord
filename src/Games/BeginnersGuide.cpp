#include "BeginnersGuide.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

BeginnersGuide::BeginnersGuide() {
	this->version = SourceGame_BeginnersGuide;
	Game::maps = {
		{"intro",       "Intro"},
		{"whisper",     "Whisper"},
		{"backwards",   "Backwards"},
		{"entering",    "Entering"},
		{"stairs",      "Stairs"},
		{"puzzle",      "Puzzle"},
		{"exiting",     "Exiting"},
		{"down",        "Down"},
		{"notes",       "Notes"},
		{"escape",      "Escape"},
		{"house",       "House"},
		{"lecturer",    "Lecture"},
		{"theater",     "Theater"},
		{"mobius",      "Mobius"},
		{"presence",    "Island"},
		{"machine",     "Machine"},
		{"tower",       "Tower"},
		{"nomansland1", "Epilogue 1"},
		{"nomansland2", "Epilogue 2"},
		{"maze2",       "Epilogue 3"},
		{"menu",        "Main Menu"},
		{"menu2",       "Main Menu"},
		{"menu3",       "Main Menu"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}

	Game::achievements = {};
}
void BeginnersGuide::LoadOffsets() {
	using namespace Offsets;

	#include "Offsets/Beginners Guide 6167.hpp"
}
const char *BeginnersGuide::Version() {
	return "Beginner's Guide (6167)";
}
const char *BeginnersGuide::ModDir() {
	return "beginnersguide";
}
