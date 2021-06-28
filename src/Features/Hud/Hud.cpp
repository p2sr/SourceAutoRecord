#include "Hud.hpp"

#include <algorithm>
#include <cstdio>
#include <map>
#include <optional>

#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Demo/GhostEntity.hpp"
#include "Modules/EngineDemoPlayer.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Modules/VGui.hpp"

#include "Variable.hpp"

Variable sar_hud_default_spacing("sar_hud_default_spacing", "1", 0, "Spacing between elements of HUD.\n");
Variable sar_hud_default_padding_x("sar_hud_default_padding_x", "2", 0, "X padding of HUD.\n");
Variable sar_hud_default_padding_y("sar_hud_default_padding_y", "2", 0, "Y padding of HUD.\n");
Variable sar_hud_default_font_index("sar_hud_default_font_index", "0", 0, "Font index of HUD.\n");
Variable sar_hud_default_font_color("sar_hud_default_font_color", "255 255 255 255", "RGBA font color of HUD.\n", 0);

Variable sar_hud_precision("sar_hud_precision", "3", 0, "Precision of HUD numbers.\n");
Variable sar_hud_velocity_precision("sar_hud_velocity_precision", "2", 0, "Precision of velocity HUD numbers.\n");

static inline int getPrecision(bool velocity = false)
{
    int p = velocity ? sar_hud_velocity_precision.GetInt() : sar_hud_precision.GetInt();
    if (p < 0) p = 0;
    if (!sv_cheats.GetBool()) {
        const int max = velocity ? 2 : 6;
        if (p > max) p = max;
    }
    return p;
}

BaseHud::BaseHud(int type, bool drawSecondSplitScreen, int version)
    : type(type)
    , drawSecondSplitScreen(drawSecondSplitScreen)
    , version(version)
{
}
bool BaseHud::ShouldDraw()
{
    if (engine->demoplayer->IsPlaying() || engine->IsOrange()) {
        return this->type & HudType_InGame;
    }

    if (!engine->hoststate->m_activeGame) {
        return this->type & HudType_Menu;
    }

    if (pauseTimer->IsActive()) {
        return this->type & HudType_Paused;
    }

    if (session->isRunning) {
        return this->type & HudType_InGame;
    }

    return this->type & HudType_LoadingScreen;
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

    int width = surface->GetFontLength(this->font, "%s", data);
    if (width > this->maxWidth) this->maxWidth = width;
}
void HudContext::DrawElementOnScreen(const int groupID, const float xPos, const float yPos, const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char data[128];
    vsnprintf(data, sizeof(data), fmt, argptr);
    va_end(argptr);

    int pixLength = surface->GetFontLength(this->font, "%s", data);

    surface->DrawTxt(font,
        xPos - pixLength / 2,
        yPos + this->group[groupID] * (this->fontSize + this->spacing),
        this->textColor,
        data);


    ++this->group[groupID];
}

