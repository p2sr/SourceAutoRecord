#pragma once
#include "SDK.hpp"

#include <random>

#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))
#define M_PI_2 1.57079632679489661923  // pi/2
#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

namespace Math {

	void SinCos(float radians, float *sine, float *cosine);
	float AngleNormalize(float angle);
	float VectorNormalize(Vector &vec);
	void VectorAdd(const Vector &a, const Vector &b, Vector &c);
	void AngleVectors(const QAngle &angles, Vector *forward);
	void AngleVectors(const QAngle &angles, Vector *forward, Vector *right, Vector *up);
	void VectorScale(Vector const &src, float b, Vector &dst);
	void VectorCopy(const Vector &src, Vector &dst);
	float RandomNumber(const float &min, const float &max);
	int RandomNumber(const int &min, const int &max);
	float Distance(const Vector &a, const Vector &b);
	float Distance(const QAngle &a, const QAngle &b);
	void Lerp(const Vector &oldPos, const Vector &newPos, float time, Vector &outPut);
}  // namespace Math

inline void Math::SinCos(float radians, float *sine, float *cosine) {
#ifdef _WIN32
	_asm {
        fld DWORD PTR[radians]
        fsincos

        mov edx, DWORD PTR[cosine]
        mov eax, DWORD PTR[sine]

        fstp DWORD PTR[edx]
        fstp DWORD PTR[eax]
	}
#else
	double __cosr, __sinr;
	__asm("fsincos"
	      : "=t"(__cosr), "=u"(__sinr)
	      : "0"(radians));

	*sine = __sinr;
	*cosine = __cosr;
#endif
}
inline void Math::VectorAdd(const Vector &a, const Vector &b, Vector &c) {
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}
inline void Math::VectorScale(Vector const &src, float b, Vector &dst) {
	dst.x = src.x * b;
	dst.y = src.y * b;
	dst.z = src.z * b;
}
inline void Math::VectorCopy(const Vector &src, Vector &dst) {
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
}
inline float Math::Distance(const Vector &a, const Vector &b) {
	return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) + std::pow(b.z - a.z, 2));
}
inline float Math::Distance(const QAngle &a, const QAngle &b) {
	return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) + std::pow(b.z - a.z, 2));
}

inline void Math::Lerp(const Vector &oldPos, const Vector &newPos, float time, Vector &outPut) {
	if (time > 1)
		time = 1;
	if (time < 0)
		time = 0;

	outPut.x = (1 - time) * oldPos.x + time * newPos.x;
	outPut.y = (1 - time) * oldPos.y + time * newPos.y;
	outPut.z = (1 - time) * oldPos.z + time * newPos.z;
}


class Matrix {
public:
	Matrix(int rows, int cols, const double init);

	double &operator()(const unsigned int x, const unsigned int y) { return mat[x][y]; }
	const double &operator()(const unsigned int x, const unsigned int y) const { return mat[x][y]; }

	Matrix &operator=(const Matrix &rhs);
	Matrix operator+(const Matrix &rhs);
	Matrix &operator+=(const Matrix &rhs);
	Matrix operator*(const Matrix &rhs);
	Matrix &operator*=(const Matrix &rhs);
	Vector operator*(const Vector &rhs);
	Vector operator*=(const Vector &rhs);

	Matrix transpose();

	void Print();

public:
	unsigned int rows;
	unsigned int cols;

private:
	std::vector<std::vector<double>> mat;
};
