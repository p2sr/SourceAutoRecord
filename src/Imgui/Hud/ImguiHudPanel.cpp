#include "ImguiHudPanel.hpp"

#include "Modules/VGui.hpp"

#include <imgui.h>

bool g_imguiEditMode = false;

void DrawDesiredHints(SnapState snapState, ImDrawList* dl, ImGuiViewport* vp, ImVec2 windowPos) {
  ImVec2 tl = vp->WorkPos;
  ImVec2 br = ImVec2(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);

  ImVec2 center = vp->GetCenter();

  float size = 40.0f;

  float x = 0.0f;
  float y = 0.0f;

  switch (snapState.desiredAnchorX) {
  case NoneX: x = windowPos.x; break;
  case Edge_Left: x = vp->WorkPos.x; break;
  case Center_Left: x = center.x - size; break;
  case Center: x = center.x - size * 0.5f; break;
  case Center_Right: x = center.x; break;
  case Edge_Right: x = vp->WorkPos.x + vp->WorkSize.x - size; break;
  }

  switch (snapState.desiredAnchorY) {
  case NoneY: y = windowPos.y; break;
  case Edge_Top: y = vp->WorkPos.y; break;
  case Middle_Top: y = center.y - size; break;
  case Middle: y = center.y - size * 0.5f; break;
  case Middle_Bottom: y = center.y; break;
  case Edge_Bottom: y = vp->WorkPos.y + vp->WorkSize.y - size; break;
  }

  if (snapState.desiredAnchorX != NoneX || snapState.desiredAnchorY != NoneY) {
    dl->AddRectFilled(ImVec2(x, y), ImVec2(x + size, y + size), ImGui::GetColorU32(ImGuiCol_Button));
  }
}

bool isInSnappingRange(float v, float target, float snapDist = 20.0f) {
  return (fabsf(v - target) < snapDist);
}

ImVec2 GetAnchorPos(SnapState snapState, ImGuiViewport* vp, ImVec2 size, ImVec2 windowPos) {
  ImVec2 c = vp->GetCenter();

  auto anchorX = snapState.anchorX;
  auto anchorY = snapState.anchorY;

  float x = 0.0f;
  float y = 0.0f;

  switch (anchorX) {
  case NoneX: x = windowPos.x; break;
  case Edge_Left: x = vp->WorkPos.x; break;
  case Center_Left: x = c.x - size.x; break;
  case Center: x = c.x - size.x * 0.5f; break;
  case Center_Right: x = c.x; break;
  case Edge_Right: x = vp->WorkPos.x + vp->WorkSize.x - size.x; break;
  }

  switch (anchorY) {
  case NoneY: y = windowPos.y; break;
  case Edge_Top: y = vp->WorkPos.y; break;
  case Middle_Top: y = c.y - size.y; break;
  case Middle: y = c.y - size.y * 0.5f; break;
  case Middle_Bottom: y = c.y; break;
  case Edge_Bottom: y = vp->WorkPos.y + vp->WorkSize.y - size.y; break;
  }

  return ImVec2(x, y);
}

