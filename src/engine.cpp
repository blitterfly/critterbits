#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <critterbits.hpp>

namespace Critterbits {

entity_id_t next_entity_id = CB_ENTITY_ID_FIRST;

Engine::Engine() {
    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_SDL_ERR("Engine::Run SDL_Init");
        return;
    }
    if (!TestBitMask<int>(IMG_Init(IMG_INIT_PNG), IMG_INIT_PNG)) {
        LOG_SDL_ERR("Engine::Run IMG_Init");
        return;
    }

    this->initialized = true;
}

Engine::~Engine() {
    SDLx::SDL_CleanUp(this->renderer, this->window);
    IMG_Quit();
    SDL_Quit();
}

void Engine::DestroyMarkedEntities() {
    // copy all the entities to a vector first, as destruction will alter underlying collections
    std::vector<std::shared_ptr<Entity>> entities;
    this->IterateActiveEntities([&entities](std::shared_ptr<Entity> entity) {
        entities.push_back(entity);
        return false;
    });

    for (auto & entity : entities) {
        if (entity->destroyed) {
            switch (entity->GetEntityType()) {
                case EntityType::Sprite:
                    if (this->scenes.current_scene != nullptr) {
                        this->scenes.current_scene->sprites.UnloadSprite(std::dynamic_pointer_cast<Sprite>(entity));
                    }
                    break;
                case EntityType::Tilemap:
                case EntityType::Viewport:
                    // tilemaps and viewports should not be destroyed like this
                    LOG_ERR("Engine::DestroyMarkedEntities something marked a tilemap or viewport for destruction");
                    entity->destroyed = false;
                    break;
                default:
                    LOG_ERR("Engine::DestroyMarkedEntities unsupported entity type " +
                            std::to_string(static_cast<int>(entity->GetEntityType())));
                    entity->destroyed = false;
                    break;
            }
        }
    }
}

std::shared_ptr<Entity> Engine::FindEntityById(entity_id_t entity_id) {
    std::shared_ptr<Entity> ret{nullptr};
    this->IterateActiveEntities([&](std::shared_ptr<Entity> entity) {
        if (entity->entity_id == entity_id) {
            ret = entity;
            return true;
        }
        return false;
    });
    return ret;
}

std::vector<std::shared_ptr<Entity>> Engine::FindEntitiesByTag(const std::string & tag) {
    std::vector<std::shared_ptr<Entity>> entities;
    if (!tag.empty()) {
        this->IterateActiveEntities([&](std::shared_ptr<Entity> entity) {
            if (entity->tag == tag) {
                entities.push_back(entity);
            }
            return false;
        });
    }
    return entities;
}

Engine & Engine::GetInstance() {
    static Engine instance;
    return instance;
}


void Engine::IterateActiveEntities(EntityIterateFunction func) {
    if (this->scenes.IsCurrentSceneActive()) {
        if (this->scenes.current_scene->HasTilemap()) {
            if (func(this->scenes.current_scene->GetTilemap())) {
                return;
            }
            for (auto & region : this->scenes.current_scene->GetTilemap()->regions) {
                if (func(region)) {
                    return;
                }
            }
        }
        for (auto & sprite : this->scenes.current_scene->sprites.sprites) {
            if (func(sprite)) {
                return;
            }
        }
    }
    if (func(this->viewport)) {
        return;
    }
}

