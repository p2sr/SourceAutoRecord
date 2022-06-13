#pragma once

#include "Math.hpp"

#define GAMEMOVEMENT_JUMP_HEIGHT 21.0f

struct CMoveData {
	bool m_bFirstRunOfFunctions : 1;  // 0
	bool m_bGameCodeMovedPlayer : 1;  // 2
	bool m_bNoAirControl : 1;         // 0
	void *m_nPlayerHandle;            // 4
	int m_nImpulseCommand;            // 8
	QAngle m_vecViewAngles;           // 12, 16, 20
	QAngle m_vecAbsViewAngles;        // 24, 28, 32
	int m_nButtons;                   // 36
	int m_nOldButtons;                // 40
	float m_flForwardMove;            // 44
	float m_flSideMove;               // 48
	float m_flUpMove;                 // 52
	float m_flMaxSpeed;               // 56
	float m_flClientMaxSpeed;         // 60
	Vector m_vecVelocity;             // 64, 68, 72
	QAngle m_vecAngles;               // 76, 80, 84
	QAngle m_vecOldAngles;            // 88, 92, 96
	float m_outStepHeight;            // 100
	Vector m_outWishVel;              // 104, 108, 112
	Vector m_outJumpVel;              // 116, 120, 124
	Vector m_vecConstraintCenter;     // 128, 132, 136
	float m_flConstraintRadius;       // 140
	float m_flConstraintWidth;        // 144
	float m_flConstraintSpeedFactor;  // 148
	bool m_bConstraintPastRadius;     // 154
	Vector m_vecAbsOrigin;            // 156
};

class CHLMoveData : public CMoveData {
public:
	bool m_bIsSprinting;
};

#define IN_ATTACK (1 << 0)
#define IN_JUMP (1 << 1)
#define IN_DUCK (1 << 2)
#define IN_FORWARD (1 << 3)
#define IN_BACK (1 << 4)
#define IN_USE (1 << 5)
#define IN_MOVELEFT (1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2 (1 << 11)
#define IN_RELOAD (1 << 13)
#define IN_SPEED (1 << 17)
#define IN_ZOOM (1 << 19)

#define FL_ONGROUND (1 << 0)
#define FL_DUCKING (1 << 1)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)
#define FL_GODMODE (1 << 14)
#define FL_NOTARGET (1 << 15)

#define WL_Feet 1
#define WL_Waist 2

#define MOVETYPE_NOCLIP 8
#define MOVETYPE_LADDER 9
