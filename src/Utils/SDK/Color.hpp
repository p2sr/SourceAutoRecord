#pragma once

#include <cstdint>
#include <cstring>

struct Color {
	Color()
		: Color(0, 0, 0) {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	uint8_t r, g, b, a;
	inline bool operator==(const Color col) const {
		return !memcmp(this, &col, sizeof col);
	}
	inline bool operator!=(const Color col) const {
		return memcmp(this, &col, sizeof col);
	}
};

struct color32 {
	bool operator!=( const struct color32 &other ) const;
	int r, g, b, a; 

	inline unsigned *asInt(void) { return reinterpret_cast<unsigned*>(this); }
	inline const unsigned *asInt(void) const { return reinterpret_cast<const unsigned*>(this); } 
	inline void Copy(const color32 &rhs) {
		*asInt() = *rhs.asInt();
	}

};

class SourceColor {
public:
	SourceColor() {
		*((int *)this) = 0;
	}
	SourceColor(int _r, int _g, int _b) {
		SetColor(_r, _g, _b, 0);
	}
	SourceColor(int _r, int _g, int _b, int _a) {
		SetColor(_r, _g, _b, _a);
	}

	void SetColor(int _r, int _g, int _b, int _a = 0) {
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	void GetColor(int &_r, int &_g, int &_b, int &_a) const {
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void SetRawColor(int color32) {
		*((int *)this) = color32;
	}

	int GetRawColor() const {
		return *((int *)this);
	}

	inline int r() const { return _color[0]; }
	inline int g() const { return _color[1]; }
	inline int b() const { return _color[2]; }
	inline int a() const { return _color[3]; }

	unsigned char &operator[](int index) {
		return _color[index];
	}

	const unsigned char &operator[](int index) const {
		return _color[index];
	}

	bool operator==(const SourceColor &rhs) const {
		return (*((int *)this) == *((int *)&rhs));
	}

	bool operator!=(const SourceColor &rhs) const {
		return !(operator==(rhs));
	}

	SourceColor &operator=(const SourceColor &rhs) {
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

	SourceColor &operator=(const color32 &rhs) {
		_color[0] = rhs.r;
		_color[1] = rhs.g;
		_color[2] = rhs.b;
		_color[3] = rhs.a;
		return *this;
	}

	color32 ToColor32() const {
		color32 newColor;
		newColor.r = _color[0];
		newColor.g = _color[1];
		newColor.b = _color[2];
		newColor.a = _color[3];
		return newColor;
	}

private:
	unsigned char _color[4];
};

enum class TextColor {
	PLAYERNAME = 3,
	GREEN = 4,
	LIGHT_GREEN = 5,
	ORANGE = 6,
};
