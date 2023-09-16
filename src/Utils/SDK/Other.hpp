#pragma once
#pragma warning(suppress : 26495)
#include "Offsets.hpp"

#include "Color.hpp"
#include "Handle.hpp"
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
class IMaterialInternal;

class CMaterial_QueueFriendly {
public:
	void* vtable;
	IMaterialInternal* m_pRealTimeVersion;
};

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

typedef struct player_info_s
{
	// network xuid
	uint64_t xuid;
	// scoreboard information
	char name[32];
	// local server user ID, unique while server is running
	int userID;
	// global unique player identifer
	char guid[32 + 1];
	// friends identification number
	uint32_t friendsID;
	// friends name
	char friendsName[32];
	// true, if player is a bot controlled by game.dll
	bool fakeplayer;
	// true if player is the HLTV proxy
	bool ishltv;
	// custom files CRC for this player
	CRC32_t customFiles[4];
	// this counter increases each time the server downloaded a new file
	unsigned char filesDownloaded;
} player_info_t;
