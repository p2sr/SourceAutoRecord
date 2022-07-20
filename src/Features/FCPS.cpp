#include "Utils/SDK.hpp"
#include "Features/EntityList.hpp"
#include "Features/OverlayRender.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Event.hpp"
#include "Command.hpp"
#include "Variable.hpp"
#include <vector>

struct FcpsResult {
	Vector start_center;
	Vector final_center;
	Vector iters[100];
	Vector extents;
	int niters;
	bool success;
};

static std::vector<FcpsResult> g_events;

enum class AnimStage {
	NONE,     // not animating
	INITIAL,  // draw the initial position
	ITERS,    // draw the iterations
	FINAL,    // draw the final position
	//PATH,     // draw the path from initial -> final position
};

static struct {
	AnimStage stage = AnimStage::NONE;
	size_t idx;
	int iter;
} g_anim_state;

static void stepAnim() {
	if (g_anim_state.stage == AnimStage::NONE) return;

	size_t i = g_anim_state.idx;

	switch (g_anim_state.stage) {
	case AnimStage::NONE:
		return;
	case AnimStage::INITIAL:
		if (g_events[i].niters > 0) {
			g_anim_state.stage = AnimStage::ITERS;
			g_anim_state.iter = 0;
		} else {
			g_anim_state.stage = AnimStage::FINAL;
		}
		return;
	case AnimStage::ITERS:
		if (++g_anim_state.iter >= g_events[i].niters) {
			g_anim_state.stage = AnimStage::FINAL;
		}
		return;
	case AnimStage::FINAL:
		g_anim_state.stage = AnimStage::NONE;
		return;
	}
}

// iter -1 is initial pos, 100 is final pos
static void drawIter(int idx, int iter, Color solid, Color wf) {
	Vector center = iter == -1 ? g_events[idx].start_center : iter == 101 ? g_events[idx].final_center : g_events[idx].iters[iter];
	OverlayRender::addBoxMesh(center, -g_events[idx].extents, g_events[idx].extents, {0,0,0}, RenderCallback::constant(solid), RenderCallback::constant(wf));
}

ON_EVENT(RENDER) {
	if (g_anim_state.stage == AnimStage::NONE) return;
	if (!sv_cheats.GetBool()) return;

	size_t i = g_anim_state.idx;

	switch (g_anim_state.stage) {
	case AnimStage::NONE:
		return;
	case AnimStage::INITIAL:
		drawIter(i, -1, {255,0,255,50}, {0,0,255,255});
		return;
	case AnimStage::ITERS:
		drawIter(i, g_anim_state.iter-1, {255,255,255,50}, {0,0,255,255});
		drawIter(i, g_anim_state.iter, {0,0,255,50}, {0,0,255,255});
		return;
	case AnimStage::FINAL:
		drawIter(i, g_events[i].niters-1, {255,255,255,50}, {0,0,255,255});
		drawIter(i, 101, g_events[i].success ? Color{0,255,0,50} : Color{255,0,0,50}, {0,0,255,255});
		return;
	}
}

static float g_last_step_time;

Variable sar_fcps_override("sar_fcps_override", "0", "Override FCPS for tracing results.\n");
Variable sar_fcps_autostep("sar_fcps_autostep", "0", 0, "Time between automatic steps of FCPS animation. 0 to disable automatic stepping.\n");

CON_COMMAND(sar_fcps_anim_start, "sar_fcps_anim_start <id> - start animating the ID'th FCPS call.\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_fcps_anim_start.ThisPtr()->m_pszHelpString);
		return;
	}

	int idx = atoi(args[1]);
	if (idx < 0) {
		idx += g_events.size();
	}

	if (idx < 0 || idx > (int)g_events.size()) {
		console->Print("No such FCPS event!\n");
		return;
	}

	g_last_step_time = engine->GetHostTime();
	g_anim_state.stage = AnimStage::INITIAL;
	g_anim_state.idx = idx;
}

CON_COMMAND(sar_fcps_anim_step, "sar_fcps_anim_step - step the FCPS animation forward.\n") {
	g_last_step_time = engine->GetHostTime();
	stepAnim();
}

CON_COMMAND(sar_fcps_clear, "sar_fcps_clear - clear the FCPS event history.\n") {
	g_anim_state.stage = AnimStage::NONE;
	g_events.clear();
}

ON_EVENT(FRAME) {
	float time = engine->GetHostTime();
	if (sar_fcps_autostep.GetFloat() == 0.0f) {
		g_last_step_time = time;
	} else {
		float delta = time - g_last_step_time;
		if (delta >= sar_fcps_autostep.GetFloat()) {
			stepAnim();
			g_last_step_time = sar_fcps_autostep.GetFloat();
		}
	}
}

