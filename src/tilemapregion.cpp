#include <cb/critterbits.hpp>
#include <SDL2_gfxPrimitives.h>

namespace Critterbits {
TilemapRegion::TilemapRegion() { this->debug = Engine::GetInstance().config->debug.draw_map_regions; }

void TilemapRegion::OnDebugRender(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip) {
    if (clip.z_index == ZIndex::Foreground) {
        rectangleRGBA(renderer, clip.dest.x, clip.dest.y, clip.dest.right(), clip.dest.bottom(), 127, 0, 127, 127);
        boxRGBA(renderer, clip.dest.x + 1, clip.dest.y + 1, clip.dest.right() - 1, clip.dest.bottom() - 1, 127, 127, 0,
                64);
    }
}
}