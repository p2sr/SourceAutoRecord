#include "Camera.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Server.hpp"

#include "Features/EntityList.hpp"

#include "Command.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

Camera* camera;

Variable sar_cam_control("sar_cam_control", "0", 0, 2,
    "sar_cam_control <type>: Change type of camera control.\n"
    "0 = Default (camera is controlled by game engine),\n"
    "1 = Drive mode (camera is separated and can be controlled by user input),\n"
    "2 = Cinematic mode (camera is controlled by predefined path).\n");

Variable sar_cam_drive("sar_cam_drive", "0", 0, 1, 
    "Enables or disables camera drive mode in-game "
    "(turning it on is not required for demo player)\n");

Variable cl_skip_player_render_in_main_view;
Variable r_drawviewmodel;
Variable ss_force_primary_fullscreen;
Variable crosshair;

Camera::Camera()
{
    controlType = Disabled;

    //a bunch of console variables later used in a janky hack
    cl_skip_player_render_in_main_view = Variable("cl_skip_player_render_in_main_view");
    r_drawviewmodel = Variable("r_drawviewmodel");
    ss_force_primary_fullscreen = Variable("ss_force_primary_fullscreen");
    crosshair = Variable("crosshair");
}

Camera::~Camera()
{
    camera->states.clear();

    //if (sar_democam_cvar_autochange.GetBool()) {
    //    cl_skip_player_render_in_main_view.SetValue(1);
    //    r_drawviewmodel.SetValue(1);
    //}
}

//if in drive mode, checks if player wants to control the camera
//for now it requires LMB input (as in demo drive mode)
bool Camera::IsDriving() {
    bool drivingInGame = sar_cam_drive.GetBool() && sv_cheats.GetBool() && engine->hoststate->m_activeGame;
    bool drivingInDemo = engine->demoplayer->IsPlaying();
    bool wantingToDrive = inputSystem->IsKeyDown(ButtonCode_t::MOUSE_LEFT);
    
    return camera->controlType == Manual && wantingToDrive && (drivingInGame || drivingInDemo);
}

