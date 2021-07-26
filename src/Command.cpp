#include "Command.hpp"

#include "Game.hpp"
#include "Modules/Tier1.hpp"
#include "SAR.hpp"

#include <cstring>

std::vector<Command *> &Command::GetList() {
	static std::vector<Command *> list;
	return list;
}

Command::Command()
	: ptr(nullptr)
	, version(SourceGame_Unknown)
	, isRegistered(false)
	, isReference(false) {
}
Command::~Command() {
	if (!this->isReference) {
		SAFE_DELETE(this->ptr)
	}
}
Command::Command(const char *name) {
	this->ptr = reinterpret_cast<ConCommand *>(tier1->FindCommandBase(tier1->g_pCVar->ThisPtr(), name));
	this->isReference = true;
}
Command::Command(const char *pName, _CommandCallback callback, const char *pHelpString, int flags, _CommandCompletionCallback completionFunc)
	: isReference(false)
	, isRegistered(false)
	, version(SourceGame_Unknown) {
	this->ptr = new ConCommand(pName, callback, pHelpString, flags, completionFunc);

	Command::GetList().push_back(this);
}
ConCommand *Command::ThisPtr() {
	return this->ptr;
}
void Command::UniqueFor(int version) {
	this->version = version;
}
void Command::Register() {
	if (!this->isRegistered) {
		this->ptr->ConCommandBase_VTable = tier1->ConCommand_VTable;
		tier1->RegisterConCommand(tier1->g_pCVar->ThisPtr(), this->ptr);
		tier1->m_pConCommandList = this->ptr;
	}
	this->isRegistered = true;
}
void Command::Unregister() {
	if (this->isRegistered) {
		tier1->UnregisterConCommand(tier1->g_pCVar->ThisPtr(), this->ptr);
	}
	this->isRegistered = false;
}
bool Command::operator!() {
	return this->ptr == nullptr;
}
int Command::RegisterAll() {
	auto result = 0;
	for (const auto &command : Command::GetList()) {
		if (command->version != SourceGame_Unknown && !sar.game->Is(command->version)) {
			continue;
		}
		command->Register();
		++result;
	}
	return result;
}
void Command::UnregisterAll() {
	for (const auto &command : Command::GetList()) {
		command->Unregister();
	}
}
Command *Command::Find(const char *name) {
	for (const auto &command : Command::GetList()) {
		if (!std::strcmp(command->ThisPtr()->m_pszName, name)) {
			return command;
		}
	}
	return nullptr;
}

bool Command::Hook(const char *name, _CommandCallback detour, _CommandCallback &original) {
	auto cc = Command(name);
	if (!!cc) {
		original = cc.ThisPtr()->m_fnCommandCallback;
		cc.ThisPtr()->m_fnCommandCallback = detour;
		return true;
	}
	return false;
}
bool Command::Unhook(const char *name, _CommandCallback original) {
	auto cc = Command(name);
	if (!!cc && original) {
		cc.ThisPtr()->m_fnCommandCallback = original;
		return true;
	}
	return false;
}
bool Command::ActivateAutoCompleteFile(const char *name, _CommandCompletionCallback callback) {
	auto cc = Command(name);
	if (!!cc) {
		cc.ThisPtr()->m_bHasCompletionCallback = true;
		cc.ThisPtr()->m_fnCompletionCallback = callback;
		return true;
	}
	return false;
}
bool Command::DectivateAutoCompleteFile(const char *name) {
	auto cc = Command(name);
	if (!!cc) {
		cc.ThisPtr()->m_bHasCompletionCallback = false;
		cc.ThisPtr()->m_fnCompletionCallback = nullptr;
		return true;
	}
	return false;
}
