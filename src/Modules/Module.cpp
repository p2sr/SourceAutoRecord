#include "Module.hpp"

#include "InputSystem.hpp"
#include "Surface.hpp"
#include "VGui.hpp"
#include "Client.hpp"

#include <vector>
#include <cstdint>

#ifndef _WIN32
#include <sys/mman.h>
#endif

static void(*oLockCursor)(void* thisptr) = nullptr;

static void ShowOSCursor(bool show) {
  if (!inputSystem->g_InputStackSystem || !inputSystem->inputContext) {
    console->Print("ShowOSCursor: missing stacsystem=%p ctx=%p\b", inputSystem->g_InputStackSystem, inputSystem->inputContext);
    return;
  }

  using _SetCursorVisible = void(__rescall*)(void*, void*, bool);
  auto fn = reinterpret_cast<_SetCursorVisible>(
    (*reinterpret_cast<void***>(inputSystem->g_InputStackSystem->ThisPtr()))[11]
  );
  console->Print("SetCursorVisible: %p, %d\n", fn, show);
  fn(inputSystem->g_InputStackSystem->ThisPtr(), inputSystem->inputContext, show);

  // console->Print("Show OS Cursor called with value: %d, ", show);
  // if (inputSystem->SetCursorVisible && inputSystem->inputContext) {
  //   console->Print("Valid func and ctx\n");
  //   inputSystem->SetCursorVisible(
  //     inputSystem->g_InputStackSystem->ThisPtr(),
  //     inputSystem->inputContext,
  //     show
  //   );
  //   return;
  // }
  // console->Print("Invalid func and ctx\n");
}

void hkLockCursor(void* thisptr) {
  console->Print("hkLockCursor called!");
  if (g_drawImgui) {
    console->Print(" And drawimgui is on\n");
    auto unlockCursor = (*reinterpret_cast<void***>(thisptr))[66];
    reinterpret_cast<void(*)(void*)>(unlockCursor)(thisptr);
    ShowOSCursor(true);
    return;
  }
  console->Print(" And drawimgui is off\n");
  ShowOSCursor(false);
  oLockCursor(thisptr);
}

static int(*oIN_KeyEvent)(void* thisptr, int eventcode, int keynum, const char* binding) = nullptr;

int hkIN_KeyEvent(void* thisptr, int eventcode, int keynum, const char* binding) {
  if (g_drawImgui) return 0; // 0 = block
  return oIN_KeyEvent(thisptr, eventcode, keynum, binding);
}

static void HookIN_KeyEvent() {
  void** vtable = *reinterpret_cast<void***>(client->g_ClientDLL->ThisPtr());
  oIN_KeyEvent = reinterpret_cast<int(*)(void*, int, int, const char*)>(vtable[20]);
#ifdef _WIN32
  DWORD old;
  VirtualProtect(&vtable[20], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
  vtable[20] = reinterpret_cast<void*>(hkIN_KeyEvent);
  VirtualProtect(&vtable[20], sizeof(void*), old, &old);
#else
  uintptr_t page = (uintptr_t)&vtable[20] & ~(uintptr_t)(4095);
  mprotect((void*)page, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
  vtable[20] = reinterpret_cast<void*>(hkIN_KeyEvent);
  mprotect((void*)page, 4096, PROT_READ | PROT_EXEC);
#endif
}

// static void HookIN_KeyEvent() {
//   // VClient016, vtable index 20
//   auto c = client->g_ClientDLL;
//   if (!c) return;
//
//   void** vtable = *reinterpret_cast<void***>(c->ThisPtr());
//   oIN_KeyEvent = reinterpret_cast<int(*)(void*, int, int, const char*)>(vtable[20]);
//
// #ifdef _WIN32
//   DWORD old;
//   VirtualProtect(&vtable[20], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
//   vtable[20] = reinterpret_cast<void*>(hkIN_KeyEvent);
//   VirtualProtect(&vtable[20], sizeof(void*), old, &old);
// #else
//   uintptr_t page = (uintptr_t)&vtable[20] & ~(uintptr_t)(4095);
//   mprotect((void*)page, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
//   vtable[20] = reinterpret_cast<void*>(hkIN_KeyEvent);
//   mprotect((void*)page, 4096, PROT_READ | PROT_EXEC);
// #endif
// }

static void HookLockCursor() {
  void* surfacePtr = surface->matsurface->ThisPtr();
  void** vtable = *reinterpret_cast<void***>(surfacePtr);

  oLockCursor = reinterpret_cast<void(*)(void*)>(vtable[65]);

#ifdef _WIN32
  DWORD old;
  VirtualProtect(&vtable[65], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
  vtable[65] = reinterpret_cast<void*>(hkLockCursor);
  VirtualProtect(&vtable[65], sizeof(void*), old, &old);
#else
  uintptr_t page = (uintptr_t)&vtable[65] & ~(uintptr_t)(4095);
  mprotect((void*)page, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
  vtable[65] = reinterpret_cast<void*>(hkLockCursor);
  mprotect((void*)page, 4096, PROT_READ | PROT_EXEC);
#endif
}

Modules::Modules()
	: list() {
}
void Modules::InitAll() {
	for (const auto &mod : this->list) {
		if (!mod->hasLoaded) {
			mod->Init();
		}
	}

  
  if (surface && surface->matsurface) {
    HookLockCursor();
  }

  // if (client && client->g_ClientDLL) {
  //   HookIN_KeyEvent();
  // }

  // HookLockCursor();
  // HookIN_KeyEvent();

  // surface->matsurface->Hook(
  //   InputSystem::LockCursor_Hook,
  //   InputSystem::LockCursor,
  //   Offsets::LockCursor
  // );
}
void Modules::ShutdownAll() {
  if (oLockCursor) {
    void* surfacePtr = surface->matsurface->ThisPtr();
    void** vtable = *reinterpret_cast<void***>(surfacePtr);
#ifdef _WIN32
    DWORD old;
    VirtualProtect(&vtable[65], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    vtable[65] = reinterpret_cast<void*>(oLockCursor);
    VirtualProtect(&vtable[65], sizeof(void*), old, &old);
#else
    uintptr_t page = (uintptr_t)&vtable[65] & ~(uintptr_t)(4095);
    mprotect((void*)page, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
    vtable[65] = reinterpret_cast<void*>(oLockCursor);
    mprotect((void*)page, 4096, PROT_READ | PROT_EXEC);
#endif
    oLockCursor = nullptr;
  }
	for (const auto &mod : this->list) {
		mod->Shutdown();
		mod->hasLoaded = false;
	}
}
void Modules::DeleteAll() {
	for (auto &mod : this->list) {
		if (mod) {
			delete mod;
		}
	}
}
Modules::~Modules() {
	this->DeleteAll();
	this->list.clear();
}
