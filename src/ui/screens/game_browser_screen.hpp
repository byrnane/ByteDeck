#pragma once

#include "data/models.hpp"
#include "ui/screen.hpp"
#include "ui/image_texture.hpp"

#include <filesystem>
#include <vector>

namespace bytedeck::ui
{
class GameBrowserScreen final : public Screen
{
public:
    GameBrowserScreen(const data::LibraryData& library, std::filesystem::path root_path, std::string system_id);

    ScreenAction handle_event(const SDL_Event& event) override;
    void render(SDL_Renderer& renderer) override;
    std::string window_title() const override;

private:
    void move_selection(int delta);
    std::string system_name() const;
    void ensure_preview_texture(SDL_Renderer& renderer);

    std::filesystem::path root_path_;
    std::string system_id_;
    std::vector<const data::GameItem*> games_;
    std::size_t selected_index_ = 0;
    ImageTexture preview_texture_;
    std::string loaded_thumbnail_path_;
};
}
