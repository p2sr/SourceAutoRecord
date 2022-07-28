#pragma once

struct CEntInfo {
	void *m_pEntity;     // 0
	int m_SerialNumber;  // 4
	CEntInfo *m_pPrev;   // 8
	CEntInfo *m_pNext;   // 12
	void *unk1;          // 16
	void *unk2;          // 20
};


#define FL_EDICT_FREE (1 << 1)
#define FL_EDICT_FULL (1 << 2)

struct CBaseEdict {
	int m_fStateFlags;          // 0
	int m_NetworkSerialNumber;  // 4
	void *m_pNetworkable;       // 8
	void *m_pUnk;               // 12

	inline bool IsFree() const {
		return (m_fStateFlags & FL_EDICT_FREE) != 0;
	}
};

struct edict_t : CBaseEdict {
};

int ENTINDEX(edict_t *pEdict);
edict_t *INDEXENT(int iEdictNum);
