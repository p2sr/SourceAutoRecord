#include "ScrollSpeed.hpp"

#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Utils/SDK/Math.hpp"
#include "Variable.hpp"

#define CONSECUTIVE_END 8
#define TOTAL_THRESHOLD 5
#define MAX_CONSECUTIVE_SCROLL_INPUTS 12
#define MAX_INPUT_LINES 12
#define AVERAGE_INPUT_AMOUNT 12

#define INPUT_SQUARE_LENGTH Vector2(40, 20)
#define INPUT_SQUARE_SPACING Vector2(2, 2)
#define INPUT_SQUARE_FULL_LENGTH (INPUT_SQUARE_LENGTH + INPUT_SQUARE_SPACING)

#define PERFECT_INPUT_COLOR     Color(0, 112, 255, 255)
#define MISS_INPUT_COLOR        Color(165, 28, 28, 255)
#define OK_INPUT_COLOR          Color(208, 208, 0, 255)
#define BAD_INPUT_COLOR         Color(255, 144, 0, 255)
#define BACKGROUND_COLOR        Color(70, 100, 150, 70)
#define AVERAGE_LINE_COLOR      Color(200, 200, 200, 255)
#define JUMP_HIGHLIGHT_COLOR    Color(100, 200, 255, 255)
#define GROUNDFRAMES_TEXT_COLOR Color(0, 0, 0, 255)

#define TILE_TEXT_FONT_ID 8

#define BAR_CPS_BEGIN 10
#define BAR_CPS_END 45

Variable sar_scrollspeed("sar_scrollspeed", "0", "Show a HUD indicating your scroll speed for coop.\n1 = bar and tiles,\n2 = bar only,\n3 = tiles only.\n");
Variable sar_scrollspeed_x("sar_scrollspeed_x", "0", "Scroll speed HUD x offset.\n");
Variable sar_scrollspeed_y("sar_scrollspeed_y", "210", "Scroll speed HUD y offset.\n");
Variable sar_scrollspeed_bar_x("sar_scrollspeed_bar_x", "30", "Scroll speed bar x offset.\n");
Variable sar_scrollspeed_bar_y("sar_scrollspeed_bar_y", "210", "Scroll speed bar y offset.\n");


int g_jumpTicks[2][MAX_INPUT_LINES][MAX_CONSECUTIVE_SCROLL_INPUTS];
bool g_jumpedTicks[2][MAX_INPUT_LINES][MAX_CONSECUTIVE_SCROLL_INPUTS];

float g_lastAverage[2];
int g_averageQueue[2][AVERAGE_INPUT_AMOUNT];

int g_jumpCounter[2];
bool g_lastLineFilled[2];
int g_currentLine[2];

int g_averageSum[2];
int g_averageLastIndex[2];
int g_averageSize[2];

bool ScrollSpeedHud::ShouldDraw() {
	return Hud::ShouldDraw() && sar_scrollspeed.GetBool();
}

void clear(int slot) {
	for (int inputLine = 0; inputLine < MAX_INPUT_LINES; inputLine++) {
		for (int jumpInput = 0; jumpInput < MAX_CONSECUTIVE_SCROLL_INPUTS; jumpInput++) {
			g_jumpTicks[slot][inputLine][jumpInput] = 0;
			g_jumpedTicks[slot][inputLine][jumpInput] = false;
		}
	}
	for (int inputQueueindex = 0; inputQueueindex < AVERAGE_INPUT_AMOUNT; inputQueueindex++) {
		g_averageQueue[slot][inputQueueindex] = 0;
	}
	g_jumpCounter[slot] = 0; //indicator that data is empty
	g_lastLineFilled[slot] = false;
	g_currentLine[slot] = 0;
	g_lastAverage[slot] = 0;
	g_averageSum[slot] = 0;
	g_averageLastIndex[slot] = 0;
	g_averageSize[slot] = 0;
}

void ClearLine(int slot, int line) {
	for (int jumpIter = 0; jumpIter < MAX_CONSECUTIVE_SCROLL_INPUTS; jumpIter++) {
		g_jumpedTicks[slot][line][jumpIter] = false;
		g_jumpTicks[slot][line][jumpIter] = 0;
	}
}


static inline int GetTickDifference(int slot, int lineIter, int jumpIter) {
	return g_jumpTicks[slot][lineIter][jumpIter] - g_jumpTicks[slot][lineIter][jumpIter - 1];
}

static inline int BoundIndex(int lineIter, int side = 0, int arraySize = MAX_INPUT_LINES) {  //side -1 for left check, 0 for both, 1 for right check
	if (side <= 0 && lineIter < 0) {
		lineIter += arraySize;
	}
	if (side >= 0 && lineIter >= MAX_INPUT_LINES) {
		lineIter -= arraySize;
	}
	return lineIter;
}

