#pragma once
#include "Features/Feature.hpp"
#include "Game.hpp"
#include "Utils/SDK.hpp"

#include <vector>

class ChapterMenu : public Feature {
private:
	std::vector<ChapterContextData_t> original;
	int originalNumChapters;

public:
	ChapterMenu();
	~ChapterMenu();
	void LoadMaps(SourceGameVersion version);
};

extern ChapterMenu *chapterMenu;
