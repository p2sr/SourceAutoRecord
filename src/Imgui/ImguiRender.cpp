#include "ImguiRender.hpp"

#include <imgui_internal.h>

#include "Modules/Surface.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Hud/ImguiHudPanel.hpp"
#include "DemoPlayer.hpp"
#include "Imgui/ToastPos.hpp"
#include "FileBrowser.hpp"
#include "ImguiWindow.hpp"
#include "Hud/ImguiHuds.hpp"
#include "ImguiHud.hpp"
#include "Command.hpp"
#include "Features/Timer/PauseTimer.hpp"

#include "Imgui/fonts/din_font.hpp"
#include "Imgui/fonts/fa_solid.hpp"

enum class ToolbarItemType {
  Button,
  Toggle,
  Menu,
};

struct ToolbarItem {
  const char* id;
  const char* icon;
  const char* tooltip;
  ToolbarItemType type;

  bool* toggleValue = nullptr;
  std::function<void()> onClick;
};

std::vector<ToolbarItem> g_toolbarItems;

bool g_drawImgui = false;

static bool g_drawWithFontAtlas = true;

static int g_imguiFontTextureId = -1;

CON_COMMAND(sar_imgui_toggle, "sar_imgui_toggle - Toggle ImGui\n") {
  g_drawImgui = !g_drawImgui;
}

CON_COMMAND(sar_imgui_toggle_edit_mode, "sar_imgui_toggle_edit_mode - Toggle ImGui edit mode\n") {
  g_imguiEditMode = !g_imguiEditMode;
}

