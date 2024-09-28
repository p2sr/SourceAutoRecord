#include "StepSlopeBoostDebug.hpp"
#include "Event.hpp"
#include "Variable.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Cheats.hpp"
#include "Features/OverlayRender.hpp"


#include <map>

Variable sar_debug_step_slope_boost("sar_debug_step_slope_boost", "0", 0, 1, "Debugs step slope boosts\n");

StepMoveData g_currentStepMoveData;
bool g_stepMoving;
int g_tryPlayerMoveCount;
bool g_stepMoved;
bool g_slopeBoosted;

void StepSlopeBoostDebug::OnStartStepMove(CMoveData *moveData) {
	auto player = server->GetPlayer(1);
	if (!player) return;
	g_currentStepMoveData.wasDucking = player->ducked();

	g_stepMoving = true;
	g_stepMoved = false;
	g_slopeBoosted = false;
	g_tryPlayerMoveCount = 0;

	g_currentStepMoveData.initialPosition = moveData->m_vecAbsOrigin;
	g_currentStepMoveData.initialVelocity = moveData->m_vecVelocity;

}

void StepSlopeBoostDebug::OnTryPlayerMoveEnd(CMoveData *currMoveData) {
	if (!g_stepMoving) return;

	if (g_tryPlayerMoveCount == 0) {
		g_currentStepMoveData.afterHitPosition = currMoveData->m_vecAbsOrigin;
		g_currentStepMoveData.afterHitVelocity = currMoveData->m_vecVelocity;
	}
	else if (g_tryPlayerMoveCount == 1) {
		g_currentStepMoveData.afterStepPosition = currMoveData->m_vecAbsOrigin;
		g_currentStepMoveData.afterStepVelocity = currMoveData->m_vecVelocity;
	}

	g_tryPlayerMoveCount++;
}

void StepSlopeBoostDebug::OnFinishStepMove(CMoveData *currMoveData) {
	g_stepMoving = false;

	if (currMoveData->m_vecVelocity != g_currentStepMoveData.afterHitVelocity) {
		g_slopeBoosted = true;
	}
	
	g_stepMoved = true;
}

ON_EVENT(SESSION_START) {
	g_stepMoved = false;
}

void drawBox(Vector position, Vector velocity, bool ducked, Color color) {
	auto center = position + (ducked ? Vector{0, 0, 18} : Vector{0, 0, 36});
	auto size = ducked ? Vector{32, 32, 36} : Vector{32, 32, 72};

	OverlayRender::addBoxMesh(
		center, 
		-size / 2, 
		size / 2, 
		{0, 0, 0}, 
		RenderCallback::constant({color.r, color.g, color.b, 20}), 
		RenderCallback::constant({color.r, color.g, color.b, 255})
	);
}

ON_EVENT(RENDER) {
	if (!sar_debug_step_slope_boost.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	if (!g_stepMoved) return;

	drawBox(g_currentStepMoveData.initialPosition, g_currentStepMoveData.initialVelocity, g_currentStepMoveData.wasDucking, {0, 255, 255});
	drawBox(g_currentStepMoveData.afterHitPosition, g_currentStepMoveData.afterHitVelocity, g_currentStepMoveData.wasDucking, {0, 0, 255});
	drawBox(g_currentStepMoveData.afterStepPosition, g_currentStepMoveData.afterStepVelocity, g_currentStepMoveData.wasDucking, {0, 255, 0});

	std::string hoverStr = "";

	Vector hitVel = g_currentStepMoveData.afterHitVelocity;
	Vector stepVel = g_currentStepMoveData.afterStepVelocity;

	hoverStr += Utils::ssprintf("hitVel: %.1f %.1f %.1f\n", hitVel.x, hitVel.y, hitVel.z);
	hoverStr += Utils::ssprintf("stepVel: %.1f %.1f %.1f\n", stepVel.x, stepVel.y, stepVel.z);

	OverlayRender::addText(
		g_currentStepMoveData.afterStepPosition + Vector{0,0,64}, 
		hoverStr, 3.0f, true, true);
}
