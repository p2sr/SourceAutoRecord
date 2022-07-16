#pragma once

#include "Utils/SDK/KeyValues.hpp"
#include "Modules/Engine.hpp"

namespace AchievementTracker {
	struct Achievement {
		const char *keyName;
		const char *displayName;
		bool coop;
		bool claimed = false;
	};

	std::vector<Achievement>& GetAchievements();
	int GetClaimedAchievementsCount();
	int GetAllAchievementsCount();
	void CheckKeyValuesForAchievement(KeyValues* pKeyValues);
	void ClaimAchievement(std::string achievement);
	void ResetAchievements();
}
