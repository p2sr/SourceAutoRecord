#pragma once
#include "Modules/Tier1.hpp"

using namespace Tier1;

struct Command {
private:
    ConCommand* ptr = nullptr;
    std::unique_ptr<uint8_t[]> data;
    bool isRegistered = false;
    
    using _ShouldRegisterCallback = bool(*)();
    _ShouldRegisterCallback shouldRegister = NULL;

    static std::vector<Command*> list;

public:
    Command(const char* name)
    {
        this->ptr = reinterpret_cast<ConCommand*>(FindCommandBase(g_pCVar->GetThisPtr(), name));
    }
    Command(const char* pName, _CommandCallback callback, const char* pHelpString, int flags = 0, _CommandCompletionCallback completionFunc = nullptr)
    {
        auto size = sizeof(ConCommand);
        data = std::make_unique<uint8_t[]>(size);
        this->ptr = reinterpret_cast<ConCommand*>(data.get());
        std::memset(this->ptr, 0, size);

        // Store data temporarily so we can "register" it later in the engine
        this->ptr->m_pszName = pName;
        this->ptr->m_pCommandCallback = callback;
        this->ptr->m_pszHelpString = pHelpString;
        this->ptr->m_nFlags = flags;
        this->ptr->m_pCommandCompletionCallback = completionFunc;

        Command::list.push_back(this);
    }
    ConCommand* GetPtr()
    {
        return this->ptr;
    }
    Command* UniqueFor(_ShouldRegisterCallback callback)
    {
        shouldRegister = callback;
        return this;
    }
    void Register()
    {
        ConCommandCtor(this->ptr,
            this->ptr->m_pszName,
            this->ptr->m_pCommandCallback,
            this->ptr->m_pszHelpString,
            this->ptr->m_nFlags,
            (int*)this->ptr->m_pCommandCompletionCallback);
        isRegistered = true;
    }
    void Unregister()
    {
        if (isRegistered)
            UnregisterConCommand(g_pCVar->GetThisPtr(), this->ptr);
    }
    static int RegisterAll()
    {
        auto result = 0;
        for (auto command : Command::list) {
            if (command->shouldRegister && !command->shouldRegister()) {
                continue;
            }
            command->Register();
            result++;
        }
        return result;
    }
    static void UnregisterAll()
    {
        for (auto command : Command::list) {
            command->Unregister();
        }
    }
    static Command* Find(const char* name)
    {
        for (auto command : Command::list) {
            if (std::strcmp(command->GetPtr()->m_pszName, name) == 0) {
                return command;
            }
        }
        return nullptr;
    }
};

std::vector<Command*> Command::list;

#define CON_COMMAND(name, description) \
    namespace Commands { \
        void name##_callback(const CCommand& args); \
        Command name = Command(#name, name##_callback, description); \
    } \
    void Commands::name##_callback(const CCommand& args)

#define CON_COMMAND_F(name, description, flags) \
    namespace Commands { \
        void name##_callback(const CCommand& args); \
        Command name = Command(#name, name##_callback, description, flags); \
    } \
    void Commands::name##_callback(const CCommand& args)

#define CON_COMMAND_F_COMPLETION(name, description, flags, completion) \
    namespace Commands { \
        void name##_callback(const CCommand& args); \
        Command name = Command(#name, name##_callback, description, flags, completion); \
    } \
    void Commands::name##_callback(const CCommand& args)

#define DECLARE_AUTOCOMPLETION_FUNCTION(command, subdirectory, extension) \
    namespace Commands { \
        CBaseAutoCompleteFileList command##Complete(#command, subdirectory, #extension); \
        int command##_CompletionFunc(const char *partial, \
            char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) { \
            return command##Complete.AutoCompletionFunc(partial, commands); \
        } \
    }

#define AUTOCOMPLETION_FUNCTION(command) \
    command##_CompletionFunc

#define CON_COMMAND_AUTOCOMPLETEFILE(name, description, flags, subdirectory, extension) \
    DECLARE_AUTOCOMPLETION_FUNCTION(name, subdirectory, extension) \
    CON_COMMAND_F_COMPLETION(name, description, flags, AUTOCOMPLETION_FUNCTION(name))

#define DETOUR_COMMAND(name) \
    namespace Original { _CommandCallback name##_callback; } \
    namespace Detour { void name##_callback(const CCommand& args); } \
    void Detour::##name##_callback(const CCommand& args)
#define HOOK_COMMAND(name) \
    auto command = Command(#name); \
    if (command.GetPtr()) { \
        Original::##name##_callback = command.GetPtr()->m_fnCommandCallback; \
        command.GetPtr()->m_fnCommandCallback =  Detour::##name##_callback; \
    }
#define UNHOOK_COMMAND(name) \
    auto command = Command(#name); \
    if (command.GetPtr() && Original::##name##_callback) { \
        command.GetPtr()->m_fnCommandCallback =  Original::##name##_callback; \
    }

#define ACTIVATE_AUTOCOMPLETEFILE(name) \
    auto name = Command(#name); \
    if (name##.GetPtr()) { \
        name##.GetPtr()->m_bHasCompletionCallback = true; \
        name##.GetPtr()->m_fnCompletionCallback = Commands::##name##_CompletionFunc; \
    }