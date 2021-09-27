#pragma once
#include "Game.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <array>
#include <vector>

enum HudType {
	HudType_NotSpecified = 0,
	HudType_InGame = (1 << 0),
	HudType_Paused = (1 << 1),
	HudType_Menu = (1 << 2),
	HudType_LoadingScreen = (1 << 3)
};

class BaseHud {
public:
	int type;
	bool drawSecondSplitScreen;
	int version;

public:
	BaseHud(int type, bool drawSecondSplitScreen, int version);
	virtual bool ShouldDraw();
};

class Hud : public BaseHud {
public:
	static std::vector<Hud *> &GetList();

public:
	Hud(int type, bool drawSecondSplitScreen = false, int version = SourceGame_Unknown);

public:
	virtual bool GetCurrentSize(int &xSize, int &ySize) = 0;
	virtual void Paint(int slot) = 0;

	float PositionFromString(const char *str, bool isX);
};

class HudContext {
public:
	int font = 0;
	int xPadding = 0;
	int yPadding = 0;
	int fontSize = 0;
	int spacing = 0;
	Color textColor = Color(255, 255, 255);
	int elements = 0;
	int maxWidth = 0;
	std::array<int, 256> group{0};

public:
	int slot = 0;

public:
	void DrawElement(const char *fmt, ...);
	void DrawElementOnScreen(const int nbElement, const float xPos, const float yPos, const char *fmt, ...);
	void Reset(int slot);
};

using _PaintCallback = void (*)(HudContext *ctx);
using _PaintCallbackMode = void (*)(HudContext *ctx, int mode);
using _PaintCallbackString = void (*)(HudContext *ctx, const char *text);

class HudElement : public BaseHud {
public:
	int orderIndex;

	union {
		_PaintCallback callbackDefault;
		_PaintCallbackMode callbackMode;
		_PaintCallbackString callbackString;
	};

protected:
	Variable *variable;
	const char *name;

public:
	static std::vector<HudElement *> &GetList();
	static void IndexAll();

public:
	HudElement(Variable *variable, int type, bool drawSecondSplitScreen, int version);
	HudElement(Variable *variable, _PaintCallback callback, int type, bool drawSecondSplitScreen = false, int version = SourceGame_Unknown);
	HudElement(const char *name, _PaintCallback callback, int type, bool drawSecondSplitScreen = false, int version = SourceGame_Unknown);
	bool ShouldDraw() override { return (!this->variable || this->variable->GetBool()) && BaseHud::ShouldDraw(); }
	virtual void Paint(HudContext *ctx) { this->callbackDefault(ctx); }
	const char *ElementName() const { return this->variable ? this->variable->ThisPtr()->m_pszName : this->name; }
};

class HudModeElement : public HudElement {
public:
	HudModeElement(Variable *variable, _PaintCallbackMode callback, int type, bool drawSecondSplitScreen = false, int version = SourceGame_Unknown);
	bool ShouldDraw() override { return this->variable->GetBool() && BaseHud::ShouldDraw(); }
	void Paint(HudContext *ctx) { this->callbackMode(ctx, this->variable->GetInt()); }
};

class HudStringElement : public HudElement {
public:
	HudStringElement(Variable *variable, _PaintCallbackString callback, int type, bool drawSecondSplitScreen = false, int version = SourceGame_Unknown);
	bool ShouldDraw() override { return this->variable->GetString()[0] != '\0' && BaseHud::ShouldDraw(); }
	void Paint(HudContext *ctx) { this->callbackString(ctx, this->variable->GetString()); }
};

// First screen
#define HUD_ELEMENT_NO_DISABLE(name, type)                                                     \
	void sar_hud_element_##name##_callback(HudContext *ctx);                                      \
	HudElement sar_hud_element_##name("sar_hud_" #name, sar_hud_element_##name##_callback, type); \
	void sar_hud_element_##name##_callback(HudContext *ctx)
#define HUD_ELEMENT(name, value, desc, type)                                                  \
	Variable sar_hud_##name("sar_hud_" #name, value, desc);                                      \
	void sar_hud_element_##name##_callback(HudContext *ctx);                                     \
	HudElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type); \
	void sar_hud_element_##name##_callback(HudContext *ctx)
#define HUD_ELEMENT_STRING(name, value, desc, type)                                                 \
	Variable sar_hud_##name("sar_hud_" #name, value, desc, 0);                                         \
	void sar_hud_element_##name##_callback(HudContext *ctx, const char *text);                         \
	HudStringElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type); \
	void sar_hud_element_##name##_callback(HudContext *ctx, const char *text)
