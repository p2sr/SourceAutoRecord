#include "InputHud.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Modules/InputSystem.hpp"
#include "Utils/SDK.hpp"
#include "Utils/lodepng.hpp"
#include "Variable.hpp"

#include <cstring>
#include <sstream>

Variable sar_ihud("sar_ihud", "0", 0, 1, "Enables or disables movement inputs HUD of client.\n", FCVAR_NEVER_AS_STRING | FCVAR_DONTRECORD);
Variable sar_ihud_x("sar_ihud_x", "2", "X position of input HUD.\n", FCVAR_DONTRECORD);
Variable sar_ihud_y("sar_ihud_y", "2", "Y position of input HUD.\n", FCVAR_DONTRECORD);
Variable sar_ihud_grid_padding("sar_ihud_grid_padding", "2", 0, "Padding between grid squares of input HUD.\n", FCVAR_NEVER_AS_STRING | FCVAR_DONTRECORD);
Variable sar_ihud_grid_size("sar_ihud_grid_size", "60", 0, "Grid square size of input HUD.\n", FCVAR_NEVER_AS_STRING | FCVAR_DONTRECORD);
Variable sar_ihud_analog_image_scale("sar_ihud_analog_image_scale", "0.6", 0, 1, "Scale of analog input images against max extent.\n", FCVAR_NEVER_AS_STRING | FCVAR_DONTRECORD);
Variable sar_ihud_analog_view_deshake("sar_ihud_analog_view_deshake", "0", "Try to eliminate small fluctuations in the movement analog.\n", FCVAR_NEVER_AS_STRING | FCVAR_DONTRECORD);

InputHud inputHud;

InputHud::InputHud()
	: Hud(HudType_InGame | HudType_LoadingScreen, true) {

	elements = {
		{"forward", false, IN_FORWARD},
		{"back", false, IN_BACK},
		{"moveleft", false, IN_MOVELEFT},
		{"moveright", false, IN_MOVERIGHT},
		{"jump", false, IN_JUMP},
		{"duck", false, IN_DUCK},
		{"zoom", false, IN_ZOOM},
		{"use", false, IN_USE},
		{"attack", false, IN_ATTACK},
		{"attack2", false, IN_ATTACK2},
		{"movement", true, 0},
		{"angles", true, 1}
	};

	ApplyPreset("normal", true);
}

void InputHud::SetInputInfo(int slot, int buttonBits, Vector movement) {

	auto &info = this->inputInfo[slot];

	// this was supposed to ensures that the button press is visible for at least one frame
	// idk doesn't seem to make much difference lmfao
	if (info.awaitingFrameDraw) {
		info.buttonBits |= buttonBits;
	} else {
		info.buttonBits = buttonBits;
	}
	
	info.movement = movement;
	info.awaitingFrameDraw = true;
}

bool InputHud::ShouldDraw() {
	return sar_ihud.GetBool() && Hud::ShouldDraw();
}

