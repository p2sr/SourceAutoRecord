#include "Camera.hpp"

#include "Command.hpp"
#include "Features/EntityList.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Server.hpp"
#include "Modules/VGui.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#include <climits>

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

Variable sar_cam_ortho("sar_cam_ortho", "0", 0, 1, "Enables or disables camera orthographic projection");
Variable sar_cam_ortho_scale("sar_cam_ortho_scale", "1", 0.001, "Changes the scale of orthographic projection (how many units per pixel)");
Variable sar_cam_ortho_nearz("sar_cam_ortho_nearz", "1", -10000, 10000, "Changes the near Z plane of orthographic projection.");

Variable cl_skip_player_render_in_main_view;
Variable r_drawviewmodel;
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
	r_drawviewmodel = Variable("r_drawviewmodel");
	ss_force_primary_fullscreen = Variable("ss_force_primary_fullscreen");
}

Camera::~Camera() {
	camera->states.clear();
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

	//linear interp. in case you dont want anything cool
	//return points[PREV].y + (points[NEXT].y - points[PREV].y) * t;

	//cubic hermite spline... kind of? maybe? no fucking clue, just leave me alone
	float t2 = t * t, t3 = t * t * t;
	float x0 = (points[FIRST].x - points[PREV].x) / (points[NEXT].x - points[PREV].x);
	float x1 = 0, x2 = 1;
	float x3 = (points[LAST].x - points[PREV].x) / (points[NEXT].x - points[PREV].x);
	float m1 = ((points[NEXT].y - points[PREV].y) / (x2 - x1) + (points[PREV].y - points[FIRST].y) / (x1 - x0)) / 2;
	float m2 = ((points[LAST].y - points[NEXT].y) / (x3 - x2) + (points[NEXT].y - points[PREV].y) / (x2 - x1)) / 2;

	return (2 * t3 - 3 * t2 + 1) * points[PREV].y + (t3 - 2 * t2 + t) * m1 + (-2 * t3 + 3 * t2) * points[NEXT].y + (t3 - t2) * m2;
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

//Overrides view.
void Camera::OverrideView(CViewSetup *m_View) {
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
					if (engine->hoststate->m_activeGame)
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
					speed *= engine->GetHostFrameTime();

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
	if (IsDriving()) {
		cmd->buttons = 0;
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
	}
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
				campos.origin.x, campos.origin.x, campos.origin.x, campos.angles.x, campos.angles.x, campos.angles.x, campos.fov};
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

CON_COMMAND(sar_cam_path_showkfs, "sar_cam_path_showkfs - display information about all camera path keyframes\n") {
	if (!engine->demoplayer->IsPlaying())
		return console->Print("Cinematic mode cannot be used outside of demo player.\n");

	if (args.ArgC() == 1) {
		for (auto const &state : camera->states) {
			console->Print("%d: ", state.first);
			console->Print(std::string(state.second).c_str());
			console->Print("\n");
		}
	} else {
		return console->Print(sar_cam_path_showkfs.ThisPtr()->m_pszHelpString);
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

CON_COMMAND_F_COMPLETION(
	sar_cam_path_remkf,
	"sar_cam_path_remkf <frame> - removes camera path keyframe at specified frame\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_cam_path_showkf)) {
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
