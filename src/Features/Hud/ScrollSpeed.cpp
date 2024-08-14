#include "ScrollSpeed.hpp"

#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"
#include "Modules/Scheme.hpp"

#define CONSECUTIVE_END 8
#define TOTAL_THRESHOLD 5
#define MAX_CONSECUTIVE_SCROLL_INPUTS 12
#define MAX_INPUT_LINES 12
#define AVERAGE_INPUT_AMOUNT 12

#define INPUT_SQUARE_HEIGHT 20
#define INPUT_SQUARE_WIDTH 40
#define INPUT_SQUARE_X_SPACING 2
#define INPUT_SQUARE_Y_SPACING 2

#define PERFECT_INPUT_COLOR Color{0, 112, 255, 255}
#define MISS_INPUT_COLOR Color{165, 28, 28, 255}
#define OK_INPUT_COLOR Color{208, 208, 0, 255}
#define BAD_INPUT_COLOR Color{255, 144, 0, 255}
#define BACKGROUND_COLOR Color(70, 100, 150, 70)
#define AVERAGE_LINE_COLOR Color(200, 200, 200, 255)
#define JUMP_HIGHLIGHT_COLOR Color(100, 200, 255, 255)
#define GROUNDFRAMES_TEXT_COLOR Color(0, 0, 0, 255)

#define BAR_CPS_BEGIN 10
#define BAR_CPS_END 45

Variable sar_scrollspeed("sar_scrollspeed", "0", "Show a HUD indicating your scroll speed for coop.\n");
Variable sar_scrollspeed_x("sar_scrollspeed_x", "0", "Scroll speed HUD x offset\n");
Variable sar_scrollspeed_y("sar_scrollspeed_y", "210", "Scroll speed HUD y offset\n");
Variable sar_scrollspeed_bar_x("sar_scrollspeed_bar_x", "30", "Scroll speed bar x offset\n");
Variable sar_scrollspeed_bar_y("sar_scrollspeed_bar_y", "210", "Scroll speed bar y offset\n");

int g_jumpTicks[2][MAX_INPUT_LINES][MAX_CONSECUTIVE_SCROLL_INPUTS];
bool g_jumpedTicks[2][MAX_INPUT_LINES][MAX_CONSECUTIVE_SCROLL_INPUTS];

float lastAverage[2];
int averageQueue[2][AVERAGE_INPUT_AMOUNT];

int lastGroundedTick;
int jumpCounter[2];
bool lastLineFilled[2];
int currentLine[2];

int averageSum[2];
int averageLastIndex[2];
int averageSize[2];

bool insideSession;
bool isEnabled;

void clear() {
	for (int slot = 0; slot < 2; slot++) {
		for (int inputLine = 0; inputLine < MAX_INPUT_LINES; inputLine++) {
			for (int jumpInput = 0; jumpInput < MAX_CONSECUTIVE_SCROLL_INPUTS; jumpInput++) {
				g_jumpTicks[slot][inputLine][jumpInput] = 0;
				g_jumpedTicks[slot][inputLine][jumpInput] = false;
			}
		}
		for (int inputQueueindex = 0; inputQueueindex < AVERAGE_INPUT_AMOUNT; inputQueueindex++) {
			averageQueue[slot][inputQueueindex] = 0;
		}
		jumpCounter[slot] = 1;
		lastLineFilled[slot] = false;
		currentLine[slot] = 0;
		lastAverage[slot] = 0;
		averageSum[slot] = 0;
		averageLastIndex[slot] = 0;
		averageSize[slot] = 0;
	}
}

bool ScrollSpeedHud::ShouldDraw() {
	if (!sar_scrollspeed.GetBool()) {
		isEnabled = false;
		return false;
	}
	if (!insideSession) {
		return false;
	}
	if (!isEnabled) {
		clear();
	}
	isEnabled = true;
	return true;
}

