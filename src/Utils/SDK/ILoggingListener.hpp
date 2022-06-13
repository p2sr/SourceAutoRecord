#pragma once

#include "Color.hpp"

typedef enum {
	LSEV_MESSAGE,
	LSEV_WARNING,
	LSEV_ASSERT,
	LSEV_ERROR,
} LoggingSeverity;

typedef enum {
	LCF_CONSOLE_ONLY = 0x1,
	LCF_DO_NOT_ECHO = 0x2,
} LoggingChannelFlags;

struct LoggingContext {
	int channelID;
	LoggingChannelFlags flags;
	LoggingSeverity severity;
	Color color;
};

class ILoggingListener {
public:
	virtual void Log(const LoggingContext *ctx, const char *msg) = 0;
};
