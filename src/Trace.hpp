#pragma once
#include <Utils/SDK/Math.hpp>
#include <Features/Trace/TraceHitbox.hpp>
#include <Features/Trace/TracePortal.hpp>
#include <string>
#include <vector>

namespace Trace {
	struct PlayerState {
		Vector position;
		Vector eye_position;
		QAngle angles;
		Vector velocity;
		bool grounded;
		bool crouched;
	};

	struct TraceData {
		int startSessionTick;
		int startTasTick;
		std::vector<PlayerState> players[2];
		std::vector<HitboxList> hitboxes;
		std::vector<PortalsList> portals;
		bool draw = true;

		void RecordNewPoint(void *player, int slot, bool server_side);
	};

	TraceData* GetByName(std::string name);
	TraceData* GetOrCreateByName(std::string name);
	TraceData* CreateByName(std::string name);
	TraceData *GetCurrent();
	TraceData *GetOrCreateCurrent();
	void ClearByName(std::string name);
	void ClearAll();
	bool ShouldRecord();
}