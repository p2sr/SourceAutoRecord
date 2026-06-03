#include "VGui.hpp"

#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Features/Stitcher.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Console.hpp"
#include "SAR.hpp"
#include "Surface.hpp"
#include <cstdint>
#include <imgui.h>
#include <imgui_internal.h>
#include "Utils/SDK/Math.hpp"
#include "Modules/InputSystem.hpp"
#include "Imgui/ImguiWindow.hpp"
#include "Imgui/ToastPos.hpp"
#include "Imgui/DemoPlayer.hpp"
#include "Imgui/FileBrowser.hpp"
#include "Imgui/ImguiHud.hpp"

#include <algorithm>
#include <cstring>

bool g_drawImgui = false;

static bool g_keysDown[512] = {};
static bool g_keysPressed[512] = {};
static bool g_keysReleased[512] = {};

CON_COMMAND(sar_imgui_toggle, "sar_imgui_toggle - Toggle ImGui\n") {
  g_drawImgui = !g_drawImgui;
  // Stupid stupid stupid
  engine->ExecuteCommand("toggleconsole; toggleconsole");
}

CON_COMMAND(sar_imgui_toggle_window, "sar_imgui_toggle_window <window> - toggles the visibility of the given imgui window") {
  if (args.ArgC() != 2) {
		return console->Print(sar_imgui_toggle_window.ThisPtr()->m_pszHelpString);
  }

  bool noneOpen = true;
  for (auto& w : imguiWindows) {
    if (strcmp(w.internalName, args[1]) == 0) {
      g_drawImgui = true;
      w.open = !w.open;
    }

    if (w.open) noneOpen = false;
  }

  if (noneOpen) g_drawImgui = false;
}

CON_COMMAND(sar_imgui_list_windows, "sar_imgui_list_windows - lists all available windows") {
  console->Print("Available windows:\n");
  for (auto& w : imguiWindows) {
    console->Print("  %s\n", w.internalName);
  }
}

static bool g_drawWithFontAtlas = true;

static int g_imguiFontTextureId = -1;
static int g_imguiWhiteTextureId = -1;

static void UploadImGuiFontTexture() {
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  g_imguiFontTextureId = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);
  surface->DrawSetTextureRGBA(
    surface->matsurface->ThisPtr(),
    g_imguiFontTextureId,
    pixels,
    width,
    height
  );

  io.Fonts->SetTexID((ImTextureID)(intptr_t)g_imguiFontTextureId);
  console->Print("ImGui Font ID: %d", g_imguiFontTextureId);
}

// static void UploadImGuiWhiteTexture() {
//   g_imguiWhiteTextureId = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);
//   unsigned char whitePixel[4] = { 255, 255, 255, 255 };
//   surface->DrawSetTextureRGBA(
//     surface->matsurface->ThisPtr(),
//     g_imguiWhiteTextureId,
//     whitePixel,
//     1,
//     1
//   );
// }

REDECL(VGui::Paint);
REDECL(VGui::UpdateProgressBar);

Variable sar_hud_bg("sar_hud_bg", "0", "Enable the SAR HUD background.\n", FCVAR_DONTRECORD);
Variable sar_hud_orange_only("sar_hud_orange_only", "0", "Only display the SAR HUD for orange, for solo coop (fullscreen PIP).\n", FCVAR_DONTRECORD);

void VGui::Draw(Hud *const &hud) {
	if (hud->ShouldDraw()) {
		hud->Paint(this->context.slot);
	}
}
void VGui::Draw(HudElement *const &element) {
	if (element->ShouldDraw()) {
		element->Paint(&this->context);
	}
}