void InputHud::Paint(int slot) {

	auto &inputInfo = this->inputInfo[slot];

	// update angles array
	if (inputInfo.awaitingFrameDraw) {
		inputInfo.angles[1] = inputInfo.angles[0];
		inputInfo.angles[0] = engine->GetAngles(engine->IsOrange() ? 0 : slot);
		inputInfo.awaitingFrameDraw = false;
	}

	int tick;
	{
		int tmp1, tmp2;
		engine->GetTicks(tick, tmp1, tmp2);
	}

	// do the actual drawing
	auto hudX = PositionFromString(sar_ihud_x.GetString(), true);
	auto hudY = PositionFromString(sar_ihud_y.GetString(), false);

	auto btnSize = sar_ihud_grid_size.GetFloat();
	auto btnPadding = sar_ihud_grid_padding.GetFloat();

	if (this->bgTextureId != -1) {
		surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, 255);
		surface->DrawSetTexture(surface->matsurface->ThisPtr(), this->bgTextureId);
		int x = hudX + this->bgGridX * (btnSize + btnPadding) - btnPadding;
		int y = hudY + this->bgGridY * (btnSize + btnPadding) - btnPadding;
		int w = this->bgGridW * (btnSize + btnPadding) - btnPadding;
		int h = this->bgGridH * (btnSize + btnPadding) - btnPadding;
		if (w < 0) w = 0;
		if (h < 0) h = 0;
		surface->DrawTexturedRect(surface->matsurface->ThisPtr(), x, y, x + w, y + h);
	}

	for (auto &element : elements) {
		if (!element.enabled) continue;

		int eX = hudX + element.x * (btnSize + btnPadding);
		int eY = hudY + element.y * (btnSize + btnPadding);

		int eWidth = element.width * btnSize + std::max(0, element.width - 1) * btnPadding;
		int eHeight = element.height * btnSize + std::max(0, element.height -1) * btnPadding;

		if (element.isVector) {
			//drawing movement and angles vector displays

			int font = scheme->GetFontByID(element.textFont);

			int fontHeight = element.textFont >= 0 ? surface->GetFontHeight(font) : 0;

			// trying some kind of responsiveness here and getting the smallest side
			const int joystickSize = std::min((int)(eHeight - fontHeight * 2.2), eWidth);
			int r = joystickSize / 2;

			int jX = eX + eWidth/2 - r;
			int jY = eY + eHeight/2 - r;

			if (element.imageTextureId == -1) {
				surface->DrawRect(element.background, eX, eY, eX + eWidth, eY + eHeight);

				Color linesColor1 = element.textHighlight;
				Color linesColor2 = linesColor1;
				linesColor2.a /= 2;

				surface->DrawColoredLine(jX, jY, jX + joystickSize, jY, linesColor1);
				surface->DrawColoredLine(jX, jY, jX, jY + joystickSize, linesColor1);
				surface->DrawColoredLine(jX + joystickSize, jY, jX + joystickSize, jY + joystickSize, linesColor1);
				surface->DrawColoredLine(jX, jY + joystickSize, jX + joystickSize, jY + joystickSize, linesColor1);

				//surface->DrawFilledCircle(jX + r, jY + r, r, Color(0,0,0,40));
				surface->DrawCircle(jX + r, jY + r, r, linesColor1);

				surface->DrawColoredLine(jX, jY + r, jX + joystickSize, jY + r, linesColor2);
				surface->DrawColoredLine(jX + r, jY, jX + r, jY + joystickSize, linesColor2);
			}

			Vector v, visV;
			if (element.type == 0) {
				// recalculate movement values into controller inputs
				v = inputInfo.movement;
				v.y /= cl_forwardspeed.GetFloat();
				v.x /= cl_sidespeed.GetFloat();
				visV = v;
				visV.y *= -1;
			} else {
				// calculating the difference between angles in two frames
				v = {
					inputInfo.angles[1].y - inputInfo.angles[0].y,
					inputInfo.angles[1].x - inputInfo.angles[0].x,
				};

				while (v.x < -180.0f) v.x += 360.0f;
				if (v.x > 180.0f) v.x -= 360.0f;

				if (sar_ihud_analog_view_deshake.GetBool()) {
					// Viewangle can fluctuate a tiny bit in normal situations
					// sometimes, which looks super weird. To deal with this, don't
					// record tiny changes on the HUD. As an exception to this, if
					// we're meant to be at 0, always move it back, as a small
					// discrepancy when the camera isn't moving at all is very
					// noticable.
					float dx = fabsf(v.x - inputInfo.prevUsedAngles.x);
					float dy = fabsf(v.y - inputInfo.prevUsedAngles.y);
					if (dx < 0.02 && v.x != 0.0) v.x = inputInfo.prevUsedAngles.x;
					if (dy < 0.02 && v.y != 0.0) v.y = inputInfo.prevUsedAngles.y;
				}

				inputInfo.prevUsedAngles = VectorToQAngle(v);

				// make lower range of inputs easier to notice
				if (v.Length() > 0) {
					visV = v.Normalize() * pow(v.Length() / 180.0f, 0.2);
					visV.y *= -1;
				}
			}

			if (element.imageTextureId == -1) {
				Color pointerColor = element.highlight;
				Vector pointerPoint = {jX + r + r * visV.x, jY + r + r * visV.y};
				surface->DrawColoredLine(jX + r, jY + r, pointerPoint.x, pointerPoint.y, pointerColor);
				surface->DrawFilledCircle(pointerPoint.x, pointerPoint.y, 5, pointerColor);

				Color textColor = element.textColor;
				if (fontHeight > 0) {
					surface->DrawTxt(font, jX, jY + joystickSize + 2, textColor, "x:%.3f", v.x);
					surface->DrawTxt(font, jX + r, jY + joystickSize + 2, textColor, "y:%.3f", v.y);
					surface->DrawTxt(font, jX, jY - fontHeight, textColor, element.text.c_str(), v.x);
				}
			} else {
				// vector drawing with an image
				int size = (float)eWidth * sar_ihud_analog_image_scale.GetFloat();
				int padding = (eWidth-size)/2;
				int x = eX + padding + padding*visV.x;
				int y = eY + padding + padding*visV.y;
				surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, 255);
				surface->DrawSetTexture(surface->matsurface->ThisPtr(), element.imageTextureId);
				surface->DrawTexturedRect(surface->matsurface->ThisPtr(), x, y, x + size, y + size);
			}
		} else {
			// drawing normal buttons
			bool pressed = false;
			if (element.isNormalKey)
				pressed = inputSystem->IsKeyDown((ButtonCode_t)element.type);
			else
				pressed = inputInfo.buttonBits & element.type;

			if (pressed && element.pressedTick == -1) {
				element.pressedTick = tick;
			}

			if (!pressed && tick - element.pressedTick < element.minHold) {
				pressed = true;
			}

			if (!pressed && element.pressedTick != -1) {
				element.pressedTick = -1;
			}

			if (element.imageTextureId == -1) {
				surface->DrawRectAndCenterTxt(
					pressed ? element.highlight : element.background,
					eX, eY, eX + eWidth, eY + eHeight,
					scheme->GetFontByID(element.textFont),
					pressed ? element.textHighlight : element.textColor,
					element.text.c_str()
				);
			} else {
				int tex = pressed && element.highlightImageTextureId > -1 ? element.highlightImageTextureId : element.imageTextureId;
				surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, 255);
				surface->DrawSetTexture(surface->matsurface->ThisPtr(), tex);
				surface->DrawTexturedRect(surface->matsurface->ThisPtr(), eX, eY, eX + eWidth, eY + eHeight);
			}
		}
	}

	// now that we've drawn pressed buttons, we can release them
}

