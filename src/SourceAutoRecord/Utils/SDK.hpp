#pragma once
#include <cmath>
#include <float.h>

struct Vector {
	float x, y, z;
	float Length() {
		return sqrt(x * x + y * y + z * z);
	}
	float Length2D() {
		return sqrt(x * x + y * y);
	}
	Vector operator*(float fl) const {
		Vector res;
		res.x = x * fl;
		res.y = y * fl;
		res.z = z * fl;
		return res;
	}
};

struct QAngle {
	float x, y, z;
};

struct Color {
	Color() {
		*((int *)this) = 255;
	}
	Color(int _r, int _g, int _b) {
		SetColor(_r, _g, _b, 255);
	}
	Color(int _r, int _g, int _b, int _a) {
		SetColor(_r, _g, _b, _a);
	}
	void SetColor(int _r, int _g, int _b, int _a = 255) {
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}
	inline int r() const { return _color[0]; }
	inline int g() const { return _color[1]; }
	inline int b() const { return _color[2]; }
	inline int a() const { return _color[3]; }
	unsigned char _color[4];
};

enum SignonState {
	None = 0,
	Challenge = 1,
	Connected = 2,
	New = 3,
	Prespawn = 4,
	Spawn = 5,
	Full = 6,
	Changelevel = 7
};

#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

void inline SinCos(float radians, float *sine, float *cosine)
{
	register double __cosr, __sinr;
	__asm ("fsincos" : "=t" (__cosr), "=u" (__sinr) : "0" (radians));

	*sine = __sinr;
	*cosine = __cosr;
}

void AngleVectors(const QAngle &angles, Vector* forward)
{
	float sp, sy, cp, cy;

	SinCos(DEG2RAD(angles.y), &sy, &cy);
	SinCos(DEG2RAD(angles.x), &sp, &cp);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

float VectorNormalize(Vector &vec)
{
	float radius = sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
	float iradius = 1.f / ( radius + FLT_EPSILON );

	vec.x *= iradius;
	vec.y *= iradius;
	vec.z *= iradius;

	return radius;
}

void VectorAdd(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}

class CMoveData
{
	public:
	bool m_bFirstRunOfFunctions : 1;
	bool m_bGameCodeMovedPlayer : 1;
	void* m_nPlayerHandle;
	int m_nImpulseCommand;
	QAngle m_vecViewAngles;
	QAngle m_vecAbsViewAngles;
	int m_nButtons;
	int m_nOldButtons;
	float m_flForwardMove;
	float m_flSideMove;
	float m_flUpMove;
	float m_flMaxSpeed;
	float m_flClientMaxSpeed;
	Vector m_vecVelocity;
	QAngle m_vecAngles;
	QAngle m_vecOldAngles;
	float m_outStepHeight;
	Vector m_outWishVel; 
	Vector m_outJumpVel;
	Vector m_vecConstraintCenter;
	float m_flConstraintRadius;
	float m_flConstraintWidth;
	float m_flConstraintSpeedFactor;
	void SetAbsOrigin(const Vector &vec);
	const Vector &GetAbsOrigin() const;
private:
	Vector m_vecAbsOrigin;
};

class CHLMoveData : public CMoveData
{
public:
	bool m_bIsSprinting;
};