static void DrawHudBackground(int slot, HudContext &ctx) {
	if (!sar_hud_bg.GetBool()) return;

	int height =
		ctx.elements == 0 ? 0 : ctx.elements * ctx.fontSize + (ctx.elements - 1) * ctx.spacing + 4;

	static int maxWidths[2][100];
	memmove(maxWidths[slot], maxWidths[slot] + 1, sizeof maxWidths[slot] - sizeof maxWidths[slot][0]);
	maxWidths[slot][99] = ctx.maxWidth;

	int width = 0;

	for (size_t i = 0; i < sizeof maxWidths[slot] / sizeof maxWidths[slot][0]; ++i) {
		if (maxWidths[slot][i] > width) width = maxWidths[slot][i];
	}

	if (width % 5) width += 5 - width % 5;

	if (width != 0) width += 4;

	int align = sar_hud_align.GetInt();
	int offset = !align ? 2 : align == 1 ? width / 2 : width - 3;

	int x = ctx.xPadding - offset;
	int y = ctx.yPadding - 2;

	surface->DrawRect(Color{0, 0, 0, 192}, x, y, x + width, y + height);
}

static Color prevCol;
void ImGui_RenderDrawData_Source(ImDrawData* drawData) {
  if (!drawData || drawData->CmdListsCount == 0) return;

  for (int n = 0; n < drawData->CmdListsCount; n++) {
    const ImDrawList* cmdList = drawData->CmdLists[n];
    const ImDrawVert* vtxBuffer = cmdList->VtxBuffer.Data;
    const ImDrawIdx* idxBuffer = cmdList->IdxBuffer.Data;

    for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
      const ImDrawCmd* cmd = &cmdList->CmdBuffer[i];

      if (cmd->UserCallback) {
        cmd->UserCallback(cmdList, cmd);
        continue;
      }

      int cx0 = (int)cmd->ClipRect.x;
      int cy0 = (int)cmd->ClipRect.y;
      int cx1 = (int)cmd->ClipRect.z;
      int cy1 = (int)cmd->ClipRect.w;
      
      // Clamp to screen
      // if (cx0 < 0) cx0 = 0;
      // if (cy0 < 0) cy0 = 0;

      // bool isFont = (texId == g_imguiFontTextureId);

      for (int idx = 0; idx < cmd->ElemCount; idx += 3) {
        ImDrawIdx i0 = idxBuffer[cmd->IdxOffset + idx + 0];
        ImDrawIdx i1 = idxBuffer[cmd->IdxOffset + idx + 1];
        ImDrawIdx i2 = idxBuffer[cmd->IdxOffset + idx + 2];

        const ImDrawVert& v0 = vtxBuffer[cmd->VtxOffset + i0];
        const ImDrawVert& v1 = vtxBuffer[cmd->VtxOffset + i1];
        const ImDrawVert& v2 = vtxBuffer[cmd->VtxOffset + i2];

        auto unpack = [](ImU32 c) -> Color {
          return Color{
            (uint8_t)((c >> 0) & 0xFF), // R
            (uint8_t)((c >> 8) & 0xFF), // G
            (uint8_t)((c >> 16) & 0xFF), // B
            (uint8_t)((c >> 24) & 0xFF), // A
          };
        };

        Color col = unpack(v0.col); // Simplification. Only uses v0's col
        // if (col.a == 0) continue;

        // Makes some glyphs look better. Why? We may never know. And i don't
        // want to
        if (col.a == 0) {
          col = prevCol;
        }

        prevCol = col;

        Vertex_t verts[3];
        verts[0] = {{v0.pos.x, v0.pos.y}, {v0.uv.x, v0.uv.y}};
        verts[1] = {{v1.pos.x, v1.pos.y}, {v1.uv.x, v1.uv.y}};
        verts[2] = {{v2.pos.x, v2.pos.y}, {v2.uv.x, v2.uv.y}};

        ImVec2 whiteUV = ImGui::GetDrawListSharedData()->TexUvWhitePixel;

        auto isWhitePixelTri = [&](const ImDrawVert& v0, const ImDrawVert& v1, const ImDrawVert& v2) {
          const float eps = 0.0001f;
          auto near = [&](ImVec2 uv) {
            return fabs(uv.x - whiteUV.x) < eps && fabs(uv.y - whiteUV.y) < eps;
          };
          return near(v0.uv) && near(v1.uv) && near(v2.uv);
        };

        bool isShape = isWhitePixelTri(v0, v1, v2);

        if (isShape || !g_drawWithFontAtlas) {
          // console->Print("SHAPE!!");
          // I don't know why, I don't want to know why, I shouldn't have to
          // wonder why, but for whatever reason if I set this to the white 
          // texture, everything looks fucked?! This is probably the same 
          // reason why the stupid borders look fucked (they use the texture
          // atlas). No texture works fine for now.
          surface->DrawSetTexture(surface->matsurface->ThisPtr(), -1);
        } else {
          int texId = (int)(intptr_t)cmd->GetTexID();
          surface->DrawSetTexture(surface->matsurface->ThisPtr(), texId);
        }

        surface->DrawSetColor(surface->matsurface->ThisPtr(), col.r, col.g, col.b, col.a);
        surface->DrawTexturedPolygon(surface->matsurface->ThisPtr(), 3, verts, true);
      }
    }
  }
}

