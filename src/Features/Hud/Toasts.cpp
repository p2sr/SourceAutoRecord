#include "Toasts.hpp"

#include <deque>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"

#define TOAST_GAP 10
#define LINE_PAD 6
#define COMPACT_TOAST_PAD 2
#define SIDE_PAD 6
#define COMPACT_SIDE_PAD 3

#define TOAST_BACKGROUND(a) Color{0, 0, 0, 192 * (a) / 255}

#define SLIDE_RATE 200 // thousandths of screen / s
#define FADE_TIME 300 // ms

enum class Alignment
{
    LEFT,
    CENTER,
    RIGHT,
};

enum class Background
{
    NONE,
    TEXT_ONLY,
    FULL,
};

struct Toast
{
    std::string text;
    Color color;
    std::chrono::time_point<std::chrono::steady_clock> created;
    uint32_t duration; // ms
    uint8_t opacity;
};

static std::deque<Toast> g_toasts;

static std::chrono::time_point<std::chrono::steady_clock> g_slideOffTime;
static int g_slideOffStart;
static int g_slideOff;

#define STR(s) #s
#define EXP_STR(s) STR(s)

Variable sar_toast_disable("sar_toast_disable", "0", "Disable all toasts from showing.\n");
Variable sar_toast_font("sar_toast_font", "6", 0, "The font index to use for toasts.\n");
Variable sar_toast_width("sar_toast_width", "250", 2 * SIDE_PAD + 10, "The maximum width for toasts.\n");
Variable sar_toast_x("sar_toast_x", EXP_STR(TOAST_GAP), 0, "The horizontal position of the toasts HUD.\n");
Variable sar_toast_y("sar_toast_y", EXP_STR(TOAST_GAP), 0, "The vertical position of the toasts HUD.\n");
Variable sar_toast_align("sar_toast_align", "0", 0, 2, "The side to align toasts to horizontally. 0 = left, 1 = center, 2 = right.\n");
Variable sar_toast_anchor("sar_toast_anchor", "1", 0, 1, "Where to put new toasts. 0 = bottom, 1 = top.\n");
Variable sar_toast_compact("sar_toast_compact", "0", "Enables a compact form of the toasts HUD.\n");
Variable sar_toast_background("sar_toast_background", "1", 0, 2, "Sets the background highlight for toasts. 0 = no background, 1 = text width only, 2 = full width.\n");

CON_COMMAND(sar_toast_setpos, "sar_toast_setpos <bottom/top> <left/center/right> - set the position of the toasts HUD.\n")
{
    if (args.ArgC() != 3) {
        return console->Print(sar_toast_setpos.ThisPtr()->m_pszHelpString);
    }

    int screenWidth, screenHeight;
#ifdef _WIN32
    engine->GetScreenSize(screenWidth, screenHeight);
#else
    engine->GetScreenSize(nullptr, screenWidth, screenHeight);
#endif

    if (!strcmp(args[1], "bottom")) {
        sar_toast_anchor.SetValue(0);
        sar_toast_y.SetValue(screenHeight - TOAST_GAP);
    } else {
        sar_toast_anchor.SetValue(1);
        sar_toast_y.SetValue(TOAST_GAP);
    }

    if (!strcmp(args[2], "left")) {
        sar_toast_align.SetValue(0);
        sar_toast_x.SetValue(TOAST_GAP);
    } else if (!strcmp(args[2], "center")) {
        sar_toast_align.SetValue(1);
        sar_toast_x.SetValue((screenWidth - sar_toast_width.GetInt()) / 2);
    } else {
        sar_toast_align.SetValue(2);
        sar_toast_x.SetValue(screenWidth - sar_toast_width.GetInt() - TOAST_GAP);
    }
}

ToastHud::ToastHud()
    : Hud(HudType_InGame | HudType_Paused | HudType_Menu, true)
{
}

bool ToastHud::ShouldDraw()
{
    return Hud::ShouldDraw() && !sar_toast_disable.GetBool();
}

bool ToastHud::GetCurrentSize(int &xSize, int &ySize)
{
    return false;
}

static std::vector<std::string> splitIntoLines(Surface::HFont font, std::string text, int maxWidth) {
    int length = text.length();
    const char *str = text.c_str();
    const char *end = str + length;

    std::vector<std::string> lines;

    int32_t lastSpace = -1;

    size_t i = 1;
    while (i < length) {
        if (str[i] == '\n') {
            lines.push_back({str, i + 1});
            str += i + 1;
            length -= i + 1;
            i = 1;
            continue;
        }

        int width = surface->GetFontLength(font, "%.*s", i, str);

        if (width > maxWidth && i > 1) {
            // We have to split onto a new line!
            if (lastSpace != -1) {
                // We've seen a space - split there
                lines.push_back({str, (unsigned)lastSpace});
                str += lastSpace + 1;
                length -= lastSpace + 1;
                i = 1;
                lastSpace = -1;
                continue;
            } else {
                // There's been no space - just split the string here
                lines.push_back({str, i});
                str += i;
                length -= i;
                i = 1;
                continue;
            }
        }

        if (str[i] == ' ') {
            lastSpace = i;
        }

        ++i;
    }

    if (length > 0) {
        lines.push_back({str});
    }

    return lines;
}

