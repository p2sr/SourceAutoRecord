#include "Entity.hpp"
#include "Utils/Memory.hpp"
#include "Modules/Console.hpp"

static inline EntField::Type translateSendType(SendPropType t) {
#define C(a, b) case DPT_##a: return EntField::Type::b;
	switch (t) {
		C(Int, ANY_INT)
		C(Float, FLOAT)
		C(Vector, VECTOR)
		C(String, STRING)
		C(DataTable, OTHER)
		default: return EntField::Type::NONE;
	}
#undef C
}

static inline EntField::Type translateFieldType(fieldtype_t t) {
#define C(a, b) case a: return EntField::Type::b;
	switch (t) {
		C(FIELD_VOID, NONE)
		C(FIELD_FLOAT, FLOAT)
		C(FIELD_STRING, STRING)
		C(FIELD_VECTOR, VECTOR)
		C(FIELD_INTEGER, INT)
		C(FIELD_BOOLEAN, BOOL)
		C(FIELD_SHORT, SHORT)
		C(FIELD_CHARACTER, CHAR)
		C(FIELD_COLOR32, COLOR)
		C(FIELD_EMBEDDED, OTHER)
		C(FIELD_CUSTOM, OTHER)
		C(FIELD_CLASSPTR, POINTER)
		C(FIELD_EHANDLE, HANDLE)
		C(FIELD_EDICT, EDICT)
		C(FIELD_POSITION_VECTOR, VECTOR)
		C(FIELD_TIME, FLOAT)
		C(FIELD_TICK, INT)
		C(FIELD_MODELNAME, STRING)
		C(FIELD_SOUNDNAME, STRING)
		C(FIELD_VMATRIX, VMATRIX)
		C(FIELD_VMATRIX_WORLDSPACE, VMATRIX)
		C(FIELD_MATRIX3X4_WORLDSPACE, MATRIX3X4)
		default: return EntField::Type::NONE;
	}
#undef C
}

static void traverseSendTables(SendTable *table, size_t off, std::unordered_map<std::string, std::pair<size_t, EntField::Type>> &out) {
	for (int i = 0; i < table->m_nProps; ++i) {
		SendProp *prop = &table->m_pProps[i];

		if (!prop->m_pVarName) continue;
		std::string name(prop->m_pVarName);

		out[name] = {off + prop->m_Offset, translateSendType(prop->m_Type)};

		// handle arrays
		size_t idx = name.find("[0]");
		if (idx != std::string::npos) {
			std::string base(name);
			base.erase(idx);
			out[base] = {off + prop->m_Offset, EntField::Type::ARRAY};
		}

		if (prop->m_Type == SendPropType::DPT_DataTable) {
			traverseSendTables(prop->m_pDataTable, off + prop->m_Offset, out);
		}
	}
}

static void traverseRecvTables(RecvTable *table, size_t off, std::unordered_map<std::string, std::pair<size_t, EntField::Type>> &out) {
	for (int i = 0; i < table->m_nProps; ++i) {
		RecvProp *prop = &table->m_pProps[i];

		if (!prop->m_pVarName) continue;
		std::string name(prop->m_pVarName);

		out[name] = {off + prop->m_Offset, translateSendType(prop->m_RecvType)};

		// handle arrays
		size_t idx = name.find("[0]");
		if (idx != std::string::npos) {
			std::string base(name);
			base.erase(idx);
			out[base] = {off + prop->m_Offset, EntField::Type::ARRAY};
		}

		if (prop->m_RecvType == SendPropType::DPT_DataTable) {
			traverseRecvTables(prop->m_pDataTable, off + prop->m_Offset, out);
		}
	}
}

