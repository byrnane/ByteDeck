#pragma once

#include <SDL.h>

#include <filesystem>

namespace bytedeck::ui
{
class ImageTexture
{
public:
    ImageTexture() = default;
    ~ImageTexture();

    ImageTexture(const ImageTexture&) = delete;
    ImageTexture& operator=(const ImageTexture&) = delete;

    bool load(SDL_Renderer& renderer, const std::filesystem::path& path);
    void reset();
    bool is_loaded() const;
    void render(SDL_Renderer& renderer, const SDL_Rect& bounds) const;

private:
    SDL_Texture* texture_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};
}
