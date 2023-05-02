#pragma once
#include "Game.hpp"
#include "Modules/Tier1.hpp"

#include <cstring>
#include <vector>
#include <filesystem>
#include <algorithm>

class Command {
private:
	ConCommand *ptr;

public:
	int version;
	bool isRegistered;
	bool isReference;

public:
	static std::vector<Command *> &GetList();

public:
	Command();
	~Command();
	Command(const char *name);
	Command(const char *pName, _CommandCallback callback, const char *pHelpString, int flags = 0, _CommandCompletionCallback completionFunc = nullptr);

	ConCommand *ThisPtr();

	void UniqueFor(int version);
	void Register();
	void Unregister();

	bool operator!();

	static int RegisterAll();
	static void UnregisterAll();
	static Command *Find(const char *name);

	static bool Hook(const char *name, _CommandCallback detour, _CommandCallback &original);
	static bool Unhook(const char *name, _CommandCallback original);
	static bool HookCompletion(const char *name, _CommandCompletionCallback callback, _CommandCompletionCallback &original);
	static bool UnhookCompletion(const char *name, _CommandCompletionCallback original);
	static bool ActivateAutoCompleteFile(const char *name, _CommandCompletionCallback callback);
	static bool DectivateAutoCompleteFile(const char *name);
};

#define CON_COMMAND(name, description)                        \
	void name##_callback(const CCommand &args);                  \
	Command name(#name, name##_callback, description); \
	void name##_callback(const CCommand &args)

#define CON_COMMAND_F(name, description, flags)                      \
	void name##_callback(const CCommand &args);                         \
	Command name(#name, name##_callback, description, flags); \
	void name##_callback(const CCommand &args)

#define CON_COMMAND_F_COMPLETION(name, description, flags, completion)           \
	void name##_callback(const CCommand &args);                                     \
	Command name(#name, name##_callback, description, flags, completion); \
	void name##_callback(const CCommand &args)

#define DECL_DECLARE_AUTOCOMPLETION_FUNCTION(command) \
	int command##_CompletionFunc(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])

#define AUTOCOMPLETION_FUNCTION(command) \
	command##_CompletionFunc

#define DECL_DETOUR_COMMAND(name)         \
	static _CommandCallback name##_callback; \
	static void name##_callback_hook(const CCommand &args)
#define DETOUR_COMMAND(name) \
	void name##_callback_hook(const CCommand &args)

#define DECL_COMMAND_COMPLETION(command)          \
	DECL_DECLARE_AUTOCOMPLETION_FUNCTION(command) {  \
		const char *cmd = #command " ";                 \
		char *match = (char *)partial;                  \
		if (std::strstr(partial, cmd) == partial) {     \
			match = match + std::strlen(cmd);              \
		}                                               \
		static auto items = std::vector<std::string>(); \
		items.clear();
#define DECL_AUTO_COMMAND_COMPLETION(command, completion)             \
	DECL_DECLARE_AUTOCOMPLETION_FUNCTION(command) {                      \
		const char *cmd = #command " ";                                     \
		char *match = (char *)partial;                                      \
		if (std::strstr(partial, cmd) == partial) {                         \
			match = match + std::strlen(cmd);                                  \
		}                                                                   \
		static auto items = std::vector<std::string>();                     \
		items.clear();                                                      \
		static auto list = std::vector<std::string> completion;             \
		for (auto &item : list) {                                           \
			if (items.size() == COMMAND_COMPLETION_MAXITEMS) {                 \
				break;                                                            \
			}                                                                  \
			if (std::strlen(match) != std::strlen(cmd)) {                      \
				if (std::strstr(item.c_str(), match)) {                           \
					items.push_back(item);                                           \
				}                                                                 \
			} else {                                                           \
				items.push_back(item);                                            \
			}                                                                  \
		}                                                                   \
		auto count = 0;                                                     \
		for (auto &item : items) {                                          \
			std::strcpy(commands[count++], (std::string(cmd) + item).c_str()); \
		}                                                                   \
		return count;                                                       \
	}
// clang-format off
#define FINISH_COMMAND_COMPLETION()                                        \
	}                                                                      \
	auto count = 0;                                                        \
	for (auto& item : items) {                                             \
		if (count > COMMAND_COMPLETION_MAXITEMS - 1) break; \
		std::string s = std::string(cmd) + item; \
		if (s.size() > COMMAND_COMPLETION_ITEM_LENGTH - 1) s = s.substr(0, COMMAND_COMPLETION_ITEM_LENGTH - 1); \
		std::strcpy(commands[count++], s.c_str()); \
	}                                                                      \
	return count;
// clang-format on
#define CON_COMMAND_COMPLETION(name, description, completion) \
	DECL_AUTO_COMMAND_COMPLETION(name, completion)               \
	CON_COMMAND_F_COMPLETION(name, description, 0, AUTOCOMPLETION_FUNCTION(name))

std::vector<std::string> ParsePartialArgs(const char *partial);
int _FileCompletionFunc(std::string extension, std::string rootdir, int exp_args, const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

#define DECL_COMMAND_FILE_COMPLETION(command, extension, rootdir, exp_args) \
	DECL_DECLARE_AUTOCOMPLETION_FUNCTION(command) { \
		return _FileCompletionFunc(extension, rootdir, exp_args, partial, commands); \
	}
