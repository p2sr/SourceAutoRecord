#include "Hud.hpp"
#include <vector>

class ToastHud : public Hud
{
public:
    ToastHud();
    bool ShouldDraw() override;
    bool GetCurrentSize(int& xSize, int& ySize) override;
    void AddToast(std::string text, Color color, double duration, bool doConsole = true);
    void Update();
    void Paint(int slot) override;
};

extern ToastHud toastHud;