bool InputHud::GetCurrentSize(int &xSize, int &ySize) {
	// getting the size in grid cells
	int gridWidth = 0;
	int gridHeight = 0;
	for (auto &element : elements) {
		if (!element.enabled)continue;
		gridWidth = std::max(gridWidth, element.x + element.width);
		gridHeight = std::max(gridHeight, element.y + element.height);
	}

	if (this->bgTextureId != -1) {
		int bgWidth = this->bgGridX + this->bgGridW;
		int bgHeight = this->bgGridY + this->bgGridH;
		if (bgWidth > gridWidth) gridWidth = bgWidth;
		if (bgHeight > gridHeight) gridHeight = bgHeight;
	}

	//transforming them into actual hud width and height
	auto btnSize = sar_ihud_grid_size.GetFloat();
	auto btnPadding = sar_ihud_grid_padding.GetFloat();

	xSize = gridWidth * btnSize + std::max(0, gridWidth - 1) * btnPadding;
	ySize = gridHeight * btnSize + std::max(0, gridHeight - 1) * btnPadding;

	return true;
}

InputHud::InputHudElement *InputHud::GetElementByName(std::string name) {
	for (size_t i = 0; i < elements.size(); i++) {
		if (elements[i].name.compare(name) == 0) {
			return &elements[i];
		}
	}

	return nullptr;
}

