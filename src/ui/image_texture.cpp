#include "ui/image_texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>

namespace bytedeck::ui
{
ImageTexture::~ImageTexture()
{
    reset();
}


bool ImageTexture::load(SDL_Renderer& renderer, const std::filesystem::path& path)
{
    reset();

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
    if (pixels == nullptr)
    {
        return false;
    }

    texture_ = SDL_CreateTexture(&renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    if (texture_ == nullptr)
    {
        stbi_image_free(pixels);
        return false;
    }

    SDL_UpdateTexture(texture_, nullptr, pixels, width * 4);
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);

    stbi_image_free(pixels);

    width_ = width;
    height_ = height;
    return true;
}


void ImageTexture::reset()
{
    if (texture_ != nullptr)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    width_ = 0;
    height_ = 0;
}


bool ImageTexture::is_loaded() const
{
    return texture_ != nullptr;
}


void ImageTexture::render(SDL_Renderer& renderer, const SDL_Rect& bounds) const
{
    if (texture_ == nullptr || width_ <= 0 || height_ <= 0)
    {
        return;
    }

    const float scale_x = static_cast<float>(bounds.w) / static_cast<float>(width_);
    const float scale_y = static_cast<float>(bounds.h) / static_cast<float>(height_);
    const float scale = std::min(scale_x, scale_y);

    const int render_width = static_cast<int>(width_ * scale);
    const int render_height = static_cast<int>(height_ * scale);
    SDL_Rect destination {
        bounds.x + (bounds.w - render_width) / 2,
        bounds.y + (bounds.h - render_height) / 2,
        render_width,
        render_height
    };

    SDL_RenderCopy(&renderer, texture_, nullptr, &destination);
}
}
