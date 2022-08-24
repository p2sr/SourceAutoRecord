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

		// state
		int pressedTick = -1;
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
