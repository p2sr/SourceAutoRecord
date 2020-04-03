#include "Hud.hpp"

#include <algorithm>
#include <cstdio>

#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Modules/VGui.hpp"

#include "Variable.hpp"

Variable sar_hud_default_spacing("sar_hud_default_spacing", "1", 0, "Spacing between elements of HUD.\n");
Variable sar_hud_default_padding_x("sar_hud_default_padding_x", "2", 0, "X padding of HUD.\n");
Variable sar_hud_default_padding_y("sar_hud_default_padding_y", "2", 0, "Y padding of HUD.\n");
Variable sar_hud_default_font_index("sar_hud_default_font_index", "0", 0, "Font index of HUD.\n");
Variable sar_hud_default_font_color("sar_hud_default_font_color", "255 255 255 255", "RGBA font color of HUD.\n", 0);

BaseHud::BaseHud(int type, bool drawSecondSplitScreen, int version)
    : type(type)
    , drawSecondSplitScreen(drawSecondSplitScreen)
    , version(version)
{
}
bool BaseHud::ShouldDraw()
{
    return ((this->type & HudType_InGame) && session->isRunning)
        || ((this->type & HudType_Paused) && pauseTimer->IsActive())
        || ((this->type & HudType_Menu) && engine->m_szLevelName[0] == '\0')
        || ((this->type & HudType_LoadingScreen) && !session->isRunning);
}

std::vector<Hud*>& Hud::GetList()
{
    static std::vector<Hud*> list;
    return list;
}

Hud::Hud(int type, bool drawSecondSplitScreen, int version)
    : BaseHud(type, drawSecondSplitScreen, version)
{
    Hud::GetList().push_back(this);
}
Color Hud::GetColor(const char* source)
{
    int r, g, b, a;
    std::sscanf(source, "%i%i%i%i", &r, &g, &b, &a);
    return Color(r, g, b, a);
}

void HudContext::DrawElement(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[128];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);

    surface->DrawTxt(font,
        this->xPadding,
        this->yPadding + this->elements * (this->fontSize + this->spacing),
        this->textColor,
        data);

    ++this->elements;
}
void HudContext::Reset(int slot)
{
    this->slot = slot;

    this->elements = 0;
    this->xPadding = sar_hud_default_padding_x.GetInt();
    this->yPadding = sar_hud_default_padding_y.GetInt();
    this->spacing = sar_hud_default_spacing.GetInt();

    this->font = scheme->GetDefaultFont() + sar_hud_default_font_index.GetInt();
    this->fontSize = surface->GetFontHeight(font);

    int r, g, b, a;
    sscanf(sar_hud_default_font_color.GetString(), "%i%i%i%i", &r, &g, &b, &a);
    this->textColor = Color(r, g, b, a);
}

std::vector<HudElement*>& HudElement::GetList()
{
    static std::vector<HudElement*> list;
    return list;
}

HudElement::HudElement(Variable* variable, int type, bool drawSecondSplitScreen, int version)
    : BaseHud(type, drawSecondSplitScreen, version)
    , orderIndex(-1)
    , variable(variable)
{
    HudElement::GetList().push_back(this);
}
HudElement::HudElement(Variable* variable, _PaintCallback callback, int type, bool drawSecondSplitScreen, int version)
    : HudElement(variable, type, drawSecondSplitScreen, version)
{
    this->callbackDefault = callback;
}

HudModeElement::HudModeElement(Variable* variable, _PaintCallbackMode callback, int type, bool drawSecondSplitScreen, int version)
    : HudElement(variable, type, drawSecondSplitScreen, version)
{
    this->callbackMode = callback;
}

HudStringElement::HudStringElement(Variable* variable, _PaintCallbackString callback, int type, bool drawSecondSplitScreen, int version)
    : HudElement(variable, type, drawSecondSplitScreen, version)
{
    this->callbackString = callback;
}

// Default order
std::vector<std::string> elementOrder = {
    "text",
    "position",
    "angles",
    "velocity",
    "session",
    "last_session",
    "sum",
    "timer",
    "avg",
    "cps",
    "pause_timer",
    "demo",
    "jumps",
    "portals",
    "steps",
    "jump",
    "jump_peak",
    "velocity_peak",
    "trace",
    "frame",
    "last_frame",
    "inspection",
    "velocity_angle",
    "acceleration",
    "player_info"
};

void HudElement::IndexAll()
{
    auto elements = HudElement::GetList();
    auto index = 0;

    for (const auto& name : elementOrder) {
        auto element = std::find_if(elements.begin(), elements.end(), [name](HudElement* element) {
            return Utils::ICompare(element->variable->ThisPtr()->m_pszName + 8, name);
        });

        if (element != elements.end()) {
            (*element)->orderIndex = index;
        } else {
            console->Warning("HUD name %s is not in element list!\n", name.c_str());
        }

        ++index;
    }
}

