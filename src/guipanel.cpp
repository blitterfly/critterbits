#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {
namespace Gui {

void GuiPanel::OnRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {

    }
}

void GuiPanel::OnDebugRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_info) {
    if (clip_info.z_index == ZIndex::Gui) {
        rectangleRGBA(renderer, clip_info.dest.x, clip_info.dest.y, clip_info.dest.right(), clip_info.dest.bottom(),
                        127, 0, 127, 127);
        std::string label = this->panel_name + ":" + std::to_string(this->entity_id);
        boxRGBA(renderer, clip_info.dest.x, clip_info.dest.bottom(), clip_info.dest.x + label.length() * 8 + 2,
                clip_info.dest.bottom() + 10, 127, 0, 127, 127);
        stringRGBA(renderer, clip_info.dest.x + 1, clip_info.dest.bottom() + 1, label.c_str(), 255, 255, 255,
                    255);
    }
}

}
}