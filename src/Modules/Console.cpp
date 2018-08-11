#include "Console.hpp"

bool Console::Init()
{
    auto tier0 = Memory::GetModuleHandleByName(MODULE(TIER0));
    if (tier0) {
        this->Msg = Memory::GetSymbolAddress<_Msg>(tier0, MSG_SYMBOL);
        this->ColorMsg = Memory::GetSymbolAddress<_ColorMsg>(tier0, CONCOLORMSG_SYMBOL);
        this->Warning = Memory::GetSymbolAddress<_Warning>(tier0, WARNING_SYMBOL);
        this->DevMsg = Memory::GetSymbolAddress<_DevMsg>(tier0, DEVMSG_SYMBOL);
        this->DevWarning = Memory::GetSymbolAddress<_DevWarning>(tier0, DEVWARNINGMSG_SYMBOL);

        Memory::CloseModuleHandle(tier0);
    }

    return this->hasLoaded = tier0
        && this->Msg
        && this->ColorMsg
        && this->Warning
        && this->DevMsg
        && this->DevWarning;
}
void Console::Shutdown()
{
}

Console* console;
