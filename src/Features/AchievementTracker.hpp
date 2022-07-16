#pragma once

#include "Game.hpp"
#include "Utils/SDK/KeyValues.hpp"
#include "Modules/Engine.hpp"

namespace AchievementTracker {
	int GetClaimedAchievementsCount();
	int GetAllAchievementsCount();
	void CheckKeyValuesForAchievement(KeyValues* pKeyValues);
	void ClaimAchievement(std::string achievement);
	void ResetAchievements();
}