void HudContext::Reset(int slot)
{
    this->slot = slot;

    this->elements = 0;
    this->group.fill(0);
    this->xPadding = sar_hud_default_padding_x.GetInt();
    this->yPadding = sar_hud_default_padding_y.GetInt();
    this->spacing = sar_hud_default_spacing.GetInt();
    this->maxWidth = 0;

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
HudElement::HudElement(const char* name, _PaintCallback callback, int type, bool drawSecondSplitScreen, int version)
    : HudElement(nullptr, type, drawSecondSplitScreen, version)
{
    this->name = name;
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
            return Utils::ICompare(element->ElementName() + 8, name);
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

CON_COMMAND_COMPLETION(sar_hud_default_order_top, "sar_hud_default_order_top <name> - orders hud element to top\n", (elementOrder))
{
    if (args.ArgC() != 2) {
        return console->Print("Orders hud element to top: sar_hud_default_order_top <name>\n");
    }

    auto elements = &vgui->elements;

    auto name = std::string("sar_hud_") + std::string(args[1]);

    auto result = std::find_if(elements->begin(), elements->end(), [name](const HudElement* a) {
        if (Utils::ICompare(a->ElementName(), name)) {
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
CON_COMMAND_COMPLETION(sar_hud_default_order_bottom, "sar_hud_default_order_bottom <name> - orders hud element to bottom\n", (elementOrder))
{
    if (args.ArgC() != 2) {
        return console->Print("Set!\n");
    }

    auto elements = &vgui->elements;

    auto name = std::string("sar_hud_") + std::string(args[1]);

    auto result = std::find_if(elements->begin(), elements->end(), [name](const HudElement* a) {
        if (Utils::ICompare(a->ElementName(), name)) {
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

    console->Print("Moved HUD element %s to bottom.\n", args[1]);
}
CON_COMMAND(sar_hud_default_order_reset, "sar_hud_default_order_reset - resets order of hud element\n")
{
    std::sort(vgui->elements.begin(), vgui->elements.end(), [](const HudElement* a, const HudElement* b) {
        return a->orderIndex < b->orderIndex;
    });
    console->Print("Reset default HUD element order!\n");
}

// HUD

struct TextComponent
{
    std::optional<Color> color;
    std::string text;
};

struct TextLine
{
    bool draw;
    Color defaultColor;
    std::vector<TextComponent> components;
};

static std::map<long, TextLine> sar_hud_text_vals;
HUD_ELEMENT2_NO_DISABLE(text, HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen)
{
    for (auto& t : sar_hud_text_vals) {
        int x = ctx->xPadding;
        int y = ctx->yPadding + ctx->elements * (ctx->fontSize + ctx->spacing);
        if (t.second.draw) {
            for (auto &c : t.second.components) {
                Color color = c.color ? *c.color : t.second.defaultColor;
                int pixLen = surface->GetFontLength(ctx->font, "%s", c.text.c_str());
                surface->DrawTxt(ctx->font, x, y, color, c.text.c_str());
                x += pixLen;
            }

            ++ctx->elements;

            int width = x - ctx->xPadding;
            if (width > ctx->maxWidth) ctx->maxWidth = width;
        }
    }
}

Variable sar_hud_text("sar_hud_text", "", "DEPRECATED: Use sar_hud_set_text.", 0);
void sar_hud_text_callback(void* var, const char* pOldVal, float fOldVal)
{
    console->Print("WARNING: sar_hud_text is deprecated. Please use sar_hud_set_text instead.\n");
    sar_hud_text_vals[0].draw = sar_hud_text.GetString()[0];
    sar_hud_text_vals[0].components = { { { { 255, 255, 255, 255 } }, { sar_hud_text.GetString() } } };
}

long parseIdx(const char* idxStr)
{
    char* end;
    long idx = std::strtol(idxStr, &end, 10);
    if (*end != 0 || end == idxStr) {
        return -1;
    }
    return idx;
}

CON_COMMAND(sar_hud_set_text, "sar_hud_set_text <id> <text>... - sets and shows the nth text value in the HUD\n") {
    if (args.ArgC() < 3) {
        console->Print(sar_hud_set_text.ThisPtr()->m_pszHelpString);
        return;
    }

    long idx = parseIdx(args[1]);
    if (idx == -1) {
        console->Print(sar_hud_set_text.ThisPtr()->m_pszHelpString);
        return;
    }

    if (sar_hud_text_vals.find(idx) == sar_hud_text_vals.end()) {
        sar_hud_text_vals[idx].defaultColor = Color{ 255, 255, 255, 255 };
    }

    const char *txt;

    if (args.ArgC() == 3) {
        txt = args[2];
    } else {
        txt = args.m_pArgSBuffer + args.m_nArgv0Size;

        while (isspace(*txt)) ++txt;

        if (*txt == '"') {
            txt += strlen(args[1]) + 2;
        } else {
            txt += strlen(args[1]);
        }

        while (isspace(*txt)) ++txt;
    }

    std::optional<Color> curColor;
    std::string component = "";

    std::vector<TextComponent> components;

    while (*txt) {
        if (*txt == '#') {
            ++txt;
            if (*txt == '#') {
                component += '#';
                ++txt;
                continue;
            } else {
                int r, g, b;
                int end = -1;
                if (sscanf(txt, "%2x%2x%2x%n", &r, &g, &b, &end) == 3 && end == 6) {
                    txt += 6;
                    if (!curColor || curColor->r() != r || curColor->g() != g || curColor->b() != b) {
                        components.push_back({ curColor, component });
                        curColor = Color{ r, g, b, 255 };
                        component = "";
                    }
                    continue;
                }
                component += '#';
                continue;
            }
        }
        component += *txt;
        ++txt;
    }

    components.push_back({ curColor, component });

    sar_hud_text_vals[idx].draw = true;
    sar_hud_text_vals[idx].components = components;
}

CON_COMMAND(sar_hud_set_text_color, "sar_hud_set_text_color <id> <color> - sets the color of the nth text value in the HUD\n")
{
    if (args.ArgC() != 3 && args.ArgC() != 5) {
        console->Print(sar_hud_set_text_color.ThisPtr()->m_pszHelpString);
        return;
    }

    long idx = parseIdx(args[1]);
    if (idx == -1) {
        console->Print(sar_hud_set_text_color.ThisPtr()->m_pszHelpString);
        return;
    }

    int r, g, b;

    if (args.ArgC() == 3) {
        const char *col = args[2];
        if (col[0] == '#') {
            ++col;
        }

        int end = -1;
        if (sscanf(col, "%2x%2x%2x%n", &r, &g, &b, &end) != 3 || end != 6) {
            return console->Print("Invalid color code '%s'\n", args[2]);
        }
    } else {
        char *end;

        r = strtol(args[2], &end, 10);
        if (*end || end == args[2]) {
            return console->Print("Invalid color component '%s'\n", args[2]);
        }

        g = strtol(args[3], &end, 10);
        if (*end || end == args[3]) {
            return console->Print("Invalid color component '%s'\n", args[3]);
        }

        b = strtol(args[4], &end, 10);
        if (*end || end == args[4]) {
            return console->Print("Invalid color component '%s'\n", args[4]);
        }
    }

    sar_hud_text_vals[idx].defaultColor = Color{ r, g, b, 255 };
}

CON_COMMAND(sar_hud_hide_text, "sar_hud_hide_text <id> - hides the nth text value in the HUD\n")
{
    if (args.ArgC() < 2) {
        console->Print(sar_hud_hide_text.ThisPtr()->m_pszHelpString);
        return;
    }

    long idx = parseIdx(args[1]);
    if (idx == -1) {
        console->Print(sar_hud_hide_text.ThisPtr()->m_pszHelpString);
        return;
    }

    sar_hud_text_vals[idx].draw = false;
}

CON_COMMAND(sar_hud_show_text, "sar_hud_show_text <id> - shows the nth text value in the HUD\n")
{
    if (args.ArgC() < 2) {
        console->Print(sar_hud_show_text.ThisPtr()->m_pszHelpString);
        return;
    }

    long idx = parseIdx(args[1]);
    if (idx == -1) {
        console->Print(sar_hud_hide_text.ThisPtr()->m_pszHelpString);
        return;
    }

    sar_hud_text_vals[idx].draw = true;
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
        int p = getPrecision();
        ctx->DrawElement("pos: %.*f %.*f %.*f", p, pos.x, p, pos.y, p, pos.z);
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
    // When we're orange (and not splitscreen), for some fucking reason,
    // the *engine* thinks we're slot 0, but everything else thinks
    // we're slot 1
    auto ang = engine->GetAngles(engine->IsOrange() ? 0 : ctx->slot);
    int p = getPrecision();
    if (mode == 1) {
        ctx->DrawElement("ang: %.*f %.*f", p, ang.x, p, ang.y);
    } else {
        ctx->DrawElement("ang: %.*f %.*f %.*f", p, ang.x, p, ang.y, p, ang.z);
    }
}
HUD_ELEMENT_MODE2(velocity, "0", 0, 4, "Draws velocity of the client.\n"
                                       "0 = Default,\n"
                                       "1 = X, Y, Z\n"
                                       "2 = X:Y\n"
                                       "3 = X:Y, Z\n"
                                       "4 = X:Y:Z\n",
    HudType_InGame | HudType_Paused | HudType_LoadingScreen)
{
    auto player = client->GetPlayer(ctx->slot + 1);
    if (player) {
        int p = getPrecision(true);
        auto vel = client->GetLocalVelocity(player);
        switch (mode) {
        case 1:
            ctx->DrawElement("vel: %.*f %.*f %.*f", p, vel.x, p, vel.y, p, vel.z);
            break;
        case 2:
            ctx->DrawElement("vel: %.*f", p, vel.Length2D());
            break;
        case 3:
            ctx->DrawElement("vel: %.*f %.*f", p, vel.Length2D(), p, vel.z);
            break;
        case 4:
            ctx->DrawElement("vel: %.*f", p, vel.Length());
            break;
        }
    } else {
        ctx->DrawElement("vel: -");
    }
}
