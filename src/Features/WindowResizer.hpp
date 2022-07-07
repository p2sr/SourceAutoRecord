#pragma once

namespace WindowResizer {
	bool HasValidWindowHandle();
	void EnableResizing();
	bool GetScreenSize(int &x, int &y);
	bool GetWindowPos(int &x, int &y);
	bool SetWindowPos(int x, int y);
}
