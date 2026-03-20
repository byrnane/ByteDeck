#pragma once

#include "data/models.hpp"
#include "ui/screen.hpp"

#include <string>
#include <vector>

namespace bytedeck::ui
{
class MainMenuScreen final : public Screen
{
public:
    explicit MainMenuScreen(const data::LibraryData& library);

    ScreenAction handle_event(const SDL_Event& event) override;
    std::string screen_id() const override;
    UiBindings build_bindings() const override;
    std::string window_title() const override;

private:
    void move_selection(int delta);

    std::vector<std::string> items_;
    std::vector<int> item_counts_;
    std::size_t selected_index_ = 0;
};
}
