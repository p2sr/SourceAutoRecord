#pragma once

#include "Math.hpp"

struct CUserCmd {
	void *VMT;              // 0
	int command_number;     // 4
	int tick_count;         // 8
	QAngle viewangles;      // 12, 16, 20
	float forwardmove;      // 24
	float sidemove;         // 28
	float upmove;           // 32
	int buttons;            // 36
	unsigned char impulse;  // 40
	int weaponselect;       // 44
	int weaponsubtype;      // 48
	int random_seed;        // 52
	short mousedx;          // 56
	short mousedy;          // 58
	bool hasbeenpredicted;  // 60
};


typedef unsigned long CRC32_t;

class CVerifiedUserCmd {
public:
	CUserCmd m_cmd;
	CRC32_t m_crc;
};

enum JoystickAxis_t {
	JOY_AXIS_X = 0,
	JOY_AXIS_Y,
	JOY_AXIS_Z,
	JOY_AXIS_R,
	JOY_AXIS_U,
	JOY_AXIS_V,
	MAX_JOYSTICK_AXES,
};

typedef struct {
	unsigned int AxisFlags;   // 0
	unsigned int AxisMap;     // 4
	unsigned int ControlMap;  // 8
} joy_axis_t;


struct CameraThirdData_t {
	float m_flPitch;      // 0
	float m_flYaw;        // 4
	float m_flDist;       // 8
	float m_flLag;        // 12
	Vector m_vecHullMin;  // 16, 20, 24
	Vector m_vecHullMax;  // 28, 32, 36
};

struct kbutton_t {
	struct Split_t {
		int down[2];
		int state;
	} m_PerUser[2];
};

struct PerUserInput_t {
	float m_flAccumulatedMouseXMovement;     // ?
	float m_flAccumulatedMouseYMovement;     // ?
	float m_flPreviousMouseXPosition;        // ?
	float m_flPreviousMouseYPosition;        // ?
	float m_flRemainingJoystickSampleTime;   // ?
	float m_flKeyboardSampleTime;            // 12
	float m_flSpinFrameTime;                 // ?
	float m_flSpinRate;                      // ?
	float m_flLastYawAngle;                  // ?
	joy_axis_t m_rgAxes[MAX_JOYSTICK_AXES];  // ???
	bool m_fCameraInterceptingMouse;         // ?
	bool m_fCameraInThirdPerson;             // ?
	bool m_fCameraMovingWithMouse;           // ?
	Vector m_vecCameraOffset;                // 104, 108, 112
	bool m_fCameraDistanceMove;              // 116
	int m_nCameraOldX;                       // 120
	int m_nCameraOldY;                       // 124
	int m_nCameraX;                          // 128
	int m_nCameraY;                          // 132
	bool m_CameraIsOrthographic;             // 136
	QAngle m_angPreviousViewAngles;          // 140, 144, 148
	QAngle m_angPreviousViewAnglesTilt;      // 152, 156, 160
	float m_flLastForwardMove;               // 164
	int m_nClearInputState;                  // 168
	CUserCmd *m_pCommands;                   // 172
	CVerifiedUserCmd *m_pVerifiedCommands;   // 176
	unsigned long m_hSelectedWeapon;         // 180 CHandle<C_BaseCombatWeapon>
	CameraThirdData_t *m_pCameraThirdData;   // 184
	int m_nCamCommand;                       // 188
};

enum TOGGLE_STATE {
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN
};

typedef enum {
	USE_OFF = 0,
	USE_ON = 1,
	USE_SET = 2,
	USE_TOGGLE = 3
} USE_TYPE;



struct democmdinfo_t {
	struct Split_t {
		int flags;

		Vector viewOrigin;
		QAngle viewAngles;
		QAngle localViewAngles;

		Vector viewOrigin2;
		QAngle viewAngles2;
		QAngle localViewAngles2;
	};

	Split_t u[2];
};

struct DemoCommandQueue {
	int tick;
	democmdinfo_t info;
	int filepos;
};
