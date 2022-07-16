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

	std::vector<Achievement>& GetAchievements() {
		static std::vector<Achievement> gAchievements;

		if (gAchievements.size() > 0) return gAchievements;
		
		// TODO - move these to corresponding game class' constructors once they're cleaned up.

		if (sar.game->Is(SourceGame_Portal2)) gAchievements = {
			{"ACH.SURVIVE_CONTAINER_RIDE", "Wake Up Call", false},
			{"ACH.WAKE_UP", "You Monster", false},
			{"ACH.LASER", "Undiscouraged", false},
			{"ACH.BRIDGE", "Bridge Over Troubling Water", false},
			{"ACH.BREAK_OUT", "SaBOTour", false},
			{"ACH.STALEMATE_ASSOCIATE", "Stalemate Associate", false},
			{"ACH.ADDICTED_TO_SPUDS", "Tater Tote", false},
			{"ACH.BLUE_GEL", "Vertically Unchallenged", false},
			{"ACH.ORANGE_GEL", "Stranger Than Friction", false},
			{"ACH.WHITE_GEL", "White Out", false},
			{"ACH.TRACTOR_BEAM", "Tunnel of Funnel", false},
			{"ACH.TRIVIAL_TEST", "Dual Pit Experiment", false},
			{"ACH.WHEATLEY_TRIES_TO", "The Part Where He Kills You", false},
			{"ACH.SHOOT_THE_MOON", "Lunacy", false},
			{"ACH.BOX_HOLE_IN_ONE", "Drop Box", false},
			{"ACH.SPEED_RUN_LEVEL", "Overclocker", false},
			{"ACH.COMPLIANT", "Pit Boss", false},
			{"ACH.SAVE_CUBE", "Preservation of Mass", false},
			{"ACH.LAUNCH_TURRET", "Pturretdactyl", false},
			{"ACH.CLEAN_UP", "Final Transmission", false},
			{"ACH.REENTER_TEST_CHAMBERS", "Good Listener", false},
			{"ACH.NOT_THE_DROID", "Scanned Alone", false},
			{"ACH.SAVE_REDEMPTION_TURRET", "No Hard Feelings", false},
			{"ACH.CATCH_CRAZY_BOX", "Schrodinger's Catch", false},
			{"ACH.NO_BOAT", "Ship Overboard", false},
			{"ACH.A3_DOORS", "Door Prize", false},
			{"ACH.PORTRAIT", "Portrait of a Lady", false},
			{"ACH.DEFIANT", "You Made Your Point", false},
			{"ACH.BREAK_MONITORS", "Smash TV", false},
			{"ACH.HI_FIVE_YOUR_PARTNER", "High Five", true},
			{"ACH.TEAM_BUILDING", "Team Building", true},
			{"ACH.MASS_AND_VELOCITY", "Confidence Building", true},
			{"ACH.HUG_NAME", "Bridge Building", true},
			{"ACH.EXCURSION_FUNNELS", "Obstacle Building", true},
			{"ACH.NEW_BLOOD", "You Saved Science", true},
			{"ACH.NICE_CATCH", "Iron Grip", true},
			{"ACH.TAUNTS", "Gesticul-8", true},
			{"ACH.YOU_MONSTER", "Can't Touch This", true},
			{"ACH.PARTNER_DROP", "Empty Gesture", true},
			{"ACH.PARTY_OF_THREE", "Party of Three", true},
			{"ACH.PORTAL_TAUNT", "Narbacular Drop", true},
			{"ACH.TEACHER", "Professor Portal", true},
			{"ACH.WITH_STYLE", "Air Show", true},
			{"ACH.LIMITED_PORTALS", "Portal Conservation Society", true},
			{"ACH.FOUR_PORTALS", "Four Ring Circus", true},
			{"ACH.SPEED_RUN_COOP", "Triple Crown", true},
			{"ACH.STAYING_ALIVE", "Still Alive", true},
			{"ACH.TAUNT_CAMERA", "Asking for Trouble", true},
			{"ACH.ROCK_CRUSHES_ROBOT", "Rock Portal Scissors", true},
			{"ACH.SPREAD_THE_LOVE", "Friends List With Benefits", true},
			{"ACH.SUMMER_SALE", "Talent Show", true}
		};

		else if (sar.game->Is(SourceGame_PortalStoriesMel)) gAchievements = {
			{"ACH.A3_DOORS", "Welcome to Aperture"},
			{"ACH.ADDICTED_TO_SPUDS", "Long-Term Relaxation"},
			{"ACH.BLUE_GEL", "Voices from Above"},
			{"ACH.SUMMER_SALE", "Firefighting 101"},
			{"ACH.BREAK_MONITORS", "Back on Track"},
			{"ACH.WHITE_GEL", "Persistent"},
			{"ACH.WHEATLEY_TRIES_TO", "Testing the Waters"},
			{"ACH.WAKE_UP", "Forever Alone"},
			{"ACH.BREAK_OUT", "Deja-Vu"},
			{"ACH.BRIDGE", "Organic Complications"},
			{"ACH.YOU_MONSTER", "Back off track"},
			{"ACH.CATCH_CRAZY_BOX", "Welcome to my Domain"},
			{"ACH.CLEAN_UP", "System Shutdown"},
			{"ACH.TRIVIAL_TEST", "Under the Stairs"},
			{"ACH.TRACTOR_BEAM", "You Shouldn't be Here"},
			{"ACH.SURVIVE_CONTAINER_RIDE", "Single rainbow"},
			{"ACH.STALEMATE_ASSOCIATE", "Burned in Goo"},
			{"ACH.SPEED_RUN_LEVEL", "Crushed"},
			{"ACH.SHOOT_THE_MOON", "Shot"},
			{"ACH.SAVE_REDEMPTION_TURRET", "Electrocution"},
			{"ACH.ORANGE_GEL", "In the Vents"},
			{"ACH.TEAM_BUILDING", "Beyond your Range of Hearing"},
			{"ACH.NO_BOAT", "Into Darkness"},
			{"ACH.PORTRAIT", "Ignorant"},
			{"ACH.HI_FIVE_YOUR_PARTNER", "Curious"},
			{"ACH.REENTER_TEST_CHAMBERS", "Determined"},
			{"ACH.NOT_THE_DROID", "2056"},
			{"ACH.HUG_NAME", "Story Shutdown"}
		};
		return gAchievements;
	}
	

	int GetClaimedAchievementsCount() {
		int claimedCount = 0;
		for (const Achievement& achv : GetAchievements()) {
			if (achv.coop && sar_achievement_tracker_ignore_coop.GetBool()) continue;
			if (achv.claimed) claimedCount++;
		}
		return claimedCount;	
	}

	int GetAllAchievementsCount() {
		int count = 0;
		for (const Achievement &achv : GetAchievements()) {
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
		for (Achievement &achv : GetAchievements()) {
			if (achv.keyName != achievement) continue;

			if (achv.claimed) break;

			achv.claimed = true;

			std::string toastText = Utils::ssprintf(
				"Got an achievement: %s (%d/%d)", 
				achv.displayName, 
				GetClaimedAchievementsCount(), 
				GetAllAchievementsCount()
			);

			toastHud.AddToast(ACH_TRACKER_TOAST_TAG, toastText);

			break;
		}
	}

	void ResetAchievements() {
		for (Achievement &achv : GetAchievements()) {
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

	console->Print("Remaining achievements:\n");
	for (AchievementTracker::Achievement &achv : AchievementTracker::GetAchievements()) {
		if (achv.claimed) continue;
		console->Print(" - %s\n", achv.displayName);
	}
}

CON_COMMAND(sar_achievement_tracker_reset, "sar_achievement_tracker_reset - resets the status of achievement tracker.") {
	AchievementTracker::ResetAchievements();
}