// Copies the behaviour of FCPS, tracking the result.
// Adapter should be non-NULL for both FCPS1 and FCPS2
static FcpsResult tracedFcps(const Vector &orig_center, const Vector &extents, const Vector &ind_push, FcpsTraceAdapter *adapter, bool fcps2) {
	FcpsResult result;
	result.start_center = orig_center;
	result.extents = extents;

	Vector orig_extents = extents;
	Vector center = orig_center;
	Vector grow_size = extents * (1.0f / 101.0f);
	Vector cur_extents = extents - grow_size;

	center.z += 0.001f; // satisfy swept trace on first pass

	Ray_t ent_ray;
	ent_ray.m_Extents = extents;
	ent_ray.m_IsRay = false;
	ent_ray.m_IsSwept = true;
	ent_ray.m_StartOffset = VectorAligned{0, 0, 0};

	Ray_t test_ray;
	test_ray.m_Extents = grow_size;
	test_ray.m_IsRay = false;
	test_ray.m_IsSwept = true;
	test_ray.m_StartOffset = VectorAligned{0, 0, 0};

	CGameTrace traces[2];

	for (int iteration = 0; iteration < 100; ++iteration) {
		if (!fcps2) {
			// fcps1 checks if the position is valid here, whereas fcps2 does
			// it a bit later to consider the validity of the vertices
			ent_ray.m_Start = center;
			ent_ray.m_Delta = orig_center - center;
			adapter->traceFunc(ent_ray, &traces[0], adapter);
			if (!traces[0].startsolid) {
				result.final_center = traces[0].endpos;
				result.niters = iteration;
				result.success = true;
				return result;
			}
		}

		bool extent_invalid[8];
		bool any_invalid = false;
		float extents_validation[8];
		Vector extents[8];

		for (int i = 0; i < 8; ++i) {
			extents[i] = center;
			extents[i].x += (i & (1<<0)) ? cur_extents.x : -cur_extents.x;
			extents[i].y += (i & (1<<1)) ? cur_extents.y : -cur_extents.y;
			extents[i].z += (i & (1<<2)) ? cur_extents.z : -cur_extents.z;

			extents_validation[i] = 0.0f;

			extent_invalid[i] = adapter->pointOutsideWorldFunc(extents[i], adapter);
			any_invalid |= extent_invalid[i];
		}

		for (unsigned i = 0; i < 7; ++i) {
			for (unsigned j = i; j < 8; ++j) {
				// trace i to j
				if (extent_invalid[i]) {
					traces[0].startsolid = true;
					traces[0].fraction = 0.0f;
				} else {
					test_ray.m_Start = extents[i];
					test_ray.m_Delta = extents[j] - extents[i];
					adapter->traceFunc(test_ray, &traces[0], adapter);
				}

				// trace j to i
				if (extent_invalid[j]) {
					traces[1].startsolid = true;
					traces[1].fraction = 0.0f;
				} else {
					test_ray.m_Start = extents[j];
					test_ray.m_Delta = extents[i] - extents[j];
					adapter->traceFunc(test_ray, &traces[1], adapter);
				}

				float dist = (extents[j] - extents[i]).Length();

				if (fcps2) {
					// one sided collision stuff
					if (traces[0].fraction == 1.0f && traces[1].fraction != 1.0f) {
						traces[0].startsolid = true;
						traces[0].fraction = 0.0f;
					} else if (traces[1].fraction == 1.0f && traces[0].fraction != 1.0f) {
						traces[1].startsolid = true;
						traces[1].fraction = 0.0f;
					}
				}

				if (traces[0].startsolid) {
					if (fcps2) {
						extent_invalid[i] = true;
						any_invalid = true;
					} else {
						extents_validation[i] -= 100.0f;
					}
				} else {
					extents_validation[i] += traces[0].fraction * dist;
				}

				if (traces[1].startsolid) {
					if (fcps2) {
						extent_invalid[j] = true;
						any_invalid = true;
					} else {
						extents_validation[j] -= 100.0f;
					}
				} else {
					extents_validation[j] += traces[1].fraction * dist;
				}
			}
		}

		// fcps2 checks for success here, so it can check for invalid
		// corners and not accept a position if it has any
		if (fcps2 && !any_invalid) {
			ent_ray.m_Start = center;
			ent_ray.m_Delta = orig_center - center;

			// this check deals with more one-sided collision bs
			adapter->traceFunc(ent_ray, &traces[0], adapter);
			if (!traces[0].startsolid) {
				result.final_center = traces[0].endpos;
				result.niters = iteration;
				result.success = !adapter->pointOutsideWorldFunc(result.final_center, adapter);
				return result;
			}
		}

		// find the direction to shift the box in based on the validity of
		// each vertex
		Vector new_origin_direction{0.0f, 0.0f, 0.0f};
		float total_validation = 0.0f;
		for (int i = 0; i < 8; ++i) {
			bool valid = fcps2 ? !extent_invalid[i] : extents_validation[i] > 0.0f;
			if (valid) {
				new_origin_direction += (extents[i] - center) * extents_validation[i];
				total_validation += extents_validation[i];
			}
		}

		// move the box!
		if (total_validation != 0.0f) {
			center += new_origin_direction / total_validation;

			// increase ray sizes
			test_ray.m_Extents += grow_size; // increase ray size
			cur_extents -= grow_size; // reduce overall test region size so the maximum ray extents are the same
		} else {
			// every vertex invalid - apply indecisive push
			center += ind_push;

			// reset ray sizes
			test_ray.m_Extents = grow_size;
			cur_extents = fcps2 ? orig_extents - grow_size : orig_extents; // fcps1 has a small bug here, fixed in fcps2
		}

		result.iters[iteration] = center;
	}

	// failed, how sad
	result.final_center = orig_center;
	result.niters = 100;
	result.success = false;
	return result;
}

