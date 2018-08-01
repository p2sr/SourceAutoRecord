#pragma once
#include "Modules/Tier1.hpp"

using namespace Tier1;

struct Command {
private:
    ConCommand* ptr;
    bool isRegistered;
    bool isReference;

    using _ShouldRegisterCallback = bool (*)();
    _ShouldRegisterCallback shouldRegister;

    static std::vector<Command*> list;

public:
    Command()
        : ptr(nullptr)
        , isRegistered(false)
        , isReference(false)
        , shouldRegister(nullptr)
    {
    }
    ~Command()
    {
        if (!isReference) {
            delete ptr;
        }
    }
    Command(const char* name)
    {
        this->ptr = reinterpret_cast<ConCommand*>(FindCommandBase(g_pCVar->GetThisPtr(), name));
        this->isReference = true;
    }
    Command(const char* pName, _CommandCallback callback, const char* pHelpString, int flags = 0,
        _CommandCompletionCallback completionFunc = nullptr)
    {
        this->ptr = new ConCommand();
        this->ptr->m_pszName = pName;
        this->ptr->m_pszHelpString = pHelpString;
        this->ptr->m_nFlags = flags;
        this->ptr->m_fnCommandCallback = callback;
        this->ptr->m_fnCompletionCallback = completionFunc;

        Command::list.push_back(this);
    }
    ConCommand* GetPtr()
    {
        return this->ptr;
    }
    void UniqueFor(_ShouldRegisterCallback callback)
    {
        this->shouldRegister = callback;
    }
    void Register()
    {
        if (!this->isRegistered) {
            this->ptr->ConCommandBase_VTable = Original::ConCommand_VTable;
            RegisterConCommand(g_pCVar->GetThisPtr(), this->ptr);
        }
        this->isRegistered = true;
    }
    void Unregister()
    {
        if (this->isRegistered) {
            UnregisterConCommand(g_pCVar->GetThisPtr(), this->ptr);
        }
        this->isRegistered = false;
    }
    static int RegisterAll()
    {
        auto result = 0;
        for (const auto& command : Command::list) {
            if (command->shouldRegister && !command->shouldRegister()) {
                continue;
            }
            command->Register();
            ++result;
        }
        return result;
    }
    static void UnregisterAll()
    {
        for (const auto& command : Command::list) {
            command->Unregister();
        }
    }
    static Command* Find(const char* name)
    {
        for (const auto& command : Command::list) {
            if (!std::strcmp(command->GetPtr()->m_pszName, name)) {
                return command;
            }
        }
        return nullptr;
    }
};

std::vector<Command*> Command::list;

#define CON_COMMAND(name, description)                               \
    namespace Commands {                                             \
        void name##_callback(const CCommand& args);                  \
        Command name = Command(#name, name##_callback, description); \
    }                                                                \
    void Commands::name##_callback(const CCommand& args)

#define CON_COMMAND_F(name, description, flags)                             \
    namespace Commands {                                                    \
        void name##_callback(const CCommand& args);                         \
        Command name = Command(#name, name##_callback, description, flags); \
    }                                                                       \
    void Commands::name##_callback(const CCommand& args)

#define CON_COMMAND_F_COMPLETION(name, description, flags, completion)                  \
    namespace Commands {                                                                \
        void name##_callback(const CCommand& args);                                     \
        Command name = Command(#name, name##_callback, description, flags, completion); \
    }                                                                                   \
    void Commands::name##_callback(const CCommand& args)

#define DECLARE_AUTOCOMPLETION_FUNCTION(command, subdirectory, extension)                \
    namespace Commands {                                                                 \
        CBaseAutoCompleteFileList command##Complete(#command, subdirectory, #extension); \
        int command##_CompletionFunc(const char* partial,                                \
            char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])  \
        {                                                                                \
            return command##Complete.AutoCompletionFunc(partial, commands);              \
        }                                                                                \
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
#define HOOK_COMMAND(name)                                                  \
    auto c_##name = Command(#name);                                         \
    if (c_##name.GetPtr()) {                                                \
        Original::name##_callback = c_##name.GetPtr()->m_fnCommandCallback; \
        c_##name.GetPtr()->m_fnCommandCallback = Detour::name##_callback;   \
    }
#define UNHOOK_COMMAND(name)                                                \
    auto c_##name = Command(#name);                                         \
    if (c_##name.GetPtr() && Original::name##_callback) {                   \
        c_##name.GetPtr()->m_fnCommandCallback = Original::name##_callback; \
    }

#define ACTIVATE_AUTOCOMPLETEFILE(name)                                              \
    auto c_##name = Command(#name);                                                  \
    if (c_##name.GetPtr()) {                                                         \
        c_##name.GetPtr()->m_bHasCompletionCallback = true;                          \
        c_##name.GetPtr()->m_fnCompletionCallback = Commands::name##_CompletionFunc; \
    }
#define DEACTIVATE_AUTOCOMPLETEFILE(name)                    \
    auto c_##name = Command(#name);                          \
    if (c_##name.GetPtr()) {                                 \
        c_##name.GetPtr()->m_bHasCompletionCallback = false; \
        c_##name.GetPtr()->m_fnCompletionCallback = nullptr; \
    }