int Engine::Run() {
    LOG_INFO("Entering Engine::Run()");

    if (!this->initialized) {
        LOG_ERR("Engine::Run engine initialization was unsuccessful");
        return 1;
    }

    // check that we have valid configuration
    if (!this->config->IsValid()) {
        LOG_ERR("Engine::Run engine configuration is not valid");
        return 1;
    }

    // initialize controllers, if needed

    // discover display bounds
    SDL_GetDisplayBounds(0, &this->display_bounds);
    LOG_INFO("Engine::Run current display size " + std::to_string(this->display_bounds.w) + "x" +
             std::to_string(this->display_bounds.h));
    if (this->config->window.full_screen) {
        // create full screen window
        this->window = SDL_CreateWindow(this->config->window.title.c_str(), 0, 0, this->display_bounds.w,
                                        this->display_bounds.h, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
        this->config->window.width = this->display_bounds.w;
        this->config->window.height = this->display_bounds.h;
    } else {
        // create window of desired size in center of screen
        LOG_INFO("Engine::Run desired window size " + std::to_string(this->config->window.width) + "x" +
                 std::to_string(this->config->window.height));
        CB_Point window_origin = CB_Point::CenterInside(this->display_bounds.w, this->display_bounds.h,
                                                        this->config->window.width, this->config->window.height);

        this->window = SDL_CreateWindow(this->config->window.title.c_str(), window_origin.x, window_origin.y,
                                        this->config->window.width, this->config->window.height, SDL_WINDOW_SHOWN);
    }
    if (this->window == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateWindow");
        return 1;
    }

    // configure viewport
    this->viewport->dim = {0, 0, this->config->window.width, this->config->window.height};

    // create renderer
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderer == nullptr) {
        LOG_SDL_ERR("Engine::Run SDL_CreateRenderer");
        return 1;
    }

    // get some information on the renderer we just created
    SDL_RendererInfo r_info;
    if (SDL_GetRendererInfo(this->renderer, &r_info) != 0) {
        LOG_SDL_ERR("Engine::Run SDL_GetRendererInfo");
        return 1;
    }

    LOG_INFO("Engine::Run renderer name " + std::string(r_info.name));
    if (TestBitMask(r_info.flags, (unsigned int)SDL_RENDERER_SOFTWARE)) {
        LOG_ERR("Engine::Run no hardware renderer available");
        return 1;
    }

    this->max_texture_height = r_info.max_texture_height;
    this->max_texture_width = r_info.max_texture_width;
    LOG_INFO("Engine::Run renderer max_texture_width " + std::to_string(r_info.max_texture_width));
    LOG_INFO("Engine::Run renderer max_texture_height " + std::to_string(r_info.max_texture_height));

    // setup input methods
    this->input.SetControllerActive(this->config->input.controller);
    this->input.SetKeyboardActive(this->config->input.keyboard);
    this->input.SetMouseActive(this->config->input.mouse);

    // load first scene
    if (!this->scenes.LoadScene(CB_FIRST_SCENE)) {
        LOG_ERR("Engine::Run cannot load startup scene");
        return 1;
    }

    // start main loop
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        // timing update
        this->counters.NewFrame();

        // check for waiting SDL events
        while (SDL_PollEvent(&e)) {
            // If user closes the window
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            // InputManager will process the event if it's input-related
            this->input.AddSdlEvent(e);
        }

        // check input state (non-event)
        this->input.CheckInputs();

        // begin simulation loop
        while (this->counters.GetRemainingFrameTime() > 0) {
            float dt = this->counters.GetDeltaFromRemainingFrameTime();

            // Execute pre-update events
            EngineEventQueue::GetInstance().ExecutePreUpdate();

            // Update cycle
            this->IterateActiveEntities([dt](std::shared_ptr<Entity> entity) {
                // start entity if it hasn't already
                entity->Start();

                // call frame update methods
                entity->Update(dt);

                return false;
            });

            // resolve collision events
            EngineEventQueue::GetInstance().ExecuteCollision();

            // timing update
            this->counters.Updated();
        }

        // Render pass
        if (this->scenes.current_scene != nullptr && this->scenes.current_scene->HasTilemap()) {
            SDL_Color bg_color = this->scenes.current_scene->GetTilemap()->bg_color;
            SDL_SetRenderDrawColor(this->renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        } else {
            SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
        }
        SDL_RenderClear(this->renderer);
        for (auto z_index : {ZIndex::Background, ZIndex::Midground, ZIndex::Foreground}) {
            this->IterateActiveEntities([this, z_index](std::shared_ptr<Entity> entity) {
                if (entity->dim.intersects(this->viewport->dim)) {
                    CB_ViewClippingInfo clip{this->viewport->GetViewableRect(entity->dim)};
                    clip.z_index = z_index;
                    entity->Render(this->renderer, clip);
                    if (z_index == ZIndex::Background) {
                        // guard is to make sure we only count each entity rendered once per frame
                        this->counters.RenderedEntity();
                    }
                }
                if (z_index == ZIndex::Background) {
                    this->counters.CountedEntity();
                }
                return false;
            });
        }

        if (this->config->debug.draw_info_pane) {
            this->RenderDebugPane(this->counters.GetTotalEntitiesCount());
        }
        SDL_RenderPresent(this->renderer);

        // Clean up entities that were marked for deletion
        this->DestroyMarkedEntities();
    }

    LOG_INFO("Exiting Engine::Run()");
    return 0;
}

void Engine::RenderDebugPane(int entity_count) {
    std::stringbuf info;
    std::ostream os(&info);

    os << "view" << this->viewport->dim.xy().to_string();
    os << " ent(" << this->counters.GetRenderedEntitiesCount() << "/" << entity_count << ")";
    os << " fps " << std::fixed << std::setprecision(1) << this->counters.GetAverageFps();

    roundedBoxRGBA(this->renderer, -6, this->viewport->dim.h - 12, info.str().length() * 8 + 10,
                   this->viewport->dim.h + 6, 6, 0, 0, 0, 127);
    stringRGBA(this->renderer, 2, this->viewport->dim.h - 10, info.str().c_str(), 255, 255, 255, 255);
}

void Engine::SetConfiguration(std::shared_ptr<EngineConfiguration> config) { this->config = std::move(config); }
}