// TODO: check for move parent in fcps1
// TODO: collision group stuff?

bool RecordFcps2(const Vector &orig_center, const Vector &extents, const Vector &ind_push, Vector &center_out, FcpsTraceAdapter *adapter) {
	FcpsResult result = tracedFcps(orig_center, extents, ind_push, adapter, true);
	g_events.push_back(result);
	console->Print("Saved FCPS2 result %d - %d iters\n", g_events.size() - 1, result.niters);
	center_out = result.final_center;
	return result.success;
}

static void fcps1Trace(const Ray_t &ray, CGameTrace *result, FcpsTraceAdapter *adapter) {
	engine->TraceRay(engine->engineTrace->ThisPtr(), ray, adapter->mask, adapter->traceFilter, result);
}

static bool fcps1PointOutsideWorld(const Vector &test, FcpsTraceAdapter *adapter) {
	return engine->PointOutsideWorld(engine->engineTrace->ThisPtr(), test);
}

static inline float dotProdAbs(Vector vec, float *vals) {
	return fabsf(vec.x * vals[0]) + fabsf(vec.y * vals[1]) + fabsf(vec.z * vals[2]);
}

bool RecordFcps1(void *entity, const Vector &ind_push, int mask) {
	CBaseHandle move_parent_handle = SE(entity)->field<CBaseHandle>("m_hMoveParent");
	if (entityList->LookupEntity(move_parent_handle)) return true;

	ICollideable *coll = &SE(entity)->collision();

	CTraceFilterSimple filter;
	filter.SetPassEntity(entity);
	filter.SetCollisionGroup(coll->GetCollisionGroup());

	FcpsTraceAdapter adapter;
	adapter.traceFunc = &fcps1Trace;
	adapter.pointOutsideWorldFunc = &fcps1PointOutsideWorld;
	adapter.traceFilter = &filter;
	adapter.mask = mask;

	Vector center, extents; // world space

	{
		Vector ent_mins = coll->OBBMins();
		Vector ent_maxs = coll->OBBMaxs();

		Vector local_center = (ent_mins + ent_maxs) / 2;
		Vector local_extents = ent_maxs - local_center;

		matrix3x4_t mat = coll->CollisionToWorldTransform();

		center = mat.VectorTransform(local_center);
		extents.x = dotProdAbs(local_extents, mat.m_flMatVal[0]);
		extents.y = dotProdAbs(local_extents, mat.m_flMatVal[1]);
		extents.z = dotProdAbs(local_extents, mat.m_flMatVal[2]);
	}

	FcpsResult result = tracedFcps(center, extents, ind_push, &adapter, false);
	g_events.push_back(result);
	console->Print("Saved FCPS1 result %d - %d iters\n", g_events.size() - 1, result.niters);

	if (result.success) {
		Vector final_pos = result.final_center + (server->GetAbsOrigin(entity) - center);
		using _Teleport = void (__rescall *)(void *entity, const Vector *pos, const QAngle *ang, const Vector *vel, bool useSlowHighAccuracyContacts);
		_Teleport Teleport = Memory::VMT<_Teleport>(entity, Offsets::StartTouch + 11);
		Teleport(entity, &final_pos, nullptr, nullptr, true);
	}

	return result.success;
}
