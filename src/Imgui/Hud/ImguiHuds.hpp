#pragma once

#include <vector>
#include <memory>
#include <utility>
#include "ImguiHudPanel.hpp"

#include "ShowposHud.hpp"
#include "ImguiInputHud.hpp"

extern std::vector<std::unique_ptr<ImguiHudPanel>> g_imguiHuds;

template<typename T, typename... Args>
T* AddHud(Args&&... args) {
  static_assert(std::is_base_of_v<ImguiHudPanel, T>);

  auto hud = std::make_unique<T>(std::forward<Args>(args)...);
  T* ptr = hud.get();

  g_imguiHuds.push_back(std::move(hud));

  return ptr;
}