float GetNextAverage(int slot, int nextValue) {
	g_averageSum[slot] = g_averageSum[slot] - g_averageQueue[slot][g_averageLastIndex[slot]] + nextValue;
	g_averageQueue[slot][g_averageLastIndex[slot]] = nextValue;
	g_averageLastIndex[slot]++;
	if (g_averageSize[slot] < AVERAGE_INPUT_AMOUNT) {
		g_averageSize[slot]++;
	}
	if (g_averageLastIndex[slot] >= AVERAGE_INPUT_AMOUNT) {
		g_averageLastIndex[slot] -= AVERAGE_INPUT_AMOUNT;
	}
	return (float)g_averageSum[slot] / g_averageSize[slot];
}

void DrawJumpRectangle(int jumpIter, int tickDifference, int currentLine, bool isJumped, bool isPrevMiss, unsigned long font, Vector2<int> hudOffset) {
	Vector2<int> localOffset = hudOffset + Vector2<int>(jumpIter * INPUT_SQUARE_FULL_LENGTH.x, (currentLine - 1) * INPUT_SQUARE_FULL_LENGTH.y);
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
	//text
	std::string displayedGroundFrames;
	int minGroundFrames = 0;
	int maxGroundFrames = tickDifference - 1;
	if (isPrevMiss) {
		minGroundFrames += 1;
	}
	if (minGroundFrames == maxGroundFrames) {
		displayedGroundFrames = std::to_string(minGroundFrames);
	} else {
		displayedGroundFrames = std::to_string(minGroundFrames) + "-" + std::to_string(maxGroundFrames);
	}
	if (tickDifference < 2) {
		displayedGroundFrames = "+1 ";
	}

	if (isJumped) {
		Vector2<int> highlightSquareBegin = localOffset - (INPUT_SQUARE_SPACING / 2);
		Vector2<int> highlightSquareEnd = localOffset + INPUT_SQUARE_LENGTH + (INPUT_SQUARE_SPACING / 2);
		Vector2<int> inputSquareBegin = highlightSquareBegin + INPUT_SQUARE_SPACING;
		Vector2<int> inputSquareEnd = highlightSquareEnd - INPUT_SQUARE_SPACING;
		surface->DrawRect(JUMP_HIGHLIGHT_COLOR, highlightSquareBegin, highlightSquareEnd);  //outline
		surface->DrawRectAndCenterTxt(curColor, inputSquareBegin, inputSquareEnd, font, GROUNDFRAMES_TEXT_COLOR, displayedGroundFrames);
	} else {
		surface->DrawRectAndCenterTxt(curColor, localOffset, localOffset + INPUT_SQUARE_LENGTH, font, GROUNDFRAMES_TEXT_COLOR, displayedGroundFrames);
	}
}

void DrawScrollTiles(int slot, unsigned long font, Vector2<int> hudOffset) {
	Vector2<int> hudLocalBegin = Vector2(INPUT_SQUARE_FULL_LENGTH.x * 2, -INPUT_SQUARE_FULL_LENGTH.y);
	Vector2<int> hudLocalEnd = Vector2(MAX_CONSECUTIVE_SCROLL_INPUTS * INPUT_SQUARE_FULL_LENGTH.x, (MAX_INPUT_LINES - 1) * INPUT_SQUARE_FULL_LENGTH.y);
	surface->DrawRect(BACKGROUND_COLOR, hudOffset + hudLocalBegin, hudOffset + hudLocalEnd);
	//scroll tiles
	for (int fakeLineIter = 0; fakeLineIter < MAX_INPUT_LINES; fakeLineIter++) {  //making sure it ends
		int lineIter = BoundIndex(fakeLineIter + g_currentLine[slot], 1); //data line
		int drawLine = BoundIndex(fakeLineIter - 1, -1);
		if (!g_lastLineFilled[slot]) {
			drawLine = lineIter;
		}
		for (int jumpIter = 2; jumpIter < MAX_CONSECUTIVE_SCROLL_INPUTS; jumpIter++) {
			bool isPrevMiss = g_jumpedTicks[slot][lineIter][jumpIter - 1] && GetTickDifference(slot, lineIter, jumpIter - 1) <= 1;
			if (g_jumpTicks[slot][lineIter][jumpIter] != 0) {
				DrawJumpRectangle(jumpIter, GetTickDifference(slot, lineIter, jumpIter), drawLine, 
					g_jumpedTicks[slot][lineIter][jumpIter], isPrevMiss, font, hudOffset);
			} else {
				break;
			}
		}
	}
}

