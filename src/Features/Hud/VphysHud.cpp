#include "VphysHud.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/EntityList.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

VphysHud vphysHud;

Variable sar_vphys_hud("sar_vphys_hud", "0", 0, "Enables or disables the vphys HUD.\n");
Variable sar_vphys_hud_font("sar_vphys_hud_font", "1", 0, "Sets font of the vphys HUD.\n");
Variable sar_vphys_hud_precision("sar_vphys_hud_precision", "3", 1, 16, "Sets decimal precision of the vphys HUD.\n");
Variable sar_vphys_hud_x("sar_vphys_hud_x", "0", 0, "The x position of the vphys HUD.\n");
Variable sar_vphys_hud_y("sar_vphys_hud_y", "0", 0, "The y position of the vphys HUD.\n");

Variable sar_vphys_hud_show_hitboxes("sar_vphys_hud_show_hitboxes", "2", 0, 3, 
	"Sets visibility of hitboxes when vphys hud is active.\n"
	"0 = hitboxes are not drawn\n"
	"1 = only active vphys hitbox is drawn\n"
	"2 = active vphys and player's bounding box are drawn\n"
	"3 = both vphys hitboxes and player's bounding box are drawn\n"
);

VphysHud::VphysHud()
	: Hud(HudType_InGame, true) {
}
bool VphysHud::ShouldDraw() {
	return sar_vphys_hud.GetBool() && Hud::ShouldDraw() && sv_cheats.GetBool();
}
void VphysHud::Paint(int slot) {
	auto font = scheme->GetFontByID(sar_vphys_hud_font.GetInt());

	int cX = sar_vphys_hud_x.GetInt();
	int cY = sar_vphys_hud_y.GetInt();
	 
	float fh = surface->GetFontHeight(font);

	auto drawPhysicsInfo = [=](int x, int y, bool crouched, const char *name) {
		VphysShadowInfo shadowInfo = GetVphysInfo(slot, crouched);

		Color posColor = Color(255,255,255,255);
		Color enableColor = Color(128, 255, 128, 255);
		Color disableColor = Color(255, 128, 128, 255);

		if (!shadowInfo.collisionEnabled) {
			posColor.a = 100;
			enableColor.a = 100;
			disableColor.a = 100;
		}

		int p = sar_vphys_hud_precision.GetInt();

		surface->DrawTxt(font, x, y, posColor, "%s: ", name);
		surface->DrawTxt(font, x + 10, (y += fh), posColor, "pos: %.*f, %.*f, %.*f", 
			p, shadowInfo.position.x, p, shadowInfo.position.y, p, shadowInfo.position.z);
		surface->DrawTxt(font, x + 10, (y += fh), posColor, "ang: %.*f, %.*f, %.*f", 
			p, shadowInfo.rotation.x, p, shadowInfo.rotation.y, p, shadowInfo.rotation.z);
		surface->DrawTxt(font, x + 10, (y += fh), posColor, "vel: %.*f, %.*f, %.*f", 
			p, shadowInfo.velocity.x, p, shadowInfo.velocity.y, p, shadowInfo.velocity.z);
		surface->DrawTxt(font, x + 10, (y += fh), posColor, "angvel: %.*f, %.*f, %.*f", 
			p, shadowInfo.angularVelocity.x, p, shadowInfo.angularVelocity.y, p, shadowInfo.angularVelocity.z);

		surface->DrawTxt(font, x + 10, (y += fh), shadowInfo.collisionEnabled ? enableColor : disableColor,
			"collision: %s", shadowInfo.collisionEnabled ? "enabled" : "disabled");
		surface->DrawTxt(font, x + 10, (y += fh), shadowInfo.gravityEnabled ? enableColor : disableColor,
			"gravity: %s", shadowInfo.gravityEnabled ? "enabled" : "disabled");
		surface->DrawTxt(font, x + 10, (y += fh), shadowInfo.asleep ? enableColor : disableColor,
			"asleep: %s", shadowInfo.asleep ? "yes" : "no");
	};

	drawPhysicsInfo(cX + 5, cY + 5, false, "shadowStand");
	drawPhysicsInfo(cX + 5, cY + 5 + fh * 9.5f, true, "shadowCrouch");
}

bool VphysHud::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}


