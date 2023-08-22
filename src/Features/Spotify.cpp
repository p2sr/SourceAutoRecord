#include "Spotify.hpp"

#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Console.hpp"

#include "Variable.hpp"
#include "Command.hpp"

#include <TlHelp32.h>

Variable sar_spotify_hud("sar_spotify_hud", "0", 0, "Draw the current playing song on spotify.\n");
Variable sar_spotify_hud_x("sar_spotify_hud_x", "2", 0, "X offset of the HUD.\n");
Variable sar_spotify_hud_y("sar_spotify_hud_y", "2", 0, "Y offset of the HUD.\n");
Variable sar_spotify_hud_font_index("sar_spotify_hud_font_index", "0", 0, "Font index of the HUD.\n");

Spotify spotify;

Spotify::Spotify()
	: Hud(HudType_InGame | HudType_Paused | HudType_Menu, false) {
}
bool Spotify::ShouldDraw() {
	return sar_spotify_hud.GetBool() && Hud::ShouldDraw();
}

void getHandleFromProcessPath(const char *szExeName, DWORD &dwPID) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (_stricmp(entry.szExeFile, szExeName) == 0) {
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				dwPID = entry.th32ProcessID;

				CloseHandle(hProcess);

				break;
			}
		}
	}

	CloseHandle(snapshot);
}

struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};

BOOL isMainWindow(HWND handle) {
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enumWindowsCallback(HWND handle, LPARAM lParam) {
	handle_data &data = *(handle_data *)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !isMainWindow(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND findMainWindow(unsigned long process_id) {
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enumWindowsCallback, (LPARAM)&data);
	return data.window_handle;
}

void Spotify::Init() {
	DWORD pid;
	getHandleFromProcessPath("Spotify.exe", pid);

	g_hWnd = findMainWindow(pid);
}

void Spotify::SendCommand(SpotifyAction command) {
	SendMessageA(g_hWnd, WM_APPCOMMAND, NULL, (LPARAM)command);
}

void Spotify::Paint(int slot) {
#ifndef _WIN32
	console->Print("sar_spotify_hud is currently working for Windows only.\n");
	sar_spotify_hud.SetValue(0);
	return;
#endif
	if (!g_hWnd || !IsWindow(g_hWnd)) {
		Init();
		if (!g_hWnd) {
			console->Print("Spotify is not open, open spotify and set sar_spotify_hud to 1 again.\n");
			sar_spotify_hud.SetValue(0);
		}
	}

	char title[256];
	GetWindowTextA(g_hWnd, title, sizeof(title));

	// if there is - most likely song is playing
	if (!strstr(title, "-"))
		return;

	auto font = scheme->GetFontByID(sar_spotify_hud_font_index.GetInt());
	surface->DrawTxt(font, sar_spotify_hud_x.GetInt(), sar_spotify_hud_y.GetInt(), {255, 255, 255}, "%s", title);
}

bool Spotify::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}

CON_COMMAND(sar_spotify_next, "Skips to the next song.\n") {
	spotify.SendCommand(SpotifyAction::NextTrack);
}
CON_COMMAND(sar_spotify_prev, "Skips to the previous song.\n") {
	spotify.SendCommand(SpotifyAction::PreviousTrack);
}
CON_COMMAND(sar_spotify_togglepause, "Plays/pauses the playback.\n") {
	spotify.SendCommand(SpotifyAction::PlayPause);
}