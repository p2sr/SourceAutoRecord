#include "Camera.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/EntityList.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Server.hpp"
#include "Modules/VGui.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#include <climits>
#include <fstream>

Camera *camera;

Variable sar_cam_control("sar_cam_control", "0", 0, 3,
                         "sar_cam_control <type>: Change type of camera control.\n"
                         "0 = Default (camera is controlled by game engine),\n"
                         "1 = Drive mode (camera is separated and can be controlled by user input),\n"
                         "2 = Cinematic mode (camera is controlled by predefined path).\n"
                         "3 = Follow mode (Camera is following the player but not rotating, useful when strafing on gel).\n");

Variable sar_cam_drive("sar_cam_drive", "1", 0, 1,
                       "Enables or disables camera drive mode in-game "
                       "(turning it on is not required for demo player)\n");

Variable sar_cam_ortho("sar_cam_ortho", "0", 0, 1, "Enables or disables camera orthographic projection.\n");
Variable sar_cam_ortho_scale("sar_cam_ortho_scale", "1", 0.001, "Changes the scale of orthographic projection (how many units per pixel).\n");
Variable sar_cam_ortho_nearz("sar_cam_ortho_nearz", "1", -10000, 10000, "Changes the near Z plane of orthographic projection.\n");

Variable sar_cam_force_eye_pos("sar_cam_force_eye_pos", "0", 0, 1,
                       "Forces camera to be placed exactly on the player's eye position\n");

Variable sar_cam_path_interp("sar_cam_path_interp", "2", 0, 2, 
                       "Sets interpolation type between keyframes for cinematic camera.\n"
                       "0 = Linear interpolation\n"
                       "1 = Cubic spline\n"
                       "2 = Piecewise Cubic Hermite Interpolating Polynomial (PCHIP)\n"
);

Variable sar_cam_path_draw("sar_cam_path_draw", "0", 0, 1, "Draws a representation of the camera path in the world. Disabled in cinematic mode.\n");

Variable cl_skip_player_render_in_main_view;
Variable ss_force_primary_fullscreen;

void ResetCameraRelatedCvars() {
	cl_skip_player_render_in_main_view.SetValue(cl_skip_player_render_in_main_view.GetString());
	r_drawviewmodel.SetValue(r_drawviewmodel.GetString());
	ss_force_primary_fullscreen.SetValue(ss_force_primary_fullscreen.GetString());
	in_forceuser.SetValue(in_forceuser.GetString());
}

Camera::Camera() {
	controlType = Default;

	//a bunch of console variables later used in a janky hack
	cl_skip_player_render_in_main_view = Variable("cl_skip_player_render_in_main_view");
	ss_force_primary_fullscreen = Variable("ss_force_primary_fullscreen");
}

Camera::~Camera() {
	camera->states.clear();
	ResetCameraRelatedCvars();
}

ON_EVENT(SAR_UNLOAD) {
	ResetCameraRelatedCvars();
}

//if in drive mode, checks if player wants to control the camera
//for now it requires LMB input (as in demo drive mode)
bool Camera::IsDriving() {
	bool drivingInGame = sar_cam_drive.GetBool() && sv_cheats.GetBool() && engine->hoststate->m_activeGame;
	bool drivingInDemo = engine->demoplayer->IsPlaying();
	bool wantingToDrive = inputSystem->IsKeyDown(ButtonCode_t::MOUSE_LEFT);
	bool isUI = vgui->IsUIVisible();

	return (camera->controlType == Drive || camera->controlType == Follow) && wantingToDrive && (drivingInGame || drivingInDemo) && !isUI;
}