void InputHud::ModifyElementParam(std::string name, std::string parameter, std::string value) {
	// recursively handling the "all" name to apply changes to all of the elements
	if (name.compare("all") == 0) {
		for (auto &element : elements) {
			ModifyElementParam(element.name, parameter, value);
		}
	}

	auto *element = GetElementByName(name);
	if (!element) return;

	int valueInt = std::atoi(value.c_str());
	auto valueColor = Utils::GetColor(value.c_str(), false);

	// changing given parameter
	if (parameter.compare("enabled") == 0) {
		element->enabled = valueInt > 0;
	} else if (parameter.compare("background") == 0) {
		element->background = valueColor.value_or(element->background);
	} else if (parameter.compare("highlight") == 0) {
		element->highlight = valueColor.value_or(element->highlight);
	} else if (parameter.compare("font") == 0) {
		element->textFont = valueInt;
	} else if (parameter.compare("textcolor") == 0) {
		element->textColor = valueColor.value_or(element->textColor);
	} else if (parameter.compare("texthighlight") == 0) {
		element->textHighlight = valueColor.value_or(element->textHighlight);
	} else if (parameter.compare("text") == 0) {
		element->text = value;
	} else if (parameter.compare("x") == 0) {
		element->x = valueInt;
	} else if (parameter.compare("y") == 0) {
		element->y = valueInt;
	} else if (parameter.compare("width") == 0) {
		element->width = valueInt;
	} else if (parameter.compare("height") == 0) {
		element->height = valueInt;
	} else if (parameter.compare("pos") == 0) {
		int x, y, width, height;
		if (sscanf(value.c_str(), "%d %d %d %d", &x, &y, &width, &height) == 4) {
			element->x = x;
			element->y = y;
			element->width = width;
			element->height = height;
		} else if (sscanf(value.c_str(), "%d %d", &x, &y) == 2) {
			element->x = x;
			element->y = y;
		}
	} else if (parameter.compare("image") == 0) {
		element->imageTexture = value;
		if (value == "") {
			element->imageTextureId = -1;
		} else {
			std::vector<uint8_t> buf;
			unsigned w, h;
			unsigned err = lodepng::decode(buf, w, h, value + ".png");
			if (err) {
				console->Warning("%s\n", lodepng_error_text(err));
				element->imageTextureId = -1;
				return;
			}
			if (element->imageTextureId == -1) {
				element->imageTextureId = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);
			}
			surface->DrawSetTextureRGBA(surface->matsurface->ThisPtr(), element->imageTextureId, buf.data(), w, h);
		}
	} else if (parameter.compare("highlightimage") == 0) {
		element->highlightImageTexture = value;
		if (value == "") {
			element->highlightImageTextureId = -1;
		} else {
			std::vector<uint8_t> buf;
			unsigned w, h;
			unsigned err = lodepng::decode(buf, w, h, value + ".png");
			if (err) {
				console->Warning("%s\n", lodepng_error_text(err));
				element->highlightImageTextureId = -1;
				return;
			}
			if (element->highlightImageTextureId == -1) {
				element->highlightImageTextureId = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);
			}
			surface->DrawSetTextureRGBA(surface->matsurface->ThisPtr(), element->highlightImageTextureId, buf.data(), w, h);
		}
	} else if (parameter.compare("minhold") == 0) {
		element->minHold = valueInt;
	}
}

