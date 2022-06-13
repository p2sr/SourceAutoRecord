#pragma once

typedef enum {
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_VectorXY,
	DPT_String,
	DPT_Array,
	DPT_DataTable,
	DPT_Int64,
	DPT_NUMSendPropTypes
} SendPropType;

struct SendProp;
struct RecvProp;
struct SendTable;

typedef void (*RecvVarProxyFn)(const void *pData, void *pStruct, void *pOut);
typedef void (*ArrayLengthRecvProxyFn)(void *pStruct, int objectID, int currentArrayLength);
typedef void (*DataTableRecvVarProxyFn)(const RecvProp *pProp, void **pOut, void *pData, int objectID);
typedef void (*SendVarProxyFn)(const SendProp *pProp, const void *pStructBase, const void *pData, void *pOut, int iElement, int objectID);
typedef int (*ArrayLengthSendProxyFn)(const void *pStruct, int objectID);
typedef void *(*SendTableProxyFn)(const SendProp *pProp, const void *pStructBase, const void *pData, void *pRecipients, int objectID);

struct RecvTable {
	RecvProp *m_pProps;
	int m_nProps;
	void *m_pDecoder;
	char *m_pNetTableName;
	bool m_bInitialized;
	bool m_bInMainList;
};

struct RecvProp {
	char *m_pVarName;
	SendPropType m_RecvType;
	int m_Flags;
	int m_StringBufferSize;
	bool m_bInsideArray;
	const void *m_pExtraData;
	RecvProp *m_pArrayProp;
	ArrayLengthRecvProxyFn m_ArrayLengthProxy;
	RecvVarProxyFn m_ProxyFn;
	DataTableRecvVarProxyFn m_DataTableProxyFn;
	RecvTable *m_pDataTable;
	int m_Offset;
	int m_ElementStride;
	int m_nElements;
	const char *m_pParentArrayPropName;
};

struct SendProp {
	void *VMT;                                  // 0
	RecvProp *m_pMatchingRecvProp;              // 4
	SendPropType m_Type;                        // 8
	int m_nBits;                                // 12
	float m_fLowValue;                          // 16
	float m_fHighValue;                         // 20
	SendProp *m_pArrayProp;                     // 24
	ArrayLengthSendProxyFn m_ArrayLengthProxy;  // 28
	int m_nElements;                            // 32
	int m_ElementStride;                        // 36
	char *m_pExcludeDTName;                     // 40
	char *m_pParentArrayPropName;               // 44
	char *m_pVarName;                           // 48
	float m_fHighLowMul;                        // 52
	char m_priority;                            // 56
	int m_Flags;                                // 60
	SendVarProxyFn m_ProxyFn;                   // 64
	SendTableProxyFn m_DataTableProxyFn;        // 68
	SendTable *m_pDataTable;                    // 72
	int m_Offset;                               // 76
	const void *m_pExtraData;                   // 80
};

struct SendTable {
	SendProp *m_pProps;
	int m_nProps;
	char *m_pNetTableName;
	void *m_pPrecalc;
	bool m_bInitialized : 1;
	bool m_bHasBeenWritten : 1;
	bool m_bHasPropsEncodedAgainstCurrentTickCount : 1;
};

typedef void *(*CreateClientClassFn)(int entnum, int serialNum);
typedef void *(*CreateEventFn)();

struct ClientClass {
	CreateClientClassFn m_pCreateFn;
	CreateEventFn m_pCreateEventFn;
	char *m_pNetworkName;
	RecvTable *m_pRecvTable;
	ClientClass *m_pNext;
	int m_ClassID;
};

struct ServerClass {
	char *m_pNetworkName;
	SendTable *m_pTable;
	ServerClass *m_pNext;
	int m_ClassID;
	int m_InstanceBaselineIndex;
};
