#include "Command.hpp"

#include "Game.hpp"
#include "Modules/Tier1.hpp"
#include "SAR.hpp"

#include <cstring>
#include <set>
#include <algorithm>

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
	this->version = SourceGame_Unknown;
	this->isRegistered = false;
	this->isReference = true;
}
Command::Command(const char *pName, _CommandCallback callback, const char *pHelpString, int flags, _CommandCompletionCallback completionFunc)
	: version(SourceGame_Unknown)
	, isRegistered(false)
	, isReference(false) {
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
		*(void **)this->ptr = tier1->ConCommand_VTable; // stealing vtable from the game
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
	Command cc(name);
	if (!!cc) {
		original = cc.ThisPtr()->m_fnCommandCallback;
		cc.ThisPtr()->m_fnCommandCallback = detour;
		return true;
	}
	return false;
}
bool Command::Unhook(const char *name, _CommandCallback original) {
	Command cc(name);
	if (!!cc && original) {
		cc.ThisPtr()->m_fnCommandCallback = original;
		return true;
	}
	return false;
}
bool Command::HookCompletion(const char *name, _CommandCompletionCallback detour, _CommandCompletionCallback &original) {
	Command cc(name);
	if (!!cc) {
		original = cc.ThisPtr()->m_bHasCompletionCallback ? cc.ThisPtr()->m_fnCompletionCallback : nullptr;
		cc.ThisPtr()->m_bHasCompletionCallback = true;
		cc.ThisPtr()->m_fnCompletionCallback = detour;
		return true;
	}
	return false;
}
bool Command::UnhookCompletion(const char *name, _CommandCompletionCallback original) {
	Command cc(name);
	if (!!cc) {
		cc.ThisPtr()->m_bHasCompletionCallback = !!original;
		cc.ThisPtr()->m_fnCompletionCallback = original;
		return true;
	}
	return false;
}
bool Command::ActivateAutoCompleteFile(const char *name, _CommandCompletionCallback callback) {
	Command cc(name);
	if (!!cc) {
		cc.ThisPtr()->m_bHasCompletionCallback = true;
		cc.ThisPtr()->m_fnCompletionCallback = callback;
		return true;
	}
	return false;
}
bool Command::DectivateAutoCompleteFile(const char *name) {
	Command cc(name);
	if (!!cc) {
		cc.ThisPtr()->m_bHasCompletionCallback = false;
		cc.ThisPtr()->m_fnCompletionCallback = nullptr;
		return true;
	}
	return false;
}

std::vector<std::string> ParsePartialArgs(const char *partial) {
	std::vector<std::string> args;

	while (isspace(*partial)) {
		++partial;
	}

	bool trailing;
	while (*partial) {
		trailing = false;

		std::string arg;

		if (*partial == '"') {
			++partial;
			while (*partial && *partial != '"') {
				arg += *partial;
				++partial;
			}
			if (*partial == '"') {
				++partial;
				trailing = true;
			}
		} else {
			while (*partial && !isspace(*partial)) {
				arg += *partial;
				++partial;
			}
		}

		args.push_back(arg);

		while (isspace(*partial)) {
			++partial;
			trailing = true;
		}
	}

	if (trailing) args.push_back("");

	return args;
}

int _FileCompletionFunc(std::string extension, std::string rootdir, int exp_args, const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	auto args = ParsePartialArgs(partial);

	int completed_args = args.size() - 1;
	if (completed_args > exp_args + 1) completed_args = exp_args + 1;
	
	std::string part;
	for (int i = 0; i < completed_args; ++i) {
		if (args[i].find(" ") != std::string::npos) {
			part += "\"" + args[i] + "\" ";
		} else {
			part += args[i] + " ";
		}
	}

	if (args.size() == 1 || completed_args == exp_args + 1) {
		part = part.substr(0, part.size() - 1); // strip trailing space
		std::strncpy(commands[0], part.c_str(), COMMAND_COMPLETION_ITEM_LENGTH - 1);
		commands[0][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;
		return 1;
	}

	std::string cur = args[args.size() - 1];

	size_t last_slash = cur.rfind('/');
	std::string dirpart = last_slash == std::string::npos ? "" : cur.substr(0, last_slash) + "/";
	size_t dirpart_len = dirpart.size();

	std::string cur_lower = cur.substr(dirpart_len);
	std::transform(cur_lower.begin(), cur_lower.end(), cur_lower.begin(), tolower);
	

	std::vector<std::string> items;

	try {
		std::set<std::string> sorted;

		for (auto &file : std::filesystem::directory_iterator(rootdir + std::string("/") + dirpart)) {
			try {
				if (file.is_directory() || Utils::EndsWith(file.path().extension().string(), extension)) {
					std::string path = dirpart + file.path().stem().string();
					std::replace(path.begin(), path.end(), '\\', '/');
					if (file.is_directory()) path += "/";
					sorted.insert(path);
				}
			} catch (std::system_error &e) {
			}
		}

		for (auto &path : sorted) {
			std::string qpath =
				path.find(" ") == std::string::npos
				? path
				: Utils::ssprintf("\"%s\"", path.c_str());

			std::string path_lower = path;
			std::transform(path_lower.begin(), path_lower.end(), path_lower.begin(), tolower);

			if (path == cur) {
				items.insert(items.begin(), part + qpath);
			} else if (path_lower.find(cur_lower, dirpart_len) != std::string::npos) {
				items.push_back(part + qpath);
			}

			if (items.size() >= COMMAND_COMPLETION_MAXITEMS) break;
		}
	} catch (std::filesystem::filesystem_error &e) {
	}

	for (size_t i = 0; i < items.size(); ++i) {
		std::strncpy(commands[i], items[i].c_str(), COMMAND_COMPLETION_ITEM_LENGTH - 1);
		commands[i][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;
	}

	return items.size();
}
