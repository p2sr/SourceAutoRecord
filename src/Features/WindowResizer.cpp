#include "WindowResizer.hpp"

#include "Variable.hpp"
#include "Event.hpp"
#include "Utils.hpp"
#include "Modules/Engine.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

Variable sar_allow_resizing_window("sar_allow_resizing_window", "0", 0, 1, "EXPERIMENTAL! Forces resizing on game's window.\n");

#ifdef _WIN32
static HWND g_windowHandle = NULL;
#endif

ON_EVENT(FRAME) {
	if (!sar_allow_resizing_window.GetBool()) return;

#ifndef _WIN32
	console->Print("sar_allow_resizing_window is currently working for Windows only.");
	sar_allow_resizing_window.SetValue(0);
#endif

	static int pos_x, pos_y;
	static bool queued_pos_change = false;

	if (queued_pos_change) {
		queued_pos_change = false;
		WindowResizer::SetWindowPos(pos_x, pos_y);
	}

	WindowResizer::EnableResizing();

	static int prev_x, prev_y;
	int x, y;
	if (!WindowResizer::GetScreenSize(x, y)) return;

	if (x != prev_x || y != prev_y) {
		prev_x = x;
		prev_y = y;

		bool fetchedpos = WindowResizer::GetWindowPos(pos_x, pos_y);
		if(fetchedpos)queued_pos_change = true;
		engine->ExecuteCommand(Utils::ssprintf("mat_setvideomode %d %d 1", x, y).c_str(), true);
	}
}

bool WindowResizer::HasValidWindowHandle() {
#ifdef _WIN32
	if (!SendMessage(g_windowHandle, WM_NULL, NULL, NULL)) {
		g_windowHandle = NULL;
	}
	if (g_windowHandle == NULL) {
		g_windowHandle = GetActiveWindow();
	}
	return g_windowHandle != NULL;
#endif
	return false;
}

void WindowResizer::EnableResizing() {
#ifdef _WIN32
	if (!HasValidWindowHandle()) return;
	auto style = GetWindowLong(g_windowHandle, GWL_STYLE);
	if ((style & WS_THICKFRAME) == 0 || (style & WS_MAXIMIZEBOX) == 0) {
		style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		SetWindowLong(g_windowHandle, GWL_STYLE, style);
	}
#endif
}

bool WindowResizer::GetScreenSize(int &x, int &y) {
#ifdef _WIN32
	if (!HasValidWindowHandle()) return false;
	RECT rect;
	if (GetClientRect(g_windowHandle, &rect)) {
		x = rect.right;
		y = rect.bottom;
		return true;
	}
#endif
	return false;
}

bool WindowResizer::GetWindowPos(int &x, int &y) {
#ifdef _WIN32
	if (!HasValidWindowHandle()) return false;
	RECT rect;
	if (GetWindowRect(g_windowHandle, &rect)) {
		x = rect.left;
		y = rect.top;
		return true;
	}
#endif
	return false;
}

bool WindowResizer::SetWindowPos(int x, int y) {
#ifdef _WIN32
	if (!HasValidWindowHandle()) return false;
	return SetWindowPos(g_windowHandle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif
	return false;
}
