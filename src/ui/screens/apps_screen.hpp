#pragma once

#include "data/models.hpp"
#include "launch/launch_service.hpp"
#include "ui/screen.hpp"
#include "ui/image_texture.hpp"

#include <filesystem>
#include <functional>
#include <vector>

namespace bytedeck::ui
{
class AppsScreen final : public Screen
{
public:
    using LaunchAppCallback = std::function<launch::LaunchResult(const data::AppItem&)>;

    AppsScreen(const data::LibraryData& library, std::filesystem::path root_path, LaunchAppCallback launch_app);

    ScreenAction handle_event(const SDL_Event& event) override;
    void render(SDL_Renderer& renderer) override;
    std::string window_title() const override;

private:
    void move_selection(int delta);
    void ensure_preview_texture(SDL_Renderer& renderer);
    void launch_selected_app();

    std::filesystem::path root_path_;
    LaunchAppCallback launch_app_;
    std::vector<const data::AppItem*> apps_;
    std::size_t selected_index_ = 0;
    ImageTexture preview_texture_;
    std::string loaded_icon_path_;
    std::string launch_status_;
    bool launch_status_ok_ = false;
};
}
