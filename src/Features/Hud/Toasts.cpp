#include "Toasts.hpp"

#include <deque>
#include <chrono>
#include <algorithm>
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"

#define SLOTS 2
#define PADDING 6
#define TOAST_GAP 10
#define TOAST_BACKGROUND Color{0, 0, 0, 192}

struct Toast
{
    std::string text;
    Color color;
    std::chrono::time_point<std::chrono::steady_clock> dismissAt;
};

static std::deque<Toast> g_toasts;

Variable sar_toasts_disable("sar_toasts_disable", "0", "Disable all toasts from showing.\n");
Variable sar_toasts_font("sar_toasts_font", "6", 0, "The font index to use for toasts.\n");
Variable sar_toasts_width("sar_toasts_width", "250", 2 * PADDING, "The maximum width for toasts.\n");

ToastHud::ToastHud()
    : Hud(HudType_InGame | HudType_Paused | HudType_Menu, true)
{
}

bool ToastHud::ShouldDraw()
{
    return Hud::ShouldDraw() && !sar_toasts_disable.GetBool();
}

bool ToastHud::GetCurrentSize(int &xSize, int &ySize)
{
    return false;
}

void ToastHud::AddToast(std::string text, Color color, double duration)
{
    g_toasts.push_back({
        text,
        color,
        NOW_STEADY() + std::chrono::microseconds((int64_t)(duration * 1000000)),
    });
}

void ToastHud::Update()
{
    auto now = NOW_STEADY();

    g_toasts.erase(
        std::remove_if(
            g_toasts.begin(),
            g_toasts.end(),
            [=](Toast toast) {
                return now >= toast.dismissAt;
            }
        ),
        g_toasts.end()
    );
}

static std::vector<std::string> splitIntoLines(Surface::HFont font, std::string text, int maxWidth) {
    int length = text.length();
    const char *str = text.c_str();
    const char *end = str + length;

    std::vector<std::string> lines;

    ssize_t lastSpace = -1;

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

void ToastHud::Paint(int slot)
{
    if (slot != 0 && !engine->IsOrange()) {
        // Don't double-draw
        return;
    }

    Update();

    Surface::HFont font = scheme->GetDefaultFont() + sar_toasts_font.GetInt();

    int screenWidth, screenHeight;
#ifdef _WIN32
    engine->GetScreenSize(screenWidth, screenHeight);
#else
    engine->GetScreenSize(nullptr, screenWidth, screenHeight);
#endif

    int maxWidth = sar_toasts_width.GetInt();

    int lineHeight = surface->GetFontHeight(font) + PADDING;

    int yOffset = TOAST_GAP;
    int xRight = screenWidth - TOAST_GAP;

    for (Toast toast : g_toasts) {
        auto lines = splitIntoLines(font, toast.text, maxWidth - 2 * PADDING);

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

        int width = longestLine + 2 * PADDING;
        int height = lines.size() * lineHeight + PADDING;

        int xLeft = xRight - width;

        surface->DrawRect(TOAST_BACKGROUND, xLeft, yOffset, xRight, yOffset + height);

        yOffset += PADDING;

        for (std::string line : lines) {
            int length = surface->GetFontLength(font, "%s", line.c_str());
            surface->DrawTxt(font, xLeft + PADDING, yOffset, toast.color, "%s", line.c_str());
            yOffset += lineHeight;
        }

        yOffset += TOAST_GAP;
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
