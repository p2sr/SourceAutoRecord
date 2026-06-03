#include "InputSystem.hpp"

#include "Cheats.hpp"
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"
#include "VGui.hpp"
#include "Surface.hpp"

REDECL(InputSystem::SleepUntilInput);
#ifdef _WIN32
REDECL(InputSystem::GetRawMouseAccumulators);
#endif

REDECL(InputSystem::LockCursor);
// REDECL(InputSystem::IN_KeyEvent);

DETOUR_T(void, InputSystem::LockCursor) {
  if (g_drawImgui) {
    surface->matsurface->Original<void(__rescall*)(void*)>(Offsets::UnlockCursor)(surface->matsurface->ThisPtr());

    if (inputSystem->inputContext) {
      inputSystem->SetCursorVisible(
        inputSystem->g_InputStackSystem->ThisPtr(),
        inputSystem->inputContext,
        true
      );
    }
    return;
  }
  return InputSystem::LockCursor(thisptr);
}

ButtonCode_t InputSystem::GetButton(const char *pString) {
	return this->StringToButtonCode(this->g_InputSystem->ThisPtr(), pString);
}

bool InputSystem::IsKeyDown(ButtonCode_t key) {
	return this->IsButtonDown(this->g_InputSystem->ThisPtr(), key);
}

void InputSystem::GetCursorPos(int &x, int &y) {
	return this->GetCursorPosition(this->g_InputSystem->ThisPtr(), x, y);
}

void InputSystem::SetCursorPos(int x, int y) {
	return this->SetCursorPosition(this->g_InputSystem->ThisPtr(), x, y);
}

Variable sar_dpi_scale("sar_dpi_scale", "1", 1, "Fraction to scale mouse DPI down by.\n");
void InputSystem::DPIScaleDeltas(int &x, int &y) {
	static int saved_x = 0;
	static int saved_y = 0;

	static int last_dpi_scale = 1;

	int scale = sar_dpi_scale.GetInt();
	if (scale < 1) scale = 1;

	if (scale != last_dpi_scale) {
		saved_x = 0;
		saved_y = 0;
		last_dpi_scale = scale;
	}

	saved_x += x;
	saved_y += y;

	x = saved_x / scale;
	y = saved_y / scale;

	saved_x %= scale;
	saved_y %= scale;
}

// CInputSystem::SleepUntilInput
DETOUR(InputSystem::SleepUntilInput, int nMaxSleepTimeMS) {
	if (sar_disable_no_focus_sleep.GetBool()) {
		nMaxSleepTimeMS = 0;
	}

	return InputSystem::SleepUntilInput(thisptr, nMaxSleepTimeMS);
}

#ifdef _WIN32
// CInputSystem::GetRawMouseAccumulators
DETOUR_T(void, InputSystem::GetRawMouseAccumulators, int &x, int &y) {
	InputSystem::GetRawMouseAccumulators(thisptr, x, y);
	inputSystem->DPIScaleDeltas(x, y);
}
#endif

bool InputSystem::Init() {
	this->g_InputSystem = Interface::Create(this->Name(), "InputSystemVersion001");
  this->g_InputStackSystem = Interface::Create(this->Name(), "InputStackSystemVersion001");
	if (this->g_InputSystem) {
		this->StringToButtonCode = this->g_InputSystem->Original<_StringToButtonCode>(Offsets::StringToButtonCode);

		this->g_InputSystem->Hook(InputSystem::SleepUntilInput_Hook, InputSystem::SleepUntilInput, Offsets::SleepUntilInput);
#ifdef _WIN32
		this->g_InputSystem->Hook(InputSystem::GetRawMouseAccumulators_Hook, InputSystem::GetRawMouseAccumulators, Offsets::GetRawMouseAccumulators);
#endif
		this->IsButtonDown = this->g_InputSystem->Original<_IsButtonDown>(Offsets::IsButtonDown);
		this->GetCursorPosition = this->g_InputSystem->Original<_GetCursorPosition>(Offsets::GetCursorPosition);
		this->SetCursorPosition = this->g_InputSystem->Original<_SetCursorPosition>(Offsets::SetCursorPosition);
    int offset = 26;
    console->Print("GetInputContext offset: %d\n", offset);
		this->GetInputContext = this->g_InputSystem->Original<_GetInputContext>(offset);
    // for (int i = 20; i < 35; i++) {
    //   // using _GetInputContext = void*(__rescall*)(void*, int);
    //   auto fn = reinterpret_cast<_GetInputContext>(
    //       (*reinterpret_cast<void***>(this->g_InputSystem->ThisPtr()))[i]
    //   );
    //   void* ctx = fn(this->g_InputSystem->ThisPtr(), 0);
    //   console->Print("vtable[%d] GetInputContext = %p\n", i, ctx);
    // }
	}

  if (this->g_InputStackSystem) {
    this->SetCursorVisible = this->g_InputStackSystem->Original<_SetCursorVisible>(Offsets::SetCursorVisible);
  }

  if (this->g_InputSystem) {
    this->inputContext = this->GetInputContext(this->g_InputSystem->ThisPtr(), 0);
    console->Print("GetInputContext: %p\n", this->inputContext);
  }

  // client->g_ClientDLL->Hook(Input)

	auto unbind = Command("unbind");
	if (!!unbind) {
		auto cc_unbind_callback = (uintptr_t)unbind.ThisPtr()->m_pCommandCallback;
		this->KeySetBinding = Memory::Read<_KeySetBinding>(cc_unbind_callback + Offsets::Key_SetBinding);
	}

	return this->hasLoaded = this->g_InputSystem && !!unbind;
}
void InputSystem::Shutdown() {
	Interface::Delete(this->g_InputSystem);
}

InputSystem *inputSystem;
