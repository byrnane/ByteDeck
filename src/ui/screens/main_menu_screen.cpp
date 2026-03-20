#include "ui/screens/main_menu_screen.hpp"

#include "ui/navigation_input.hpp"
#include "ui/ui_bindings.hpp"

namespace bytedeck::ui
{
MainMenuScreen::MainMenuScreen(const data::LibraryData& library)
    : items_ { "Games", "Settings", "Apps" }
{
    item_counts_.push_back(static_cast<int>(library.games.size()));
    item_counts_.push_back(0);
    item_counts_.push_back(static_cast<int>(library.apps.size()));
}


ScreenAction MainMenuScreen::handle_event(const SDL_Event& event)
{
    switch (navigation_input_from_event(event))
    {
    case NavigationInput::left:
    case NavigationInput::up:
        move_selection(-1);
        return {};
    case NavigationInput::right:
    case NavigationInput::down:
        move_selection(1);
        return {};
    case NavigationInput::accept:
        if (selected_index_ == 0)
        {
            return { ScreenActionType::open_games };
        }
        if (selected_index_ == 1)
        {
            return { ScreenActionType::open_settings_stub };
        }
        return { ScreenActionType::open_apps };
    case NavigationInput::back:
    case NavigationInput::quit:
        return { ScreenActionType::quit };
    case NavigationInput::none:
    default:
        return {};
    }
}


std::string MainMenuScreen::screen_id() const
{
    return "main_menu";
}


UiBindings MainMenuScreen::build_bindings() const
{
    UiBindings items = UiBindings::array();
    for (std::size_t index = 0; index < items_.size(); ++index)
    {
        const bool selected = index == selected_index_;
        std::string status = "EMPTY";
        if (item_counts_[index] > 0)
        {
            status = std::to_string(item_counts_[index]) + " ITEMS";
        }
        else if (index == 1)
        {
            status = "STUB";
        }

        items.push_back({
            { "title", items_[index] },
            { "status", status },
            { "hint", selected ? "PRESS A" : "READY" },
            { "selected", selected }
        });
    }

    return UiBindings {
        { "brand", "BYTEDECK" },
        { "subtitle", "MAIN MENU" },
        { "items", items }
    };
}


std::string MainMenuScreen::window_title() const
{
    return "Main Menu - " + items_[selected_index_];
}


void MainMenuScreen::move_selection(int delta)
{
    const int item_count = static_cast<int>(items_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count) % item_count);
}
}