void InputHud::ApplyPreset(const char* preset, bool start) {

#define PARAM(x, y, z) ModifyElementParam(x, y, z)
	if (!strcmp(preset, "normal") || !strcmp(preset, "normal_mouse")) {
		PARAM("all", "background", "0 0 0 200");
		PARAM("all", "textcolor", "255 255 255 255");
		PARAM("all", "highlight", "255 255 255 255");
		PARAM("all", "texthighlight", "255 255 255 255");
		PARAM("all", "font", "1");
		PARAM("all", "enabled", "1");
		PARAM("all", "image", "");
		PARAM("all", "highlightimage", "");
		PARAM("all", "minhold", "0");

		PARAM("forward", "text", "W");
		PARAM("back", "text", "S");
		PARAM("moveleft", "text", "A");
		PARAM("moveright", "text", "D");
		PARAM("jump", "text", "Jump");
		PARAM("use", "text", "+use");
		PARAM("duck", "text", "Duck");
		PARAM("attack", "text", "LMB");
		PARAM("attack2", "text", "RMB");

		PARAM("forward", "pos", "2 0 1 1");
		PARAM("back", "pos", "2 1 1 1");
		PARAM("moveleft", "pos", "1 1 1 1");
		PARAM("moveright", "pos", "3 1 1 1");
		PARAM("jump", "pos", "1 2 3 1");
		PARAM("use", "pos", "3 0 1 1");
		PARAM("duck", "pos", "0 2 1 1");
		PARAM("attack", "pos", "4 2 1 1");
		PARAM("attack2", "pos", "5 2 1 1");

		PARAM("movement", "enabled", "0");
		PARAM("angles", "enabled", "0");
		PARAM("zoom", "enabled", "0");

		this->bgTextureId = -1;

		if (!start) sar_ihud_grid_size.SetValue(60);

		if (!strcmp(preset, "normal_mouse")) {
			PARAM("forward", "pos", "1 0 1 1");
			PARAM("back", "pos", "1 1 1 1");
			PARAM("moveleft", "pos", "0 1 1 1");
			PARAM("moveright", "pos", "2 1 1 1");
			PARAM("use", "pos", "2 0 1 1");
			PARAM("jump", "pos", "1 2 2 1");
			PARAM("attack", "pos", "4 0 1 1");
			PARAM("attack2", "pos", "5 0 1 1");

			PARAM("angles", "enabled", "1");
			PARAM("angles", "pos", "4 1 2 2");
			PARAM("angles", "font", "-5");
			PARAM("angles", "texthighlight", "255 255 255 20");
		}
	} else if (!strcmp(preset, "tas")) {
		PARAM("all", "background", "0 0 0 200");
		PARAM("all", "textcolor", "255 255 255 255");
		PARAM("all", "highlight", "255 255 255 255");
		PARAM("all", "texthighlight", "255 255 255 255");
		PARAM("all", "font", "1");
		PARAM("all", "enabled", "1");
		PARAM("all", "image", "");
		PARAM("all", "highlightimage", "");
		PARAM("all", "minhold", "0");

		PARAM("forward", "enabled", "0");
		PARAM("back", "enabled", "0");
		PARAM("moveleft", "enabled", "0");
		PARAM("moveright", "enabled", "0");
		PARAM("zoom", "enabled", "0");

		PARAM("jump", "text", "Jump");
		PARAM("use", "text", "Use");
		PARAM("duck", "text", "Duck");
		PARAM("attack", "text", "Blue");
		PARAM("attack2", "text", "Orange");

		PARAM("movement", "pos", "0 0 5 5");
		PARAM("angles", "pos", "5 0 5 5");
		PARAM("duck", "pos", "0 5 2 1");
		PARAM("use", "pos", "2 5 2 1");
		PARAM("jump", "pos", "4 5 2 1");
		PARAM("attack", "pos", "6 5 2 1");
		PARAM("attack2", "pos", "8 5 2 1");

		PARAM("movement", "highlight", "255 150 0 255");
		PARAM("angles", "highlight", "0 150 255 255");
		PARAM("movement", "texthighlight", "255 200 100 100");
		PARAM("angles", "texthighlight", "100 200 255 100");
		PARAM("movement", "font", "13");
		PARAM("angles", "font", "13");
		PARAM("movement", "text", "move analog");
		PARAM("angles", "text", "view analog");

		this->bgTextureId = -1;

		if (!start) sar_ihud_grid_size.SetValue(40);
	} else {
		console->Print("Unknown input hud preset %s!\n", preset);
	}

#undef PARAM
}

bool InputHud::HasElement(const char* elementName) {
	bool elementExists = false;
	for (auto &element : elements) {
		if (element.name.compare(elementName) == 0) {
			elementExists = true;
			break;
		}
	}
	return elementExists;
}