CON_COMMAND(sar_imgui_toggle_window, "sar_imgui_toggle_window <window> - toggles the visibility of the given imgui window\n") {
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

CON_COMMAND(sar_imgui_toggle_hud, "sar_imgui_toggle_hud <hud> - toggles the visibility of the given imgui hud") {
  if (args.ArgC() != 2) {
    return console->Print(sar_imgui_toggle_hud.ThisPtr()->m_pszHelpString);
  }

  for (auto& h : g_imguiHuds) {
    if (strcmp(h->GetHandle(), args[1]) == 0) {
      h->Toggle();
    }
  }
}

CON_COMMAND(sar_imgui_list_windows, "sar_imgui_list_windows - lists all available windows") {
  console->Print("Available windows:\n");
  for (auto& w : imguiWindows) {
    console->Print("  %s\n", w.internalName);
  }
}

CON_COMMAND(sar_imgui_list_huds, "sar_imgui_list_huds - list all available huds") {
  console->Print("Available huds:\n");
  for (auto& h : g_imguiHuds) {
    console->Print("  %s\n", h->GetHandle());
  }
}

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

// Awesome font icon ranges
static const ImWchar icon_ranges[] = {
  0xF000,
  0xF8FF,
  0
};

void InitImgui() {
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromMemoryTTF((void*)din_ttf, din_ttf_len, 18.0f);

  ImFontConfig config;
  config.MergeMode = true;
  config.PixelSnapH = true;

  io.Fonts->AddFontFromMemoryTTF(
    (void*)fa_solid_ttf,
    fa_solid_ttf_len,
    16.0f,
    &config,
    icon_ranges
  );

  io.Fonts->AddFontDefault();

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
  style.WindowBorderSize = 0.0f;

  // Title bar
  g_toolbarItems.push_back({
    "game",
    "\xef\x80\x91", // Power off
    "",
    ToolbarItemType::Menu,
    nullptr,
    [] {
      if (engine->hoststate->m_activeGame) {
        if (ImGui::MenuItem("Disconnect")) {
          engine->ExecuteCommand("disconnect");
        }
      }

      if (ImGui::MenuItem("Quit")) {
        engine->ExecuteCommand("quit");
      }
    }
  });

  g_toolbarItems.push_back({
    "settings",
    // Should include a header with intuitive names... im lazy
    "\xef\x80\x93", // Cog
    "",
    ToolbarItemType::Button,
    nullptr,
    [] { console->Print("Pressed!!\n"); }
  });

  g_toolbarItems.push_back({
    "edit_mode",
    "\xef\x8c\x83", // Pencil thingy
    "Edit Mode",
    ToolbarItemType::Toggle,
    &g_imguiEditMode,
    nullptr
  });

  // Windows
  AddDemoPlayer();
//   AddHud();

  AddHud<ShowposHud>();
  AddHud<ImguiInputHud>();
}

// NAMING SUCKS!!!!
static bool g_keysDown[512] = {};
static bool g_keysPressed[512] = {};
static bool g_keysReleased[512] = {};
static bool prevDown[512] = {};

// This is a bad bad evil way to do this. Should use hook or something to take keyboard input directly
// from the engine, but this is fine for now. Noone is going to be writing essays with sar hopefully, unless
// Radio in hole% becomes unnaturaly popular.
static ImGuiKey ToImGuiKey(ButtonCode_t key) {
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

// Helper functions
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

void DrawImguiHuds() {
  for (auto& h : g_imguiHuds) {
    if (!h->Enabled() || !h->ShouldDraw()) {
      if (!g_imguiEditMode) continue;
      else if (!g_drawImgui) continue;
    }

    h->Render();
  }
}

// === Top bar === //

void DrawImguiTopbarRightSide() {
  float h = ImGui::GetFrameHeight();

  ImGui::SameLine(ImGui::GetWindowWidth() - h * g_toolbarItems.size() - ImGui::GetStyle().WindowPadding.x);

  ImDrawList* dl = ImGui::GetWindowDrawList();

  for (int i = (int)g_toolbarItems.size() - 1; i >= 0; i--) {
    const ToolbarItem& item = g_toolbarItems[i];

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(h, h);

    ImGui::InvisibleButton(item.id, size);

    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    ImRect rect(pos, ImVec2(pos.x + h, pos.y + h));

    // background
    if (hovered) {
      dl->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(ImGuiCol_HeaderHovered));

      if (strcmp(item.tooltip, "") != 0) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::BeginTooltip();
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::TextUnformatted(item.tooltip);
        ImGui::PopFont();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::EndTooltip();

        ImGui::PopStyleVar(2);
      }
    }

    if (item.type == ToolbarItemType::Toggle && item.toggleValue && *item.toggleValue) {
      dl->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(ImGuiCol_HeaderActive));
    }

    // icon centered
    ImVec2 textSize = ImGui::CalcTextSize(item.icon);
    ImVec2 textPos(
      pos.x + (h - textSize.x) * 0.5f,
      pos.y + (h - textSize.y) * 0.5f
    );

    dl->AddText(textPos, IM_COL32_WHITE, item.icon);

    if (clicked) {
      switch (item.type) {
        case ToolbarItemType::Button:
          if (item.onClick)
            item.onClick();
          break;

        case ToolbarItemType::Toggle:
          if (item.toggleValue)
            *item.toggleValue = !*item.toggleValue;
          break;

        case ToolbarItemType::Menu:
          ImGui::OpenPopup(item.id);
          break;
      }
    }

    if (item.type == ToolbarItemType::Menu) {
      if (ImGui::BeginPopup(item.id)) {
        if (item.onClick)
          item.onClick();

        ImGui::EndPopup();
      }
    }

    ImGui::SameLine(0.0f, 0.0f);
  }
}

void DrawImguiTopBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Sar")) {
      if (ImGui::MenuItem("Close Imgui")) {
        g_drawImgui = false;
      }

      for (auto& w : imguiWindows) {
        if (w.category == ImguiWindowCategory::Sar) {
          ImGui::MenuItem(w.name, nullptr, &w.open);
        }
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Hud")) {
      for (auto& h : g_imguiHuds) {
        if (ImGui::MenuItem(h->GetName(), nullptr, &h->Enabled())) {

        }
      }

      ImGui::Separator();

      for (auto& w : imguiWindows) {
        if (w.category == ImguiWindowCategory::Hud) {
          ImGui::MenuItem(w.name, nullptr, &w.open);
        }
      }
      ImGui::EndMenu();
    }

    DrawImguiTopbarRightSide();

    ImGui::EndMainMenuBar();
  }
}

