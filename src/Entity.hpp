#pragma once

#include <string>
#include <typeinfo>
#include <unordered_map>
#include "Utils/SDK.hpp"

namespace EntField {
	enum class Type {
		NONE, // cannot access fields of type none
		BOOL,
		CHAR,
		SHORT,
		INT,
		ANY_INT, // bool, char, short, int
		FLOAT,
		STRING,
		VECTOR,
		COLOR,
		HANDLE,
		EDICT,
		POINTER,
		VMATRIX,
		MATRIX3X4,
		ARRAY,
		OTHER,
	};

#define M1(expect, ty) template <> inline bool matchFieldType<ty>(Type t) { return t == Type::OTHER || t == Type::expect; }
#define M2(exp1, exp2, ty) template <> inline bool matchFieldType<ty>(Type t) { return t == Type::OTHER || t == Type::exp1 || t == Type::exp2; }
	template <typename T> inline bool matchFieldType(Type t) { return t == Type::OTHER; }
	M2(ANY_INT, BOOL, bool)
	M2(ANY_INT, CHAR, char)
	M2(ANY_INT, CHAR, unsigned char)
	M2(ANY_INT, CHAR, signed char)
	M2(ANY_INT, SHORT, unsigned short)
	M2(ANY_INT, SHORT, signed short)
	M2(ANY_INT, INT, signed int)
	M2(ANY_INT, INT, unsigned int)
	M1(FLOAT, float)
	M1(STRING, const char *)
	M1(STRING, char *)
	M2(ARRAY, VECTOR, Vector)
	M2(ARRAY, VECTOR, QAngle)
	M1(COLOR, Color)
	M2(ANY_INT, HANDLE, CBaseHandle)
	M1(EDICT, const edict_t *)
	M1(EDICT, edict_t *)
	M1(POINTER, const void *)
	M1(POINTER, void *)
	M1(VMATRIX, VMatrix)
	M1(MATRIX3X4, matrix3x4_t)
#undef M1
#undef M2

	const std::pair<size_t, Type> &getServerOffset(void *ent, const char *field);
	const std::pair<size_t, Type> &getClientOffset(void *ent, const char *field);

	void warnBadFieldType(void *ent, const char *field, const char *expect_type, Type actual_type, bool server);

	template <typename T>
	void assertVoidEnt(T val) {
		constexpr bool is_ptr = std::is_same<decltype(val), void *>::value;
		constexpr bool is_hptr = std::is_same<decltype(val), IHandleEntity *>::value;
		constexpr bool is_int = std::is_same<decltype(val), uintptr_t>::value;
		static_assert(is_ptr || is_hptr || is_int, "Expected int or generic ptr argument to SE/CE");
	}
}

// Helper macros for converting code to the new class-based system
// DO NOT USE THESE IN NEW CODE
#define SE(e) (EntField::assertVoidEnt(e), (ServerEnt *)(e))
#define CE(e) (EntField::assertVoidEnt(e), (ClientEnt *)(e))

// Helper macro for defining specific accessors for common fields
#define FIELD(name, type, internal) type &name() { return this->field<type>(internal); }

struct ServerEnt {
	// Ensure type is opaque
	ServerEnt(const ServerEnt &) = delete;

	template <typename T> T &field(const char *field) {
		auto val = EntField::getServerOffset(this, field);
		if (!EntField::matchFieldType<T>(val.second)) EntField::warnBadFieldType(this, field, typeid(T).name(), val.second, true);
		return *(T *)((uintptr_t)this + val.first);
	}

	template <typename T> T &fieldOff(const char *field, int off) {
		auto val = EntField::getServerOffset(this, field);
		return *(T *)((uintptr_t)this + val.first + off);
	}

	FIELD(portals_placed, int, "iNumPortalsPlaced")
	FIELD(abs_origin, Vector, "m_vecAbsOrigin")
	FIELD(abs_angles, QAngle, "m_angAbsRotation")
	FIELD(abs_velocity, Vector, "m_vecAbsVelocity")
	FIELD(local_velocity, Vector, "m_vecVelocity")
	FIELD(flags, int, "m_fFlags")
	FIELD(eflags, int, "m_iEFlags")
	FIELD(max_speed, float, "m_flMaxspeed")
	FIELD(gravity, float, "m_flGravity")
	FIELD(view_offset, Vector, "m_vecViewOffset")
	FIELD(portal_local, CPortalPlayerLocalData, "m_PortalLocal")
	FIELD(name, char *, "m_iName")
	FIELD(classname, char *, "m_iClassname")
	FIELD(player_state, CPlayerState, "pl")
	FIELD(ground_entity, CBaseHandle, "m_hGroundEntity")
	FIELD(ducked, bool, "m_bDucked")
	FIELD(collision, ICollideable, "m_Collision")
	FIELD(active_weapon, CBaseHandle, "m_hActiveWeapon")
};

struct ClientEnt {
	// Ensure type is opaque
	ClientEnt(const ClientEnt &) = delete;

	template <typename T> T &field(const char *field) {
		auto val = EntField::getClientOffset(this, field);
		if (!EntField::matchFieldType<T>(val.second)) EntField::warnBadFieldType(this, field, typeid(T).name(), val.second, false);
		return *(T *)((uintptr_t)this + val.first);
	}

	template <typename T> T &fieldOff(const char *field, int off) {
		auto val = EntField::getClientOffset(this, field);
		return *(T *)((uintptr_t)this + val.first + off);
	}

	FIELD(abs_origin, Vector, "m_vecAbsOrigin")
	FIELD(abs_angles, QAngle, "m_angAbsRotation")
	FIELD(abs_velocity, Vector, "m_vecAbsVelocity")
	FIELD(local_velocity, Vector, "m_vecVelocity")
	FIELD(view_offset, Vector, "m_vecViewOffset")
	FIELD(portal_local, CPortalPlayerLocalData, "m_PortalLocal")
	FIELD(player_state, CPlayerState, "pl")
	FIELD(ground_entity, CBaseHandle, "m_hGroundEntity")
	FIELD(ducked, bool, "m_bDucked")
	FIELD(active_weapon, CBaseHandle, "m_hActiveWeapon")
};

#undef FIELD
