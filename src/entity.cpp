#include <cb/critterbits.hpp>

namespace Critterbits {

void Entity::Render(SDL_Renderer * renderer, const CB_ViewClippingInfo & clip_rect) {
    if (this->IsActive()) {
        this->OnRender(renderer, clip_rect);
        if (this->debug) {
            this->OnDebugRender(renderer, clip_rect);
        }
    }
}

void Entity::Start() {
    if (!this->started) {
        if (this->OnStart()) {
            this->state = EntityState::Active;
            if (this->HasScript()) {
                this->script->CallStart(shared_from_this());
            }
            this->started = true;
        }
    }
}

void Entity::Update(float delta_time) {
    if (this->IsActive()) {
        if (this->HasScript()) {
            this->script->CallUpdate(shared_from_this(), delta_time * this->time_scale);
        }
        this->OnUpdate(delta_time * this->time_scale);
    }
}
}