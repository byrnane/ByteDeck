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
    LaunchGameCallback launch_game
)
    : root_path_(std::move(root_path))
    , system_id_(std::move(system_id))
    , launch_game_(std::move(launch_game))
{
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
        { "subtitle", "GAME BROWSER" },
        { "empty", games_.empty() },
        { "has_items", !games_.empty() },
        { "empty_title", "NO GAMES FOUND" },
        { "empty_body", "CHECK ROMS AND GAMELIST.XML" },
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
    bindings["preview_title"] = "PREVIEW";
    bindings["preview_path"] = selected_game.thumbnail;
    bindings["preview_placeholder"] = selected_game.thumbnail.empty() ? "NO THUMBNAIL" : "THUMBNAIL MISSING";
    bindings["game_title"] = selected_game.title;
    bindings["genre_line"] = "GENRE: " + (selected_game.genre.empty() ? std::string("N/A") : selected_game.genre);
    bindings["players_line"] = "PLAYERS: " + (selected_game.players.empty() ? std::string("N/A") : selected_game.players);
    bindings["date_line"] = "DATE: " + (selected_game.release_date.empty() ? std::string("N/A") : selected_game.release_date);
    bindings["source_line"] = "SOURCE: " + selected_game.metadata_source;
    bindings["description_title"] = "DESCRIPTION";
    bindings["description_text"] = selected_game.description.empty() ? "NO DESCRIPTION AVAILABLE" : selected_game.description;
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
    if (system_id_ == "nes")
    {
        return "NES";
    }
    if (system_id_ == "snes")
    {
        return "SNES";
    }
    if (system_id_ == "megadrive")
    {
        return "Mega Drive";
    }
    if (system_id_ == "psp")
    {
        return "PSP";
    }
    return system_id_;
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
        launch_status_ = "LAUNCH CALLBACK MISSING";
        launch_status_ok_ = false;
        platform::Logger::instance().error("Launch callback missing for game browser");
        return;
    }

    const launch::LaunchResult result = launch_game_(*games_[selected_index_]);
    launch_status_ = TextRenderer::truncate_to_width(result.message, 36);
    launch_status_ok_ = result.success;
}
}
