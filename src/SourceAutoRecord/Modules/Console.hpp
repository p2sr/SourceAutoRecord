#pragma once
#include "Utils.hpp"

#ifdef _WIN32
#define TIER0 "tier0"
#define CONCOLORMSG_SYMBOL "?ConColorMsg@@YAXABVColor@@PBDZZ"
#define DEVMSG_SYMBOL "?DevMsg@@YAXPBDZZ"
#define DEVWARNINGMSG_SYMBOL "?DevWarning@@YAXPBDZZ"
#else
#define TIER0 "libtier0"
#define CONCOLORMSG_SYMBOL "_Z11ConColorMsgRK5ColorPKcz"
#define DEVMSG_SYMBOL "_Z6DevMsgPKcz"
#define DEVWARNINGMSG_SYMBOL "_Z10DevWarningPKcz"
#endif

#define MSG_SYMBOL "Msg"
#define WARNING_SYMBOL "Warning"

#define SAR_PRINT_COLOR Color(247, 214, 68)
#define SAR_PRINT_ACTIVE_COLOR Color(110, 247, 76)

class Console {
public:
    using _Msg = void(__cdecl*)(const char* pMsgFormat, ...);
    using _Warning = void(__cdecl*)(const char* pMsgFormat, ...);
    using _ColorMsg = void(__cdecl*)(const Color& clr, const char* pMsgFormat, ...);
    using _DevMsg = void(__cdecl*)(const char* pMsgFormat, ...);
    using _DevWarning = void(__cdecl*)(const char* pMsgFormat, ...);

    _Msg Msg;
    _ColorMsg ColorMsg;
    _Warning Warning;
    _DevMsg DevMsg;
    _DevWarning DevWarning;

    bool Init();

    template <typename... T>
    void Print(const char* fmt, T... args)
    {
        this->ColorMsg(SAR_PRINT_COLOR, fmt, args...);
    }

    template <typename... T>
    void PrintActive(const char* fmt, T... args)
    {
        this->ColorMsg(SAR_PRINT_ACTIVE_COLOR, fmt, args...);
    }
};

extern Console* console;
