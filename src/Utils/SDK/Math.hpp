#pragma once

#include <cmath>

struct Vector {
	float x, y, z;
	inline Vector()
		: x(0)
		, y(0)
		, z(0) {
	}
	inline Vector(float x, float y, float z = 0)
		: x(x)
		, y(y)
		, z(z) {
	}
	inline float SquaredLength() const {
		return x * x + y * y + z * z;
	}
	inline float Length() const {
		return std::sqrt(x * x + y * y + z * z);
	}
	inline float Length2D() const {
		return std::sqrt(x * x + y * y);
	}
	inline float Dot(const Vector &vOther) const {
		return Vector::DotProduct(*this, vOther);
	}
	inline Vector operator*(float fl) const {
		Vector res;
		res.x = x * fl;
		res.y = y * fl;
		res.z = z * fl;
		return res;
	}
	inline Vector &operator*=(float fl) {
		x = x * fl;
		y = y * fl;
		z = z * fl;
		return *this;
	}
	inline Vector operator/(float fl) const {
		return *this * (1 / fl);
	}
	inline Vector &operator+=(const Vector &vec) {
		x = x + vec.x;
		y = y + vec.y;
		z = z + vec.z;
		return *this;
	}
	inline Vector operator+(const Vector vec) const {
		Vector res;
		res.x = x + vec.x;
		res.y = y + vec.y;
		res.z = z + vec.z;
		return res;
	}
	inline Vector &operator-=(const Vector &vec) {
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}
	inline Vector operator-(const Vector vec) const {
		Vector res;
		res.x = x - vec.x;
		res.y = y - vec.y;
		res.z = z - vec.z;
		return res;
	}
	inline Vector operator-() const {
		return Vector{0, 0, 0} - *this;
	}
	inline float &operator[](int i) {
		return ((float *)this)[i];
	}
	inline float operator[](int i) const {
		return ((float *)this)[i];
	}
	inline bool operator==(const Vector vec) const {
		return x == vec.x && y == vec.y && z == vec.z;
	}
	inline bool operator!=(const Vector vec) const {
		return !(*this == vec);
	}
	static inline float DotProduct(const Vector &a, const Vector &b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	inline Vector Cross(const Vector &v) {
		Vector out;
		out.x = this->y * v.z - this->z * v.y;
		out.y = this->z * v.x - this->x * v.z;
		out.z = this->x * v.y - this->y * v.x;
		return out;
	}
	inline Vector Normalize() {
		return *this / this->Length();
	}
};
template <typename T>
struct Vector2 {
	T x, y;
	inline Vector2()
		: x(0)
		, y(0) {
	}
	inline Vector2(const T &x, const T &y)
		: x(x)
		, y(y) {
	}
	inline float Length() const {
		return sqrt(x * x + y * y);
	}

	inline Vector2 operator+(const Vector2 &other) const {
		Vector2 res;
		res.x = x + other.x;
		res.y = y + other.y;
		return res;
	}
	inline Vector2 operator+=(const Vector2 &other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	inline Vector2 operator-(const Vector2 &other) const {
		Vector2 res;
		res.x = x - other.x;
		res.y = y - other.y;
		return res;
	}
	inline Vector2 operator-=(const Vector2 &other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}
	inline Vector2 operator*(float val) const {
		Vector2 res;
		res.x = x * val;
		res.y = y * val;
		return res;
	}
	inline Vector2 operator*=(float val) {
		x *= val;
		y *= val;
		return *this;
	}
	inline Vector2 operator/(float val) const {
		Vector2 res;
		res.x = x / val;
		res.y = y / val;
		return res;
	}
	inline Vector2 operator/=(float val) {
		x /= val;
		y /= val;
		return *this;
	}
	inline T Dot(const Vector2 &other) const {
		return x * other.x + y * other.y;
	}
	inline T Cross(const Vector2 &other) const {
		return x * other.y - y * other.x;
	}
	inline Vector2 Normalize() {
		return *this / this->Length();
	}
};

template <typename T>
struct Bounds {
	Vector2<T> vBegin, vEnd;
	inline Bounds()
		: vBegin(Vector2<T>((T)0, (T)0))
		, vEnd(Vector2<T>((T)0, (T)0)) {
	}
	inline Bounds(const T &x0, const T &y0, const T &x1, const T &y1)
		: vBegin(Vector2<T>(x0, y0))
		, vEnd(Vector2<T>(x1, y1)) {
	}
	inline Bounds(const Vector2<T> &v0, const Vector2<T> &v1)
		: vBegin(v0)
		, vEnd(v1) {
	}
	inline Bounds Scale(const Vector2<T> &stretchPoint, float amp) {
		vBegin = stretchPoint - (stretchPoint - vBegin) * amp;
		vEnd = stretchPoint + (vEnd - stretchPoint) * amp;
		return *this;
	}
	inline Bounds Scale(float amp) {
		this.Scale(vBegin, amp);
		return *this;
	}
	inline Bounds operator+(const Bounds &other) const {
		Bounds<T> res;
		res.vBegin = vBegin + other.vBegin;
		res.vEnd = vEnd + other.vEnd;
		return res;
	}
	inline Bounds operator+=(const Bounds &other) {
		vBegin += other.vBegin;
		vEnd += other.vEnd;
		return *this;
	}
	inline Bounds operator-(const Bounds &other) const {
		Bounds<T> res;
		res.vBegin = vBegin - other.vBegin;
		res.vEnd = vEnd - other.vEnd;
		return res;
	}
	inline Bounds operator-=(const Bounds &other) {
		vBegin -= other.vBegin;
		vEnd -= other.vEnd;
		return *this;
	}
};


struct QAngle {
	float x, y, z;
};

inline QAngle VectorToQAngle(const Vector &v) {
	return {v.x, v.y, v.z};
}

inline Vector QAngleToVector(const QAngle &a) {
	return {a.x, a.y, a.z};
}

struct VectorAligned : public Vector {
	VectorAligned()
		: Vector()
		, w(0){};
	VectorAligned(float x, float y, float z)
		: Vector(x, y, z)
		, w(0) {
	}
	VectorAligned(Vector v)
		: Vector(v.x, v.y, v.z)
		, w(0) {
	}
	float w;
};

struct matrix3x4_t {
	float m_flMatVal[3][4];

	inline Vector VectorTransform(Vector v) const {
		const float(*m)[4] = m_flMatVal;
		return Vector{
			m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
			m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
			m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3],
		};
	}
};

struct VMatrix {
	float m[4][4];

	inline Vector VectorTransform(Vector vVec) const {
		return Vector(
			m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z,
			m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z,
			m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z);
	}

	inline Vector PointTransform(Vector vVec) const {
		return Vector(
			m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z + m[0][3],
			m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z + m[1][3],
			m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z + m[2][3]);
	}
};
