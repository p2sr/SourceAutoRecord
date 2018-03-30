#pragma once
#include "Tier1.hpp"

#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"

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

	bool ConVarLoaded()
	{
		auto cnv = SAR::Find("ConVar_Ctor3");
		if (cnv.Found) {
			ConVarCtor = reinterpret_cast<_ConVar>(cnv.Address);
		}
		return cnv.Found;
	}

	struct ConVar {
		void* Ptr = nullptr;
		std::unique_ptr<uint8_t[]> Blob;

		ConVar() = default;
		ConVar(const ConVar& other) = delete;
		ConVar(ConVar&& other) = default;
		ConVar& operator=(const ConVar& other) = delete;
		ConVar& operator=(ConVar&& other) = default;

		ConVar(void* ptr) {
			Ptr = ptr;
		}
		bool GetBool() const {
			return !!GetInt();
		}
		int GetInt() const {
			return ((ConVarData*)Ptr)->IntValue;
		}
		float GetFloat() const {
			return ((ConVarData*)Ptr)->FloatValue;
		}
		const char* GetString() const {
			return ((ConVarData*)Ptr)->String;
		}
		const int GetFlags() const {
			return ((ConVarData*)Ptr)->Flags;
		}
		void SetValue(const char* value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::InternalSetValue);
			if (vf) ((_InternalSetValue)vf)(Ptr, value);
		}
		void SetValue(float value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::InternalSetFloatValue);
			if (vf) ((_InternalSetFloatValue)vf)(Ptr, value);
		}
		void SetValue(int value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::InternalSetIntValue);
			if (vf) ((_InternalSetIntValue)vf)(Ptr, value);
		}
		void SetFlags(int value) {
			((ConVarData*)Ptr)->Flags = value;
		}
		void AddFlag(int value) {
			SetFlags(GetFlags() | value);
		}
		void RemoveFlag(int value) {
			SetFlags(GetFlags() & ~(value));
		}
	};

	ConVar CreateVar(const char* name, const char* value, int flags = 0, const char* helpstr = "", bool hasmin = false, float min = 0, bool hasmax = false, float max = 0)
	{
		ConVar cv;
		size_t size = sizeof(ConVarData);

		cv.Blob = std::make_unique<uint8_t[]>(size);
		cv.Ptr = cv.Blob.get();

		std::memset(cv.Ptr, 0, size);
		ConVarCtor(cv.Ptr, name, value, flags, helpstr, hasmin, min, hasmax, max);

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