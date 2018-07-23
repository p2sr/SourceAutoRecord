#pragma once
#include "Utils.hpp"

#define SAR_PLUGIN_SIGNATURE \
    new char[25] { 65, 114, 101, 32, 121, 111, 117, 32, 104, 97, 112, 112, 121, 32, 110, 111, 119, 44, 32, 74, 97, 109, 101, 114, 63 }

// CServerPlugin
#define CServerPlugin_m_Size 16
#define CServerPlugin_m_Plugins 4

InterfaceReg* InterfaceReg::s_pInterfaceRegs = nullptr;

void* CreateInterfaceInternal(const char* pName, int* pReturnCode)
{
    InterfaceReg* pCur;

    for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext) {
        if (strcmp(pCur->m_pName, pName) == 0) {
            if (pReturnCode) {
                *pReturnCode = 0;
            }
            return pCur->m_CreateFn();
        }
    }

    if (pReturnCode) {
        *pReturnCode = 1;
    }
    return NULL;
}

DLL_EXPORT void* CreateInterface(const char* pName, int* pReturnCode)
{
    return CreateInterfaceInternal(pName, pReturnCode);
}