//used by camera state interpolation function. all the math is here.
float InterpolateCurve(std::vector<Vector> points, float x, bool dealingWithAngles = false) {
	enum { FIRST,
		      PREV,
		      NEXT,
		      LAST };

	//fixing Y values in case they're angles, for proper interpolation
	if (dealingWithAngles) {
		float oldY = 0;
		for (int i = 0; i < 4; i++) {
			float angDif = points[i].y - oldY;
			angDif += (angDif > 180) ? -360 : (angDif < -180) ? 360
																																																					: 0;
			points[i].y = oldY += angDif;
			oldY = points[i].y;
		}
	}

	if (x <= points[PREV].x)
		return points[PREV].y;
	if (x >= points[NEXT].x)
		return points[NEXT].y;

	float t = (x - points[PREV].x) / (points[NEXT].x - points[PREV].x);

	switch (sar_cam_path_interp.GetInt()) {
	case 1: {
		// cubic spline... i think? No idea what the fuck 2019 me has put here
		// and it's not like i got any more intelligent over time
		// ~Krzyhau
		float t2 = t * t, t3 = t * t * t;
		float x0 = (points[FIRST].x - points[PREV].x) / (points[NEXT].x - points[PREV].x);
		float x1 = 0, x2 = 1;
		float x3 = (points[LAST].x - points[PREV].x) / (points[NEXT].x - points[PREV].x);
		float m1 = ((points[NEXT].y - points[PREV].y) / (x2 - x1) + (points[PREV].y - points[FIRST].y) / (x1 - x0)) / 2;
		float m2 = ((points[LAST].y - points[NEXT].y) / (x3 - x2) + (points[NEXT].y - points[PREV].y) / (x2 - x1)) / 2;

		return (2 * t3 - 3 * t2 + 1) * points[PREV].y + (t3 - 2 * t2 + t) * m1 + (-2 * t3 + 3 * t2) * points[NEXT].y + (t3 - t2) * m2;
	}
	case 2: {
		//very sloppy implementation of pchip. no idea what I'm doing here
		float ds[4];
		float hl = 0, dl = 0;
		for (int i = 0; i < 3; i++) {
			float hr = points[i + 1].x - points[i].x;
			float dr = (points[i + 1].y - points[i].y) / hr;

			if (i == 0 || dl*dr < 0.0f || dl == 0.0f || dr == 0.0f) {
				ds[i] = 0;
			} else {
				float wl = 2 * hl + hr;
				float wr = hl + 2 * hr;
				ds[i] = (wl + wr) / (wl / dl + wr / dr);
			}
				
			hl = hr;
			dl = dr;
		}
		// normally you'd calculate edge derivatives but i dont need them here
		// so only 1st and 2nd is set

		float h = points[NEXT].x - points[PREV].x;

		float t1 = (points[NEXT].x - x) / h;
		float t2 = (x - points[PREV].x) / h;

		float f1 = points[PREV].y * (3.0f * t1 * t1 - 2.0f * t1 * t1 * t1);
		float f2 = points[NEXT].y * (3.0f * t2 * t2 - 2.0f * t2 * t2 * t2);
		float f3 = ds[PREV] * h * (t1 * t1 * t1 - t1 * t1);
		float f4 = ds[NEXT] * h * (t2 * t2 * t2 - t2 * t2);

		return f1 + f2 - f3 + f4;
	}
	default:
		//linear interp. in case you dont want anything cool
		return points[PREV].y + (points[NEXT].y - points[PREV].y) * t;
	}
}

//creates vector array for specified parameter. it can probably be done in much more elegant way
std::vector<Vector> CameraStatesToInterpolationPoints(float *x, CameraState *y, CameraStateParameter param) {
	std::vector<Vector> points;
	for (int i = 0; i < 4; i++) {
		Vector v;
		v.x = x[i];
		CameraState cs = y[i];
		switch (param) {
		case ORIGIN_X:
			v.y = cs.origin.x;
			break;
		case ORIGIN_Y:
			v.y = cs.origin.y;
			break;
		case ORIGIN_Z:
			v.y = cs.origin.z;
			break;
		case ANGLES_X:
			v.y = cs.angles.x;
			break;
		case ANGLES_Y:
			v.y = cs.angles.y;
			break;
		case ANGLES_Z:
			v.y = cs.angles.z;
			break;
		case FOV:
			v.y = cs.fov;
			break;
		}
		points.push_back(v);
	}
	return points;
}

//Creates interpolated camera state based on states array and given time.
//This is the closest I could get to valve's demo spline camera path
CameraState Camera::InterpolateStates(float time) {
	enum { FIRST,
		      PREV,
		      NEXT,
		      LAST };

	//reading 4 frames closest to time
	float frameTime = time * 60.0f;
	int frames[4] = {INT_MIN, INT_MIN, INT_MAX, INT_MAX};
	for (auto const &state : camera->states) {
		int stateTime = state.first;
		if (frameTime - stateTime >= 0 && frameTime - stateTime < frameTime - frames[PREV]) {
			frames[FIRST] = frames[PREV];
			frames[PREV] = stateTime;
		}
		if (stateTime - frameTime >= 0 && stateTime - frameTime < frames[NEXT] - frameTime) {
			frames[LAST] = frames[NEXT];
			frames[NEXT] = stateTime;
		}
		if (stateTime > frames[FIRST] && stateTime < frames[PREV]) {
			frames[FIRST] = stateTime;
		}
		if (stateTime > frames[NEXT] && stateTime < frames[LAST]) {
			frames[LAST] = stateTime;
		}
	}

	//x values for interpolation
	float x[4];
	for (int i = 0; i < 4; i++) {
		x[i] = (float)frames[i];
	}

	//making sure all X values are correct before reading Y values,
	//"frames" fixed for read, "x" fixed for interpolation.
	//if there's at least one cam state in the map, none of these should be MIN or MAX after this.
	if (frames[PREV] == INT_MIN) {
		x[PREV] = (float)frames[NEXT];
		frames[PREV] = frames[NEXT];
	}
	if (frames[NEXT] == INT_MAX) {
		x[NEXT] = (float)frames[PREV];
		frames[NEXT] = frames[PREV];
	}
	if (frames[FIRST] == INT_MIN) {
		x[FIRST] = (float)(2 * frames[PREV] - frames[NEXT]);
		frames[FIRST] = frames[PREV];
	}
	if (frames[LAST] == INT_MAX) {
		x[LAST] = (float)(2 * frames[NEXT] - frames[PREV]);
		frames[LAST] = frames[NEXT];
	}

	//filling Y values
	CameraState y[4];
	for (int i = 0; i < 4; i++) {
		y[i] = camera->states[frames[i]];
	}

	//interpolating each parameter
	CameraState interp;
	interp.origin.x = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, ORIGIN_X), frameTime);
	interp.origin.y = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, ORIGIN_Y), frameTime);
	interp.origin.z = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, ORIGIN_Z), frameTime);
	interp.angles.x = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, ANGLES_X), frameTime, true);
	interp.angles.y = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, ANGLES_Y), frameTime, true);
	interp.angles.z = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, ANGLES_Z), frameTime, true);
	interp.fov = InterpolateCurve(CameraStatesToInterpolationPoints(x, y, FOV), frameTime);

	return interp;
}


