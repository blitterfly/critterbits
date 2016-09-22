#include <string>
#include <limits.h>
#include <stdlib.h>

#include <cb/critterbits.hpp>

namespace Critterbits {

EngineConfiguration::EngineConfiguration(const std::string & source_path) {
    char * base_path = SDL_GetBasePath();

    std::string unexpanded_path;
    if (source_path.empty() || source_path[0] == '.' || source_path[0] != PATH_SEP) {
        // relative path
        unexpanded_path = std::string(base_path) + source_path;
    } else {
        // absolute path
        unexpanded_path = source_path;
    }

    SDL_free(base_path);

    LOG_INFO("EngineConfiguration unexpanded source path: " + unexpanded_path);

    std::string expanded_path = this->GetExpandedPath(unexpanded_path);
    if (!expanded_path.empty()) {
        this->asset_path = std::string(expanded_path) + PATH_SEP;
        LOG_INFO("EngineConfiguration expanded source path: " + this->asset_path);
        BaseResourcePath res_path;
        res_path.base_path = this->asset_path;
        res_path.source = ResourceSource::File;
        this->loader = ResourceLoader::GetResourceLoader(res_path);
        this->ReloadConfiguration();
    }
}

std::string EngineConfiguration::GetExpandedPath(const std::string & unexpanded_path) {
#ifdef _WIN32
    char * expanded_path = _fullpath(NULL, unexpanded_path.c_str(), _MAX_PATH);
#else
    char * expanded_path = realpath(unexpanded_path.c_str(), NULL);
#endif
    if (expanded_path != nullptr) {
        std::string n_expanded_path{expanded_path};
        delete expanded_path;
        return n_expanded_path;
    } else {
        return std::string{};
    }
}

bool EngineConfiguration::ReloadConfiguration() {
    LOG_INFO("EngineConfiguration::ReloadConfiguration reading configuration from " + this->asset_path + CB_CONFIG_FILE);
    try {
        auto config_res = this->loader->OpenTextResource(CB_CONFIG_FILE);
        auto config = Toml::TomlParser{config_res};
        if (config.IsReady()) {
            this->valid = false;

            // debug settings
            this->debug.draw_gui_rects = config.GetTableBool("debug.draw_gui_rects", this->debug.draw_gui_rects);
            this->debug.draw_info_pane = config.GetTableBool("debug.draw_info_pane", this->debug.draw_info_pane);
            this->debug.draw_map_regions = config.GetTableBool("debug.draw_map_regions", this->debug.draw_map_regions);
            this->debug.draw_sprite_rects = config.GetTableBool("debug.draw_sprite_rects", this->debug.draw_sprite_rects);

            // input
            this->input.controller = config.GetTableBool("input.controller", this->input.controller);
            this->input.keyboard = config.GetTableBool("input.keyboard", this->input.keyboard);
            this->input.mouse = config.GetTableBool("input.mouse", this->input.mouse);

            // window settings
            this->window.full_screen = config.GetTableBool("window.full_screen", this->window.full_screen);
            this->window.height = config.GetTableInt("window.height", this->window.height);
            this->window.title = config.GetTableString("window.title", this->window.title);
            this->window.width = config.GetTableInt("window.width", this->window.width);

            // fonts
            config.IterateTableArray("font", [](const Toml::TomlParser & table) {
                FontManager::GetInstance().RegisterNamedFont(
                    table.GetTableString("name"),
                    table.GetTableString("file"),
                    table.GetTableInt("size")
                );
            });

            this->Validate();
        }
    } catch (cpptoml::parse_exception & e) {
        LOG_ERR("EngineConfiguration::ReloadConfiguration TOML parsing error " + std::string(e.what()));
    }

    return this->valid;
}

bool EngineConfiguration::Validate() {
    bool b_valid = true;

    if (this->window.width < 100 || this->window.height < 100) {
        b_valid = false;
    }

    if (!(this->input.controller || this->input.keyboard || this->input.mouse)) {
        b_valid = false;
    }

    // TODO: validate more settings ...

    this->valid = b_valid;
    return b_valid;
}
}