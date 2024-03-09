#include "StrafeHud.hpp"

#include "Command.hpp"
#include "Modules/Client.hpp"
#include "Modules/Server.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"

#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTools/StrafeTool.hpp"
#include "Features/Tas/TasTools/TasUtils.hpp"


Variable sar_strafehud("sar_strafehud", "0", "Enables or disables strafe hud.\n");
Variable sar_strafehud_x("sar_strafehud_x", "-10", "The X position of the strafe hud.\n", 0);
Variable sar_strafehud_y("sar_strafehud_y", "10", "The Y position of the strafe hud.\n", 0);
Variable sar_strafehud_size("sar_strafehud_size", "256", "The width and height of the strafe hud.\n", 0);
Variable sar_strafehud_font("sar_strafehud_font", "13", "Font used by strafe hud.\n", 0);
Variable sar_strafehud_detail_scale("sar_strafehud_detail_scale", "4", "The detail scale for the lines of hud.\n", 0);

Variable sar_strafehud_use_friction("sar_strafehud_use_friction", "0", "Use ground friction when calculating acceleration.\n");
Variable sar_strafehud_avg_sample_count("sar_strafehud_avg_sample_count", "60", 1, 9999, "How many samples to use for average counter.\n");
Variable sar_strafehud_match_accel_scale("sar_strafehud_match_accel_scale", "0", "Match the scales for minimum and maximum deceleration.\n");
Variable sar_strafehud_lock_mode("sar_strafehud_lock_mode", "1", 0, 2,
	"Lock mode used by strafe hud:\n"
	"0 - view direction\n"
	"1 - velocity direction\n"
	"2 - absolute angles\n");

StrafeHud strafeHud;

void StrafeHud::SetData(int slot, void *player, CUserCmd *cmd, bool serverside) {
	if (!sar_strafehud.GetBool() || !sv_cheats.GetBool()) return;

	data[slot].accelValues.clear();

	// generate predicted accelerate values in a circle
	auto playerInfo = tasPlayer->GetPlayerInfo(slot, player, cmd, !serverside);
	float oldVel = playerInfo.velocity.Length2D();

	if (!sar_strafehud_use_friction.GetBool()) {
		oldVel = autoStrafeTool->GetGroundFrictionVelocity(playerInfo).Length2D();
	}

	auto bestAng = autoStrafeTool->GetFastestStrafeAngle(playerInfo);
	float biggestAccel = autoStrafeTool->GetVelocityAfterMove(playerInfo, cosf(bestAng), sinf(bestAng)).Length2D() - oldVel;
	float smallestAccel = 0.0f;

	float relAng = 0.0f;
	int lockMode = sar_strafehud_lock_mode.GetInt();
	if (lockMode > 0) {
		float velAng = TasUtils::GetVelocityAngles(&playerInfo).x;
		float lookAng = playerInfo.angles.y;

		relAng += lookAng;
		if (lockMode == 1) relAng -= velAng;
		relAng = DEG2RAD(relAng);
	}

	float detail = sar_strafehud_size.GetInt() * sar_strafehud_detail_scale.GetFloat();
	float step = 1.0f / detail;

	for (float i = 0.0f; i < 1.0f; i += step) {
		float ang = i * 2.0f * M_PI + relAng;
		float newVel = autoStrafeTool->GetVelocityAfterMove(playerInfo, cosf(ang), sinf(ang)).Length2D();
		float accel = newVel - oldVel;
		data[slot].accelValues.push_back(accel);

		if (accel > biggestAccel) biggestAccel = accel;
		if (i == 0 || accel < smallestAccel) smallestAccel = accel;
	}

	if (sar_strafehud_match_accel_scale.GetBool()) {
		float max = biggestAccel;
		if (fabsf(smallestAccel) > max) max = fabsf(smallestAccel);
		smallestAccel = -max;
		biggestAccel = max;
	}

	for (size_t i = 0; i < data[slot].accelValues.size(); i++) {
		if (data[slot].accelValues[i] > 0 && biggestAccel > 0) {
			data[slot].accelValues[i] /= biggestAccel;
		}
		if (data[slot].accelValues[i] < 0 && smallestAccel < 0) {
			data[slot].accelValues[i] /= fabsf(smallestAccel);
		}
	}

	while (data[slot].precisionLog.size() > fmaxf(sar_strafehud_avg_sample_count.GetInt(), 0)) {
		data[slot].precisionLog.pop_front();
	}

	float nextVel = autoStrafeTool->GetVelocityAfterMove(playerInfo, cmd->forwardmove, cmd->sidemove).Length2D();
	float nextAccel = nextVel - oldVel;
	if (nextAccel > biggestAccel) biggestAccel = nextAccel;

	// assign calculated values
	data[slot].maxAccel = biggestAccel;
	data[slot].minAccel = smallestAccel;

	float precision = (data[slot].maxAccel == 0) ? 1.0f : nextAccel / data[slot].maxAccel;
	data[slot].precisionLog.push_back(precision);
	data[slot].avgPrecision = 0;
	if (data[slot].precisionLog.size()) {
		for (auto const &i : data[slot].precisionLog) {
			data[slot].avgPrecision += i;
		}
		data[slot].avgPrecision /= data[slot].precisionLog.size();
	}

	// assign wishDir for paint function to draw a line
	Vector wishDir = {
		cosf(relAng) * cmd->sidemove - sinf(relAng) * cmd->forwardmove, 
		sinf(relAng) * cmd->sidemove + cosf(relAng) * cmd->forwardmove
	};



	if (wishDir.y > 0.0) {
		wishDir.y /= cl_forwardspeed.GetFloat();
	} else {
		wishDir.y /= cl_backspeed.GetFloat();
	}

	wishDir.x /= cl_sidespeed.GetFloat();

	if (wishDir.Length2D() > 1.0f) wishDir = wishDir.Normalize();

	data[slot].wishDir = wishDir;
}