void Camera::DrawInWorld() const {

	if (camera->states.size() < 2) return;

	if (!(sv_cheats.GetBool() || engine->demoplayer->IsPlaying()) || !sar_cam_path_draw.GetBool() || sar_cam_control.GetInt() == 2) return;

	MeshId mesh_path = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 255, 255 }, true));
	MeshId mesh_cams = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 0, 0 }, true));
	MeshId mesh_currentCam = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 255, 0 }, true));

	float frameTime = 1.0 / 60;
	int maxTimeTicks = 0;
	int minTimeTicks = INT_MAX;
	for (auto const &state : camera->states) {
		maxTimeTicks = std::fmaxf(maxTimeTicks, state.first);
		minTimeTicks = std::fminf(minTimeTicks, state.first);
	}

	// changing in-game ticks to seconds.
	float maxTime = maxTimeTicks * frameTime;
	float minTime = minTimeTicks * frameTime;

	// for each frame, calculate interpolated path
	Vector pos = camera->InterpolateStates(minTime).origin;
	for (float t = minTime; t <= maxTime + frameTime; t += frameTime) {
		Vector new_pos = camera->InterpolateStates(t).origin;

		// Don't draw a 0 length line
		float pos_delta = (pos - new_pos).Length();
		if (pos_delta > 0.001) {
			OverlayRender::addLine(mesh_path, pos, new_pos);
			pos = new_pos;
		}
	}

	// draw fov things at each keyframe and the current one
	// the way this is done is rather sacrilegious

	float currentTime = engine->GetClientTime() - timeOffset;
	CameraState currentCameraState = camera->InterpolateStates(currentTime);

	std::vector<int> keyframeTicks(camera->states.size());
	int i = 0;
	for (auto const &state : camera->states) {
		keyframeTicks[i++] = state.first;
	}

	int w, h;
	engine->GetScreenSize(nullptr, w, h);
	float aspect = (float)h / (float)w;

	Vector uvs[] = {
			{-1, -1},
			{ 1, -1},
			{ 1,  1},
			{-1,  1},
	};

	for (size_t stateI = 0; stateI < camera->states.size() + 1; stateI++) {
		bool isKeyframe = stateI < camera->states.size();
		auto state = isKeyframe ? camera->states[keyframeTicks[stateI]] : currentCameraState;
		auto mesh = isKeyframe ? mesh_cams : mesh_currentCam;

		OverlayRender::addBoxMesh(
			state.origin,
			{ -2, -2, -2 },
			{  2,  2,  2 },
			state.angles,
			RenderCallback::constant({ 255, (uint8_t)(isKeyframe ? 0 : 255), 0, 20 }, true),
			RenderCallback::constant({ 255, (uint8_t)(isKeyframe ? 0 : 255), 0, 255}, true)
		);
		
		Vector forward, right, up;
		float cp, sp, cy, sy, cr, sr;
		cp = cosf(DEG2RAD(state.angles.x));
		sp = sinf(DEG2RAD(state.angles.x));
		cy = cosf(DEG2RAD(state.angles.y));
		sy = sinf(DEG2RAD(state.angles.y));
		cr = cosf(DEG2RAD(state.angles.z));
		sr = sinf(DEG2RAD(state.angles.z));

		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;

		right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right.y = (-1 * sr * sp * sy + -1 * cr * cy);
		right.z = -1 * sr * cp;

		up = right.Cross(forward);


		float fovScalar = tanf(DEG2RAD(state.fov / 2));

		Vector points[4];

		for (int i = 0; i < 4; i++) {
			points[i] = state.origin + (forward + right * fovScalar * uvs[i].x + up * fovScalar * aspect * uvs[i].y) * 5;
			OverlayRender::addLine(mesh, state.origin, points[i]);
			if (i > 0) OverlayRender::addLine(mesh, points[i - 1], points[i]);
			if (i == 3) OverlayRender::addLine(mesh, points[i], points[0]);
		}
	}
}

