#include "ui/screens/games_screen.hpp"

#include "ui/navigation_input.hpp"
namespace bytedeck::ui
{
namespace
{
constexpr int kColumns = 2;
}


GamesScreen::GamesScreen(const data::LibraryData& library, const platform::TranslationCatalog& translations)
    : translations_(translations)
{
    for (const data::SystemEntry& entry : library.systems)
    {
        if (entry.visible && (entry.type == "game_system" || entry.type == "collection_group"))
        {
            entries_.push_back(entry);
        }
    }
}


ScreenAction GamesScreen::handle_event(const SDL_Event& event)
{
    switch (navigation_input_from_event(event))
    {
    case NavigationInput::left:
        move_selection(-1);
        return {};
    case NavigationInput::right:
        move_selection(1);
        return {};
    case NavigationInput::up:
        move_selection(-kColumns);
        return {};
    case NavigationInput::down:
        move_selection(kColumns);
        return {};
    case NavigationInput::accept:
        if (!entries_.empty() && entries_[selected_index_].type == "game_system")
        {
            return { ScreenActionType::open_game_browser, entries_[selected_index_].id };
        }
        return { ScreenActionType::open_placeholder, translations_.translate("screen.collections") };
    case NavigationInput::back:
        return { ScreenActionType::pop };
    case NavigationInput::quit:
        return { ScreenActionType::quit };
    case NavigationInput::none:
    default:
        return {};
    }
}


std::string GamesScreen::screen_id() const
{
    return "games";
}


UiBindings GamesScreen::build_bindings() const
{
    UiBindings items = UiBindings::array();
    for (std::size_t index = 0; index < entries_.size(); ++index)
    {
        items.push_back({
            { "title", entries_[index].name },
            { "meta", std::to_string(entries_[index].item_count) + " " + translations_.translate("common.items_suffix") },
            { "hint", index == selected_index_ ? translations_.translate("common.press_a_to_open") : translations_.translate("common.ready") },
            { "system_id", entries_[index].id },
            { "selected", index == selected_index_ }
        });
    }

    return UiBindings {
        { "title", translations_.translate("menu.games") },
        { "subtitle", translations_.translate("games.subtitle") },
        { "empty", entries_.empty() },
        { "has_items", !entries_.empty() },
        { "empty_title", translations_.translate("games.empty_title") },
        { "empty_body", translations_.translate("games.empty_body") },
        { "items", items }
    };
}


std::string GamesScreen::window_title() const
{
    if (entries_.empty())
    {
        return translations_.translate("menu.games");
    }

    return translations_.translate("menu.games") + " - " + entries_[selected_index_].name;
}


void GamesScreen::move_selection(int delta)
{
    if (entries_.empty())
    {
        return;
    }

    const int item_count = static_cast<int>(entries_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count * 4) % item_count);
}
}
