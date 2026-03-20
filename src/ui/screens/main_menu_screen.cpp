#include "ui/screens/main_menu_screen.hpp"

#include "ui/navigation_input.hpp"
#include "ui/ui_bindings.hpp"

namespace bytedeck::ui
{
namespace
{
std::string item_count_text(const platform::TranslationCatalog& translations, int count)
{
    return std::to_string(count) + " " + translations.translate("common.items_suffix");
}
}


MainMenuScreen::MainMenuScreen(const data::LibraryData& library, const platform::TranslationCatalog& translations)
    : library_(library)
    , translations_(translations)
    , items_ { "games", "settings", "apps" }
{
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
            return { ScreenActionType::open_settings };
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
    const std::vector<int> item_counts = {
        static_cast<int>(library_.games.size()),
        1,
        static_cast<int>(library_.apps.size())
    };
    UiBindings items = UiBindings::array();
    for (std::size_t index = 0; index < items_.size(); ++index)
    {
        const bool selected = index == selected_index_;
        std::string status = translations_.translate("common.empty");
        if (index == 1)
        {
            status = translations_.translate("settings.ready_status");
        }
        else if (item_counts[index] > 0)
        {
            status = item_count_text(translations_, item_counts[index]);
        }

        items.push_back({
            { "title", translations_.translate("menu." + items_[index]) },
            { "status", status },
            { "hint", selected ? translations_.translate("common.press_a") : translations_.translate("common.ready") },
            { "selected", selected }
        });
    }

    return UiBindings {
        { "brand", "BYTEDECK" },
        { "subtitle", translations_.translate("screen.main_menu") },
        { "items", items }
    };
}


std::string MainMenuScreen::window_title() const
{
    return translations_.translate("screen.main_menu") + " - " + translations_.translate("menu." + items_[selected_index_]);
}


void MainMenuScreen::move_selection(int delta)
{
    const int item_count = static_cast<int>(items_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count) % item_count);
}
}