void DrawImguiWindows() {
  for (auto& w : imguiWindows) {
    if (!w.open) continue;

    ImGui::SetNextWindowSize(w.windowSize, ImGuiCond_Appearing);
    ImGui::Begin(w.name, &w.open);
    w.render();
    ImGui::End();
  }
  
  // Draw file browser
  FileBrowser::Render();
}

void DrawImguiTopBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      if (ImGui::MenuItem("Quit")) {
        engine->ExecuteCommand("quit");
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Sar")) {
      for (auto& w : imguiWindows) {
        if (w.category == ImguiWindowCategory::Sar) {
          ImGui::MenuItem(w.name, nullptr, &w.open);
        }
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Hud")) {
      for (auto& w : imguiWindows) {
        if (w.category == ImguiWindowCategory::Hud) {
          ImGui::MenuItem(w.name, nullptr, &w.open);
        }
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

static ImGuiKey ToImGuiKey(ButtonCode_t key)
{
  switch (key) {
    case KEY_A: return ImGuiKey_A;
    case KEY_B: return ImGuiKey_B;
    case KEY_C: return ImGuiKey_C;
    case KEY_D: return ImGuiKey_D;
    case KEY_E: return ImGuiKey_E;
    case KEY_F: return ImGuiKey_F;
    case KEY_G: return ImGuiKey_G;
    case KEY_H: return ImGuiKey_H;
    case KEY_I: return ImGuiKey_I;
    case KEY_J: return ImGuiKey_J;
    case KEY_K: return ImGuiKey_K;
    case KEY_L: return ImGuiKey_L;
    case KEY_M: return ImGuiKey_M;
    case KEY_N: return ImGuiKey_N;
    case KEY_O: return ImGuiKey_O;
    case KEY_P: return ImGuiKey_P;
    case KEY_Q: return ImGuiKey_Q;
    case KEY_R: return ImGuiKey_R;
    case KEY_S: return ImGuiKey_S;
    case KEY_T: return ImGuiKey_T;
    case KEY_U: return ImGuiKey_U;
    case KEY_V: return ImGuiKey_V;
    case KEY_W: return ImGuiKey_W;
    case KEY_X: return ImGuiKey_X;
    case KEY_Y: return ImGuiKey_Y;
    case KEY_Z: return ImGuiKey_Z;

    case KEY_SPACE: return ImGuiKey_Space;
    case KEY_ENTER: return ImGuiKey_Enter;
    case KEY_ESCAPE: return ImGuiKey_Escape;
    case KEY_BACKSPACE: return ImGuiKey_Backspace;

    // case KEY_LSHIFT: case KEY_RSHIFT: return ImGuiKey_LeftShift;
    // case KEY_LCONTROL: case KEY_RCONTROL: return ImGuiKey_LeftCtrl;
    // case KEY_LALT: case KEY_RALT: return ImGuiKey_LeftAlt;
  }

  return ImGuiKey_None;
}

static bool prevDown[512] = {};

// CEngineVGui::Paint
DETOUR(VGui::Paint, PaintMode_t mode) {
	if (mode & PAINT_UIPANELS) {
		surface->StartDrawing(surface->matsurface->ThisPtr());
		networkManager.UpdateSyncUi();
		surface->FinishDrawing();
	}

	static HudContext lastCtx[2];

	auto result = VGui::Paint(thisptr, mode);

	surface->StartDrawing(surface->matsurface->ThisPtr());

	auto ctx = &vgui->context;

	ctx->Reset(GET_SLOT());

	if (ctx->slot == 0) {
		if ((mode & PAINT_UIPANELS) && !sar_hud_orange_only.GetBool() && !Stitcher::Paint()) {
			DrawHudBackground(0, lastCtx[0]);

			for (auto const &hud : vgui->huds) {
				vgui->Draw(hud);
			}

			for (auto const &element : vgui->elements) {
				vgui->Draw(element);
			}

			lastCtx[0] = *ctx;
		}
	} else if (ctx->slot == 1) {
		DrawHudBackground(1, lastCtx[1]);

		for (auto const &hud : vgui->huds) {
			if (hud->drawSecondSplitScreen) {
				vgui->Draw(hud);
			}
		}

		for (auto const &element : vgui->elements) {
			if (element->drawSecondSplitScreen) {
				vgui->Draw(element);
			}
		}

		lastCtx[1] = *ctx;
	}

  if (g_drawImgui) {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    int screenX, screenY;
    engine->GetScreenSize(engine, screenX, screenY);

    io.DisplaySize = ImVec2(screenX, screenY);

    int mouseX, mouseY;
    inputSystem->GetCursorPos(mouseX, mouseY);

    bool mouse1Down = inputSystem->IsKeyDown(MOUSE_LEFT);

    bool scrollDown = inputSystem->IsKeyDown(MOUSE_WHEEL_DOWN);
    bool scrollUp = inputSystem->IsKeyDown(MOUSE_WHEEL_UP);

    float mDelta = 0.0f;
    if (scrollUp) mDelta += 1.0f;
    if (scrollDown) mDelta -= 1.0f;

    io.MousePos = ImVec2(mouseX, mouseY);
    io.MouseDown[0] = mouse1Down ? 1 : 0;
    io.MouseDown[1] = 0;
    io.MouseDown[2] = 0;
    io.MouseWheel = (scrollUp ? 1.0f : 0.0f) + (scrollDown ? -1.0f : 0.0f);

    memset(g_keysPressed, 0, sizeof(g_keysPressed));
    memset(g_keysReleased, 0, sizeof(g_keysReleased));

    // io.KeyCtrl = g_keysDown[KEY_LCONTROL] || g_keysDown[KEY_RCONTROL];
    // io.KeyShift = g_keysDown[KEY_LSHIFT] || g_keysDown[KEY_RSHIFT];
    // io.KeyAlt = g_keysDown[KEY_LALT] || g_keysDown[KEY_RALT];

    // PLEASE REDO ME!!! This uses like "raw" key states. It should take
    // character input from engine directly, not this shit
    for (int i = 0; i < KEY_COUNT; i++) {
      ButtonCode_t key = (ButtonCode_t)i;

      bool down = inputSystem->IsKeyDown(key);

      ImGuiKey igKey = ToImGuiKey(key);
      if (igKey != ImGuiKey_None) {
        io.AddKeyEvent(igKey, down);
      }
    }

    for (int i = KEY_A; i <= KEY_Z; i++) {
      ButtonCode_t key = (ButtonCode_t)i;
      bool down = inputSystem->IsKeyDown(key);
      ImGuiKey igKey = ToImGuiKey(key);

      if (down && !prevDown[i]) {
        char base = 'a' + (i - KEY_A);
        bool shift = inputSystem->IsKeyDown(KEY_LSHIFT);
        char c = shift ? (base - 32) : base;
        // console->Print("%c\n", c);
        ImGui::GetIO().AddInputCharacter(c);
      }

      prevDown[i] = down;
    }

    // if (inputSystem->IsKeyDown(KEY_A)) {
    //   console->Print("OMG!!\n");
    // }

    ImGui::NewFrame();

    // ImGui::Begin("Input test");
    //
    // ImGui::Text("KeysDown A: %d", ImGui::IsKeyDown(ImGuiKey_A));
    // // ImGui::Text("KeysDown W: %d", io.KeysDown[KEY_W]);
    //
    // ImGui::Text("Mouse: %.1f %.1f", io.MousePos.x, io.MousePos.y);
    // ImGui::Text("Wheel: %.1f", io.MouseWheel);
    //
    // ImGui::End();

    DrawImguiTopBar();
    DrawImguiWindows();

    ImGui::Render();

    ImGui_RenderDrawData_Source(
      ImGui::GetDrawData()
    );

    if (true) { // if (engine->demoplayer->IsPlaying()) {
      int size = 10;
      Vertex_t verts[3];
      verts[0] = {{mouseX, mouseY}};
      verts[1] = {{mouseX + size, mouseY}};
      verts[2] = {{mouseX, mouseY + size}};

      surface->DrawSetTexture(surface->matsurface->ThisPtr(), -1);
      surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, 255);
      surface->DrawTexturedPolygon(surface->matsurface->ThisPtr(), 3, verts, true);
    }
  }

	surface->FinishDrawing();

  // static void* inputCtx = engine->GetInputContext(engine->engineClient->ThisPtr(), 0);
  //
  // inputSystem->SetCursorVisible(inputSystem->g_InputSystem->ThisPtr(), inputCtx, false);

	return result;
}

DETOUR(VGui::UpdateProgressBar, int progress) {
	if (vgui->lastProgressBar != progress) {
		vgui->lastProgressBar = progress;
		vgui->progressBarCount = 0;
	}
	vgui->progressBarCount++;
	if (sar_disable_progress_bar_update.GetInt() == 1 && vgui->progressBarCount > 1) {
		return 0;
	}
	if (sar_disable_progress_bar_update.GetInt() == 2) {
		return 0;
	}
	return VGui::UpdateProgressBar(thisptr, progress);
}

bool VGui::IsUIVisible() {
	return this->IsGameUIVisible(this->enginevgui->ThisPtr());
}

bool VGui::Init() {
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("./din.ttf", 18.0f);
  io.Fonts->Build();
  UploadImGuiFontTexture();
  // UploadImGuiWhiteTexture();
  io.IniFilename = nullptr;
  io.LogFilename = nullptr;
  io.DeltaTime = 1.0f / 60.0f;
  
  ImGuiStyle& style = ImGui::GetStyle();
  // Could be set to 4 but vgui makes it look poopoo
  float rounding = 0.f;
  style.TabRounding = rounding;
  style.FrameRounding = rounding;
  style.GrabRounding = rounding;
  style.WindowRounding = rounding;
  style.PopupRounding = rounding;

  // Windows
  AddDemoPlayer();
  AddHud();

	this->enginevgui = Interface::Create(this->Name(), "VEngineVGui001");
	if (this->enginevgui) {
		this->IsGameUIVisible = this->enginevgui->Original<_IsGameUIVisible>(Offsets::IsGameUIVisible);

		this->enginevgui->Hook(VGui::Paint_Hook, VGui::Paint, Offsets::Paint);
		this->enginevgui->Hook(VGui::UpdateProgressBar_Hook, VGui::UpdateProgressBar, Offsets::Paint + 12);

		for (auto &hud : Hud::GetList()) {
			if (hud->version == SourceGame_Unknown || sar.game->Is(hud->version)) {
				this->huds.push_back(hud);
			}
		}

		HudElement::IndexAll();

		for (auto const &element : HudElement::GetList()) {
			if (element->version == SourceGame_Unknown || sar.game->Is(element->version)) {
				this->elements.push_back(element);
			}
		}

		std::sort(this->elements.begin(), this->elements.end(), [](const HudElement *a, const HudElement *b) {
			return a->orderIndex < b->orderIndex;
		});
	}

	return this->hasLoaded = this->enginevgui;
}
void VGui::Shutdown() {
	Interface::Delete(this->enginevgui);
	this->huds.clear();
}

VGui *vgui;