void ImguiHudPanel::Render() {
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoScrollbar;
  flags |= ImGuiWindowFlags_NoBackground;

  if (!g_drawImgui || !g_imguiEditMode || mLocked) {
    flags |= ImGuiWindowFlags_NoMove;
  }

  ImGuiIO& io = ImGui::GetIO();
  ImVec2 mouse = io.MousePos;
  ImGuiViewport* vp = ImGui::GetMainViewport();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  if (!mEnabled || !ShouldDraw()) {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
  }

  ImGui::Begin(GetName(), nullptr, flags);

  DrawContent();

  ImVec2 pos = ImGui::GetWindowPos();
  ImVec2 size = ImGui::GetWindowSize();

  if (g_imguiEditMode && g_drawImgui && (!ShouldDraw() || ImGui::IsWindowHovered())) {
    ImVec2 p0 = pos;
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(p0, p1, IM_COL32(186, 186, 186, 30));
  }

  ImVec2 center = vp->GetCenter();

  // Lol
  bool moving = ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && ImGui::IsWindowFocused() && g_drawImgui && g_imguiEditMode && !mLocked;
  bool rightClicked = ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && g_drawImgui && g_imguiEditMode;

  if (rightClicked) {
    ImGui::OpenPopup("HudContextMenu");
  }

  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
  if (ImGui::BeginPopup("HudContextMenu")) {
    ContextMenu();

    if (mContextMenuDefined) ImGui::Separator();

    ImGui::Checkbox("Lock position", &mLocked);

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();

  if (moving) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    char buf[64];
    snprintf(buf, sizeof(buf), "X: %.0f, Y: %.0f", pos.x, pos.y);
    ImVec2 textSize = ImGui::CalcTextSize(buf);

    ImVec2 p0 = ImVec2(mouse.x + 12.0f, mouse.y + 12.0f);

    ImFont* defaultFont = io.Fonts->Fonts[0];

    ImGui::GetForegroundDrawList()->AddText(defaultFont, 12.0f, p0, IM_COL32(255, 255, 255, 255), buf);

    ImVec2 tl = vp->WorkPos;
    ImVec2 br = ImVec2(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);

    static float thickness = 2.0f;
    dl->AddRectFilled(ImVec2(center.x + thickness*0.5f, tl.y), ImVec2(center.x - thickness*0.5f, br.y), IM_COL32(255, 255, 255, 255));
    dl->AddRectFilled(ImVec2(tl.x, center.y + thickness*0.5f), ImVec2(br.x, center.y - thickness*0.5f), IM_COL32(255, 255, 255, 255));

    // X
    if (isInSnappingRange(pos.x, vp->WorkPos.x)) {
      mSnapState.desiredAnchorX = Edge_Left;
    }
    else if (isInSnappingRange(pos.x, vp->WorkPos.x + vp->WorkSize.x - size.x)) {
      mSnapState.desiredAnchorX = Edge_Right;
    }
    else if (isInSnappingRange(pos.x, center.x - size.x * 0.5f, size.x * 0.5f)) {
      mSnapState.desiredAnchorX = Center;
    }
    else if (isInSnappingRange(pos.x, center.x - size.x)) {
      mSnapState.desiredAnchorX = Center_Left;
    }
    else if (isInSnappingRange(pos.x, center.x)) {
      mSnapState.desiredAnchorX = Center_Right;
    }
    else {
      mSnapState.desiredAnchorX = NoneX;
    }

    // Y
    if (isInSnappingRange(pos.y, vp->WorkPos.y)) {
      mSnapState.desiredAnchorY = Edge_Top;
    }
    else if (isInSnappingRange(pos.y, vp->WorkPos.y + vp->WorkSize.y - size.y)) {
      mSnapState.desiredAnchorY = Edge_Bottom;
    }
    else if (isInSnappingRange(pos.y, center.y - size.y * 0.5f, size.y * 0.5f)) {
      mSnapState.desiredAnchorY = Middle;
    }
    else if (isInSnappingRange(pos.y, center.y - size.y)) {
      mSnapState.desiredAnchorY = Middle_Top;
    }
    else if (isInSnappingRange(pos.y, center.y)) {
      mSnapState.desiredAnchorY = Middle_Bottom;
    }
    else {
      mSnapState.desiredAnchorY = NoneY;
    }


    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
    DrawDesiredHints(mSnapState, dl, vp, pos);
    ImGui::PopStyleVar();
  } else if (mPrevMoving) {
    mSnapState.anchorX = mSnapState.desiredAnchorX;
    mSnapState.anchorY = mSnapState.desiredAnchorY;
  } else {
    ImGui::SetWindowPos(GetAnchorPos(mSnapState, vp, size, pos));
  }

  mPrevMoving = moving;

  if (mLocked && g_drawImgui && g_imguiEditMode) {
    ImGui::GetForegroundDrawList()->AddText(ImVec2(pos.x + 2.0f, pos.y + 2.0f), IM_COL32(255, 255, 255, 255), "\xef\x80\xa3"); // LOCK
  } else {
    // ImGui::GetForegroundDrawList()->AddText(ImVec2(pos.x + 2.0f, pos.y + 2.0f), IM_COL32(255, 255, 255, 255), "\xef\x82\x9c"); // UNLOCK
    if (ImGui::IsWindowHovered() && !moving && g_imguiEditMode && g_drawImgui) {
      ImDrawList* dl = ImGui::GetForegroundDrawList();
      ImVec2 p = ImVec2(pos.x + size.x * 0.5f - 8.0f, pos.y + size.y * 0.5f - 8.0f);
      const char* txt = "\xef\x81\x87";
      dl->AddText(ImVec2(p.x + 1, p.y + 1), IM_COL32(0,0,0,120), txt);
      dl->AddText(ImVec2(p.x - 1, p.y + 1), IM_COL32(0,0,0,80), txt);
      dl->AddText(ImVec2(p.x + 1, p.y - 1), IM_COL32(0,0,0,80), txt);
      dl->AddText(p, IM_COL32(255,255,255,255), txt);
      // ImGui::GetForegroundDrawList()->AddText(ImVec2(pos.x + size.x * 0.5f - 8.0f, pos.y + size.y * 0.5f - 8.0f), IM_COL32(255, 255, 255, 255), "\xef\x81\x87"); // Arrows
    }
  }

  ImGui::End();
  if (!mEnabled || !ShouldDraw()) {
    ImGui::PopStyleVar(2);
  } else {
    ImGui::PopStyleVar();
  }
}

void ImguiHudPanel::ContextMenu() {
  mContextMenuDefined = false;
}
