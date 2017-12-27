#pragma once
#include "Cvar.hpp"
#include "Tier1.hpp"

#include "Game.hpp"

namespace Tier1
{
	_ConVar ConVarCtor;

	struct ConVarData : ConCommandBase {
		void* VTable_IConVar;
		void* Parent;
		const char* DefaultValue;
		char* String;
		int StringLength;
		float FloatValue;
		int IntValue;
		bool HasMin;
		float MinVal;
		bool HasMax;
		float MaxVal;
		void* ChangeCallback;
		int unk1;
		int unk2;
		int unk3;
		int unk4;
	};

	void SetConVar(uintptr_t conVarAddr)
	{
		ConVarCtor = reinterpret_cast<_ConVar>(conVarAddr);
	}

	struct ConVar {
		void* Ptr = nullptr;
		std::unique_ptr<uint8_t[]> Blob;

		ConVar() = default;
		ConVar(const ConVar& other) = delete;
		ConVar(ConVar&& other) = default;
		ConVar& operator=(const ConVar& other) = delete;
		ConVar& operator=(ConVar&& other) = default;

		ConVar::ConVar(const char* ref) {
			Ptr = Cvar::FindVar(Cvar::Ptr, nullptr, ref);
		}
		bool ConVar::GetBool() const {
			return !!GetInt();
		}
		int ConVar::GetInt() const {
			return ((ConVarData*)Ptr)->IntValue;
		}
		float ConVar::GetFloat() const {
			return ((ConVarData*)Ptr)->FloatValue;
		}
		const char* ConVar::GetString() const {
			return ((ConVarData*)Ptr)->String;
		}
		const int ConVar::GetFlags() const {
			return ((ConVarData*)Ptr)->Flags;
		}
		void ConVar::SetValue(const char* value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::InternalSetValue);
			if (vf) ((_SetValueString)vf)(Ptr, nullptr, value);
		}
		void ConVar::SetValue(float value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::InternalSetFloatValue);
			if (vf) ((_SetValueFloat)vf)(Ptr, nullptr, value);
		}
		void ConVar::SetValue(int value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::InternalSetIntValue);
			if (vf) ((_SetValueInt)vf)(Ptr, nullptr, value);
		}
		void ConVar::SetFlags(int value) {
			((ConVarData*)Ptr)->Flags = value;
		}
		void ConVar::AddFlag(int value) {
			SetFlags(GetFlags() | value);
		}
		void ConVar::RemoveFlag(int value) {
			SetFlags(GetFlags() & ~(value));
		}
	};

	ConVar CreateVar(const char* name, const char* value, int flags = 0, const char* helpstr = "", bool hasmin = false, float min = 0, bool hasmax = false, float max = 0)
	{
		ConVar cv;
		size_t size = 0;

		switch (Game::Version) {
		case 0:
		case 1:
			size = sizeof(ConVarData);
			break;
		}

		cv.Blob = std::make_unique<uint8_t[]>(size);
		cv.Ptr = cv.Blob.get();

		std::memset(cv.Ptr, 0, size);
		ConVarCtor(cv.Ptr, nullptr, name, value, flags, helpstr, hasmin, min, hasmax, max);

		ConVarCount++;

		return cv;
	}
	ConVar CreateBoolean(const char* name, const char* value, const char* helpstr = "", int flags = FCVAR_NEVER_AS_STRING)
	{
		return CreateVar(name, value, flags, helpstr, true, 0, true, 1);
	}
	ConVar CreateFloat(const char* name, const char* value, float min, const char* helpstr = "", int flags = FCVAR_NEVER_AS_STRING)
	{
		return CreateVar(name, value, flags, helpstr, true, min);
	}
	ConVar CreateFloat(const char* name, const char* value, float min, float max, const char* helpstr = "", int flags = FCVAR_NEVER_AS_STRING)
	{
		return CreateVar(name, value, flags, helpstr, true, min, true, max);
	}
	ConVar CreateString(const char* name, const char* value, const char* helpstr = "")
	{
		return CreateVar(name, value, 0, helpstr);
	}
	ConVar CreateStringFloat(const char* name, const char* value, float min, float max, const char* helpstr = "")
	{
		return CreateVar(name, value, 0, helpstr, true, min, true, max);
	}
}