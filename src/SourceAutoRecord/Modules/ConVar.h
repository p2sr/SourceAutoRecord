#pragma once
#include "Cvar.h"
#include "Offsets.h"
#include "Tier1.h"

namespace Tier1
{
	_ConVar ConVarCtor;

	// Portal 2 6879
	// INFRA 6905
	struct ConVarData0 : ConCommandBase {
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

	void InitConVar(uintptr_t conVarAddr)
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
			switch (Offsets::Variant) {
			case 0: // Portal 2 6879
			case 1: // INFRA 6905
				return ((ConVarData0*)Ptr)->IntValue;
			}
			return 0;
		}
		float ConVar::GetFloat() const {
			switch (Offsets::Variant) {
			case 0: // Portal 2 6879
			case 1: // INFRA 6905
				return ((ConVarData0*)Ptr)->FloatValue;
			}
			return 0;
		}
		const char* ConVar::GetString() const {
			switch (Offsets::Variant) {
			case 0: // Portal 2 6879
			case 1: // INFRA 6905
				return ((ConVarData0*)Ptr)->String;
			}
			return nullptr;
		}
		void ConVar::SetValue(const char* value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::SetValueString);
			if (vf) ((_SetValueString)vf)(Ptr, nullptr, value);
		}
		void ConVar::SetValue(float value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::SetValueFloat);
			if (vf) ((_SetValueFloat)vf)(Ptr, nullptr, value);
		}
		void ConVar::SetValue(int value) {
			auto vf = GetVirtualFunctionByIndex(Ptr, Offsets::SetValueInt);
			if (vf) ((_SetValueInt)vf)(Ptr, nullptr, value);
		}
	};

	ConVar CreateVar(const char* name, const char* value, int flags = 0, const char* helpstr = "", bool hasmin = false, float min = 0, bool hasmax = false, float max = 0)
	{
		ConVar ret;
		size_t size = 0;

		switch (Offsets::Variant) {
		case 0:	// Portal 2 6879
		case 1: // INFRA 6905
			size = sizeof(ConVarData0);
			break;
		}

		ret.Blob = std::make_unique<uint8_t[]>(size);
		ret.Ptr = ret.Blob.get();

		std::memset(ret.Ptr, 0, size);
		ConVarCtor(ret.Ptr, nullptr, name, value, flags, helpstr, hasmin, min, hasmax, max);

		return ret;
	}
	ConVar CreateBoolean(const char* name, const char* value, const char* helpstr = "")
	{
		return CreateVar(name, value, FCVAR_NEVER_AS_STRING, helpstr, true, 0, true, 1);
	}
	ConVar CreateFloat(const char* name, const char* value, float min, const char* helpstr = "")
	{
		return CreateVar(name, value, FCVAR_NEVER_AS_STRING, helpstr, true, min);
	}
	ConVar CreateFloat(const char* name, const char* value, float min, float max, const char* helpstr = "")
	{
		return CreateVar(name, value, FCVAR_NEVER_AS_STRING, helpstr, true, min, true, max);
	}
	ConVar CreateFloatString(const char* name, const char* value, float min, float max, const char* helpstr = "")
	{
		return CreateVar(name, value, 0, helpstr, true, min, true, max);
	}
	ConVar CreateString(const char* name, const char* value, const char* helpstr = "")
	{
		return CreateVar(name, value, 0, helpstr);
	}
}