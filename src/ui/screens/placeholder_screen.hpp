#pragma once

#include "ui/screen.hpp"

#include <string>

namespace bytedeck::ui
{
class PlaceholderScreen final : public Screen
{
public:
    explicit PlaceholderScreen(std::string title);

    ScreenAction handle_event(const SDL_Event& event) override;
    void render(SDL_Renderer& renderer) override;
    std::string window_title() const override;

private:
    std::string title_;
};
}