ON_EVENT(RENDER) {
	camera->DrawInWorld();
}

//Overrides view.
void Camera::OverrideView(CViewSetup *m_View) {
	rawState.origin = m_View->origin;
	rawState.angles = m_View->angles;
	rawState.fov = m_View->fov;

	static std::chrono::time_point<std::chrono::steady_clock> last_frame;
	auto now = NOW_STEADY();
	float real_frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame).count();
	last_frame = now;

	if (sar_cam_force_eye_pos.GetBool() && sv_cheats.GetBool()) {
		Vector eyePos;
		QAngle eyeAng;
		if (GetEyePos<false>(GET_SLOT(), eyePos, eyeAng)) {
			m_View->angles = eyeAng;
			m_View->origin = eyePos;
		}
	}

	if (timeOffsetRefreshRequested) {
		timeOffset = engine->GetClientTime() - engine->demoplayer->GetTick() / 60.0f;
		timeOffsetRefreshRequested = false;
	}

	auto newControlType = static_cast<CameraControlType>(sar_cam_control.GetInt());

	//don't allow cinematic mode outside of demo player
	if (!engine->demoplayer->IsPlaying() && newControlType == Cinematic) {
		if (controlType != Cinematic) {
			console->Print("Cinematic mode cannot be used outside of demo player.\n");
		} else {
			controlType = Default;
			ResetCameraRelatedCvars();
		}
		newControlType = controlType;
		sar_cam_control.SetValue(controlType);
	}

	//don't allow drive mode when not using sv_cheats
	if (newControlType == Drive && !sv_cheats.GetBool() && !engine->demoplayer->IsPlaying()) {
		if (controlType != Drive) {
			console->Print("Drive mode requires sv_cheats 1 or demo player.\n");
		} else {
			controlType = Default;
			ResetCameraRelatedCvars();
		}
		newControlType = controlType;
		sar_cam_control.SetValue(controlType);
	}

	//don't allow follow mode when not using sv_cheats
	if (newControlType == Follow && !sv_cheats.GetBool() && !engine->demoplayer->IsPlaying()) {
		if (controlType != Follow) {
			console->Print("Follow mode requires sv_cheats 1 or demo player.\n");
		} else {
			controlType = Default;
			ResetCameraRelatedCvars();
		}
		newControlType = controlType;
		sar_cam_control.SetValue(controlType);
	}

	//janky hack mate
	//overriding cvar values, boolean (int) values only.
	//this way the cvar themselves are unchanged
	if (newControlType != Default) {
		cl_skip_player_render_in_main_view.ThisPtr()->m_nValue = 0;
		r_drawviewmodel.ThisPtr()->m_nValue = 0;
		ss_force_primary_fullscreen.ThisPtr()->m_nValue = 1;
	}

	//handling camera control type switching
	if (newControlType != controlType) {
		if (controlType == Default && newControlType != Default) {
			//enabling
			if (newControlType == Follow)
				this->RequestCameraRefresh();
		} else if (controlType != Default && newControlType == Default) {
			//disabling
			//resetting cvars to their actual values when switching control off
			ResetCameraRelatedCvars();
			this->manualActive = false;
		}
		controlType = newControlType;
	}

	//don't do anything if not in game or demo player
	if (engine->hoststate->m_activeGame || engine->demoplayer->IsPlaying()) {
		if (controlType == Default || cameraRefreshRequested) {
			currentState.origin = (controlType == Follow ? Vector(0, 0, 64) : m_View->origin);
			currentState.angles = m_View->angles;
			currentState.fov = m_View->fov;
			if (cameraRefreshRequested)
				cameraRefreshRequested = false;
		}
		if (controlType != Default) {
			//manual camera view control
			if (controlType == Drive || controlType == Follow) {
				if (IsDriving()) {
					if (!manualActive) {
						inputSystem->GetCursorPos(mouseHoldPos[0], mouseHoldPos[1]);
						manualActive = true;
					}
				} else {
					if (manualActive) {
						in_forceuser.SetValue(in_forceuser.GetString());
						manualActive = false;
					}
				}
				if (manualActive) {
					//even junkier hack. lock mouse movement using fake in_forceuser 2 LMAO
					if (engine->hoststate->m_activeGame && !tasPlayer->IsActive())
						in_forceuser.ThisPtr()->m_nValue = 2;

					//getting mouse movement and resetting the cursor position
					int mX, mY;
					inputSystem->GetCursorPos(mX, mY);
					mX -= mouseHoldPos[0];
					mY -= mouseHoldPos[1];
					inputSystem->SetCursorPos(mouseHoldPos[0], mouseHoldPos[1]);

					if (inputSystem->IsKeyDown(ButtonCode_t::MOUSE_RIGHT)) {
						//allow fov manipulation
						currentState.fov = fminf(fmaxf(currentState.fov + mY * 0.22f, 0.1f), 179.0f);
						currentState.angles.z += (float)mX * 0.22f;
					} else {
						currentState.angles.x += (float)mY * 0.22f;
						currentState.angles.y -= (float)mX * 0.22f;
					}

					//limit both values between -180 and 180
					currentState.angles.x = remainderf(currentState.angles.x, 360.0f);
					currentState.angles.y = remainderf(currentState.angles.y, 360.0f);

					//calculating vectors out of angles for movement
					Vector forward, right;
					float cp, sp, cy, sy, cr, sr;
					cp = cosf(DEG2RAD(currentState.angles.x));
					sp = sinf(DEG2RAD(currentState.angles.x));
					cy = cosf(DEG2RAD(currentState.angles.y));
					sy = sinf(DEG2RAD(currentState.angles.y));
					cr = cosf(DEG2RAD(currentState.angles.z));
					sr = sinf(DEG2RAD(currentState.angles.z));

					forward.x = cp * cy;
					forward.y = cp * sy;
					forward.z = -sp;

					right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
					right.y = (-1 * sr * sp * sy + -1 * cr * cy);
					right.z = -1 * sr * cp;

					//applying movement
					bool shiftdown = inputSystem->IsKeyDown(KEY_LSHIFT) || inputSystem->IsKeyDown(KEY_RSHIFT);
					bool controldown = inputSystem->IsKeyDown(KEY_LCONTROL) || inputSystem->IsKeyDown(KEY_RCONTROL);
					float speed = shiftdown ? 525.0f : (controldown ? 60.0f : 175.0f);
					speed *= engine->IsAdvancing() ? real_frame_time : engine->GetHostFrameTime();

					if (inputSystem->IsKeyDown(ButtonCode_t::KEY_W)) {
						currentState.origin = currentState.origin + (forward * speed);
					}
					if (inputSystem->IsKeyDown(ButtonCode_t::KEY_S)) {
						currentState.origin = currentState.origin + (forward * -speed);
					}
					if (inputSystem->IsKeyDown(ButtonCode_t::KEY_A)) {
						currentState.origin = currentState.origin + (right * -speed);
					}
					if (inputSystem->IsKeyDown(ButtonCode_t::KEY_D)) {
						currentState.origin = currentState.origin + (right * speed);
					}
				}
			}
			//cinematic camera - move it along predefined path
			if (controlType == Cinematic) {
				//don't do interpolation when there are no points
				if (states.size() > 0) {
					float currentTime = engine->GetClientTime() - timeOffset;
					currentState = InterpolateStates(currentTime);
				}
			}
			//applying custom view
			if (controlType == Follow) {
				auto player = client->GetPlayer(GET_SLOT() + 1);
				if (player)
					m_View->origin = (client->GetAbsOrigin(player) + currentState.origin);
			} else {
				m_View->origin = currentState.origin;
			}
			m_View->angles = currentState.angles;
			m_View->fov = currentState.fov;
		}
	}
	if (sar_cam_ortho.GetBool() && sv_cheats.GetBool()) {
		m_View->m_bOrtho = true;

		int width, height;
		engine->GetScreenSize(nullptr, width, height);

		float halfWidth = width * 0.5f * sar_cam_ortho_scale.GetFloat();
		float halfHeight = height * 0.5f * sar_cam_ortho_scale.GetFloat();

		m_View->m_OrthoRight = halfWidth;
		m_View->m_OrthoLeft = -halfWidth;
		m_View->m_OrthoBottom = halfHeight;
		m_View->m_OrthoTop = -halfHeight;

		m_View->zNear = sar_cam_ortho_nearz.GetFloat();
	}
}

