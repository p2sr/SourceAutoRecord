#include "Command.hpp"

#include "Modules/Tier1.hpp"

Command::Command()
    : ptr(nullptr)
    , isRegistered(false)
    , isReference(false)
    , shouldRegister(nullptr)
{
}
Command::~Command()
{
    if (!isReference) {
        delete ptr;
    }
}
Command::Command(const char* name)
{
    this->ptr = reinterpret_cast<ConCommand*>(Tier1::FindCommandBase(Tier1::g_pCVar->ThisPtr(), name));
    this->isReference = true;
}
Command::Command(const char* pName, _CommandCallback callback, const char* pHelpString, int flags,
    _CommandCompletionCallback completionFunc)
{
    this->ptr = new ConCommand();
    this->ptr->m_pszName = pName;
    this->ptr->m_pszHelpString = pHelpString;
    this->ptr->m_nFlags = flags;
    this->ptr->m_fnCommandCallback = callback;
    this->ptr->m_fnCompletionCallback = completionFunc;
    this->ptr->m_bHasCompletionCallback = completionFunc != nullptr;
    this->ptr->m_bUsingNewCommandCallback = true;

    Command::list.push_back(this);
}
ConCommand* Command::ThisPtr()
{
    return this->ptr;
}
void Command::UniqueFor(_ShouldRegisterCallback callback)
{
    this->shouldRegister = callback;
}
void Command::Register()
{
    if (!this->isRegistered) {
        this->ptr->ConCommandBase_VTable = Tier1::ConCommand_VTable;
        Tier1::RegisterConCommand(Tier1::g_pCVar->ThisPtr(), this->ptr);
    }
    this->isRegistered = true;
}
void Command::Unregister()
{
    if (this->isRegistered) {
        Tier1::UnregisterConCommand(Tier1::g_pCVar->ThisPtr(), this->ptr);
    }
    this->isRegistered = false;
}
bool Command::operator!()
{
    return this->ptr == nullptr;
}
int Command::RegisterAll()
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
void Command::UnregisterAll()
{
    for (const auto& command : Command::list) {
        command->Unregister();
    }
}
Command* Command::Find(const char* name)
{
    for (const auto& command : Command::list) {
        if (!std::strcmp(command->ThisPtr()->m_pszName, name)) {
            return command;
        }
    }
    return nullptr;
}

std::vector<Command*> Command::list;
