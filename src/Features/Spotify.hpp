#pragma once
#include "Hud/Hud.hpp"
#include "Variable.hpp"

enum class SpotifyAction {
	PlayPause = 917504,
	PreviousTrack = 786432,
	NextTrack = 720896,
};

class Spotify : public Hud {
private:
#ifdef _WIN32
	HWND g_hWnd;
#endif

public:
	Spotify();
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;

	void Init();
	void SendCommand(SpotifyAction command);
};

extern Spotify spotify;

extern Variable sar_spotify_hud;
extern Variable sar_spotify_hud_x;
extern Variable sar_spotify_hud_y;
extern Variable sar_spotify_hud_font_index;