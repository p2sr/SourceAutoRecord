#include "SDK.hpp"

#include <cstring>

#include "Modules/Engine.hpp"

#include "Platform.hpp"

InterfaceReg* InterfaceReg::s_pInterfaceRegs = nullptr;

static void* CreateInterfaceInternal(const char* pName, int* pReturnCode)
{
    InterfaceReg* pCur;

    for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext) {
        if (!std::strcmp(pCur->m_pName, pName)) {
            if (pReturnCode) {
                *pReturnCode = 0;
            }
            return pCur->m_CreateFn();
        }
    }

    if (pReturnCode) {
        *pReturnCode = 1;
    }
    return nullptr;
}

DLL_EXPORT void* CreateInterface(const char* pName, int* pReturnCode)
{
    return CreateInterfaceInternal(pName, pReturnCode);
}

kbutton_t::Split_t& kbutton_t::GetPerUser(int nSlot)
{
    if (nSlot == -1) {
        nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
    }
    return m_PerUser[nSlot];
}