ON_EVENT(SESSION_START) {
	int screenWidth, screenHeight;
	engine->GetScreenSize(nullptr, screenWidth, screenHeight);
	insideSession = true;
	clear();
}

ON_EVENT(SESSION_END) {
	insideSession = false;
}

float GetNextAverage(int slot, int nextValue) {
	averageSum[slot] = averageSum[slot] - averageQueue[slot][averageLastIndex[slot]] + nextValue;
	averageQueue[slot][averageLastIndex[slot]] = nextValue;
	averageLastIndex[slot]++;
	if (averageSize[slot] < AVERAGE_INPUT_AMOUNT) {
		averageSize[slot]++;
	}
	if (averageLastIndex[slot] >= AVERAGE_INPUT_AMOUNT) {
		averageLastIndex[slot] -= AVERAGE_INPUT_AMOUNT;
	}
	return (float)averageSum[slot] / averageSize[slot];
}

void DrawJumpRectangle(int jumpIter, int tickDifference, int currentLine, bool isJumped, bool isPrevMiss, unsigned long font, int xOffset, int yOffset) {
	int x = xOffset + jumpIter * (INPUT_SQUARE_WIDTH + INPUT_SQUARE_X_SPACING);
	int y = yOffset + (currentLine - 1) * (INPUT_SQUARE_HEIGHT + INPUT_SQUARE_Y_SPACING);
	Color curColor;
	switch (tickDifference) {
	case 0:
		curColor = MISS_INPUT_COLOR;
		break;
	case 1:
		curColor = MISS_INPUT_COLOR;
		break;
	case 2:
		curColor = PERFECT_INPUT_COLOR;
		break;
	case 3:
		curColor = OK_INPUT_COLOR;
		break;
	default:
		curColor = BAD_INPUT_COLOR;
	}
	if (isJumped) {
		surface->DrawRect(JUMP_HIGHLIGHT_COLOR, x - (INPUT_SQUARE_X_SPACING / 2), y - (INPUT_SQUARE_Y_SPACING / 2), 
			x + INPUT_SQUARE_WIDTH + (INPUT_SQUARE_X_SPACING / 2), y + INPUT_SQUARE_HEIGHT + (INPUT_SQUARE_Y_SPACING / 2));
		surface->DrawRect(curColor, x + INPUT_SQUARE_X_SPACING, y + INPUT_SQUARE_Y_SPACING, 
			x + INPUT_SQUARE_WIDTH - INPUT_SQUARE_X_SPACING, y + INPUT_SQUARE_HEIGHT - INPUT_SQUARE_Y_SPACING);
	} else {
		surface->DrawRect(curColor, x, y, x + INPUT_SQUARE_WIDTH, y + INPUT_SQUARE_HEIGHT);
	}
	//text
	std::string displayedGroundFrames = "";
	int minGroundFrames = 0;
	int maxGroundFrames = tickDifference - 1;
	int textXOffset = 2 * INPUT_SQUARE_X_SPACING;
	if (isPrevMiss) {
		minGroundFrames += 1;
	}
	//centering text
	if (minGroundFrames == maxGroundFrames) {
		displayedGroundFrames = std::to_string(minGroundFrames);
		textXOffset += 12;
	} else {
		displayedGroundFrames = std::to_string(minGroundFrames) + "-" + std::to_string(maxGroundFrames);
		textXOffset += 4;
	}
	if (tickDifference < 2) {
		displayedGroundFrames = " +1";
		textXOffset = 2 * INPUT_SQUARE_X_SPACING;
	}
	surface->DrawTxt(font, x + textXOffset, y + INPUT_SQUARE_Y_SPACING, GROUNDFRAMES_TEXT_COLOR, displayedGroundFrames.c_str());
}

