#pragma once
#pragma warning(suppress : 26495)
#include "Color.hpp"
#include "Handle.hpp"
#include "Offsets.hpp"
#include "Trace.hpp"
#include "UtlMemory.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>


struct cmdalias_t {
	cmdalias_t *next;
	char name[32];
	char *value;
};

struct GameOverlayActivated_t {
	uint8_t m_bActive;
};

enum PaintMode_t {
	PAINT_UIPANELS = (1 << 0),
	PAINT_INGAMEPANELS = (1 << 1),
};


struct PaintPowerInfo_t {
	void *vtable;
	Vector m_SurfaceNormal;
	Vector m_ContactPoint;
	int m_PaintPowerType;
	CBaseHandle m_HandleToOther;
	int m_State;
	bool m_IsOnThinSurface;
};


enum {
	PORTAL_COND_TAUNTING = 0,
	PORTAL_COND_POINTING,
	PORTAL_COND_DROWNING,
	PORTAL_COND_DEATH_CRUSH,
	PORTAL_COND_DEATH_GIB,
	PORTAL_COND_LAST
};


class IMaterial;

struct CFontAmalgam {
	struct TFontRange {
		int lowRange;
		int highRange;
		void *pFont;
	};

	CUtlVector<TFontRange> m_Fonts;
	int m_iMaxWidth;
	int m_iMaxHeight;
};


struct FcpsTraceAdapter {
	void (*traceFunc)(const Ray_t &ray, CGameTrace *result, FcpsTraceAdapter *adapter);
	bool (*pointOutsideWorldFunc)(const Vector &test, FcpsTraceAdapter *adapter);
	ITraceFilter *traceFilter;
	unsigned mask;
};

class CPlayerState {
public:
	void *vtable;
	void *m_hTonemapController;
	QAngle v_angle;
};

struct ChallengeNodeData_t {
	const char *m_szMapName;
	Vector m_vecNodeOrigin;
	QAngle m_vecNodeAngles;
};

struct ChapterContextData_t {
	const char *m_szMapName;
	int m_nChapter;
	int m_nSubChapter;
};

struct PortalPlayerStatistics_t {
	void *unk;
	int iNumPortalsPlaced;
	int iNumStepsTaken;
	float fNumSecondsTaken;
	float fDistanceTaken;
};