static void traverseDataMap(datamap_t *dm, size_t off, std::unordered_map<std::string, std::pair<size_t, EntField::Type>> &out) {
	for (int i = 0; i < dm->dataNumFields; ++i) {
		typedescription_t *field = &dm->dataDesc[i];
		if (!field->fieldName) continue;
		out[std::string(field->fieldName)] = {off + field->fieldOffset, translateFieldType(field->fieldType)};
		if (field->fieldType == FIELD_EMBEDDED) {
			traverseDataMap(field->td, off + field->fieldOffset, out);
		}
	}

	if (dm->baseMap) {
		traverseDataMap(dm->baseMap, off, out);
	}
}

static std::unordered_map<std::string, std::unordered_map<std::string, std::pair<size_t, EntField::Type>>> g_server_offsets;
static std::unordered_map<std::string, std::unordered_map<std::string, std::pair<size_t, EntField::Type>>> g_client_offsets;

const std::pair<size_t, EntField::Type> &EntField::getServerOffset(void *ent, const char *field) {
	ServerClass *sc = Memory::VMT<ServerClass *(__rescall *)(void *)>(ent, Offsets::GetServerClass)(ent);
	datamap_t *dm = Memory::VMT<datamap_t *(__rescall *)(void *)>(ent, Offsets::S_GetDataDescMap)(ent);

	// dumb, but construct class name by concating datamap and serverclass names
	std::string class_name(sc->m_pNetworkName ? sc->m_pNetworkName : "");
	class_name += dm->dataClassName ? dm->dataClassName : "";

	if (g_server_offsets.find(class_name) == g_server_offsets.end()) {
		// do datamap second so it overrides as it has better type info
		traverseSendTables(sc->m_pTable, 0, g_server_offsets[class_name]);
		traverseDataMap(dm, 0, g_server_offsets[class_name]);
	}

	return g_server_offsets[class_name][std::string(field)];
}

const std::pair<size_t, EntField::Type> &EntField::getClientOffset(void *ent, const char *field) {
	// multiple inheritance is cringe
#ifdef _WIN32
	ClientClass *cc = ((ClientClass *(__rescall *)(void *))((void ***)ent)[2][2])(ent);
#else
	ClientClass *cc = Memory::VMT<ClientClass *(__rescall *)(void *)>(ent, 19)(ent);
#endif
	datamap_t *pm = Memory::VMT<datamap_t *(__rescall *)(void *)>(ent, Offsets::GetPredDescMap)(ent); // datamap used for prediction stuff
	datamap_t *dm = Memory::VMT<datamap_t *(__rescall *)(void *)>(ent, Offsets::C_GetDataDescMap)(ent);

	// dumb, but construct class name by concating datamap and clientclass names
	std::string class_name(cc->m_pNetworkName ? cc->m_pNetworkName : "");
	class_name += dm->dataClassName ? dm->dataClassName : "";

	if (g_client_offsets.find(class_name) == g_client_offsets.end()) {
		// do datamap second so it overrides as it has better type info
		traverseRecvTables(cc->m_pRecvTable, 0, g_client_offsets[class_name]);
		traverseDataMap(pm, 0, g_client_offsets[class_name]);
		traverseDataMap(dm, 0, g_client_offsets[class_name]);
	}

	return g_client_offsets[class_name][std::string(field)];
}

#define S(x) #x
static const char *g_type_names[] = {
	"void",
	"bool",
	"char",
	"short",
	"int",
	"integral type",
	"float",
	"string",
	"vector",
	"color",
	"handle",
	"edict",
	"pointer",
	"vmatrix",
	"matrix3x4_t",
	"other",
};
#undef S

void EntField::warnBadFieldType(void *ent, const char *field, const char *expect_type, Type actual_type, bool server) {
	if (actual_type == Type::NONE) {
		console->Print("ERROR: %s field %s does not exist or has no type!\n", server ? "server" : "client", field);
	} else {
		console->Print("ERROR: fetched %s field %s with incorrect type %s, should be %s!\n", server ? "server" : "client", field, expect_type, g_type_names[(int)actual_type]);
	}
}
