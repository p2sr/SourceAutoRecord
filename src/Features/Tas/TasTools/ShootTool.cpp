#include "ShootTool.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/EntityList.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasPlayer.hpp"

ShootTool tasShootTool[2] = { {0}, {1} };

void ShootTool::Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo) {
	if (params.enabled) {
		 
		void *player = server->GetPlayer(pInfo.slot + 1);

		if (player == nullptr || (int)player == -1) return;

		uintptr_t portalgun = (uintptr_t)entityList->LookupEntity(SE(player)->active_weapon());
		if (!portalgun) return;

		float nextPrimaryAttack = SE(portalgun)->field<float>("m_flNextPrimaryAttack");

		float currTime = pInfo.tick * pInfo.ticktime;

		bool canShoot = (nextPrimaryAttack <= currTime) || params.hold;
		bulk.buttonStates[params.shotPortal] = canShoot;

		if (!params.spam && !params.hold) {
			Reset();
		}
	}
}

std::shared_ptr<TasToolParams> ShootTool::ParseParams(std::vector<std::string> vp) {
	if (vp.size() < 1 || vp.size() > 2)
		throw TasParserException(Utils::ssprintf("Wrong argument count for tool %s: %d", this->GetName(), vp.size()));

	bool enabled = true;
	TasControllerInput button;
	bool hold = false;
	bool spam = false;

	for (std::string param : vp) {
		if (param == "off" || param == "stop") {
			enabled = false;
		}
		else if (param == "blue") {
			button = TasControllerInput::FireBlue;
		}
		else if (param == "orange") {
			button = TasControllerInput::FireOrange;
		} 
		else if (param == "hold") {
			hold = true;
		} 
		else if (param == "spam") {
			spam = true;
		} 
		else {
			throw TasParserException(Utils::ssprintf("Incorrect parameter for tool %s: %s", this->GetName(), param.c_str()));
		}
	}

	return std::make_shared<ShootToolParams>(enabled, button, spam, hold);
}
