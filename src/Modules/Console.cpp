#include "Console.hpp"

#include "Command.hpp"

bool Console::Init() {
	auto tier0 = Memory::GetModuleHandleByName(this->Name());
	if (tier0) {
		this->Msg = Memory::GetSymbolAddress<_Msg>(tier0, MSG_SYMBOL);
		this->ColorMsg = Memory::GetSymbolAddress<_ColorMsg>(tier0, CONCOLORMSG_SYMBOL);
		this->Warning = Memory::GetSymbolAddress<_Warning>(tier0, WARNING_SYMBOL);
		this->DevMsg = Memory::GetSymbolAddress<_DevMsg>(tier0, DEVMSG_SYMBOL);
		this->DevWarning = Memory::GetSymbolAddress<_DevWarning>(tier0, DEVWARNINGMSG_SYMBOL);

		this->LoggingSystem_PushLoggingState = Memory::GetSymbolAddress<_LoggingSystem_PushLoggingState>(tier0, "LoggingSystem_PushLoggingState");
		this->LoggingSystem_PopLoggingState = Memory::GetSymbolAddress<_LoggingSystem_PopLoggingState>(tier0, "LoggingSystem_PopLoggingState");
		this->LoggingSystem_RegisterLoggingListener = Memory::GetSymbolAddress<_LoggingSystem_RegisterLoggingListener>(tier0, "LoggingSystem_RegisterLoggingListener");
		this->LoggingSystem_HasTag = Memory::GetSymbolAddress<_LoggingSystem_HasTag>(tier0, "LoggingSystem_HasTag");
		this->LoggingSystem_SetChannelSpewLevelByTag = Memory::GetSymbolAddress<_LoggingSystem_SetChannelSpewLevelByTag>(tier0, "LoggingSystem_SetChannelSpewLevelByTag");

		Memory::CloseModuleHandle(tier0);
	}

	return this->hasLoaded = tier0 && this->Msg && this->ColorMsg && this->Warning && this->DevMsg && this->DevWarning;
}
void Console::Shutdown() {
}

ConsoleListener::ConsoleListener(std::function<void(const char *)> cbk)
	: cbk(cbk) {
	console->LoggingSystem_PushLoggingState(false, false);
	console->LoggingSystem_RegisterLoggingListener(this);
	console->LoggingSystem_SetChannelSpewLevelByTag("Console", LSEV_MESSAGE);
}

void ConsoleListener::Log(const LoggingContext *ctx, const char *msg) {
	if (!console->LoggingSystem_HasTag(ctx->channelID, "Console")) return;
	this->cbk(msg);
}

ConsoleListener::~ConsoleListener() {
	console->LoggingSystem_PopLoggingState(false);
}

Console *console;