VphysShadowInfo VphysHud::GetVphysInfo(int slot, bool crouched) {
	VphysShadowInfo info;

	info.player = server->GetPlayer(slot + 1);
	if (!info.player) return info;

	int hitboxOffset = crouched ? Offsets::m_pShadowCrouch : Offsets::m_pShadowStand;

	info.shadow = *reinterpret_cast<void **>((uintptr_t)info.player + hitboxOffset);

	using _IsAsleep = bool(__rescall *)(void *thisptr);
	using _IsEnabled = bool(__rescall *)(void *thisptr);
	using _GetPosition = void(__rescall *)(void *thisptr, Vector *worldPosition, QAngle *angles);
	using _GetVelocity = void(__rescall *)(void *thisptr, Vector *velocity, Vector *angularVelocity);

	_IsAsleep IsAsleep = Memory::VMT<_IsAsleep>(info.shadow, Offsets::IsAsleep);
	_IsEnabled IsCollisionEnabled = Memory::VMT<_IsEnabled>(info.shadow, Offsets::IsCollisionEnabled);
	_IsEnabled IsGravityEnabled = Memory::VMT<_IsEnabled>(info.shadow, Offsets::IsGravityEnabled);
	_GetPosition GetPosition = Memory::VMT<_GetPosition>(info.shadow, Offsets::GetPosition);
	_GetVelocity GetVelocity = Memory::VMT<_GetVelocity>(info.shadow, Offsets::GetVelocity);

	GetPosition(info.shadow, &info.position, &info.rotation);
	GetVelocity(info.shadow, &info.velocity, &info.angularVelocity);
	info.collisionEnabled = IsCollisionEnabled(info.shadow);
	info.gravityEnabled = IsGravityEnabled(info.shadow);
	info.asleep = IsAsleep(info.shadow);

	return info;
}


// drawing player's hitboxes
ON_EVENT(RENDER) {
	if (!sv_cheats.GetBool() || !sar_vphys_hud.GetBool() || !sar_vphys_hud_show_hitboxes.GetBool()) 
		return;

	void *player = server->GetPlayer(GET_SLOT() + 1);
	if (!player) return;

	int renderState = sar_vphys_hud_show_hitboxes.GetInt();

	const auto renderHitbox = [=](Color c, float solidScale, Vector pos, Vector size, QAngle rot) {
		RenderCallback solid = RenderCallback::constant({c.r, c.g, c.b, (uint8_t)(c.a * solidScale)});
		RenderCallback wf = RenderCallback::constant({c.r, c.g, c.b, 255});
		OverlayRender::addBoxMesh(
			pos, 
			Vector(size.x * -0.5f, size.y * -0.5f, 0), 
			Vector(size.x * 0.5f, size.y * 0.5f, size.z), 
			rot, solid, wf
		);
	};

	const auto renderVphys = [=](VphysShadowInfo info, Vector size) {
		if (info.collisionEnabled || renderState == 3) {
			Color vphysColor(255, 191, 0, info.collisionEnabled ? 255 : 64);
			renderHitbox(vphysColor, 0.25, info.position, size, info.rotation);
		}
	};

	// TODO: find actual values
	Vector standingSize = Vector(32, 32, 72);
	Vector crouchingSize = Vector(32, 32, 36);

	// drawing vphys hitboxes
	renderVphys(vphysHud.GetVphysInfo(GET_SLOT(), false), standingSize);
	renderVphys(vphysHud.GetVphysInfo(GET_SLOT(), true), crouchingSize);

	// drawing player bbox
	if (renderState > 1) {
		Vector pos = server->GetAbsOrigin(player);
		bool ducked = SE(player)->ducked();
		renderHitbox(Color(253, 106, 2), 0.1, pos, ducked ? crouchingSize : standingSize, QAngle());
	}

}


CON_COMMAND(sar_vphys_setgravity, "sar_vphys_setgravity <hitbox> <enabled> - sets gravity flag state to either standing (0) or crouching (1) havok collision shadow\n") {
	if (engine->demoplayer->IsPlaying()) {
		return;
	}
	if (!sv_cheats.GetBool()) {
		return console->Print("Cannot use sar_vphys_setgravity without sv_cheats set to 1.\n");
	}

	if (args.ArgC() != 3) {
		return console->Print(sar_vphys_setgravity.ThisPtr()->m_pszHelpString);
	}

	void *player = server->GetPlayer(1);
	void *m_pShadowStand = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowStand);
	void *m_pShadowCrouch = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowCrouch);

	using _EnableGravity = bool(__rescall *)(void *thisptr, bool enabled);
	_EnableGravity EnableGravity = Memory::VMT<_EnableGravity>(m_pShadowStand, Offsets::EnableGravity);

	int hitbox = std::atoi(args[1]);
	int enabled = std::atoi(args[2]);

	EnableGravity(hitbox ? m_pShadowCrouch : m_pShadowStand, enabled);
}

