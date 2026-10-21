#pragma once
#include "Hud.hpp"
#include "Variable.hpp"
#include <vector>
#include <string>

class InputHud : public Hud {
private:
	// info about current user input
	struct {
		int buttonBits = 0;
		Vector movement{0, 0};
		QAngle angles[2]{0, 0};  //storing two angles to calculate delta
		QAngle prevUsedAngles{0, 0};
		bool awaitingFrameDraw = true;
	} inputInfo[2];

	// input hud elements (buttons and vector displays)
	struct InputHudElement {
		//functional
		std::string name;
		bool isVector;
		int type;
		bool isNormalKey = false;

		//visual
		bool enabled;
		int x;
		int y;
		int width;
		int height;
		Color background;
		Color highlight;
		std::string text;
		int textFont;
		Color textColor;
		Color textHighlight;
		std::string imageTexture;
		int imageTextureId = -1;
		std::string highlightImageTexture;
		int highlightImageTextureId = -1;
		int minHold = 0;
		int msHoldCount = 0;
		// 0 = second line
		// 1 = top left, 2 = top right
		// 3 = btm left, 4 = btm right
		int countAnchor = 0;

		// state
		int pressedTick[2] = {-1, -1};
		int lastPressTick[2] = {-1, -1};
		bool lastDown[2] = {false, false};
		int pressCount[2] = {0, 0};
	};

	InputHudElement *GetElementByName(std::string name);

public:
	InputHud();
	void SetInputInfo(int slot, int buttonBits, Vector movement);
	bool ShouldDraw() override;
	void Paint(int slot) override;
	bool GetCurrentSize(int &xSize, int &ySize) override;

	void ModifyElementParam(std::string name, std::string parameter, std::string value);
	void ApplyPreset(const char *preset, bool start);
	bool HasElement(const char *elementName);
	bool IsValidParameter(const char *parameter);
	std::string GetParameterValue(std::string name, std::string parameter);

	int GetButtonBits(int slot) { return inputInfo[slot].buttonBits; }

	void AddElement(std::string name, int type);

	std::vector<InputHudElement> elements;
	int bgTextureId = -1;
	int bgGridX;
	int bgGridY;
	int bgGridW;
	int bgGridH;
};

extern InputHud inputHud;

extern Variable sar_ihud;
extern Variable sar_ihud_x;
extern Variable sar_ihud_y;
extern Variable sar_ihud_grid_padding;
extern Variable sar_ihud_grid_size;
