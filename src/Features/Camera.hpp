#pragma once
#include "Entity.hpp"
#include "Feature.hpp"
#include "Features/EntityList.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Client.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <map>
#include <sstream>
#include <type_traits>

extern Variable sar_cam_control;
extern Variable sar_cam_drive;
extern Variable sar_cam_ortho;
extern Variable sar_cam_ortho_scale;
extern Variable sar_cam_ortho_nearz;
extern Variable sar_cam_path_interp;

extern Variable cl_skip_player_render_in_main_view;
extern Variable ss_force_primary_fullscreen;


struct CameraState {
	Vector origin = Vector();
	QAngle angles = QAngle();
	float fov = 90;
	operator std::string() const {
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
	Default,
	Drive,
	Cinematic,
	Follow
};

class Camera : public Feature {
private:
	bool manualActive = false;
	bool cameraRefreshRequested = false;
	bool timeOffsetRefreshRequested = true;
	int mouseHoldPos[2] = {0, 0};
	float timeOffset = 0.0;

public:
	CameraControlType controlType = Default;
	CameraState currentState;
	CameraState rawState;
	std::map<int, CameraState> states;
	Camera();
	~Camera();
	bool IsDriving();
	void OverrideView(CViewSetup *m_View);
	CameraState InterpolateStates(float time);
	void DrawInWorld() const;
	void RequestTimeOffsetRefresh();
	void RequestCameraRefresh();
	void OverrideMovement(CUserCmd *cmd);

	Vector GetPosition(int slot, bool raw = true);
	Vector GetForwardVector(int slot, bool raw = true);

	template <bool serverside>
	void TransformThroughPortal(int slot, Vector &eyePos, QAngle &eyeAng) {
		using Ent = std::conditional_t<serverside, ServerEnt, ClientEnt>;

		Ent *player = serverside ? (Ent *)server->GetPlayer(slot + 1) : (Ent *)client->GetPlayer(slot + 1);
		if (!player) return;

		CBaseHandle portal_handle = player->template field<CBaseHandle>("m_hPortalEnvironment");
		Ent *portal_ent = serverside ? (Ent *)entityList->LookupEntity(portal_handle) : (Ent *)client->GetPlayer(portal_handle.GetEntryIndex());

		if (portal_ent) {
			Vector portal_pos = portal_ent->abs_origin();
			QAngle portal_ang = portal_ent->abs_angles();
			Vector portal_norm;
			Math::AngleVectors(portal_ang, &portal_norm);

			Vector eye_to_portal_center = portal_pos - eyePos;

			if (portal_norm.Dot(eye_to_portal_center) > 0.0f) {
				// eyes behind portal - translate position
				VMatrix portal_matrix = portal_ent->template field<VMatrix>("m_matrixThisToLinked");
				eyePos = portal_matrix.PointTransform(eyePos);
				// translate angles
				Vector forward, up;
				Math::AngleVectors(eyeAng, &forward, nullptr, &up);
				forward = portal_matrix.VectorTransform(forward);
				up = portal_matrix.VectorTransform(up);
				Math::VectorAngles(forward, up, &eyeAng);
			}
		}
	}

	template <bool serverside>
	bool GetEyePosFromOrigin(int slot, Vector origin, Vector &eyePos, QAngle &eyeAng) {
		using Ent = std::conditional_t<serverside, ServerEnt, ClientEnt>;

		auto player = serverside ?
			(Ent *)server->GetPlayer(slot + 1) :
			(Ent *)client->GetPlayer(slot + 1);

		if (!player) return false;

		eyePos = serverside ?
			origin + server->GetViewOffset(player) + server->GetPortalLocal(player).m_vEyeOffset :
			origin + client->GetViewOffset(player) + client->GetPortalLocal(player).m_vEyeOffset;

		eyeAng = serverside ? server->GetPlayerState(player).v_angle : client->GetPlayerState(player).v_angle;
		eyeAng.y = Math::AngleNormalize(eyeAng.y);

		TransformThroughPortal<serverside>(slot, eyePos, eyeAng);

		return true;
	}

	template <bool serverside>
	bool GetEyePos(int slot, Vector &eyePos, QAngle &eyeAng) {
		using Ent = std::conditional_t<serverside, ServerEnt, ClientEnt>;

		auto player = serverside ?
			(Ent *)server->GetPlayer(slot + 1) :
			(Ent *)client->GetPlayer(slot + 1);

		if (!player) return false;

		return GetEyePosFromOrigin<serverside>(slot, serverside ? server->GetAbsOrigin(player) : client->GetAbsOrigin(player), eyePos, eyeAng);
	}
};

extern Camera *camera;

extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(sar_cam_path_setkf);
