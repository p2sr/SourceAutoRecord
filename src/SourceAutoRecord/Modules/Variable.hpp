#pragma once
#include "Tier1.hpp"

#include "Offsets.hpp"

namespace Tier1
{
	struct Variable {
		ConVar* ptr = nullptr;
		std::unique_ptr<uint8_t[]> data;

        Variable() = default;
		Variable(const Variable& other) = delete;
		Variable(Variable&& other) = default;
		Variable& operator=(const Variable& other) = delete;
		Variable& operator=(Variable&& other) = default;

		Variable(const char* name)
        {
			this->ptr = reinterpret_cast<ConVar*>(FindVar(g_pCVar->GetThisPtr(), name));
		}
        // Boolean or String
        Variable(const char* name, const char* value, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
        {
            if (flags != 0)
                Create(name, value, flags, helpstr, true, 0, true, 1);
            else
                Create(name, value, flags, helpstr);
        }
        // Float
        Variable(const char* name, const char* value, float min, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
        {
            Create(name, value, flags, helpstr, true, min);
        }
        // Float
        Variable(const char* name, const char* value, float min, float max, const char* helpstr, int flags = FCVAR_NEVER_AS_STRING)
        {
            Create(name, value, flags, helpstr, true, min, true, max);
        }
        void Create(const char* name, const char* value, int flags = 0, const char* helpstr = "", bool hasmin = false, float min = 0, bool hasmax = false, float max = 0)
        {
            auto size = sizeof(ConVar);
            data = std::make_unique<uint8_t[]>(size);
            this->ptr = reinterpret_cast<ConVar*>(data.get());
            std::memset(this->ptr, 0, size);

            ConVarCtor(this->ptr, name, value, flags, helpstr, hasmin, min, hasmax, max);
            ConVarCount++;
        }
		bool GetBool()
        {
			return !!GetInt();
		}
		int GetInt()
        {
			return this->ptr->m_nValue;
		}
		float GetFloat()
        {
			return this->ptr->m_fValue;
		}
		const char* GetString()
        {
			return this->ptr->m_pszString;
		}
		const int GetFlags()
        {
			return this->ptr->flags;
		}
		void SetValue(const char* value)
        {
			Memory::VMT<_InternalSetValue>(this->ptr, Offsets::InternalSetValue)(ptr, value);
		}
		void SetValue(float value)
        {
            Memory::VMT<_InternalSetFloatValue>(this->ptr, Offsets::InternalSetFloatValue)(ptr, value);
		}
		void SetValue(int value)
        {
            Memory::VMT<_InternalSetIntValue>(this->ptr, Offsets::InternalSetIntValue)(ptr, value);
		}
		void SetFlags(int value)
        {
			this->ptr->flags = value;
		}
		void AddFlag(int value)
        {
			SetFlags(GetFlags() | value);
		}
		void RemoveFlag(int value)
        {
			SetFlags(GetFlags() & ~(value));
		}
	};
}