#pragma once

extern bool g_imguiEditMode;

enum SnapAnchorX {
  NoneX,

  Edge_Left,
  
  Center_Left,
  Center,
  Center_Right,

  Edge_Right,
};

enum SnapAnchorY {
  NoneY,

  Edge_Top,

  Middle_Top,
  Middle,
  Middle_Bottom,

  Edge_Bottom,
};

struct SnapState {
  SnapAnchorX anchorX = NoneX;
  SnapAnchorY anchorY = NoneY;

  SnapAnchorX desiredAnchorX = NoneX;
  SnapAnchorY desiredAnchorY = NoneY;
};

// This is for in game panels (such as showpos replacement, toasts, etc.)
class ImguiHudPanel {
public:
  virtual ~ImguiHudPanel() = default;

  virtual void DrawContent() = 0;
  virtual void ContextMenu();

  virtual void Render();

  inline virtual bool ShouldDraw() { return true; }

  // Helpers
  const bool Enabled() const { return mEnabled; }
  bool& Enabled() { return mEnabled; }
  void Toggle() { mEnabled = !mEnabled; }

  virtual const char* GetName() const = 0;
  virtual const char* GetHandle() const = 0;

protected:
  SnapState mSnapState;

private:
  bool mEnabled = false;
  bool mPrevMoving = false;
  bool mLocked = false;

  bool mContextMenuDefined = true;
};