#define HUD_ELEMENT_MODE(name, value, min, max, desc, type)                                       \
	Variable sar_hud_##name("sar_hud_" #name, value, min, max, desc);                                \
	void sar_hud_element_##name##_callback(HudContext *ctx, int mode);                               \
	HudModeElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type); \
	void sar_hud_element_##name##_callback(HudContext *ctx, int mode)
// First+second screen
#define HUD_ELEMENT2_NO_DISABLE(name, type)                                                          \
	void sar_hud_element_##name##_callback(HudContext *ctx);                                            \
	HudElement sar_hud_element_##name("sar_hud_" #name, sar_hud_element_##name##_callback, type, true); \
	void sar_hud_element_##name##_callback(HudContext *ctx)
#define HUD_ELEMENT2(name, value, desc, type)                                                       \
	Variable sar_hud_##name("sar_hud_" #name, value, desc);                                            \
	void sar_hud_element_##name##_callback(HudContext *ctx);                                           \
	HudElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type, true); \
	void sar_hud_element_##name##_callback(HudContext *ctx)
#define HUD_ELEMENT_STRING2(name, value, desc, type)                                                      \
	Variable sar_hud_##name("sar_hud_" #name, value, desc, 0);                                               \
	void sar_hud_element_##name##_callback(HudContext *ctx, const char *text);                               \
	HudStringElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type, true); \
	void sar_hud_element_##name##_callback(HudContext *ctx, const char *text)
#define HUD_ELEMENT_MODE2(name, value, min, max, desc, type)                                            \
	Variable sar_hud_##name("sar_hud_" #name, value, min, max, desc);                                      \
	void sar_hud_element_##name##_callback(HudContext *ctx, int mode);                                     \
	HudModeElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type, true); \
	void sar_hud_element_##name##_callback(HudContext *ctx, int mode)
// Specify game version
#define HUD_ELEMENT3(name, value, desc, type, showOnSecondScreen, version)                                                 \
	Variable sar_hud_##name("sar_hud_" #name, value, desc);                                                                   \
	void sar_hud_element_##name##_callback(HudContext *ctx);                                                                  \
	HudElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type, showOnSecondScreen, version); \
	void sar_hud_element_##name##_callback(HudContext *ctx)
#define HUD_ELEMENT_STRING3(name, value, desc, type, showOnSecondScreen, version)                                                \
	Variable sar_hud_##name("sar_hud_" #name, value, desc, 0);                                                                      \
	void sar_hud_element_##name##_callback(HudContext *ctx, const char *text);                                                      \
	HudStringElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type, showOnSecondScreen, version); \
	void sar_hud_element_##name##_callback(HudContext *ctx, const char *text)
#define HUD_ELEMENT_MODE3(name, value, min, max, desc, type, showOnSecondScreen, version)                                      \
	Variable sar_hud_##name("sar_hud_" #name, value, min, max, desc);                                                             \
	void sar_hud_element_##name##_callback(HudContext *ctx, int mode);                                                            \
	HudModeElement sar_hud_element_##name(&sar_hud_##name, sar_hud_element_##name##_callback, type, showOnSecondScreen, version); \
	void sar_hud_element_##name##_callback(HudContext *ctx, int mode)

extern Variable sar_hud_spacing;
extern Variable sar_hud_x;
extern Variable sar_hud_y;
extern Variable sar_hud_font_index;
extern Variable sar_hud_font_color;
extern Variable sar_hud_precision;
extern Variable sar_hud_text;
void sar_hud_text_callback(void *, const char *, float);

int HudSetPos_CompleteFunc(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

#define CON_COMMAND_HUD_SETPOS(name, helpname) \
    CON_COMMAND_F_COMPLETION( \
        name##_setpos,                                                                              \
        "Automatically sets the position of " helpname ".\n"                                          \
        "Usage: " #name "_setpos <top|center|bottom|y|y\%> <left|center|right|x|x\%>\n",              \
        0, HudSetPos_CompleteFunc                                                                     \
    ) {                                                                                               \
        if (args.ArgC() != 3) return console->Print(name##_setpos.ThisPtr()->m_pszHelpString);      \
        name##_x.SetValue(args[2]);                                                                 \
        name##_y.SetValue(args[1]);                                                                 \
	}
