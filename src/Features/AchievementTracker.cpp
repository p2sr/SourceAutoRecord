#include "AchievementTracker.hpp"

#include "Command.hpp"
#include "Variable.hpp"
#include "Modules/Console.hpp"
#include "Hud/Toasts.hpp"
#include "SAR.hpp"

#define ACH_TRACKER_TOAST_TAG "achievements"

Variable sar_achievement_tracker_show("sar_achievement_tracker_show", "0", 0, 1, 
	"Enables achievement tracker toasts (using tag \"achievements\").\n");
Variable sar_achievement_tracker_ignore_coop("sar_achievement_tracker_ignore_coop", "0", 0, 1, 
	"When set, achievement tracker ignores coop-specific achievements.\n"
);


namespace AchievementTracker {
	int GetClaimedAchievementsCount() {
		int claimedCount = 0;
		for (const AchievementData &achv : Game::achievements) {
			if (achv.coop && sar_achievement_tracker_ignore_coop.GetBool()) continue;
			if (achv.claimed) claimedCount++;
		}
		return claimedCount;
	}

	int GetAllAchievementsCount() {
		int count = 0;
		for (const AchievementData &achv : Game::achievements) {
			if (!(achv.coop && sar_achievement_tracker_ignore_coop.GetBool())) count++;
		}
		return count;
	}

	void CheckKeyValuesForAchievement(KeyValues *pKeyValues) {
		std::string kvName = pKeyValues->GetName();
		if (kvName != "write_awards") return;

		std::string achName = pKeyValues->sub->GetName();

		ClaimAchievement(achName);
	}


	void ClaimAchievement(std::string achievement) {
		for (AchievementData &achv : Game::achievements) {
			if (achv.keyName != achievement) continue;

			if (achv.claimed) break;

			achv.claimed = true;

			if (sar_achievement_tracker_show.GetBool()) {
				std::string toastText = Utils::ssprintf(
					"Got an achievement: %s (%d/%d)",
					achv.displayName,
					GetClaimedAchievementsCount(),
					GetAllAchievementsCount()
				);

				toastHud.AddToast(ACH_TRACKER_TOAST_TAG, toastText);
			}
			break;
		}
	}

	void ResetAchievements() {
		for (AchievementData &achv : Game::achievements) {
			achv.claimed = false;
		}
	}
}

CON_COMMAND(sar_achievement_tracker_status, "sar_achievement_tracker_status - shows achievement completion status.\n") {
	int claimed = AchievementTracker::GetClaimedAchievementsCount();
	int all = AchievementTracker::GetAllAchievementsCount();

	console->Print("You've collected %d achievements out of %d.%s\n", 
		claimed, all, sar_achievement_tracker_ignore_coop.GetBool() ? " (singleplayer only)" : "");

	if (claimed == all) return;

	if (claimed > 0) {
		console->Print("Collected achievements:\n");
		for (AchievementData &achv : Game::achievements) {
			if (!achv.claimed) continue;
			console->Print(" - %s\n", achv.displayName);
		}
	}

	console->Print("Remaining achievements:\n");
	for (AchievementData &achv : Game::achievements) {
		if (achv.claimed) continue;
		console->Print(" - %s\n", achv.displayName);
	}
}

CON_COMMAND(sar_achievement_tracker_reset, "sar_achievement_tracker_reset - resets the status of achievement tracker.\n") {
	AchievementTracker::ResetAchievements();
}