void StrafeHud::Paint(int slot) {
	if (!sar_strafehud.GetBool() || !sv_cheats.GetBool()) return;

	
	auto font = scheme->GetFontByID(sar_strafehud_font.GetInt());
	int fontHeight = surface->GetFontHeight(font);
	float pad = 5;
	float s = sar_strafehud_size.GetInt();
	int x = sar_strafehud_x.GetInt(), y = sar_strafehud_y.GetInt();

	int sw, sh;
	engine->GetScreenSize(nullptr, sw, sh);

	if (x < 0) x += sw - s;
	if (y < 0) y += sh - s - (fontHeight + pad) * 2;

	Color bgColor = {0, 0, 0, 192};
	Color linesColor = {64, 64, 64, 255};
	Color wishDirColor = {0, 0, 255};
	Color accelColor = {0, 255, 0};
	Color decelColor = {255, 0, 0};
	Color nocelColor = {255, 255, 0};

	// background
	surface->DrawRect(bgColor, x, y, x + s, y + s + (fontHeight + pad) * 2);  

	x += pad;
	y += pad * 2 + fontHeight;
	s -= pad * 2;

	int midX = x + s * 0.5f;
	int midY = y + s * 0.5f;

	int dX = (s - pad * 2.0f) * 0.5f;
	int dY = (s - pad * 2.0f) * 0.5f;

	// containing rect
	surface->DrawColoredLine(x, y, x, y + s, linesColor);
	surface->DrawColoredLine(x, y, x + s, y, linesColor);
	surface->DrawColoredLine(x + s, y, x + s, y + s, linesColor);
	surface->DrawColoredLine(x, y + s, x + s, y + s, linesColor);

	surface->DrawCircle(midX, midY, s * 0.5f, linesColor); // big circle
	surface->DrawCircle(midX, midY, s * 0.25f, linesColor);  // half circle

	// half-lines and diagonals
	surface->DrawColoredLine(midX, y, midX, y + s, linesColor);
	surface->DrawColoredLine(x, midY, x + s, midY, linesColor);
	surface->DrawColoredLine(x, y, x + s, y + s, linesColor);
	surface->DrawColoredLine(x, y + s, x + s, y, linesColor);

	// acceleration line
	float detail = sar_strafehud_size.GetInt() * sar_strafehud_detail_scale.GetFloat();
	for (size_t i = 0; i < data[slot].accelValues.size(); i++) {
		float ang1 = (i / detail) * 2.0f * M_PI;
		int i2 = (i + 1 >= data[slot].accelValues.size()) ? 0 : i + 1;
		float ang2 = (i2 / detail) * 2.0f * M_PI;

		float a1 = fminf(fmaxf(data[slot].accelValues[i], -1.0f), 1.0f);
		float a2 = fminf(fmaxf(data[slot].accelValues[i2], -1.0f), 1.0f);

		float ad1 = (a1 + 1.0f) * 0.5f;
		float ad2 = (a2 + 1.0f) * 0.5f;
		Color lineColor = nocelColor;
		if ((ad1 != 0 && ad2 != 0) && a1 * a2 > 0) {
			lineColor = (a1 >= 0.0f ? accelColor : decelColor);
		}

		surface->DrawColoredLine(
			midX + sinf(ang1) * dX * ad1, midY - cosf(ang1) * dY * ad1,
			midX + sinf(ang2) * dX * ad2, midY - cosf(ang2) * dY * ad2, 
			lineColor
		);
		
	}

	// draw thick wishdir line
	int lineW = 1;
	for (int ox = -lineW; ox <= lineW; ox++) {
		for (int oy = -lineW; oy <= lineW; oy++) {
			surface->DrawColoredLine(
				midX + ox, midY + oy, 
				midX + data[slot].wishDir.x * dX + ox, 
				midY - data[slot].wishDir.y * dY + oy, 
				wishDirColor 
			);
		}
	}

	// draw info text
	surface->DrawTxt(font, x, y - fontHeight - pad, {255, 255, 255}, "min:%.03f", data[slot].minAccel);
	surface->DrawTxt(font, midX, y - fontHeight - pad, {255, 255, 255}, "max:%.03f", data[slot].maxAccel);

	float p = data[slot].precisionLog.size() == 0 ? 0.0f : (data[slot].precisionLog.back() * 100.0f);
	float avgp = data[slot].avgPrecision * 100.0f;
	surface->DrawTxt(font, x, y + s + pad, {255, 255, 255}, "p: %.01f%%", p);
	surface->DrawTxt(font, midX, y + s + pad, {255, 255, 255}, "avg.p: %.01f%%", avgp);
}
