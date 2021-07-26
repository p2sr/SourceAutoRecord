#include "Math.hpp"

#include "Modules/Console.hpp"
#include "SDK.hpp"

#include <cmath>
#include <float.h>
#include <random>

float Math::AngleNormalize(float angle) {
	angle = fmodf(angle, 360.0f);
	if (angle > 180) {
		angle -= 360;
	}
	if (angle < -180) {
		angle += 360;
	}
	return angle;
}
float Math::VectorNormalize(Vector &vec) {
	auto radius = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	auto iradius = 1.f / (radius + FLT_EPSILON);

	vec.x *= iradius;
	vec.y *= iradius;
	vec.z *= iradius;

	return radius;
}
void Math::AngleVectors(const QAngle &angles, Vector *forward) {
	float sp, sy, cp, cy;

	Math::SinCos(DEG2RAD(angles.y), &sy, &cy);
	Math::SinCos(DEG2RAD(angles.x), &sp, &cp);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}
void Math::AngleVectors(const QAngle &angles, Vector *forward, Vector *right, Vector *up) {
	float sr, sp, sy, cr, cp, cy;

	Math::SinCos(DEG2RAD(angles.x), &sy, &cy);
	Math::SinCos(DEG2RAD(angles.y), &sp, &cp);
	Math::SinCos(DEG2RAD(angles.z), &sr, &cr);

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1 * sr * sp * cy + -1 * cr * -sy;
		right->y = -1 * sr * sp * sy + -1 * cr * cy;
		right->z = -1 * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}
float Math::RandomNumber(const float &min, const float &max) {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist_pitch(min, std::nextafter(max, FLT_MAX));

	return dist_pitch(mt);
}
int Math::RandomNumber(const int &min, const int &max) {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist_pitch(min, max);

	return dist_pitch(mt);
}


Matrix::Matrix(int rows, int cols, const double init)
	: rows(rows)
	, cols(cols) {
	this->mat.resize(rows);
	for (auto &it : this->mat) {
		it.resize(cols, init);
	}
}

Matrix &Matrix::operator=(const Matrix &rhs) {
	if (&rhs == this)
		return *this;

	auto new_rows = rhs.rows;
	auto new_cols = rhs.cols;

	this->mat.resize(new_rows);
	for (auto &it : this->mat) {
		it.resize(new_cols);
	}

	for (unsigned int i = 0; i < new_rows; ++i)
		for (unsigned int j = 0; j < new_cols; ++j)
			mat[i][j] = rhs(i, j);

	this->rows = new_rows;
	this->cols = new_cols;

	return *this;
}

Matrix Matrix::operator+(const Matrix &rhs) {
	Matrix result(this->rows, this->cols, 0);

	for (unsigned int i = 0; i < this->rows; ++i)
		for (unsigned int j = 0; j < this->cols; ++j)
			result(i, j) = this->mat[i][j] + rhs(i, j);

	return result;
}

Matrix &Matrix::operator+=(const Matrix &rhs) {
	auto new_rows = rhs.rows;
	auto new_cols = rhs.cols;

	for (unsigned int i = 0; i < new_rows; ++i)
		for (unsigned int j = 0; j < new_cols; ++j)
			this->mat[i][j] += rhs(i, j);

	return *this;
}

Matrix Matrix::operator*(const Matrix &rhs) {
	if (this->cols != rhs.rows) {
		console->Print("Error: rows != cols\n");
		return Matrix(1, 1, 0);
	}

	Matrix result(this->rows, rhs.cols, 0);

	int sum;
	for (unsigned int i = 0; i < this->rows; ++i) {
		for (unsigned int j = 0; j < rhs.cols; ++j) {
			sum = 0;
			for (unsigned int k = 0; k < this->rows; ++k) {
				sum += this->mat[i][k] * rhs(k, j);
			}
			result(i, j) += sum;
		}
	}

	return result;
}

Matrix &Matrix::operator*=(const Matrix &rhs) {
	Matrix result = (*this) * rhs;
	(*this) = result;
	return *this;
}

Vector Matrix::operator*(const Vector &rhs) {
	if (this->cols != 3) {
		console->Print("Error: rows != cols\n");
		return Vector(0, 0, 0);
	}

	Vector result(0, 0, 0);

	for (unsigned int i = 0; i < this->rows; ++i) {
		for (unsigned int j = 0; j < 3; ++j) {
			result[i] += this->mat[i][j] * rhs[j];
		}
	}

	return result;
}

Vector Matrix::operator*=(const Vector &rhs) {
	return (*this) * rhs;
}

void Matrix::Print() {
	for (unsigned int i = 0; i < this->rows; ++i) {
		for (unsigned int j = 0; j < this->cols; ++j) {
			console->Print("%d ", this->mat[i][j]);
		}
		console->Print("\n");
	}
}