void ToastHud::AddToast(std::string text, Color color, double duration, bool doConsole)
{
    auto now = NOW_STEADY();

    g_toasts.push_back({
        text,
        color,
        now,
        (uint32_t)(duration * 1000),
        255,
    });

    color._color[3] = 255;

    Surface::HFont font = scheme->GetDefaultFont() + sar_toast_font.GetInt();

    bool compact = sar_toast_compact.GetBool();
    int linePadding = compact ? 0 : LINE_PAD;
    int gap = compact ? 0 : TOAST_GAP;
    int toastPadding = compact ? COMPACT_TOAST_PAD : 0;
    int sidePadding = compact ? COMPACT_SIDE_PAD : SIDE_PAD;

    int lineHeight = surface->GetFontHeight(font) + linePadding;
    int maxWidth = sar_toast_width.GetInt();

    auto lines = splitIntoLines(font, text, maxWidth - 2 * sidePadding);

    g_slideOffStart = g_slideOff + (lines.size() * lineHeight + linePadding + 2 * toastPadding + gap);
    g_slideOffTime = now;

    if (doConsole) {
        Color conCol = color;
        if (color.r() == 255 && color.g() == 255 && color.b() == 255) {
            conCol.SetColor(255, 150, 50, 255);
        }
        console->ColorMsg(conCol, "%s\n", text.c_str());
    }
}

void ToastHud::Update()
{
    auto now = NOW_STEADY();

    g_toasts.erase(
        std::remove_if(
            g_toasts.begin(),
            g_toasts.end(),
            [=](Toast toast) {
                return now >= toast.created + std::chrono::milliseconds(toast.duration);
            }
        ),
        g_toasts.end()
    );

    for (Toast &toast : g_toasts) {
        uint32_t age = std::chrono::duration_cast<std::chrono::milliseconds>(now - toast.created).count();
        uint32_t op = 255;
        if (age < FADE_TIME) {
            op = 255 * age / FADE_TIME;
        } else if (toast.duration - age < FADE_TIME) {
            op = 255 * (toast.duration - age) / FADE_TIME;
        }
        toast.opacity = op;
        toast.color._color[3] = op;
    }

    int screenWidth, screenHeight;
#ifdef _WIN32
    engine->GetScreenSize(screenWidth, screenHeight);
#else
    engine->GetScreenSize(nullptr, screenWidth, screenHeight);
#endif

    g_slideOff = g_slideOffStart - SLIDE_RATE * std::chrono::duration_cast<std::chrono::milliseconds>(now - g_slideOffTime).count() * screenHeight / 1000 / 1000;
    if (g_slideOff < 0) {
        g_slideOff = 0;
    }
}

void ToastHud::Paint(int slot)
{
    if (slot != 0 && !engine->IsOrange()) {
        // Don't double-draw
        return;
    }

    Update();

    Surface::HFont font = scheme->GetDefaultFont() + sar_toast_font.GetInt();

    int maxWidth = sar_toast_width.GetInt();

    Alignment align = (Alignment)sar_toast_align.GetInt();
    bool againstTop = sar_toast_anchor.GetBool();

    int mainX = sar_toast_x.GetInt();

    int yOffset = sar_toast_y.GetInt() + (againstTop ? -1 : 1) * g_slideOff;

    bool compact = sar_toast_compact.GetBool();
    int linePadding = compact ? 0 : LINE_PAD;
    int gap = compact ? 0 : TOAST_GAP;
    int toastPadding = compact ? COMPACT_TOAST_PAD : 0;
    int sidePadding = compact ? COMPACT_SIDE_PAD : SIDE_PAD;

    int lineHeight = surface->GetFontHeight(font) + linePadding;

    Background bg = (Background)sar_toast_background.GetInt();

    for (auto iter = g_toasts.rbegin(); iter != g_toasts.rend(); ++iter) {
        auto toast = *iter;

        auto lines = splitIntoLines(font, toast.text, maxWidth - 2 * sidePadding);

        if (lines.size() == 0) {
            continue;
        }

        int longestLine = -1;
        for (std::string line : lines) {
            int length = surface->GetFontLength(font, "%s", line.c_str());
            if (length > longestLine) {
                longestLine = length;
            }
        }

        int width = longestLine + 2 * sidePadding;
        int height = lines.size() * lineHeight + linePadding + 2 * toastPadding;

        int xLeft =
            align == Alignment::LEFT ? mainX :
            align == Alignment::RIGHT ? mainX + maxWidth - width :
            mainX + (maxWidth - width) / 2;

        if (!againstTop) {
            yOffset -= height;
        }

        if (bg != Background::NONE) {
            bool full = bg == Background::FULL;
            int rectLeft = full ? mainX : xLeft;
            surface->DrawRect(TOAST_BACKGROUND(toast.opacity), rectLeft, yOffset, rectLeft + (full ? maxWidth : width), yOffset + height);
        }

        yOffset += linePadding + toastPadding;

        for (std::string line : lines) {
            int length = surface->GetFontLength(font, "%s", line.c_str());
            surface->DrawTxt(font, xLeft + sidePadding, yOffset, toast.color, "%s", line.c_str());
            yOffset += lineHeight;
        }

        yOffset += toastPadding;

        if (againstTop) {
            yOffset += gap;
        } else {
            yOffset -= height + gap;
        }
    }
}

CON_COMMAND(sar_toast_create, "sar_toast_create <duration> <r> <g> <b> <text> - create a toast.\n")
{
    if (args.ArgC() != 6) {
        console->Print(sar_toast_create.ThisPtr()->m_pszHelpString);
        return;
    }

    double duration = atof(args[1]);
    int r = atoi(args[2]);
    int g = atoi(args[3]);
    int b = atoi(args[4]);
    std::string text(args[5]);

    toastHud.AddToast(text, Color{r, g, b, 255}, duration);
}

CON_COMMAND(sar_toast_dismiss_all, "sar_toast_dismiss_all - dismiss all active toasts.\n")
{
    g_toasts.clear();
}

ToastHud toastHud;
