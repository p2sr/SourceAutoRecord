#include "ThinkingWithTimeMachine.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

ThinkingWithTimeMachine::ThinkingWithTimeMachine() {
	this->version = SourceGame_ThinkingWithTimeMachine;
	Game::maps = {
		{"tm_intro_01",          "Intro"},
		{"tm_training_01b",      "Training"},
		{"tm_map_01b",           "Bridge"},
		{"tm_scene_map-update2", "Robots"},
		{"tm_map_02b",           "Lasers"},
		{"tm_map_03b",           "Three Cubes"},
		{"tm_map_04a",           "Laser Over Goo"},
		{"tm_map_05a-update2",   "Ceiling Buttons"},
		{"tm_map_06a",           "Catapults"},
		{"tm_map_07",            "Fizzlers"},
		{"tm_map_08",            "Buttons"},
		{"tm_map_final",         "Finale"},
	};

	Game::mapNames = {};
	for (const auto &map : Game::maps) {
		Game::mapNames.push_back(map.fileName);
	}
}
void ThinkingWithTimeMachine::LoadOffsets() {
	using namespace Offsets;

	#include "Offsets/Portal 2 5723.hpp"
}
const char *ThinkingWithTimeMachine::Version() {
	return "Thinking with Time Machine (5723)";
}
const char *ThinkingWithTimeMachine::GameDir() {
	return "Thinking with Time Machine";
}