bool InputHud::IsValidParameter(const char* param) {
	const char *validParams[] = {"enabled", "text", "font", "pos", "x", "y", "width", "height", "font", "background", "highlight", "textcolor", "texthighlight", "image", "highlightimage", "minhold"};
	
	for (const char *validParam : validParams) {
		if (!strcmp(validParam, param)) {
			return true;
		}
	}
	return false;
}

std::string InputHud::GetParameterValue(std::string name, std::string parameter) {
	auto *element = GetElementByName(name);
	if (!element) return "";

	auto colorToString = [](Color color) -> std::string {
		return Utils::ssprintf("#%02X%02X%02X%02X", color.r, color.g, color.b, color.a);
	};

	if (parameter.compare("enabled") == 0) {
		return element->enabled ? "1" : "0";
	} else if (parameter.compare("background") == 0) {
		return colorToString(element->background);
	} else if (parameter.compare("highlight") == 0) {
		return colorToString(element->highlight);
	} else if (parameter.compare("font") == 0) {
		return std::to_string(element->textFont);
	} else if (parameter.compare("textcolor") == 0) {
		return colorToString(element->textColor);
	} else if (parameter.compare("texthighlight") == 0) {
		return colorToString(element->textHighlight);
	} else if (parameter.compare("text") == 0) {
		return element->text;
	} else if (parameter.compare("x") == 0) {
		return std::to_string(element->x);
	} else if (parameter.compare("y") == 0) {
		return std::to_string(element->y);
	} else if (parameter.compare("width") == 0) {
		return std::to_string(element->width);
	} else if (parameter.compare("height") == 0) {
		return std::to_string(element->height);
	} else if (parameter.compare("pos") == 0) {
		return Utils::ssprintf("x: %u y: %u width: %u height: %u", element->x, element->y, element->width, element->height);
	} else if (parameter.compare("image") == 0) {
		return element->imageTexture;
	} else if (parameter.compare("highlightimage") == 0) {
		return element->highlightImageTexture;
	} else if (parameter.compare("minhold") == 0) {
		return std::to_string(element->minHold);
	} else {
		return "";
	}
}

void InputHud::AddElement(std::string name, int type) {
	char first = name.at(0);
	if (first >= 97)
		first -= 32;  // Convert to uppercase

	elements.push_back({
		name, false, type, true,				// name, isVector, type, isNormalKey
		true,									// enabled
		0, 0, 1, 1,								// x, y, width, height
		Color(0, 0, 0, 200),                    // background
		Color(255, 255, 255, 255),              // highlight
		std::string(1, first) + name.substr(1), // text
		1,                                      // font
		Color(255, 255, 255, 255),              // text color
		Color(255, 255, 255, 255),              // text highlight
	});
}

DECL_AUTO_COMMAND_COMPLETION(sar_ihud_preset, ({"normal", "normal_mouse", "tas"}))
CON_COMMAND_F_COMPLETION(sar_ihud_preset, "sar_ihud_preset <preset> - modifies input hud based on given preset\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(sar_ihud_preset)) {
	if (args.ArgC() != 2) {
		console->Print(sar_ihud_preset.ThisPtr()->m_pszHelpString);
		return;
	}

	const char *preset = args.Arg(1);
	inputHud.ApplyPreset(preset, false);
}

