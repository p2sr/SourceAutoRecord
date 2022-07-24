#pragma once

#include "Color.hpp"
#include "Datamap.hpp"
#include "Handle.hpp"

#include <stdio.h>

struct variant_t {
	union {
		bool bVal;
		const char *iszVal;
		int iVal;
		float flVal;
		float vecVal[3];
		Color rgbaVal;
	};
	CBaseHandle eVal;

	fieldtype_t fieldType;

	static typedescription_t m_SaveBool[];
	static typedescription_t m_SaveInt[];
	static typedescription_t m_SaveFloat[];
	static typedescription_t m_SaveEHandle[];
	static typedescription_t m_SaveString[];
	static typedescription_t m_SaveColor[];
	static typedescription_t m_SaveVector[];
	static typedescription_t m_SavePositionVector[];
	static typedescription_t m_SaveVMatrix[];
	static typedescription_t m_SaveVMatrixWorldspace[];
	static typedescription_t m_SaveMatrix3x4Worldspace[];

	const char *ToString() const {
		switch (this->fieldType) {
		case FIELD_STRING:
			return this->iszVal;
		case FIELD_INTEGER:
			static char istr[32];
			sprintf(istr, "%i", this->iVal);
			return istr;
		case FIELD_BOOLEAN:
			return this->bVal ? "true" : "false";
		default:
			return "";
		}
	}
};