void Camera::RequestTimeOffsetRefresh() {
	timeOffsetRefreshRequested = true;
}

void Camera::RequestCameraRefresh() {
	cameraRefreshRequested = true;
}

void Camera::OverrideMovement(CUserCmd *cmd) {
	//blocking keyboard inputs on manual mode
	if (IsDriving() && !tasPlayer->IsActive()) {
		cmd->buttons = 0;
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
	}
}


Vector Camera::GetPosition(int slot, bool raw) {
	bool cam_control = sar_cam_control.GetInt() == 1 && sv_cheats.GetBool();

	Vector cam_pos = (!raw && cam_control) ? camera->currentState.origin : rawState.origin;

	return cam_pos;
}

Vector Camera::GetForwardVector(int slot, bool raw) {
	bool cam_control = sar_cam_control.GetInt() == 1 && sv_cheats.GetBool();

	QAngle ang = (!raw && cam_control) ? camera->currentState.angles : rawState.angles;
	Vector view_vec = Vector{
		cosf(DEG2RAD(ang.y)) * cosf(DEG2RAD(ang.x)),
		sinf(DEG2RAD(ang.y)) * cosf(DEG2RAD(ang.x)),
		-sinf(DEG2RAD(ang.x)),
	}.Normalize();

	return view_vec;
}