CON_COMMAND(sar_vphys_setangle, "sar_vphys_setangle <hitbox> <angle> [component = z] - sets rotation angle to either standing (0) or crouching (1) havok collision shadow\n") {
	if (engine->demoplayer->IsPlaying()) {
		return;
	}
	if (!sv_cheats.GetBool()) {
		return console->Print("Cannot use sar_vphys_setangle without sv_cheats set to 1.\n");
	}

	if (args.ArgC() < 3 || args.ArgC() > 4) {
		return console->Print(sar_vphys_setangle.ThisPtr()->m_pszHelpString);
	}

	void *player = server->GetPlayer(1);
	void *m_pShadowStand = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowStand);
	void *m_pShadowCrouch = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowCrouch);

	using _SetPosition = void(__rescall *)(void *thisptr, const Vector &worldPosition, const QAngle &angles, bool isTeleport);
	using _GetPosition = void(__rescall *)(void *thisptr, Vector *worldPosition, QAngle *angles);
	_SetPosition SetPosition = Memory::VMT<_SetPosition>(m_pShadowStand, Offsets::SetPosition);
	_GetPosition GetPosition = Memory::VMT<_GetPosition>(m_pShadowStand, Offsets::GetPosition);

	auto hitbox = std::atoi(args[1]);
	auto angle = float(std::atof(args[2]));
	auto component = args.ArgC() == 4 ? args[3] : "z";

	void *selected = hitbox ? m_pShadowCrouch : m_pShadowStand;

	Vector v;
	QAngle q;
	GetPosition(selected, &v, &q);

	if (!strcmp(component, "x")) {
		q.x = angle;
	} else if (!strcmp(component, "y")) {
		q.y = angle;
	} else {
		q.z = angle;
	}

	SetPosition(selected, v, q, false);
}

CON_COMMAND(sar_vphys_setspin, "sar_vphys_setspin <hitbox> <angvel> [component = x] - sets rotation speed to either standing (0) or crouching (1) havok collision shadow\n") {
	if (engine->demoplayer->IsPlaying()) {
		return;
	}
	if (!sv_cheats.GetBool()) {
		return console->Print("Cannot use sar_vphys_setspin without sv_cheats set to 1.\n");
	}

	if (args.ArgC() < 3 || args.ArgC() > 4) {
		return console->Print(sar_vphys_setspin.ThisPtr()->m_pszHelpString);
	}

	void *player = server->GetPlayer(1);
	void *m_pShadowStand = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowStand);
	void *m_pShadowCrouch = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowCrouch);

	using _GetVelocity = void(__rescall *)(void *thisptr, Vector *velocity, Vector *angularVelocity);
	_GetVelocity GetVelocity = Memory::VMT<_GetVelocity>(m_pShadowStand, Offsets::GetVelocity);
	using _SetVelocity = void(__rescall *)(void *thisptr, const Vector *velocity, const Vector *angularVelocity);
	_SetVelocity SetVelocity = Memory::VMT<_SetVelocity>(m_pShadowStand, Offsets::SetVelocity);

	auto hitbox = std::atoi(args[1]);
	auto angvel = float(std::atof(args[2]));
	auto component = args.ArgC() == 4 ? args[3] : "x";

	void *selected = hitbox ? m_pShadowCrouch : m_pShadowStand;

	Vector v;
	GetVelocity(selected, NULL, &v);
	
	if (!strcmp(component, "y")) {
		v.y = angvel;
	} else if (!strcmp(component, "z")) {
		v.z = angvel;
	} else {
		v.x = angvel;
	}

	SetVelocity(selected, NULL, &v);
}

CON_COMMAND(sar_vphys_setasleep, "sar_vphys_setasleep <hitbox> <asleep> - sets whether your standing (0) or crouching (1) havok collision shadow is asleep\n") {
	if (engine->demoplayer->IsPlaying()) {
		return;
	}
	if (!sv_cheats.GetBool()) {
		return console->Print("Cannot use sar_vphys_setasleep without sv_cheats set to 1.\n");
	}

	if (args.ArgC() != 3) {
		return console->Print(sar_vphys_setasleep.ThisPtr()->m_pszHelpString);
	}

	void *player = server->GetPlayer(1);
	if (!player) return;

	void *m_pShadowStand = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowStand);
	void *m_pShadowCrouch = *reinterpret_cast<void **>((uintptr_t)player + Offsets::m_pShadowCrouch);

	int hitbox = std::atoi(args[1]);
	int asleep = std::atoi(args[2]);

	void *selected = hitbox ? m_pShadowCrouch : m_pShadowStand;

	using _SleepWake = void(__rescall *)(void *thisptr);
	_SleepWake SleepWake = Memory::VMT<_SleepWake>(selected, asleep ? Offsets::Sleep : Offsets::Wake);
	SleepWake(selected);
}
