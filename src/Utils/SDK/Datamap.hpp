#pragma once

typedef enum _fieldtypes {
	FIELD_VOID = 0,
	FIELD_FLOAT,
	FIELD_STRING,
	FIELD_VECTOR,
	FIELD_QUATERNION,
	FIELD_INTEGER,
	FIELD_BOOLEAN,
	FIELD_SHORT,
	FIELD_CHARACTER,
	FIELD_COLOR32,
	FIELD_EMBEDDED,
	FIELD_CUSTOM,
	FIELD_CLASSPTR,
	FIELD_EHANDLE,
	FIELD_EDICT,
	FIELD_POSITION_VECTOR,
	FIELD_TIME,
	FIELD_TICK,
	FIELD_MODELNAME,
	FIELD_SOUNDNAME,
	FIELD_INPUT,
	FIELD_FUNCTION,
	FIELD_VMATRIX,
	FIELD_VMATRIX_WORLDSPACE,
	FIELD_MATRIX3X4_WORLDSPACE,
	FIELD_INTERVAL,
	FIELD_MODELINDEX,
	FIELD_MATERIALINDEX,
	FIELD_VECTOR2D,
	FIELD_TYPECOUNT
} fieldtype_t;

enum {
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED,
	TD_OFFSET_COUNT
};

struct inputdata_t;
typedef void (*inputfunc_t)(inputdata_t &data);

struct datamap_t;

struct typedescription_t {
	fieldtype_t fieldType;     // 0
	const char *fieldName;     // 4
	int fieldOffset;           // 8
	unsigned short fieldSize;  // 12
	short flags;               // 14
	const char *externalName;  // 16
	void *pSaveRestoreOps;     // 20
#ifndef _WIN32
	void *unk1;  // 24
#endif
	inputfunc_t inputFunc;                     // 24/28
	datamap_t *td;                             // 28/32
	int fieldSizeInBytes;                      // 32/36
	struct typedescription_t *override_field;  // 36/40
	int override_count;                        // 40/44
	float fieldTolerance;                      // 44/48
	int flatOffset[TD_OFFSET_COUNT];           // 48/52
	unsigned short flatGroup;                  // 56/60
};

struct datamap_t {
	typedescription_t *dataDesc;  // 0
	int dataNumFields;            // 4
	char const *dataClassName;    // 8
	datamap_t *baseMap;           // 12
	int m_nPackedSize;            // 16
	void *m_pOptimizedDataMap;    // 20
};