void DrawScrollTiles(int slot, unsigned long font, int xOffset, int yOffset) {

	int x0 = xOffset + 2 * (INPUT_SQUARE_WIDTH + INPUT_SQUARE_X_SPACING);
	int y0 = yOffset - 1 * (INPUT_SQUARE_HEIGHT + INPUT_SQUARE_Y_SPACING);
	int x1 = xOffset + MAX_CONSECUTIVE_SCROLL_INPUTS * (INPUT_SQUARE_WIDTH + INPUT_SQUARE_X_SPACING);
	int y1 = yOffset + (MAX_INPUT_LINES - 1) * (INPUT_SQUARE_HEIGHT + INPUT_SQUARE_Y_SPACING);
	surface->DrawRect(BACKGROUND_COLOR, x0, y0, x1, y1);
	//scroll tiles
	for (int lineIter = 0; lineIter < MAX_INPUT_LINES; lineIter++) {
		for (int jumpIter = 0; jumpIter < MAX_CONSECUTIVE_SCROLL_INPUTS; jumpIter++) {
			int prevJumpIter = jumpIter - 1;
			if (prevJumpIter < 0) {
				prevJumpIter += MAX_CONSECUTIVE_SCROLL_INPUTS;
			}
			int prev2JumpIter = prevJumpIter - 1;
			if (prev2JumpIter < 0) {
				prev2JumpIter += MAX_CONSECUTIVE_SCROLL_INPUTS;
			}
			bool isPrevMiss = false;
			if ((jumpIter > 0) && (g_jumpedTicks[slot][lineIter][jumpIter - 1]) && (g_jumpTicks[slot][lineIter][prevJumpIter] - g_jumpTicks[slot][lineIter][prev2JumpIter] <= 1)) {
				isPrevMiss = true; 
			}
			if ((g_jumpTicks[slot][lineIter][prevJumpIter] != 0) && (g_jumpTicks[slot][lineIter][jumpIter] >= g_jumpTicks[slot][lineIter][prevJumpIter])) {
				DrawJumpRectangle(jumpIter, g_jumpTicks[slot][lineIter][jumpIter] - g_jumpTicks[slot][lineIter][prevJumpIter], lineIter, 
					g_jumpedTicks[slot][lineIter][jumpIter], isPrevMiss, font, xOffset, yOffset);
			}
		}
	}
}

void DrawAverageBar(int slot, int xOffset, int yOffset) {
	int x0 = xOffset + 2 * (INPUT_SQUARE_WIDTH + INPUT_SQUARE_X_SPACING);
	int y0 = yOffset - 1 * (INPUT_SQUARE_HEIGHT + INPUT_SQUARE_Y_SPACING);
	int x1 = xOffset + MAX_CONSECUTIVE_SCROLL_INPUTS * (INPUT_SQUARE_WIDTH + INPUT_SQUARE_X_SPACING);
	int y1 = yOffset + (MAX_INPUT_LINES - 1) * (INPUT_SQUARE_HEIGHT + INPUT_SQUARE_Y_SPACING);

	float barHeight = (INPUT_SQUARE_HEIGHT + INPUT_SQUARE_Y_SPACING) * MAX_INPUT_LINES;
	float barScale = (float)60 / (BAR_CPS_END - BAR_CPS_BEGIN);  //scaling from 0-60 to BAR_CPS_BEGIND-BAR_CPS_END
	float barY1 = y0 + barHeight * (BAR_CPS_END - 30) / 60 * barScale;
	float barY2 = y0 + barHeight * (BAR_CPS_END - 25) / 60 * barScale;
	float barY3 = y0 + barHeight * (BAR_CPS_END - 20) / 60 * barScale;

	surface->DrawRect(MISS_INPUT_COLOR, xOffset - 10, y0, xOffset + 10, barY1);
	surface->DrawRect(PERFECT_INPUT_COLOR, xOffset - 10, barY1, xOffset + 10, barY2);
	surface->DrawRect(OK_INPUT_COLOR, xOffset - 10, barY2, xOffset + 10, barY3);
	surface->DrawRect(BAD_INPUT_COLOR, xOffset - 10, barY3, xOffset + 10, y1);

	float scaledAverage = barHeight / lastAverage[slot] * barScale;
	float averageLineY1 = y1 + BAR_CPS_BEGIN * barHeight * barScale / 60;
	float averageYPos = averageLineY1 - scaledAverage;
	if (averageYPos < y0) {
		averageYPos = y0;
	}
	if (averageYPos > y1) {
		averageYPos = y1;
	}
	surface->DrawRect(AVERAGE_LINE_COLOR, xOffset - 20, averageYPos - 2, xOffset + 20, averageYPos + 2);
}

