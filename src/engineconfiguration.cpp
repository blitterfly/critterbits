#include <string>

#include <critterbits.h>

namespace Critterbits {

/*
 * Support functions for EngineConfiguration::ReloadConfiguration()
 */
static void debug_draw_info_pane_parser(void * context, const std::string & value) {
    static_cast<EngineConfiguration *>(context)->debug.draw_info_pane = YamlParser::ToBool(value);
}

static void debug_draw_sprite_rects_parser(void * context, const std::string & value) {
    static_cast<EngineConfiguration *>(context)->debug.draw_sprite_rects = YamlParser::ToBool(value);
}

static void debug_draw_map_regions_parser(void * context, const std::string & value) {
    static_cast<EngineConfiguration *>(context)->debug.draw_map_regions = YamlParser::ToBool(value);
}

static void window_height_parser(void * context, const std::string & value) {
    static_cast<EngineConfiguration *>(context)->window_height = YamlParser::ToInt(value);
}

static void window_width_parser(void * context, const std::string & value) {
    static_cast<EngineConfiguration *>(context)->window_width = YamlParser::ToInt(value);
}

static void window_title_parser(void * context, const std::string & value) {
    static_cast<EngineConfiguration *>(context)->window_title = value;
}

static YamlValueParserCollection config_parsers = {{"draw_debug_pane", debug_draw_info_pane_parser},
                                                   {"draw_debug_sprite_rects", debug_draw_sprite_rects_parser},
                                                   {"draw_debug_map_regions", debug_draw_map_regions_parser},
                                                   {"window_height", window_height_parser},
                                                   {"window_title", window_title_parser},
                                                   {"window_width", window_width_parser}};
/*
 * End support functions
 */

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

    LOG_INFO("EngineConfiguration unexpanded source path: " + unexpanded_path);

    char * expanded_path = realpath(unexpanded_path.c_str(), NULL);
    if (expanded_path != NULL) {
        this->asset_path = std::string(expanded_path) + PATH_SEP;
        LOG_INFO("EngineConfiguration expanded source path: " + this->asset_path);
        free(expanded_path);
        this->ReloadConfiguration();
    }
}

bool EngineConfiguration::ReloadConfiguration() {
    this->valid = false;

    LOG_INFO("EngineConfiguration::ReloadConfiguration reading configuration from " + this->asset_path +
             CB_CONFIG_YAML);
    std::string * config_content = nullptr;
    if (ReadTextFile(this->asset_path + CB_CONFIG_YAML, &config_content)) {
        // parse YAML into configuration
        YamlParser parser;
        parser.value_parsers = config_parsers;
        parser.Parse(this, *config_content);

        delete config_content;
        this->valid = true;
    }

    return this->valid;
}
}