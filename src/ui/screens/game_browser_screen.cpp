#include "ui/screens/game_browser_screen.hpp"

#include "platform/logger.hpp"
#include "ui/navigation_input.hpp"
#include "ui/text_renderer.hpp"
#include <utility>

namespace bytedeck::ui
{
GameBrowserScreen::GameBrowserScreen(
    const data::LibraryData& library,
    std::filesystem::path root_path,
    std::string system_id,
    const platform::TranslationCatalog& translations,
    LaunchGameCallback launch_game
)
    : root_path_(std::move(root_path))
    , system_id_(std::move(system_id))
    , translations_(translations)
    , launch_game_(std::move(launch_game))
{
    for (const data::SystemEntry& entry : library.systems)
    {
        if (entry.id == system_id_)
        {
            system_name_ = entry.name;
            break;
        }
    }

    if (system_name_.empty())
    {
        system_name_ = system_id_;
    }

    for (const data::GameItem& game : library.games)
    {
        if (game.system_id == system_id_)
        {
            games_.push_back(&game);
        }
    }
}


ScreenAction GameBrowserScreen::handle_event(const SDL_Event& event)
{
    switch (navigation_input_from_event(event))
    {
    case NavigationInput::up:
        move_selection(-1);
        return {};
    case NavigationInput::down:
        move_selection(1);
        return {};
    case NavigationInput::accept:
        launch_selected_game();
        return {};
    case NavigationInput::back:
        return { ScreenActionType::pop };
    case NavigationInput::quit:
        return { ScreenActionType::quit };
    case NavigationInput::none:
    default:
        return {};
    }
}


std::string GameBrowserScreen::screen_id() const
{
    return "game_browser";
}


UiBindings GameBrowserScreen::build_bindings() const
{
    UiBindings bindings {
        { "title", system_name() },
        { "subtitle", translations_.translate("games.browser_subtitle") },
        { "empty", games_.empty() },
        { "has_items", !games_.empty() },
        { "empty_title", translations_.translate("games.browser_empty_title") },
        { "empty_body", translations_.translate("games.browser_empty_body") },
        { "items", UiBindings::array() }
    };

    const std::vector<const data::GameItem*> visible = visible_games();
    for (const data::GameItem* game : visible)
    {
        bindings["items"].push_back({
            { "title", game->title },
            { "selected", game == games_[selected_index_] }
        });
    }

    if (games_.empty())
    {
        return bindings;
    }

    const data::GameItem& selected_game = *games_[selected_index_];
    bindings["preview_title"] = translations_.translate("games.preview_title");
    bindings["preview_path"] = selected_game.thumbnail;
    bindings["preview_placeholder"] = selected_game.thumbnail.empty() ? translations_.translate("games.no_thumbnail") : translations_.translate("games.thumbnail_missing");
    bindings["game_title"] = selected_game.title;
    bindings["genre_line"] = translations_.translate("games.genre_prefix") + (selected_game.genre.empty() ? translations_.translate("common.na") : selected_game.genre);
    bindings["players_line"] = translations_.translate("games.players_prefix") + (selected_game.players.empty() ? translations_.translate("common.na") : selected_game.players);
    bindings["date_line"] = translations_.translate("games.date_prefix") + (selected_game.release_date.empty() ? translations_.translate("common.na") : selected_game.release_date);
    bindings["source_line"] = translations_.translate("games.source_prefix") + selected_game.metadata_source;
    bindings["description_title"] = translations_.translate("common.description");
    bindings["description_text"] = selected_game.description.empty() ? translations_.translate("common.no_description") : selected_game.description;
    bindings["system_id"] = system_id_;
    bindings["launch_status_success_visible"] = !launch_status_.empty() && launch_status_ok_;
    bindings["launch_status_error_visible"] = !launch_status_.empty() && !launch_status_ok_;
    bindings["launch_status_text"] = launch_status_;
    return bindings;
}


std::string GameBrowserScreen::window_title() const
{
    if (games_.empty())
    {
        return system_name();
    }

    return system_name() + " - " + games_[selected_index_]->title;
}


void GameBrowserScreen::move_selection(int delta)
{
    if (games_.empty())
    {
        return;
    }

    const int item_count = static_cast<int>(games_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count * 4) % item_count);
}


std::string GameBrowserScreen::system_name() const
{
    return system_name_;
}


std::vector<const data::GameItem*> GameBrowserScreen::visible_games() const
{
    std::vector<const data::GameItem*> visible;
    if (games_.empty())
    {
        return visible;
    }

    constexpr int kVisibleRows = 14;
    int start_index = 0;
    if (selected_index_ >= static_cast<std::size_t>(kVisibleRows))
    {
        start_index = static_cast<int>(selected_index_) - kVisibleRows + 1;
    }

    for (int row = 0; row < kVisibleRows; ++row)
    {
        const int item_index = start_index + row;
        if (item_index >= static_cast<int>(games_.size()))
        {
            break;
        }

        visible.push_back(games_[item_index]);
    }

    return visible;
}


void GameBrowserScreen::launch_selected_game()
{
    if (games_.empty())
    {
        return;
    }

    if (!launch_game_)
    {
        launch_status_ = translations_.translate("error.launch_callback_missing");
        launch_status_ok_ = false;
        platform::Logger::instance().error("Launch callback missing for game browser");
        return;
    }

    const launch::LaunchResult result = launch_game_(*games_[selected_index_]);
    launch_status_ = TextRenderer::truncate_to_width(result.message, 36);
    launch_status_ok_ = result.success;
}
}