void ScrollSpeedHud::Paint(int slot) {

	unsigned long font = scheme->GetFontByID(8);
	if (sar_scrollspeed.GetInt() != 2) {
		DrawScrollTiles(slot, font, sar_scrollspeed_x.GetInt(), sar_scrollspeed_y.GetInt());
	}
	if (sar_scrollspeed.GetInt() != 3) {
		DrawAverageBar(slot, sar_scrollspeed_bar_x.GetInt(), sar_scrollspeed_bar_y.GetInt());
	}
}

void MoveLines(int slot) {
	for (int line = 0; line < MAX_INPUT_LINES - 1; line++) {
		for (int curJumpIndex = 0; curJumpIndex < MAX_CONSECUTIVE_SCROLL_INPUTS; curJumpIndex++) {
			g_jumpTicks[slot][line][curJumpIndex] = g_jumpTicks[slot][line + 1][curJumpIndex];
			g_jumpedTicks[slot][line][curJumpIndex] = g_jumpedTicks[slot][line + 1][curJumpIndex];
		}
	}
	for (int curJumpIndex = 0; curJumpIndex < MAX_CONSECUTIVE_SCROLL_INPUTS; curJumpIndex++) {
		g_jumpTicks[slot][MAX_INPUT_LINES - 1][curJumpIndex] = 0;
		g_jumpedTicks[slot][MAX_INPUT_LINES - 1][curJumpIndex] = false;
	}
}

void ScrollSpeedHud::OnJump(int slot, bool grounded) {
	if (slot >= 2) {
		return;
	}
	if (!isEnabled) {
		return;
	}
	if (!insideSession) {
		return;
	}
	int tick = session->GetTick();
	int lastJumpIndex = jumpCounter[slot] - 1;
	if (lastJumpIndex < 0) {
		lastJumpIndex += MAX_CONSECUTIVE_SCROLL_INPUTS;
	}
	// Reset if it's been long enough
	if ((tick > g_jumpTicks[slot][currentLine[slot]][lastJumpIndex] + CONSECUTIVE_END) && (g_jumpTicks[slot][currentLine[slot]][lastJumpIndex] > 0)) {
		jumpCounter[slot] = 1;
		if (currentLine[slot] + 1 < MAX_INPUT_LINES) {
			currentLine[slot] += 1;
		}
		int nextLine = currentLine[slot] + 1;
		if (nextLine >= MAX_INPUT_LINES) {
			nextLine = MAX_INPUT_LINES - 1;

			//clearing upcoming lines
			if (lastLineFilled) { //if last line is not empty
				MoveLines(slot);
			}
			lastLineFilled[slot] = true;
		}
	}
	if (jumpCounter[slot] > 1) {
		lastAverage[slot] = GetNextAverage(slot, tick - g_jumpTicks[slot][currentLine[slot]][jumpCounter[slot] - 1]);
	}
	if (jumpCounter[slot] < MAX_CONSECUTIVE_SCROLL_INPUTS) {
		g_jumpTicks[slot][currentLine[slot]][jumpCounter[slot]] = tick;
		if (grounded) {
			g_jumpedTicks[slot][currentLine[slot]][jumpCounter[slot]] = true;
		}
		jumpCounter[slot]++;
	}
}

ScrollSpeedHud scrollSpeedHud;
