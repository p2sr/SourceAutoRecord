#pragma once
#include "Modules/Tier1.hpp"

struct Command {
private:
    ConCommand* ptr;
    bool isRegistered;
    bool isReference;

    using _ShouldRegisterCallback = bool (*)();
    _ShouldRegisterCallback shouldRegister;

    static std::vector<Command*> list;

public:
    Command();
    ~Command();
    Command(const char* name);
    Command(const char* pName, _CommandCallback callback, const char* pHelpString, int flags = 0,
        _CommandCompletionCallback completionFunc = nullptr);
    ConCommand* ThisPtr();
    void UniqueFor(_ShouldRegisterCallback callback);
    void Register();
    void Unregister();
    bool operator!();
    static int RegisterAll();
    static void UnregisterAll();
    static Command* Find(const char* name);
};

#define CON_COMMAND(name, description)                           \
    void name##_callback(const CCommand& args);                  \
    Command name = Command(#name, name##_callback, description); \
    void name##_callback(const CCommand& args)

#define CON_COMMAND_F(name, description, flags)                         \
    void name##_callback(const CCommand& args);                         \
    Command name = Command(#name, name##_callback, description, flags); \
    void name##_callback(const CCommand& args)

#define CON_COMMAND_F_COMPLETION(name, description, flags, completion)              \
    void name##_callback(const CCommand& args);                                     \
    Command name = Command(#name, name##_callback, description, flags, completion); \
    void name##_callback(const CCommand& args)

#define DECLARE_AUTOCOMPLETION_FUNCTION(command, subdirectory, extension)            \
    CBaseAutoCompleteFileList command##Complete(#command, subdirectory, #extension); \
    int command##_CompletionFunc(const char* partial,                                \
        char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])  \
    {                                                                                \
        return command##Complete.AutoCompletionFunc(partial, commands);              \
    }

#define AUTOCOMPLETION_FUNCTION(command) \
    command##_CompletionFunc

#define CON_COMMAND_AUTOCOMPLETEFILE(name, description, flags, subdirectory, extension) \
    DECLARE_AUTOCOMPLETION_FUNCTION(name, subdirectory, extension)                      \
    CON_COMMAND_F_COMPLETION(name, description, flags, AUTOCOMPLETION_FUNCTION(name))

#define DETOUR_COMMAND(name)                        \
    namespace Original {                            \
        _CommandCallback name##_callback;           \
    }                                               \
    namespace Detour {                              \
        void name##_callback(const CCommand& args); \
    }                                               \
    void Detour::name##_callback(const CCommand& args)
#define HOOK_COMMAND(name)                                                   \
    auto c_##name = Command(#name);                                          \
    if (!!c_##name) {                                                        \
        Original::name##_callback = c_##name.ThisPtr()->m_fnCommandCallback; \
        c_##name.ThisPtr()->m_fnCommandCallback = Detour::name##_callback;   \
    }
#define UNHOOK_COMMAND(name)                                                 \
    auto c_##name = Command(#name);                                          \
    if (!!c_##name && Original::name##_callback) {                           \
        c_##name.ThisPtr()->m_fnCommandCallback = Original::name##_callback; \
    }

#define ACTIVATE_AUTOCOMPLETEFILE(name)                                     \
    auto c_##name = Command(#name);                                         \
    if (!!c_##name) {                                                       \
        c_##name.ThisPtr()->m_bHasCompletionCallback = true;                \
        c_##name.ThisPtr()->m_fnCompletionCallback = name##_CompletionFunc; \
    }
#define DEACTIVATE_AUTOCOMPLETEFILE(name)                     \
    auto c_##name = Command(#name);                           \
    if (!!c_##name) {                                         \
        c_##name.ThisPtr()->m_bHasCompletionCallback = false; \
        c_##name.ThisPtr()->m_fnCompletionCallback = nullptr; \
    }