// Commands

CON_COMMAND_COMPLETION(sar_hud_default_order_top, "Orders hud element to top. Usage: sar_hud_default_order_top <name>\n", (elementOrder))
{
    if (args.ArgC() != 2) {
        return console->Print("Orders hud element to top : sar_hud_default_order_top <name>\n");
    }

    auto elements = &vgui->elements;

    auto name = std::string("sar_hud_") + std::string(args[1]);

    auto result = std::find_if(elements->begin(), elements->end(), [name](const HudElement* a) {
        if (Utils::ICompare(a->variable->ThisPtr()->m_pszName, name)) {
            return true;
        }
        return false;
    });

    if (result == elements->end()) {
        return console->Print("Unknown HUD element name!\n");
    }

    auto element = *result;
    elements->erase(result);
    elements->insert(elements->begin(), element);

    console->Print("Moved HUD element %s to top.\n", args[1]);
}
CON_COMMAND_COMPLETION(sar_hud_default_order_bottom, "Orders hud element to bottom : sar_hud_default_order_bottom <name>\n", (elementOrder))
{
    if (args.ArgC() != 2) {
        return console->Print("Set!\n");
    }

    auto elements = &vgui->elements;

    auto name = std::string("sar_hud_") + std::string(args[1]);

    auto result = std::find_if(elements->begin(), elements->end(), [name](const HudElement* a) {
        if (Utils::ICompare(a->variable->ThisPtr()->m_pszName, name)) {
            return true;
        }
        return false;
    });

    if (result == elements->end()) {
        return console->Print("Unknown HUD element name!\n");
    }

    auto element = *result;
    elements->erase(result);
    elements->insert(elements->end(), element);

    console->Print("Moved HUD element %s to top.\n", args[1]);
}
CON_COMMAND(sar_hud_default_order_reset, "Resets order of hud element.\n")
{
    std::sort(vgui->elements.begin(), vgui->elements.end(), [](const HudElement* a, const HudElement* b) {
        return a->orderIndex < b->orderIndex;
    });
    console->Print("Reset default HUD element order!\n");
}

// HUD

HUD_ELEMENT_STRING(text, "", "Draws specified text when not empty.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    ctx->DrawElement("%s", text);
}
HUD_ELEMENT_MODE2(position, "0", 0, 2, "Draws absolute position of the client.\n"
                                       "0 = Default,\n"
                                       "1 = Player position,\n"
                                       "2 = Camera position.\n",
    HudType_InGame | HudType_Paused | HudType_LoadingScreen)
{
    auto player = client->GetPlayer(ctx->slot + 1);
    if (player) {
        auto pos = client->GetAbsOrigin(player);
        if (mode >= 2) {
            pos = pos + client->GetViewOffset(player);
        }
        ctx->DrawElement("pos: %.3f %.3f %.3f", pos.x, pos.y, pos.z);
    } else {
        ctx->DrawElement("pos: -");
    }
}
HUD_ELEMENT_MODE2(angles, "0", 0, 2, "Draws absolute view angles of the client.\n"
                                     "0 = Default,\n"
                                     "1 = XY,\n"
                                     "2 = XYZ.\n",
    HudType_InGame | HudType_Paused | HudType_LoadingScreen)
{
    auto ang = engine->GetAngles(ctx->slot);
    if (mode == 1) {
        ctx->DrawElement("ang: %.3f %.3f", ang.x, ang.y);
    } else {
        ctx->DrawElement("ang: %.3f %.3f %.3f", ang.x, ang.y, ang.z);
    }
}
HUD_ELEMENT_MODE2(velocity, "0", 0, 3, "Draws velocity of the client.\n"
                                       "0 = Default,\n"
                                       "1 = X/Y/Z,\n"
                                       "2 = X/Y,\n"
                                       "3 = X : Y : Z.\n",
    HudType_InGame | HudType_Paused | HudType_LoadingScreen)
{
    auto player = client->GetPlayer(ctx->slot + 1);
    if (player) {
        if (mode >= 3) {
            auto vel = client->GetLocalVelocity(player);
            ctx->DrawElement("vel: x : %.3f y : %.3f z : %.3f", vel.x, vel.y, vel.z);
        } else {
            auto vel = (mode == 1)
                ? client->GetLocalVelocity(player).Length()
                : client->GetLocalVelocity(player).Length2D();
            ctx->DrawElement("vel: %.3f", vel);
        }
    } else {
        ctx->DrawElement("vel: -");
    }
}
