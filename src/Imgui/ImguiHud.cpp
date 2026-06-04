#include "ImguiHud.hpp"

#include "Modules/VGui.hpp"
#include "Features/Hud/Hud.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"

#include <imgui.h>

// enum SnapAnchor {
//   None,

//   Edge_Top,
//   Edge_Bottom,
//   Edge_Left,
//   Edge_Right,

//   Center_Top,
//   Center_Middle,
//   Center_Bottom,

//   Center_Left,
//   Center_Middle_H,
//   Center_Right
// };


void DrawImguiHud() {
  // static SnapState snapState;

  // ImGuiWindowFlags flags = 0;
  // flags |= ImGuiWindowFlags_NoTitleBar;
  // flags |= ImGuiWindowFlags_NoResize;
  // flags |= ImGuiWindowFlags_AlwaysAutoResize;
  // flags |= ImGuiWindowFlags_NoCollapse;
  // flags |= ImGuiWindowFlags_NoScrollbar;
  // flags |= ImGuiWindowFlags_NoBackground;

  // if (!g_drawImgui) {
  //   flags |= ImGuiWindowFlags_NoMove;
  // }

  // ImGuiIO& io = ImGui::GetIO();
  // ImGuiViewport* vp = ImGui::GetMainViewport();

  // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  // ImGui::Begin("##Hud", nullptr, flags);

  // ImVec2 pos = ImGui::GetWindowPos();
  // ImVec2 size = ImGui::GetWindowSize();

  // ImVec2 center = vp->GetCenter();

  // static bool prevMoving = false;

  // if (moving) {
  //   ImDrawList* dl = ImGui::GetWindowDrawList();

  //   ImVec2 tl = vp->WorkPos;
  //   ImVec2 br = ImVec2(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y + vp->WorkSize.y);

  //   static float thickness = 5.0f;
  //   dl->AddRectFilled(ImVec2(center.x + thickness*0.5f, tl.y), ImVec2(center.x - thickness*0.5f, br.y), IM_COL32(255, 255, 255, 50));
  //   dl->AddRectFilled(ImVec2(tl.x, center.y + thickness*0.5f), ImVec2(br.x, center.y - thickness*0.5f), IM_COL32(255, 255, 255, 50));

  //   // X
  //   if (isInSnappingRange(pos.x, vp->WorkPos.x)) {
  //     snapState.desiredAnchorX = Edge_Left;
  //   }
  //   else if (isInSnappingRange(pos.x, vp->WorkPos.x + vp->WorkSize.x - size.x)) {
  //     snapState.desiredAnchorX = Edge_Right;
  //   }
  //   else if (isInSnappingRange(pos.x, center.x - size.x * 0.5f, size.x * 0.5f)) {
  //     snapState.desiredAnchorX = Center;
  //   }
  //   else if (isInSnappingRange(pos.x, center.x - size.x)) {
  //     snapState.desiredAnchorX = Center_Left;
  //   }
  //   else if (isInSnappingRange(pos.x, center.x)) {
  //     snapState.desiredAnchorX = Center_Right;
  //   }
  //   else {
  //     snapState.desiredAnchorX = NoneX;
  //   }

  //   // Y
  //   if (isInSnappingRange(pos.y, vp->WorkPos.y)) {
  //     snapState.desiredAnchorY = Edge_Top;
  //   }
  //   else if (isInSnappingRange(pos.y, vp->WorkPos.y + vp->WorkSize.y - size.y)) {
  //     snapState.desiredAnchorY = Edge_Bottom;
  //   }
  //   else if (isInSnappingRange(pos.y, center.y - size.y * 0.5f, size.y * 0.5f)) {
  //     snapState.desiredAnchorY = Middle;
  //   }
  //   else if (isInSnappingRange(pos.y, center.y - size.y)) {
  //     snapState.desiredAnchorY = Middle_Top;
  //   }
  //   else if (isInSnappingRange(pos.y, center.y)) {
  //     snapState.desiredAnchorY = Middle_Bottom;
  //   }
  //   else {
  //     snapState.desiredAnchorY = NoneY;
  //   }

  //   DrawDesiredHints(snapState, dl, vp, pos);
  // } else if (prevMoving) {
  //   snapState.anchorX = snapState.desiredAnchorX;
  //   snapState.anchorY = snapState.desiredAnchorY;
  // } else {
  //   ImGui::SetWindowPos(GetAnchorPos(snapState, vp, size, pos));
  // }

  // prevMoving = moving;

  // // Background
  // if (sar_hud_bg.GetBool() || g_drawImgui) {
  //   ImVec2 p0 = pos;
  //   ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);
  //   ImDrawList* dl = ImGui::GetWindowDrawList();

  //   dl->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 200));
  // }

  // // Position
  // auto player = client->GetPlayer(engine->GetLocalPlayerIndex());
  // if (player) {
  //   auto pos = client->GetAbsOrigin(player);
  //   int p = getPrecision();
  //   ImGui::Text("pos: %.*f %.*f %.*f", p, pos.x, p, pos.y, p, pos.z);
  // } else {
  //   ImGui::Text("pos: -");
  // }

  // ImGui::End();
  // ImGui::PopStyleVar();
}
