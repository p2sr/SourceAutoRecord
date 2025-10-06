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
class IMaterialInternal;

class CMaterial_QueueFriendly {
public:
	void *vtable;
	IMaterialInternal *m_pRealTimeVersion;
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

typedef struct player_info_s {
	uint64_t xuid;

	char name[32];

	int userID;

	char guid[32 + 1];

	uint32_t friendsID;

	char friendsName[32];

	bool fakeplayer;

	bool ishltv;

	CRC32_t customFiles[4];

	unsigned char filesDownloaded;
} player_info_t;

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

struct PortalLeaderboardItem_t {
	uint64_t m_xuid;
	char m_szName[32];
	char pad_0028[16];
	int32_t m_iScore;
};

enum ETimelineGameMode {
	k_ETimelineGameMode_Invalid = 0,
	k_ETimelineGameMode_Playing = 1,
	k_ETimelineGameMode_Staging = 2,
	k_ETimelineGameMode_Menus = 3,
	k_ETimelineGameMode_LoadingScreen = 4,
	k_ETimelineGameMode_Max,
};

enum ETimelineEventClipPriority {
	k_ETimelineEventClipPriority_Invalid = 0,
	k_ETimelineEventClipPriority_None = 1,
	k_ETimelineEventClipPriority_Standard = 2,
	k_ETimelineEventClipPriority_Featured = 3,
};

class ISteamTimeline {
public:
	virtual void SetTimelineStateDescription(const char *pchDescription, float flTimeDelta) = 0;
	virtual void ClearTimelineStateDescription(float flTimeDelta) = 0;
	virtual void AddTimelineEvent(const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32_t unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip) = 0;
	virtual void SetTimelineGameMode(ETimelineGameMode eMode) = 0;
};

enum EVoiceResult {
	k_EVoiceResultOK = 0,
	k_EVoiceResultNotInitialized = 1,
	k_EVoiceResultNotRecording = 2,
	k_EVoiceResultNoData = 3,
	k_EVoiceResultBufferTooSmall = 4,
	k_EVoiceResultDataCorrupted = 5,
	k_EVoiceResultRestricted = 6,
	k_EVoiceResultUnsupportedCodec = 7,
	k_EVoiceResultReceiverOutOfDate = 8,
	k_EVoiceResultReceiverDidNotAnswer = 9,

};

enum EMessage {
	k_EMsgServerBegin = 0,
	k_EMsgServerSendInfo = k_EMsgServerBegin + 1,
	k_EMsgServerFailAuthentication = k_EMsgServerBegin + 2,
	k_EMsgServerPassAuthentication = k_EMsgServerBegin + 3,
	k_EMsgServerUpdateWorld = k_EMsgServerBegin + 4,
	k_EMsgServerExiting = k_EMsgServerBegin + 5,
	k_EMsgServerPingResponse = k_EMsgServerBegin + 6,
	k_EMsgServerPlayerHitSun = k_EMsgServerBegin + 7,
	k_EMsgClientBegin = 500,
	k_EMsgClientBeginAuthentication = k_EMsgClientBegin + 2,
	k_EMsgClientSendLocalUpdate = k_EMsgClientBegin + 3,
	k_EMsgP2PBegin = 600,
	k_EMsgP2PSendingTicket = k_EMsgP2PBegin + 1,
	k_EMsgVoiceChatBegin = 700,
	k_EMsgVoiceChatData = k_EMsgVoiceChatBegin + 2,
	k_EForceDWORD = 0x7fffffff,
};

struct MsgVoiceChatData_t {
	MsgVoiceChatData_t()
		: m_dwMessageType((k_EMsgVoiceChatData)) {}
	unsigned long GetMessageType() const { return (m_dwMessageType); }

	void SetDataLength(uint32_t unLength) { m_uDataLength = (unLength); }
	uint32_t GetDataLength() const { return (m_uDataLength); }

	void SetSteamID(uint64_t steamID) { from_steamID = steamID; }
	uint64_t GetSteamID() const { return from_steamID; }

private:
	const unsigned long m_dwMessageType;
	uint32_t m_uDataLength;
	uint64_t from_steamID;
};

class ISteamUser {
public:
	virtual void *GetHSteamUser() = 0;
	virtual bool BLoggedOn() = 0;
	virtual uint64_t GetSteamID() = 0;
	virtual int InitiateGameConnection_DEPRECATED(void *pAuthBlob, int cbMaxAuthBlob, uint64_t steamIDGameServer, uint32_t unIPServer, uint16_t usPortServer, bool bSecure) = 0;
	virtual void TerminateGameConnection_DEPRECATED(uint32_t unIPServer, uint16_t usPortServer) = 0;
	virtual void TrackAppUsageEvent(uint64_t gameID, int eAppUsageEvent, const char *pchExtraInfo = "") = 0;
	virtual bool GetUserDataFolder(char *pchBuffer, int cubBuffer) = 0;
	virtual void StartVoiceRecording() = 0;
	virtual void StopVoiceRecording() = 0;
	virtual EVoiceResult GetAvailableVoice(uint32_t *pcbCompressed, uint32_t *pcbUncompressed_Deprecated = 0, uint32_t nUncompressedVoiceDesiredSampleRate_Deprecated = 0) = 0;
	virtual EVoiceResult GetVoice(bool bWantCompressed, void *pDestBuffer, uint32_t cbDestBufferSize, uint32_t *nBytesWritten, bool bWantUncompressed_Deprecated = false, void *pUncompressedDestBuffer_Deprecated = 0, uint32_t cbUncompressedDestBufferSize_Deprecated = 0, uint32_t *nUncompressBytesWritten_Deprecated = 0, uint32_t nUncompressedVoiceDesiredSampleRate_Deprecated = 0) = 0;
	virtual EVoiceResult DecompressVoice(const void *pCompressed, uint32_t cbCompressed, void *pDestBuffer, uint32_t cbDestBufferSize, uint32_t *nBytesWritten, uint32_t nDesiredSampleRate) = 0;
	virtual uint32_t GetVoiceOptimalSampleRate() = 0;
	// don't need the rest.
};
