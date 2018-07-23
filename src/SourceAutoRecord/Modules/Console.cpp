#include "Console.hpp"

bool Console::Init()
{
    auto tier0 = Memory::GetModuleHandleByName(MODULE(TIER0));
    if (tier0) {
        auto msgAddr = Memory::GetSymbolAddress(tier0, "Msg");
        auto colorMsgAddr = Memory::GetSymbolAddress(tier0, CONCOLORMSG_SYMBOL);
        auto warningAddr = Memory::GetSymbolAddress(tier0, "Warning");
        auto devMsgAddr = Memory::GetSymbolAddress(tier0, DEVMSG_SYMBOL);
        auto devWarningAddr = Memory::GetSymbolAddress(tier0, DEVWARNINGMSG_SYMBOL);

        Memory::CloseModuleHandle(tier0);

        if (msgAddr && colorMsgAddr && warningAddr && devMsgAddr && devWarningAddr) {
            this->Msg = reinterpret_cast<_Msg>(msgAddr);
            this->ColorMsg = reinterpret_cast<_ColorMsg>(colorMsgAddr);
            this->Warning = reinterpret_cast<_Warning>(warningAddr);
            this->DevMsg = reinterpret_cast<_DevMsg>(devMsgAddr);
            this->DevWarning = reinterpret_cast<_DevWarning>(devWarningAddr);
            return true;
        }
    }
    return false;
}

Console* console = new Console();
