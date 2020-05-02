#include "VGui.hpp"

#include <algorithm>

#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"

#include "SAR.hpp"

REDECL(VGui::Paint);

void VGui::Draw(Hud* const& hud)
{
    if (hud->ShouldDraw()) {
        hud->Paint(this->context.slot);
    }
}
void VGui::Draw(HudElement* const& element)
{
    if (element->ShouldDraw()) {
        element->Paint(&this->context);
    }
}

// CEngineVGui::Paint
DETOUR(VGui::Paint, PaintMode_t mode)
{
    auto result = VGui::Paint(thisptr, mode);

    surface->StartDrawing(surface->matsurface->ThisPtr());

    auto ctx = &vgui->context;

    ctx->Reset(GET_SLOT());

    if (ctx->slot == 0) {
        if (mode & PAINT_UIPANELS) {
            for (auto const& hud : vgui->huds) {
                vgui->Draw(hud);
            }

            for (auto const& element : vgui->elements) {
                vgui->Draw(element);
            }
        }
    } else if (ctx->slot == 1) {
        for (auto const& hud : vgui->huds) {
            if (hud->drawSecondSplitScreen) {
                vgui->Draw(hud);
            }
        }

        for (auto const& element : vgui->elements) {
            if (element->drawSecondSplitScreen) {
                vgui->Draw(element);
            }
        }
    }

    surface->FinishDrawing();

    return result;
}

bool VGui::IsUIVisible(){
    return this->IsGameUIVisible(this->enginevgui->ThisPtr());
}

bool VGui::Init()
{
    this->enginevgui = Interface::Create(this->Name(), "VEngineVGui0");
    if (this->enginevgui) {
        this->IsGameUIVisible = this->enginevgui->Original<_IsGameUIVisible>(Offsets::IsGameUIVisible);

        this->enginevgui->Hook(VGui::Paint_Hook, VGui::Paint, Offsets::Paint);

        for (auto& hud : Hud::GetList()) {
            if (hud->version == SourceGame_Unknown || sar.game->Is(hud->version)) {
                this->huds.push_back(hud);
            }
        }

        HudElement::IndexAll();

        for (auto const& element : HudElement::GetList()) {
            if (element->version == SourceGame_Unknown || sar.game->Is(element->version)) {
                this->elements.push_back(element);
            }
        }

        std::sort(this->elements.begin(), this->elements.end(), [](const HudElement* a, const HudElement* b) {
            return a->orderIndex < b->orderIndex;
        });
    }

    return this->hasLoaded = this->enginevgui;
}
void VGui::Shutdown()
{
    Interface::Delete(this->enginevgui);
    this->huds.clear();
}

VGui* vgui;
