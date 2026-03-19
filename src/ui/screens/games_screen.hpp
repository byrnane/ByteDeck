#pragma once

#include "data/models.hpp"
#include "ui/screen.hpp"

#include <vector>

namespace bytedeck::ui
{
class GamesScreen final : public Screen
{
public:
    explicit GamesScreen(const data::LibraryData& library);

    ScreenAction handle_event(const SDL_Event& event) override;
    void render(SDL_Renderer& renderer) override;
    std::string window_title() const override;

private:
    void move_selection(int delta);

    std::vector<data::SystemEntry> entries_;
    std::size_t selected_index_ = 0;
};
}