DECL_COMMAND_COMPLETION(sar_ihud_modify) {
	while (isspace(*match)) ++match;

	if (std::string(match).find(" ") != std::string::npos) {
		// we've probably started another arg; don't offer any completions
		return 0;
	}

	if (std::strstr("all", match)) items.push_back("all");

	for (auto &element : inputHud.elements) {
		if (items.size() == COMMAND_COMPLETION_MAXITEMS)
			break;

		if (std::strstr(element.name.c_str(), match)) {
			items.push_back(element.name);
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(sar_ihud_modify,
	"sar_ihud_modify <element|all> [param=value]... - modifies parameters in given element.\n"
    "Params: enabled, text, pos, x, y, width, height, font, background, highlight, textcolor, texthighlight, image, highlightimage, minhold.\n",
	FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(sar_ihud_modify)
) {
	if (args.ArgC() < 3) {
		console->Print(sar_ihud_modify.ThisPtr()->m_pszHelpString);
		return;
	}

	// checking if element exists
	const char *elementName = args[1];
	if (!inputHud.HasElement(elementName) && strcmp(elementName, "all")) {
		console->Print("Input HUD element %s doesn't exist.\n", elementName);
		//console->Print(sar_ihud_modify.ThisPtr()->m_pszHelpString);
		return;
	}

	std::vector<std::string> parameterOutput;

	auto addToParameterOutput = [&](std::string elementName, std::string parameter) {
		for (std::string &string : parameterOutput) {
			if (!Utils::StartsWith(string.c_str(), (elementName + ":").c_str())) continue;
			string += " " + parameter;
			return;
		}

		parameterOutput.push_back(std::string(elementName) + ": " + parameter);
	};

	// looping through every parameter
	for (int i = 2; i < args.ArgC(); i++) {
		std::string fullArg = args[i];
		auto separator = fullArg.find('=');
		if (separator == std::string::npos) {
			if (!inputHud.IsValidParameter(fullArg.c_str())) continue;

			// recursively handling the "all" name to print out the parameter for all elements
			if (std::string(elementName).compare("all") == 0) {
				for (auto &element : inputHud.elements) {
					addToParameterOutput(element.name, fullArg + "=" + inputHud.GetParameterValue(element.name, fullArg.c_str()));
				}
				continue;
			}

			// Print the value of the parameter
			auto parameterValue = inputHud.GetParameterValue(std::string(elementName), fullArg.c_str());
			if (parameterValue.empty()) continue;
			addToParameterOutput(elementName, fullArg + "=" + parameterValue);
			continue;
		}

		std::string param = fullArg.substr(0, separator);
		std::string value = fullArg.substr(separator + 1);

		if (inputHud.IsValidParameter(param.c_str())) {
			inputHud.ModifyElementParam(elementName, param, value);
		} else {
			console->Print("Unknown input HUD parameter %s.\n", param.c_str());
		}
	}

	if (!parameterOutput.empty()) {
		for (std::string &string : parameterOutput) {
			console->Print("%s\n", string.c_str());
		}
	}
}

CON_COMMAND_F(sar_ihud_add_key, "sar_ihud_add_key <key>\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 2) {
		console->Print(sar_ihud_add_key.ThisPtr()->m_pszHelpString);
		return;
	}

	if (inputHud.HasElement(args[1])) {
		console->Print("Input HUD already has this key.\n");
		return;
	}

	ButtonCode_t keyCode = inputSystem->GetButton(args[1]);
	if (keyCode == -1) {
		console->Print("Key %s does not exist.\n", args[1]);
		return;
	}

	inputHud.AddElement(args[1], keyCode);
}

CON_COMMAND_HUD_SETPOS(sar_ihud, "input HUD")

CON_COMMAND_F(sar_ihud_set_background, "sar_ihud_set_background <path> <grid x> <grid y> <grid w> <grid h>\n", FCVAR_DONTRECORD) {
	if (args.ArgC() != 6) {
		console->Print(sar_ihud_set_background.ThisPtr()->m_pszHelpString);
		return;
	}

	std::vector<uint8_t> buf;
	unsigned w, h;
	unsigned err = lodepng::decode(buf, w, h, std::string(args[1]) + ".png");
	if (err) {
		console->Warning("%s\n", lodepng_error_text(err));
		inputHud.bgTextureId = -1;
		return;
	}

	inputHud.bgTextureId = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);
	surface->DrawSetTextureRGBA(surface->matsurface->ThisPtr(), inputHud.bgTextureId, buf.data(), w, h);

	inputHud.bgGridX = atoi(args[2]);
	inputHud.bgGridY = atoi(args[3]);
	inputHud.bgGridW = atoi(args[4]);
	inputHud.bgGridH = atoi(args[5]);
}

CON_COMMAND_F(sar_ihud_clear_background, "sar_ihud_clear_background\n", FCVAR_DONTRECORD) {
	inputHud.bgTextureId = -1;
}
