#pragma once

#include <SDL.h>

#include <string>
#include <utility>

namespace bytedeck::ui
{
enum class ScreenActionType
{
    none,
    pop,
    quit,
    open_games,
    open_apps,
    open_placeholder,
    open_game_browser
};


struct ScreenAction
{
    ScreenActionType type = ScreenActionType::none;
    std::string value;

    ScreenAction() = default;

    ScreenAction(ScreenActionType action_type)
        : type(action_type)
    {
    }

    ScreenAction(ScreenActionType action_type, std::string action_value)
        : type(action_type)
        , value(std::move(action_value))
    {
    }
};


class Screen
{
public:
    virtual ~Screen() = default;

    virtual ScreenAction handle_event(const SDL_Event& event) = 0;
    virtual void render(SDL_Renderer& renderer) = 0;
    virtual std::string window_title() const = 0;
};
}
