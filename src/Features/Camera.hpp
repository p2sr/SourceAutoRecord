#pragma once
#include <map>
#include <sstream>

#include "Feature.hpp"

#include "Utils/SDK.hpp"
#include "Variable.hpp"
#include "Modules/Engine.hpp"


#define CAMERA_REQUIRE_DEMO_PLAYER_ERROR() console->Print("Cinematic mode cannot be used outside of demo player.\n")
#define CAMERA_REQUIRE_DEMO_PLAYER() if (!engine->demoplayer->IsPlaying()) return CAMERA_REQUIRE_DEMO_PLAYER_ERROR()

extern Variable sar_cam_control;
extern Variable sar_cam_drive;

extern Variable cl_skip_player_render_in_main_view;
extern Variable r_drawviewmodel;
extern Variable ss_force_primary_fullscreen;
extern Variable crosshair;

struct CameraState {
    Vector origin = Vector();
    QAngle angles = QAngle();
    float fov = 90;
    operator std::string() const{
        std::ostringstream s;
        s << "pos: " << origin.x << " " << origin.y << " " << origin.z << "; ";
        s << "rot: " << angles.x << " " << angles.y << " " << angles.z << "; ";
        s << "fov: " << fov;
        return s.str();
    }
};

enum CameraStateParameter {
    ORIGIN_X,
    ORIGIN_Y,
    ORIGIN_Z,
    ANGLES_X,
    ANGLES_Y,
    ANGLES_Z,
    FOV
};

enum CameraControlType {
    Disabled,
    Manual,
    Path,
};

class Camera : public Feature {
private:
    bool manualActive = false;
    bool timeOffsetRefresh = true;
    int mouseHoldPos[2] = {0,0};
    float timeOffset = 0.0;
public:
    CameraControlType controlType = Disabled;
    CameraState currentState;
    std::map<int, CameraState> states;
    Camera();
    ~Camera();
    bool IsDriving();
    void OverrideView(void* m_View);
    float InterpolateStateParam(CameraStateParameter param, float time);
    void RequestTimeOffsetRefresh();
    void OverrideMovement(CUserCmd* cmd);
};

extern Camera* camera;