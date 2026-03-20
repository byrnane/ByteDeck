#include "ui/screens/placeholder_screen.hpp"

#include "ui/navigation_input.hpp"
#include <utility>

namespace bytedeck::ui
{

PlaceholderScreen::PlaceholderScreen(std::string title)
    : title_(std::move(title))
{
}


ScreenAction PlaceholderScreen::handle_event(const SDL_Event& event)
{
    const NavigationInput input = navigation_input_from_event(event);
    if (input == NavigationInput::back)
    {
        return { ScreenActionType::pop };
    }

    if (input == NavigationInput::quit)
    {
        return { ScreenActionType::quit };
    }

    return {};
}


std::string PlaceholderScreen::screen_id() const
{
    return "placeholder";
}


UiBindings PlaceholderScreen::build_bindings() const
{
    return UiBindings {
        { "title", title_ },
        { "subtitle", "SCREEN STUB" },
        { "body", "PRESS B OR ESC TO GO BACK" }
    };
}


std::string PlaceholderScreen::window_title() const
{
    return title_;
}
}