void DrawAverageBar(int slot, Vector2<int> hudOffset) {
	int hudBeginY = hudOffset.y + -INPUT_SQUARE_FULL_LENGTH.y;
	int hudEndY = hudOffset.y + (MAX_INPUT_LINES - 1) * INPUT_SQUARE_FULL_LENGTH.y;

	float tickrate = 1.0f / engine->GetIPT();
	float barHeight = INPUT_SQUARE_FULL_LENGTH.y * MAX_INPUT_LINES;
	float barScale = tickrate / (BAR_CPS_END - BAR_CPS_BEGIN);  //scaling from 0-60 to BAR_CPS_BEGIN-BAR_CPS_END

	float barY1 = hudBeginY + barHeight * (BAR_CPS_END - 30) / tickrate * barScale;
	float barY2 = hudBeginY + barHeight * (BAR_CPS_END - 25) / tickrate * barScale;
	float barY3 = hudBeginY + barHeight * (BAR_CPS_END - 20) / tickrate * barScale;

	surface->DrawRect(MISS_INPUT_COLOR, hudOffset.x - 10, hudBeginY, hudOffset.x + 10, barY1);
	surface->DrawRect(PERFECT_INPUT_COLOR, hudOffset.x - 10, barY1, hudOffset.x + 10, barY2);
	surface->DrawRect(OK_INPUT_COLOR, hudOffset.x - 10, barY2, hudOffset.x + 10, barY3);
	surface->DrawRect(BAD_INPUT_COLOR, hudOffset.x - 10, barY3, hudOffset.x + 10, hudEndY);

	float scaledAverage = barHeight / g_lastAverage[slot] * barScale;
	float averageLineY1 = hudEndY + BAR_CPS_BEGIN * barHeight * barScale / tickrate;
	float averageYPos = averageLineY1 - scaledAverage;
	if (averageYPos < hudBeginY) {
		averageYPos = hudBeginY;
	}
	if (averageYPos > hudEndY) {
		averageYPos = hudEndY;
	}
	surface->DrawRect(AVERAGE_LINE_COLOR, hudOffset.x - 20, averageYPos - 2, hudOffset.x + 20, averageYPos + 2);
}

void ScrollSpeedHud::Paint(int slot) {
	unsigned long font = scheme->GetFontByID(TILE_TEXT_FONT_ID);
	if (sar_scrollspeed.GetInt() != 2) {
		DrawScrollTiles(slot, font, Vector2(sar_scrollspeed_x.GetInt(), sar_scrollspeed_y.GetInt()));
	}
	if (sar_scrollspeed.GetInt() != 3) {
		DrawAverageBar(slot, Vector2(sar_scrollspeed_bar_x.GetInt(), sar_scrollspeed_bar_y.GetInt()));
	}
}

void ScrollSpeedHud::OnJump(int slot, bool grounded) {
	int tick = session->GetTick();
	int lastJumpIndex = BoundIndex(g_jumpCounter[slot] - 1, -1, MAX_CONSECUTIVE_SCROLL_INPUTS);
	// Reset if it's been long enough
	if (tick > g_jumpTicks[slot][g_currentLine[slot]][lastJumpIndex] + CONSECUTIVE_END && g_jumpTicks[slot][g_currentLine[slot]][lastJumpIndex] > 0) {
		g_jumpCounter[slot] = 1;
		g_currentLine[slot] += 1;
		if (g_currentLine[slot] >= MAX_INPUT_LINES) {
			g_currentLine[slot] -= MAX_INPUT_LINES;
			g_lastLineFilled[slot] = true;
		}
		ClearLine(slot, g_currentLine[slot]);
	}
	if (g_jumpCounter[slot] > 1) {
		g_lastAverage[slot] = GetNextAverage(slot, tick - g_jumpTicks[slot][g_currentLine[slot]][g_jumpCounter[slot] - 1]);
	}
	if (g_jumpCounter[slot] < MAX_CONSECUTIVE_SCROLL_INPUTS) {
		g_jumpTicks[slot][g_currentLine[slot]][g_jumpCounter[slot]] = tick;
		g_jumpedTicks[slot][g_currentLine[slot]][g_jumpCounter[slot]] = grounded;
		g_jumpCounter[slot]++;
	}
}

ON_EVENT(SESSION_START) {
	if (sar_scrollspeed.GetBool()) {
		if (g_jumpCounter[0]) clear(0);
		if (g_jumpCounter[1]) clear(1);
	}
}

ON_EVENT(PROCESS_MOVEMENT) {
	if (!sar_scrollspeed.GetBool()) {
		if (g_jumpCounter[event.slot]) clear(event.slot);
		return;
	}
	if (event.move && event.move->m_nButtons & IN_JUMP) scrollSpeedHud->OnJump(event.slot, event.grounded);
}

ScrollSpeedHud *scrollSpeedHud = new ScrollSpeedHud();
