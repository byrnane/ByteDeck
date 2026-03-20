#pragma once

#include "data/models.hpp"
#include "launch/launch_service.hpp"
#include "ui/screen.hpp"

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
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;

private:
    void move_selection(int delta);
    std::vector<const data::AppItem*> visible_apps() const;
    void launch_selected_app();

    std::filesystem::path root_path_;
    LaunchAppCallback launch_app_;
    std::vector<const data::AppItem*> apps_;
    std::size_t selected_index_ = 0;
    std::string launch_status_;
    bool launch_status_ok_ = false;
};
}