void DrawImgui() {
  if (g_drawImgui || g_drawImguiHud) {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    int screenX, screenY;
    engine->GetScreenSize(engine, screenX, screenY);

    io.DisplaySize = ImVec2(screenX, screenY);

    int mouseX, mouseY;
    inputSystem->GetCursorPos(mouseX, mouseY);

    bool mouse1Down = inputSystem->IsKeyDown(MOUSE_LEFT);
    bool mouse2Down = inputSystem->IsKeyDown(MOUSE_RIGHT);
    bool mouse3Down = inputSystem->IsKeyDown(MOUSE_MIDDLE);

    bool scrollDown = inputSystem->IsKeyDown(MOUSE_WHEEL_DOWN);
    bool scrollUp = inputSystem->IsKeyDown(MOUSE_WHEEL_UP);

    float mDelta = 0.0f;
    if (scrollUp) mDelta += 1.0f;
    if (scrollDown) mDelta -= 1.0f;

    io.MousePos = ImVec2(mouseX, mouseY);
    io.MouseDown[0] = mouse1Down ? 1 : 0;
    io.MouseDown[1] = mouse2Down ? 1 : 0;
    io.MouseDown[2] = mouse3Down ? 1 : 0;
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

    ImGui::NewFrame();

    if (g_drawImgui) {
      if (g_imguiEditMode) {
        ImGui::GetBackgroundDrawList()->AddRectFilled(
          ImVec2(0, 0),
          ImGui::GetIO().DisplaySize,
          IM_COL32(20, 20, 20, 30)
        );
      }
    }

    if (g_drawImguiHud) {
      DrawImguiHuds();
    }

    if (g_drawImgui) {
      DrawImguiTopBar();
      DrawImguiWindows();
    }

    if ((g_drawImgui && engine->hoststate->m_activeGame && !pauseTimer->IsActive()) || (g_drawImgui && engine->demoplayer->IsPlaying())) { // if (engine->demoplayer->IsPlaying()) {
      ImGui::GetForegroundDrawList()->AddText(ImVec2(mouseX, mouseY), IM_COL32(255, 255, 255, 255), "\xef\x89\x85"); // Cursor (stupid hacky stupid)
      // ImGui::SetWindowFontScale(0.8f);
      // ImGui::GetForegroundDrawList()->AddText(ImVec2(mouseX + 1.5f, mouseY + 1.5f), IM_COL32(20, 20, 20, 255), "\xef\x89\x85");
      // ImGui::SetWindowFontScale(1.0f);
      // int size = 10;
      // Vertex_t verts[3];
      // verts[0] = {{mouseX, mouseY}};
      // verts[1] = {{mouseX + size, mouseY}};
      // verts[2] = {{mouseX, mouseY + size}};

      // surface->DrawSetTexture(surface->matsurface->ThisPtr(), -1);
      // surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, 255);
      // surface->DrawTexturedPolygon(surface->matsurface->ThisPtr(), 3, verts, true);
    }

    ImGui::Render();

    ImGui_RenderDrawData_Source(
      ImGui::GetDrawData()
    );
  }
}

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

        if (v0.pos.x < cx0 && v1.pos.x < cx0 && v2.pos.x < cx0) continue;
        if (v0.pos.x > cx1 && v1.pos.x > cx1 && v2.pos.x > cx1) continue;
        if (v0.pos.y < cy0 && v1.pos.y < cy0 && v2.pos.y < cy0) continue;
        if (v0.pos.y > cy1 && v1.pos.y > cy1 && v2.pos.y > cy1) continue;

        auto unpack = [](ImU32 c) -> Color {
          return Color{
            (uint8_t)((c >> 0) & 0xFF), // R
            (uint8_t)((c >> 8) & 0xFF), // G
            (uint8_t)((c >> 16) & 0xFF), // B
            (uint8_t)((c >> 24) & 0xFF), // A
          };
        };

        // Color col = unpack(v0.col); // Simplification. Only uses v0's col
        // // if (col.a == 0) continue;

        // // Makes some glyphs look better. Why? We may never know. And i don't
        // // want to
        // if (col.a == 0) {
        //   col = prevCol;
        // }

        // prevCol = col;
        Color c0 = unpack(v0.col);
        Color c1 = unpack(v1.col);
        Color c2 = unpack(v2.col);

        Color col;
        col.r = (uint8_t)((c0.r + c1.r + c2.r) / 3);
        col.g = (uint8_t)((c0.g + c1.g + c2.g) / 3);
        col.b = (uint8_t)((c0.b + c1.b + c2.b) / 3);
        col.a = (uint8_t)((c0.a + c1.a + c2.a) / 3);

        Vertex_t verts[3];
        verts[0] = {{v0.pos.x, v0.pos.y}, {v0.uv.x, v0.uv.y}};
        verts[1] = {{v1.pos.x, v1.pos.y}, {v1.uv.x, v1.uv.y}};
        verts[2] = {{v2.pos.x, v2.pos.y}, {v2.uv.x, v2.uv.y}};

        ImVec2 whiteUV = ImGui::GetDrawListSharedData()->TexUvWhitePixel;

        auto isWhitePixelTri = [&](const ImDrawVert& v0, const ImDrawVert& v1, const ImDrawVert& v2) {
          const float eps = 0.0001f;
          auto isNear = [&](ImVec2 uv) {
            return fabs(uv.x - whiteUV.x) < eps && fabs(uv.y - whiteUV.y) < eps;
          };
          return isNear(v0.uv) && isNear(v1.uv) && isNear(v2.uv);
        };

        bool isShape = isWhitePixelTri(v0, v1, v2);

        if (isShape || !g_drawWithFontAtlas) {
          // console->Print("SHAPE!!");
          // I don't know why, I don't want to know why, I shouldn't have to
          // wonder why, but for whatever reason if I set this to the white 
          // texture, everything looks fucked?! This is probably the same 
          // reason why the stupid borders look fucked (they use the texture
          // atlas). No texture works fine for now.
          // P.S. What the fuck does this do
          // surface->DrawSetTexture(surface->matsurface->ThisPtr(), -1);
        } else {
          int texId = (int)(intptr_t)cmd->GetTexID();
          surface->DrawSetTexture(surface->matsurface->ThisPtr(), texId);
        }

        surface->DrawSetColor(surface->matsurface->ThisPtr(), col.r, col.g, col.b, col.a);
        surface->DrawTexturedPolygon(surface->matsurface->ThisPtr(), 3, verts, true);

        // Debug lines
        // surface->DrawColoredLine({v0.pos.x, v0.pos.y}, {v1.pos.x, v1.pos.y}, col);
        // surface->DrawColoredLine({v1.pos.x, v1.pos.y}, {v2.pos.x, v2.pos.y}, col);
        // surface->DrawColoredLine({v2.pos.x, v2.pos.y}, {v0.pos.x, v0.pos.y}, col);

        // surface->DrawColoredLine({v0.pos.x, v0.pos.y}, {v1.pos.x, v1.pos.y}, {255, 255, 255, 100});
        // surface->DrawColoredLine({v1.pos.x, v1.pos.y}, {v2.pos.x, v2.pos.y}, {255, 255, 255, 100});
        // surface->DrawColoredLine({v2.pos.x, v2.pos.y}, {v0.pos.x, v0.pos.y}, {255, 255, 255, 100});
      }
    }
  }
}