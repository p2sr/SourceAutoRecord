#include "ReloadedFix.hpp"

#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Client.hpp"
#include "Utils/SDK.hpp"
#include "SAR.hpp"

Variable sar_fix_reloaded_cheats(
	"sar_fix_reloaded_cheats", "1", 0, 1,
	"Overrides map execution of specific console commands in Reloaded in order to separate map usage from player usage for these commands.\n"
);

ReloadedFix *reloadedFix;

ReloadedFix::ReloadedFix() {
	this->hasLoaded = true;
}

// Cancels execution of certain event calls in Reloaded and replaces it with custom behaviour
void ReloadedFix::OverrideInput(const char *className, const char *inputName, variant_t *parameter) {
	// only apply input override for Reloaded
	if (!sar.game->Is(SourceGame_PortalReloaded)) return;

	if (!sar_fix_reloaded_cheats.GetBool()) return;

	std::string paramStr = parameter->ToString();

	// check commands
	if (!strcmp(className, "point_servercommand") && !strcmp(inputName, "Command")) {
		if (paramStr.find("clear") == 0) {
			parameter->iszVal = "";
		} else if (paramStr.find("sv_cheats") == 0) {
			parameter->iszVal = "";
			inMapCheatsEnabled = paramStr.size() > 10 && paramStr[10] != '0';
		} else if (paramStr.find("change_portalgun_linkage_id") == 0) {
			parameter->iszVal = "";
			if (inMapCheatsEnabled) {
				std::string new_cmd = Utils::ssprintf("sv_cheats 1; %s; sv_cheats %s", paramStr.c_str(), sv_cheats.GetString());
				engine->Cbuf_AddText(2, new_cmd.c_str(), 0); // cbuf 2 = CBUF_SERVER; this is what a point_servercommand normally does
			}
		}
	}

	// override the vscript function where change_portalgun_linkage_id is used
	if (!strcmp(inputName, "RunScriptCode") && !paramStr.compare("setPortalID()")) {
		parameter->iszVal = "";
		CustomSetPortalID();
	}
}


// this is a rewrite of a vscript function implemented in Reloaded with "fixed" change_portalgun_linkage_id
void ReloadedFix::CustomSetPortalID() {
	auto player = client->GetPlayer(GET_SLOT() + 1);
	if (!player) return;

	auto pos = client->GetAbsOrigin(player);

	int linkageId = (pos.y < 0) ? 0 : 1;
	char linkageCmd[128];
	snprintf(linkageCmd, sizeof(linkageCmd), "sv_cheats 1;change_portalgun_linkage_id %d;sv_cheats %s", linkageId, sv_cheats.GetString());
	if (inMapCheatsEnabled) engine->ExecuteCommand(linkageCmd);

	if (pos.y < 0) {
		engine->ExecuteCommand("ent_fire @present_teleport Trigger");
	} else {
		engine->ExecuteCommand("ent_fire @future_teleport Trigger");
	}

	engine->ExecuteCommand("ent_fire @draw_ghosting Trigger");
}
