#pragma once

#include "Math.hpp"

struct CachedPaintPowerChoiceResult {
	Vector surfaceNormal;
	CBaseHandle surfaceEntity;
	bool wasValid;
	bool wasIgnored;
};

enum PaintPowerType {
	BOUNCE_POWER,
	REFLECT_POWER,
	SPEED_POWER,
	PORTAL_POWER,
	NO_POWER,
	PAINT_POWER_TYPE_COUNT = NO_POWER,
	PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER,
	INVALID_PAINT_POWER
};

enum StickCameraState {
	STICK_CAMERA_SURFACE_TRANSITION = 0,
	STICK_CAMERA_ROLL_CORRECT,
	STICK_CAMERA_PORTAL,
	STICK_CAMERA_WALL_STICK_DEACTIVATE_TRANSITION,
	STICK_CAMERA_SWITCH_TO_ABS_UP_MODE,
	STICK_CAMERA_ABS_UP_MODE,
	STICK_CAMERA_SWITCH_TO_LOCAL_UP,
	STICK_CAMERA_SWITCH_TO_LOCAL_UP_LOOKING_UP,
	STICK_CAMERA_LOCAL_UP_LOOKING_UP,
	STICK_CAMERA_UPRIGHT
};

enum InAirState {
	ON_GROUND,
	IN_AIR_JUMPED,
	IN_AIR_BOUNCED,
	IN_AIR_FELL
};

class CountdownTimer {
public:
	void *vtable;  // different on client vs server
	float m_duration;
	float m_timestamp;
};

class CPortalPlayerLocalData {
public:
	void *vtable;  // contains different shit on client vs server

	bool m_bShowingViewFinder;  // f-stop????
	float m_flAirControlSupressionTime;
	int m_nLocatorEntityIndices[16];
	bool m_bPlacingPhoto;  // f-stop.

	CachedPaintPowerChoiceResult m_cachedPaintPowerChoiceResults[PAINT_POWER_TYPE_COUNT];
	Vector m_stickNormal;  // adhesion gel moment
	Vector m_oldStickNormal;
	Vector m_vPreUpdateVelocity;
	Vector m_up;
	Vector m_vStickRotationAxis;
	Vector m_standHullMin;
	Vector m_standHullMax;
	Vector m_duckHullMin;
	Vector m_duckHullMax;
	Vector m_cachedStandHullMinAttempt;
	Vector m_cachedStandHullMaxAttempt;
	Vector m_cachedDuckHullMinAttempt;
	Vector m_cachedDuckHullMaxAttempt;
	Vector m_vLocalUp;
	Vector m_vEyeOffset;
	QAngle m_qQuaternionPunch;
	PaintPowerType m_paintedPowerType;
	CountdownTimer m_paintedPowerTimer;
	float m_flAirInputScale;
	float m_flCurrentStickTime;
	StickCameraState m_nStickCameraState;
	InAirState m_inAirState;
	bool m_bDoneStickInterp;
	bool m_bDoneCorrectPitch;
	bool m_bAttemptHullResize;
	bool m_bJumpedThisFrame;
	bool m_bBouncedThisFrame;
	bool m_bDuckedInAir;
	uint32_t m_hTractorBeam;
	int m_nTractorBeamCount;
	bool m_bZoomedIn;
	float m_fBouncedTime;
	bool m_bPreventedCrouchJumpThisFrame;
};