//Calculates demo camera parameter value based on time and predefined path
float Camera::InterpolateStateParam(CameraStateParameter param, float time)
{
    enum { FIRST,
        PREV,
        NEXT,
        LAST };

    //reading closest 4 frames
    float frameTime = time * 60.0f;
    int frames[4] = { INT_MIN, INT_MIN, INT_MAX, INT_MAX };
    for (auto const& state : camera->states) {
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

    //points for interpolation
    Vector points[4];

    //filling X values
    for (int i = 0; i < 4; i++) {
        points[i].x = (float)frames[i];
    }

    //making sure all X values are correct before reading Y values,
    //"frames" fixed for read, "points" fixed for interpolation.
    //if there's at least one cam state in the map, none of these should be MIN or MAX after this.
    if (frames[PREV] == INT_MIN) {
        points[PREV].x = (float)frames[NEXT];
        frames[PREV] = frames[NEXT];
    }
    if (frames[NEXT] == INT_MAX) {
        points[NEXT].x = (float)frames[PREV];
        frames[NEXT] = frames[PREV];
    }
    if (frames[FIRST] == INT_MIN) {
        points[FIRST].x = (float)(2 * frames[PREV] - frames[NEXT]);
        frames[FIRST] = frames[PREV];
    }
    if (frames[LAST] == INT_MAX) {
        points[LAST].x = (float)(2 * frames[NEXT] - frames[PREV]);
        frames[LAST] = frames[NEXT];
    }

    bool workWithAngles = param == ANGLES_X || param == ANGLES_Y || param == ANGLES_Z;

    //filling Y values
    float oldY = 0;
    for (int i = 0; i < 4; i++) {
        CameraState state = camera->states[frames[i]];
        switch (param) {
        case ANGLES_X:
            points[i].y = state.angles.x;
            break;
        case ANGLES_Y:
            points[i].y = state.angles.y;
            break;
        case ANGLES_Z:
            points[i].y = state.angles.z;
            break;
        case ORIGIN_X:
            points[i].y = state.origin.x;
            break;
        case ORIGIN_Y:
            points[i].y = state.origin.y;
            break;
        case ORIGIN_Z:
            points[i].y = state.origin.z;
            break;
        case FOV:
            points[i].y = state.fov;
            break;
        }
        if (workWithAngles) {
            float angDif = points[i].y - oldY;
            angDif += (angDif > 180) ? -360 : (angDif < -180) ? 360 : 0;
            points[i].y = oldY += angDif;
            oldY = points[i].y;
        }
    }

    if (frameTime <= points[PREV].x)
        return points[PREV].y;
    if (frameTime >= points[NEXT].x)
        return points[NEXT].y;

    float t = (frameTime - points[PREV].x) / (points[NEXT].x - points[PREV].x);
    //linear interp.

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

//Overrides view.
void Camera::OverrideView(void* m_View)
{
    if (timeOffsetRefresh) {
        timeOffset = engine->GetClientTime() - engine->demoplayer->GetTick() / 60.0f;
        timeOffsetRefresh = false;
    }

    auto newControlType = static_cast<CameraControlType>(sar_cam_control.GetInt());

    //don't allow cinematic mode outside of demo player
    if (!engine->demoplayer->IsPlaying() && newControlType == Path) {
        if (controlType != Path) {
            CAMERA_REQUIRE_DEMO_PLAYER_ERROR();
        } else {
            controlType = Disabled;
        }
        newControlType = controlType;
        sar_cam_control.SetValue(controlType);
    }

    //don't allow drive mode when not using sv_cheats
    if (newControlType == Manual && !sv_cheats.GetBool() && !engine->demoplayer->IsPlaying()) {
        if (controlType != Manual) {
            console->Print("Drive mode requires sv_cheats 1 or demo player.\n");
        } else {
            controlType = Disabled;
        }
        newControlType = controlType;
        sar_cam_control.SetValue(controlType);
    }

    //janky hack mate
    //overriding cvar values, boolean (int) values only.
    //this way the cvar themselves are unchanged
    if (newControlType != Disabled) {
        cl_skip_player_render_in_main_view.ThisPtr()->m_nValue = 0;
        r_drawviewmodel.ThisPtr()->m_nValue = 0;
        ss_force_primary_fullscreen.ThisPtr()->m_nValue = 1;
        crosshair.ThisPtr()->m_nValue = 0;
    }

    //handling camera control type switching
    if (newControlType != controlType) {
        if (controlType == Disabled && newControlType != Disabled) {
            //enabling
            //sorry nothing
        } else if (controlType != Disabled && newControlType == Disabled) {
            //disabling
            //resetting cvars to their actual values when swithing control off
            cl_skip_player_render_in_main_view.SetValue(cl_skip_player_render_in_main_view.GetString());
            r_drawviewmodel.SetValue(r_drawviewmodel.GetString());
            ss_force_primary_fullscreen.SetValue(ss_force_primary_fullscreen.GetString());
            in_forceuser.SetValue(in_forceuser.GetString());
            crosshair.SetValue(crosshair.GetString());
            this->manualActive = false;
        }
        controlType = newControlType;
    }

    //don't do anything if not in game or demo player
    if (engine->hoststate->m_activeGame || engine->demoplayer->IsPlaying()) {
        //TODO: make CViewSetup struct instead of getting stuff with offsets
        Vector* origin = reinterpret_cast<Vector*>(static_cast<int*>(m_View) + 28);
        QAngle* angles = reinterpret_cast<QAngle*>(static_cast<int*>(m_View) + 31);
        float* fov = reinterpret_cast<float*>(static_cast<int*>(m_View) + 20);
        if (controlType == Disabled) {
            currentState.origin = (*origin);
            currentState.angles = (*angles);
            currentState.fov = (*fov);
        } else {

            //manual camera view control
            if (controlType == Manual) {
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
                    if(engine->hoststate->m_activeGame)in_forceuser.ThisPtr()->m_nValue = 2;

                    //getting mouse movement and resetting the cursor position
                    int mX, mY;
                    inputSystem->GetCursorPos(mX, mY);
                    mX -= mouseHoldPos[0];
                    mY -= mouseHoldPos[1];
                    inputSystem->SetCursorPos(mouseHoldPos[0], mouseHoldPos[1]);

                    currentState.angles.x += (float)mY * 0.22f;
                    currentState.angles.y -= (float)mX * 0.22f;

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
            if (controlType == Path) {
                //don't do interpolation when there are no points
                if (states.size() > 0) {
                    float currentTime = engine->GetClientTime() - timeOffset;
                    currentState.origin.x = InterpolateStateParam(ORIGIN_X, currentTime);
                    currentState.origin.y = InterpolateStateParam(ORIGIN_Y, currentTime);
                    currentState.origin.z = InterpolateStateParam(ORIGIN_Z, currentTime);
                    currentState.angles.x = InterpolateStateParam(ANGLES_X, currentTime);
                    currentState.angles.y = InterpolateStateParam(ANGLES_Y, currentTime);
                    currentState.angles.z = InterpolateStateParam(ANGLES_Z, currentTime);
                    //currentState.fov = InterpolateStateParam(FOV, currentFrameTime);
                }
            }
            //applying custom view
            *origin = currentState.origin;
            *angles = currentState.angles;
            *fov = currentState.fov; //fov is currently not working, not sure why
            //console->Print("%f\n", engine->GetClientTime() - timeOffset);
        }
    }
}

void Camera::RequestTimeOffsetRefresh()
{
    timeOffsetRefresh = true;
}

void Camera::OverrideMovement(CUserCmd* cmd)
{
    //blocking keyboard inputs on manual mode
    if (IsDriving()) {
        cmd->buttons = 0;
        cmd->forwardmove = 0;
        cmd->sidemove = 0;
        cmd->upmove = 0;
    }
}

//COMMANDS

CON_COMMAND(sar_cam_path_setkf, "sar_cam_path_setkf [frame] [x] [y] [z] [yaw] [pitch] [roll] [fov]: Sets the camera path keyframe.\n")
{
    CAMERA_REQUIRE_DEMO_PLAYER();

    if (args.ArgC() >= 1 && args.ArgC() <= 9) {
        CameraState campos = camera->currentState;
        int curFrame = engine->demoplayer->GetTick();
        if (args.ArgC() > 1) {
            curFrame = std::atoi(args[1]);
        }
        if (args.ArgC() > 2) {
            float nums[7] = {
                campos.origin.x, campos.origin.x, campos.origin.x,
                campos.angles.x, campos.angles.x, campos.angles.x,
                campos.fov
            };
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

CON_COMMAND(sar_cam_path_showkf, "sar_cam_path_showkf [frame] : Display information about camera path keyframe at specified frame.\n")
{
    CAMERA_REQUIRE_DEMO_PLAYER();

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

CON_COMMAND(sar_cam_path_showkfs, "sar_cam_path_showkfs : Display information about all camera path keyframes.\n")
{
    CAMERA_REQUIRE_DEMO_PLAYER();

    if (args.ArgC() == 1) {
        for (auto const& state : camera->states) {
            console->Print("%d: ", state.first);
            console->Print(std::string(state.second).c_str());
            console->Print("\n");
        }
    } else {
        return console->Print(sar_cam_path_showkfs.ThisPtr()->m_pszHelpString);
    }
}

CON_COMMAND(sar_cam_path_getkfs, "sar_cam_path_getkfs : Exports commands for recreating currently made camera path.\n")
{
    CAMERA_REQUIRE_DEMO_PLAYER();

    if (args.ArgC() == 1) {
        for (auto const& state : camera->states) {
            CameraState cam = state.second;
            console->Print("sar_democam_setkf %d %f %f %f %f %f %f;\n", state.first, cam.origin.x, cam.origin.y, cam.origin.z, cam.angles.x, cam.angles.y, cam.angles.z);
        }
    } else {
        return console->Print(sar_cam_path_getkfs.ThisPtr()->m_pszHelpString);
    }
}

CON_COMMAND(sar_cam_path_remkf, "sar_cam_path_remkf [frame] : Removes camera path keyframe at specified frame.\n")
{
    CAMERA_REQUIRE_DEMO_PLAYER();

    if (args.ArgC() == 1) {
        int i = std::atoi(args[1]);
        camera->states.erase(i);
        console->Print("Camera path keyframe at frame %f removed.\n", args[1]);
    } else {
        return console->Print(sar_cam_path_remkf.ThisPtr()->m_pszHelpString);
    }
}

CON_COMMAND(sar_cam_path_remkfs, "sar_cam_path_remkfs : Removes all camera path keyframes.\n")
{
    CAMERA_REQUIRE_DEMO_PLAYER();

    if (args.ArgC() == 1) {
        camera->states.clear();
        console->Print("All camera path keyframes have been removed.\n");
    } else {
        return console->Print(sar_cam_path_remkfs.ThisPtr()->m_pszHelpString);
    }
}

CON_COMMAND(sar_cam_setang, "sar_cam_setang <pitch> <yaw> [roll] : Sets camera angle.\n")
{
    if (camera->controlType != Manual) {
        console->Print("Camera not in drive mode! Switching.\n");
        sar_cam_control.SetValue(CameraControlType::Manual);
    }

    if (args.ArgC() == 3 || args.ArgC() == 4) {
        float angles[3] = { 0, 0, 0 };
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

CON_COMMAND(sar_cam_setpos, "sar_cam_setpos <x> <y> <z> : Sets camera position.\n")
{
    if (camera->controlType != Manual) {
        console->Print("Camera not in drive mode! Switching.\n");
        sar_cam_control.SetValue(CameraControlType::Manual);
    }

    if (args.ArgC() == 4) {
        float pos[3] = { 0, 0, 0 };
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