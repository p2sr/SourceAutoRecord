#include "StanleyParable.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

StanleyParable::StanleyParable() {
	this->version = SourceGame_StanleyParable;
	Game::maps = {
		{"babygame",      "Baby Game"},
		{"blockbase",     "Blockbase"},
		{"buttonworld",   "Button World"},
		{"freedom",       "Freedom"},
		{"incorrect",     "Incorrect"},
		{"map",           "Map"},
		{"map_death",     "Map Death"},
		{"map_one",       "Map One"},
		{"map_two",       "Map Two"},
		{"map1",          "Map 1"},
		{"map2",          "Map 2"},
		{"redstair",      "Red Stairs"},
		{"seriousroom",   "Serious Room"},
		{"testchmb_a_00", "testchmb_a_00"},
		{"thefirstmap",   "The First Map"},
		{"theonlymap",    "The Only Map"},
		{"zending",       "Zending"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}

	Game::achievements = {
		{"TSP_888", "8888888888888888"},
		{"TSP_430", "Click on door 430 five times."},
		{"TSP_ACHIEVEMENT", "Achievement"},
		{"TSP_COMMITMENT", "Commitment"},
		{"TSP_WELCOMEBACK", "Welcome back!"},
		{"TSP_SPEED", "Speed run"},
		{"TSP_JUMPING", "You can't jump"},
		{"TSP_GOOUTSIDE", "Go outside"},
		{"TSP_BEATTHEGAME", "Beat the game"},
		{"TSP_UNACHIEVABLE", "Unachievable"},
	};
}
void StanleyParable::LoadOffsets() {
	using namespace Offsets;

	#include "Offsets/The Stanley Parable 5454.hpp"
}
const char *StanleyParable::Version() {
	return "The Stanley Parable (5454)";
}
const char *StanleyParable::ModDir() {
	return "thestanleyparable";
}
