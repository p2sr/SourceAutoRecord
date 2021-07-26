#pragma once
#include "Module.hpp"
#include "Utils.hpp"

#include <functional>

#ifdef _WIN32
#	define TIER0 "tier0"
#	define CONCOLORMSG_SYMBOL "?ConColorMsg@@YAXABVColor@@PBDZZ"
#	define DEVMSG_SYMBOL "?DevMsg@@YAXPBDZZ"
#	define DEVWARNINGMSG_SYMBOL "?DevWarning@@YAXPBDZZ"
#else
#	define TIER0 "libtier0"
#	define CONCOLORMSG_SYMBOL "_Z11ConColorMsgRK5ColorPKcz"
#	define DEVMSG_SYMBOL "_Z6DevMsgPKcz"
#	define DEVWARNINGMSG_SYMBOL "_Z10DevWarningPKcz"
#endif

#define MSG_SYMBOL "Msg"
#define WARNING_SYMBOL "Warning"

#define SAR_PRINT_COLOR Color(247, 214, 68)
#define SAR_PRINT_ACTIVE_COLOR Color(110, 247, 76)

class Console : public Module {
public:
	using _Msg = void(__cdecl *)(const char *pMsgFormat, ...);
	using _Warning = void(__cdecl *)(const char *pMsgFormat, ...);
	using _ColorMsg = void(__cdecl *)(const Color &clr, const char *pMsgFormat, ...);
	using _DevMsg = void(__cdecl *)(const char *pMsgFormat, ...);
	using _DevWarning = void(__cdecl *)(const char *pMsgFormat, ...);
	using _LoggingSystem_RegisterLoggingListener = void(__cdecl *)(ILoggingListener *listener);
	using _LoggingSystem_PushLoggingState = void(__cdecl *)(bool threadLocal, bool clearState);
	using _LoggingSystem_PopLoggingState = void(__cdecl *)(bool threadLocal);
	using _LoggingSystem_HasTag = bool(__cdecl *)(int channelID, const char *tag);
	using _LoggingSystem_SetChannelSpewLevelByTag = void(__cdecl *)(const char *tag, LoggingSeverity severity);

	_Msg Msg = nullptr;
	_ColorMsg ColorMsg = nullptr;
	_Warning Warning = nullptr;
	_DevMsg DevMsg = nullptr;
	_DevWarning DevWarning = nullptr;
	_LoggingSystem_PushLoggingState LoggingSystem_PushLoggingState = nullptr;
	_LoggingSystem_PopLoggingState LoggingSystem_PopLoggingState = nullptr;
	_LoggingSystem_RegisterLoggingListener LoggingSystem_RegisterLoggingListener = nullptr;
	_LoggingSystem_HasTag LoggingSystem_HasTag = nullptr;
	_LoggingSystem_SetChannelSpewLevelByTag LoggingSystem_SetChannelSpewLevelByTag = nullptr;

public:
	template <typename... T>
	void Print(const char *fmt, T... args) {
		this->ColorMsg(SAR_PRINT_COLOR, fmt, args...);
	}
	template <typename... T>
	void PrintActive(const char *fmt, T... args) {
		this->ColorMsg(SAR_PRINT_ACTIVE_COLOR, fmt, args...);
	}

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE(TIER0); }
};

class ConsoleListener : private ILoggingListener {
public:
	ConsoleListener(std::function<void(const char *)> cbk);
	virtual ~ConsoleListener();

private:
	virtual void Log(const LoggingContext *ctx, const char *msg) override;
	std::function<void(const char *)> cbk;
};

extern Console *console;