//COMMANDS

DECL_COMMAND_COMPLETION(sar_cam_path_setkf) {
	for (auto const &state : camera->states) {
		if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
			break;
		}

		char camString[512] = {};
		CameraState cam = state.second;
		sprintf(camString, "%d %.0f %.0f %.0f %.0f %.0f %.0f %.0f", state.first, cam.origin.x, cam.origin.y, cam.origin.z, cam.angles.x, cam.angles.y, cam.angles.z, cam.fov);

		camString[COMMAND_COMPLETION_ITEM_LENGTH - 1 - 19] = '\0';
		std::string camStringString(&camString[0], COMMAND_COMPLETION_ITEM_LENGTH - 19);

		if (std::strlen(match) != std::strlen(cmd)) {
			if (std::strstr(camStringString.c_str(), match)) {
				items.push_back(camStringString);
			}
		} else {
			items.push_back(camStringString);
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(
	sar_cam_path_setkf,
	"sar_cam_path_setkf [frame] [x] [y] [z] [yaw] [pitch] [roll] [fov] - sets the camera path keyframe\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_cam_path_setkf)) {
	if (!engine->demoplayer->IsPlaying())
		return console->Print("Cinematic mode cannot be used outside of demo player.\n");

	if (args.ArgC() >= 1 && args.ArgC() <= 9) {
		CameraState campos = camera->currentState;
		int curFrame = engine->demoplayer->GetTick();
		if (args.ArgC() > 1) {
			curFrame = std::atoi(args[1]);
		}
		if (args.ArgC() > 2) {
			float nums[7] = {
				campos.origin.x, campos.origin.y, campos.origin.z, campos.angles.x, campos.angles.y, campos.angles.z, campos.fov};
			for (int i = 0; i < args.ArgC() - 2; i++) {
				nums[i] = std::stof(args[i + 2]);
			}
			campos.origin.x = nums[0];
			campos.origin.y = nums[1];
			campos.origin.z = nums[2];
			campos.angles.x = nums[3];
			campos.angles.y = nums[4];
			campos.angles.z = nums[5];
			campos.fov = nums[6];
		}
		camera->states[curFrame] = campos;
		console->Print("Camera key frame %d created: ", curFrame);
		console->Print(std::string(campos).c_str());
		console->Print("\n");
	} else {
		return console->Print(sar_cam_path_setkf.ThisPtr()->m_pszHelpString);
	}
}

DECL_COMMAND_COMPLETION(sar_cam_path_showkf) {
	for (auto const &state : camera->states) {
		if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
			break;
		}

		char camString[64] = {};
		//CameraState cam = state.second;
		sprintf(camString, "%d", state.first);

		std::string camStringString = camString;

		if (std::strlen(match) != std::strlen(cmd)) {
			if (std::strstr(camStringString.c_str(), match)) {
				items.push_back(camStringString);
			}
		} else {
			items.push_back(camString);
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(
	sar_cam_path_showkf,
	"sar_cam_path_showkf <frame> - display information about camera path keyframe at specified frame\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_cam_path_showkf)) {
	if (!engine->demoplayer->IsPlaying())
		return console->Print("Cinematic mode cannot be used outside of demo player.\n");

	if (args.ArgC() == 2) {
		int i = std::atoi(args[1]);
		if (camera->states.count(i)) {
			console->Print(std::string(camera->states[i]).c_str());
			console->Print("\n");
		} else {
			console->Print("This keyframe does not exist.\n");
		}
	} else {
		return console->Print(sar_cam_path_showkf.ThisPtr()->m_pszHelpString);
	}
}

CON_COMMAND(sar_cam_path_getkfs, "sar_cam_path_getkfs - exports commands for recreating currently made camera path\n") {
	if (!engine->demoplayer->IsPlaying())
		return console->Print("Cinematic mode cannot be used outside of demo player.\n");

	if (args.ArgC() == 1) {
		for (auto const &state : camera->states) {
			CameraState cam = state.second;
			console->Print("sar_cam_path_setkf %d %f %f %f %f %f %f %f;\n", state.first, cam.origin.x, cam.origin.y, cam.origin.z, cam.angles.x, cam.angles.y, cam.angles.z, cam.fov);
		}
	} else {
		return console->Print(sar_cam_path_getkfs.ThisPtr()->m_pszHelpString);
	}
}



DECL_COMMAND_COMPLETION(sar_cam_path_remkf) {
	for (auto const &state : camera->states) {
		if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
			break;
		}

		char camString[64] = {};
		//CameraState cam = state.second;
		sprintf(camString, "%d", state.first);

		std::string camStringString = camString;

		if (std::strlen(match) != std::strlen(cmd)) {
			if (std::strstr(camStringString.c_str(), match)) {
				items.push_back(camStringString);
			}
		} else {
			items.push_back(camString);
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(
	sar_cam_path_remkf,
	"sar_cam_path_remkf <frame> - removes camera path keyframe at specified frame\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_cam_path_remkf)) {
	if (!engine->demoplayer->IsPlaying())
		return console->Print("Cinematic mode cannot be used outside of demo player.\n");

	if (args.ArgC() == 2) {
		int i = std::atoi(args[1]);
		if (camera->states.count(i)) {
			camera->states.erase(i);
			console->Print("Camera path keyframe at frame %d removed.\n", i);
		} else {
			console->Print("This keyframe does not exist.\n");
		}
	} else {
		return console->Print(sar_cam_path_remkf.ThisPtr()->m_pszHelpString);
	}
}

CON_COMMAND(sar_cam_path_remkfs, "sar_cam_path_remkfs - removes all camera path keyframes\n") {
	if (!engine->demoplayer->IsPlaying())
		return console->Print("Cinematic mode cannot be used outside of demo player.\n");

	if (args.ArgC() == 1) {
		camera->states.clear();
		console->Print("All camera path keyframes have been removed.\n");
	} else {
		return console->Print(sar_cam_path_remkfs.ThisPtr()->m_pszHelpString);
	}
}

CON_COMMAND(sar_cam_path_goto, "sar_cam_path_goto <frame> [skipto] - sends the camera to a specified frame of the camera path. If skipto is 1, goto the tick in the demo.\n") {
	if (args.ArgC() < 2) {
		return console->Print(sar_cam_path_goto.ThisPtr()->m_pszHelpString);
	}
	
	if (camera->controlType != Drive) {
		console->Print("Camera not in drive mode! Switching.\n");
		sar_cam_control.SetValue(CameraControlType::Drive);
	}

	int i = std::atoi(args[1]);
	int maxTimeTicks = 0;
	int minTimeTicks = INT_MAX;
	for (auto const &state : camera->states) {
		maxTimeTicks = std::fmaxf(maxTimeTicks, state.first);
		minTimeTicks = std::fminf(minTimeTicks, state.first);
	}
	if (i < minTimeTicks || i > maxTimeTicks) {
		return console->Print("This frame does not exist.\n");
	}
	
	camera->currentState = camera->InterpolateStates(i * (1.0 / 60));

	if (args.ArgC() == 3 && std::atoi(args[2]) == 1) {
		if (engine->demoplayer->IsPlaying()) {
			// TODO: Make this pause when rewinding (remove_broken ignores demo_pause)
			engine->ExecuteCommand(Utils::ssprintf("demo_gototick %d; demo_pause", i).c_str(), true);
		}
	}

}

CON_COMMAND(sar_cam_path_export, 
	"sar_cam_path_export <filename> [format] [framerate] - exports current camera path to a given file in given format.\n"
	"Available formats:\n"
	"kf - default, exports commands that can be used to recreate camera path. Does not use rate parameter.\n"
	"raw - exports a dump of raw camera position for each frame in given framerate (60 by default).\n"
	"davinci - exports a script for DaVinci Resolve's Camera 3D Fusion component based on raw camera dump.\n"
) {
	if (args.ArgC() < 2 || args.ArgC() > 4) {
		return console->Print(sar_cam_path_export.ThisPtr()->m_pszHelpString);
	}

	if (camera->states.size() == 0) {
		return console->Print("No camera path has been defined.\n");
	}

	std::string filename = args[1];
	std::string format = args.ArgC() >= 3 ? args[2] : "kf";

	int rate = args.ArgC() == 4 ? std::atoi(args[3]) : 60;
	if (rate <= 0) rate = 60; 

	// check if file exists before writing
	std::ifstream testFile(filename.c_str());
	if (testFile.good()) {
		testFile.close();
		return console->Print("File \"%s\" exists and cannot be overwritten.\n", filename.c_str());
	}
	testFile.close();

	std::ofstream file(filename.c_str());

	if (format == "kf") {
		// dump keyframes comamnds
		file << "sar_cam_path_remkfs\n";
		file << Utils::ssprintf("sar_cam_path_interp %d\n", sar_cam_path_interp.GetInt());
		for (auto const &state : camera->states) {
			CameraState cam = state.second;
			file << Utils::ssprintf("sar_cam_path_setkf %d %f %f %f %f %f %f %f\n", state.first, cam.origin.x, cam.origin.y, cam.origin.z, cam.angles.x, cam.angles.y, cam.angles.z, cam.fov);
		}
	} else if (format == "raw" || format == "davinci") {
		// dump raw interpolated camera positions 
		// both davinci and raw work similar in this case

		bool daVinci = (format == "davinci");

		float frameTime = 1.0 / rate;
		int maxTimeTicks = 0;
		int minTimeTicks = INT_MAX;
		for (auto const &state : camera->states) {
			maxTimeTicks = std::fmaxf(maxTimeTicks, state.first);
			minTimeTicks = std::fminf(minTimeTicks, state.first);
		}

		// changing in-game ticks to seconds.
		float maxTime = maxTimeTicks / 60.0f;
		float minTime = minTimeTicks / 60.0f;

		if (daVinci) file << "C={\n";

		// for each frame, calculate interpolated path
		for (float t = minTime; t <= maxTime + frameTime; t += frameTime) {
			auto cam = camera->InterpolateStates(t);
			if (daVinci) {
				file << Utils::ssprintf("[%f]={%f,%f,%f,%f,%f,%f,%f},\n", t, cam.origin.x, cam.origin.y, cam.origin.z, cam.angles.x, cam.angles.y, cam.angles.z, cam.fov);
			} else {
				file << Utils::ssprintf("%f %f %f %f %f %f %f %f\n", t, cam.origin.x, cam.origin.y, cam.origin.z, cam.angles.x, cam.angles.y, cam.angles.z, cam.fov);
			}
		}

		// finish boilerplate code for davinci output (also changing some variables to map it properly in Resolve's space)
		if (daVinci) {
			file << Utils::ssprintf("}\nn=9999999;m=0;t=time/%f;", (float)rate);
			file << "for i,v in pairs(C) do a=math.abs(i-t);if a<n then n=a;m=v; end end\n";
			file << "self.Transform3DOp.Translate.X = -m[2]\n"
					"self.Transform3DOp.Translate.Y = m[3]\n"
					"self.Transform3DOp.Translate.Z = -m[1]\n"
					"self.Transform3DOp.Rotate.X = -m[4]\n"
					"self.Transform3DOp.Rotate.Y = m[5]\n"
					"self.Transform3DOp.Rotate.Z = m[6]\n"
					"self.AoV = math.deg(math.atan(160.0/m[7]))"; // that will work as long as resolution is 16x9
		}
		
	} else {
		return console->Print("Unknown camera path format \"%s\".\n", format.c_str());
	}

	file.close();
	
	console->Print("Saved camera path to \"%s\".\n", filename.c_str());
}


CON_COMMAND(sar_cam_setang, "sar_cam_setang <pitch> <yaw> [roll] - sets camera angle (requires camera Drive Mode)\n") {
	if (camera->controlType != Drive) {
		console->Print("Camera not in drive mode! Switching.\n");
		sar_cam_control.SetValue(CameraControlType::Drive);
	}

	if (args.ArgC() == 3 || args.ArgC() == 4) {
		float angles[3] = {0, 0, 0};
		for (int i = 0; i < args.ArgC() - 1; i++) {
			angles[i] = std::stof(args[i + 1]);
		}
		camera->currentState.angles.x = angles[0];
		camera->currentState.angles.y = angles[1];
		camera->currentState.angles.z = angles[2];
	} else {
		return console->Print(sar_cam_setang.ThisPtr()->m_pszHelpString);
	}
}

CON_COMMAND(sar_cam_setpos, "sar_cam_setpos <x> <y> <z> - sets camera position (requires camera Drive Mode)\n") {
	if (camera->controlType != Drive) {
		console->Print("Camera not in drive mode! Switching.\n");
		sar_cam_control.SetValue(CameraControlType::Drive);
	}

	if (args.ArgC() == 4) {
		float pos[3] = {0, 0, 0};
		for (int i = 0; i < args.ArgC() - 1; i++) {
			pos[i] = std::stof(args[i + 1]);
		}
		camera->currentState.origin.x = pos[0];
		camera->currentState.origin.y = pos[1];
		camera->currentState.origin.z = pos[2];
	} else {
		return console->Print(sar_cam_setpos.ThisPtr()->m_pszHelpString);
	}
}

CON_COMMAND(sar_cam_setfov, "sar_cam_setfov <fov> - sets camera field of view (requires camera Drive Mode)\n") {
	if (camera->controlType != Drive) {
		console->Print("Camera not in drive mode! Switching.\n");
		sar_cam_control.SetValue(CameraControlType::Drive);
	}

	if (args.ArgC() == 2) {
		float fov = (float)std::atof(args[1]);
		camera->currentState.fov = fov;
	} else {
		return console->Print(sar_cam_setfov.ThisPtr()->m_pszHelpString);
	}
}

CON_COMMAND(sar_cam_reset, "sar_cam_reset - resets camera to its default position\n") {
	if (args.ArgC() == 1) {
		if (camera->controlType == Drive) {
			camera->RequestCameraRefresh();
		}
	} else {
		return console->Print(sar_cam_reset.ThisPtr()->m_pszHelpString);
	}
}
