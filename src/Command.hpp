#pragma once
#include "Game.hpp"
#include "Modules/Tier1.hpp"

#include <cstring>
#include <vector>

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
	static bool ActivateAutoCompleteFile(const char *name, _CommandCompletionCallback callback);
	static bool DectivateAutoCompleteFile(const char *name);
};

#define CON_COMMAND(name, description)                        \
	void name##_callback(const CCommand &args);                  \
	Command name = Command(#name, name##_callback, description); \
	void name##_callback(const CCommand &args)

#define CON_COMMAND_F(name, description, flags)                      \
	void name##_callback(const CCommand &args);                         \
	Command name = Command(#name, name##_callback, description, flags); \
	void name##_callback(const CCommand &args)

#define CON_COMMAND_F_COMPLETION(name, description, flags, completion)           \
	void name##_callback(const CCommand &args);                                     \
	Command name = Command(#name, name##_callback, description, flags, completion); \
	void name##_callback(const CCommand &args)

#define DECL_DECLARE_AUTOCOMPLETION_FUNCTION(command) \
	int command##_CompletionFunc(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
#define DECLARE_AUTOCOMPLETION_FUNCTION(command, subdirectory, extension)                                                        \
	CBaseAutoCompleteFileList command##Complete(#command, subdirectory, #extension);                                                \
	int command##_CompletionFunc(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) { \
		return command##Complete.AutoCompletionFunc(partial, commands);                                                                \
	}

#define AUTOCOMPLETION_FUNCTION(command) \
	command##_CompletionFunc

#define CON_COMMAND_AUTOCOMPLETEFILE(name, description, flags, subdirectory, extension) \
	DECLARE_AUTOCOMPLETION_FUNCTION(name, subdirectory, extension)                         \
	CON_COMMAND_F_COMPLETION(name, description, flags, AUTOCOMPLETION_FUNCTION(name))

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
        std::strcpy(commands[count++], (std::string(cmd) + item).c_str()); \
    }                                                                      \
    return count;
// clang-format on
#define CON_COMMAND_COMPLETION(name, description, completion) \
	DECL_AUTO_COMMAND_COMPLETION(name, completion)               \
	CON_COMMAND_F_COMPLETION(name, description, 0, name##_CompletionFunc)
