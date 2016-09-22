#include <cassert>
#include <fstream>
#include <memory>
#include <sys/stat.h>

#include <cb/critterbits.hpp>
#include <SDL_image.h>
#include <SDL_ttf.h>

namespace Critterbits {
std::shared_ptr<ResourceLoader> ResourceLoader::GetResourceLoader(const BaseResourcePath & base_path) {
    if (base_path.source == ResourceSource::File) {
        return std::make_shared<FileResourceLoader>(base_path);
    } else {
        assert(false);
        // TODO
        return nullptr;
    }
}

std::string ResourceLoader::StripAssetNameFromPath(const std::string & asset_path) {
    int index_a = asset_path.find_last_of('\\');
    int index_b = asset_path.find_last_of('/');
    return asset_path.substr(0, std::max(index_a, index_b));
}

std::shared_ptr<TTF_Font> FileResourceLoader::GetFontResource(const std::string & asset_path, int pt_size) const {
    std::string font_path = this->res_path.base_path + asset_path;
    TTF_Font * font = TTF_OpenFont(font_path.c_str(), pt_size);
    if (font == nullptr) {
        LOG_SDL_ERR("FileResourceLoader::GetFontResource unable to load font " + font_path);
        return nullptr;
    }
    // FIXME: deleter is currently commented out because TTF_CloseFont is causing segmentation faults in libfreetype
    std::shared_ptr<TTF_Font> font_ptr{font, [](TTF_Font * font) { /*if (font != nullptr) { TTF_CloseFont(font); }*/ }};
    return std::move(font_ptr);
}

std::shared_ptr<SDL_Texture> FileResourceLoader::GetImageResource(const std::string & asset_path) const {
    std::string texture_path = this->res_path.base_path + asset_path;
    SDL_Texture * texture = IMG_LoadTexture(Engine::GetInstance().GetRenderer(), texture_path.c_str());
    if (texture == nullptr) {
        LOG_SDL_ERR("FileResourceLoader::GetImageResource unable to load image to texture " + texture_path);
        return nullptr;
    }
    std::shared_ptr<SDL_Texture> texture_ptr{texture, [](SDL_Texture * texture) { SDLx::SDL_CleanUp(texture); }};
    return std::move(texture_ptr);
}

bool FileResourceLoader::GetTextResourceContents(const std::string & asset_path, std::string ** text_content) const {
    std::ifstream ifs;

    ifs.open(this->res_path.base_path + asset_path, std::ifstream::in);
    if (ifs.good()) {
        *text_content = new std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        return true;
    }
    return false;
}

std::shared_ptr<std::istream> FileResourceLoader::OpenTextResource(const std::string & asset_path) const {
    std::shared_ptr<std::ifstream> ifs = std::make_shared<std::ifstream>();
    ifs->open(this->res_path.base_path + asset_path, std::ifstream::in);
    if (!ifs->good()) {
        LOG_ERR("FileResourceLoader::OpenTextResource unable to read from text resource " + asset_path);
    }
    return std::move(ifs);
}

bool FileResourceLoader::ResourceExists(const std::string & asset_path) const {
    std::string file_path{this->res_path.base_path + asset_path};
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}
}