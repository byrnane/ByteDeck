#include "ui/screens/game_browser_screen.hpp"

#include "platform/logger.hpp"
#include "ui/text_renderer.hpp"

#include <SDL.h>

#include <utility>

namespace bytedeck::ui
{
namespace
{
SDL_Color kTextPrimary { 245, 241, 230, 255 };
SDL_Color kTextMuted { 147, 157, 176, 255 };
SDL_Color kHighlight { 228, 183, 86, 255 };
}


GameBrowserScreen::GameBrowserScreen(const data::LibraryData& library, std::filesystem::path root_path, std::string system_id)
    : root_path_(std::move(root_path))
    , system_id_(std::move(system_id))
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
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            move_selection(-1);
            return {};
        case SDLK_DOWN:
            move_selection(1);
            return {};
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!games_.empty())
            {
                platform::Logger::instance().info("Launch requested for " + games_[selected_index_]->id);
            }
            return {};
        case SDLK_ESCAPE:
            return { ScreenActionType::pop };
        default:
            return {};
        }
    }

    if (event.type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch (event.cbutton.button)
        {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            move_selection(-1);
            return {};
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            move_selection(1);
            return {};
        case SDL_CONTROLLER_BUTTON_A:
            if (!games_.empty())
            {
                platform::Logger::instance().info("Launch requested for " + games_[selected_index_]->id);
            }
            return {};
        case SDL_CONTROLLER_BUTTON_B:
            return { ScreenActionType::pop };
        default:
            return {};
        }
    }

    return {};
}


void GameBrowserScreen::render(SDL_Renderer& renderer)
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(&renderer, &width, &height);

    SDL_Rect left_panel { 36, 36, width / 2 - 54, height - 72 };
    SDL_Rect right_panel { width / 2 + 18, 36, width / 2 - 54, height - 72 };

    SDL_SetRenderDrawColor(&renderer, 26, 31, 42, 255);
    SDL_RenderFillRect(&renderer, &left_panel);
    SDL_SetRenderDrawColor(&renderer, 22, 27, 36, 255);
    SDL_RenderFillRect(&renderer, &right_panel);

    TextRenderer::draw_text(renderer, system_name(), left_panel.x + 24, left_panel.y + 20, 4, kTextPrimary);
    TextRenderer::draw_text(renderer, "GAME BROWSER", left_panel.x + 24, left_panel.y + 64, 2, kTextMuted);

    if (games_.empty())
    {
        TextRenderer::draw_text(renderer, "NO GAMES FOUND", left_panel.x + 24, left_panel.y + 130, 3, kTextPrimary);
        TextRenderer::draw_text(renderer, "CHECK ROMS AND GAMELIST.XML", left_panel.x + 24, left_panel.y + 175, 2, kTextMuted);
        return;
    }

    const int visible_rows = 14;
    int start_index = 0;
    if (selected_index_ >= static_cast<std::size_t>(visible_rows))
    {
        start_index = static_cast<int>(selected_index_) - visible_rows + 1;
    }

    for (int row = 0; row < visible_rows; ++row)
    {
        const int item_index = start_index + row;
        if (item_index >= static_cast<int>(games_.size()))
        {
            break;
        }

        const int item_y = left_panel.y + 110 + row * 38;
        SDL_Rect item_rect { left_panel.x + 16, item_y, left_panel.w - 32, 30 };
        const bool selected = static_cast<std::size_t>(item_index) == selected_index_;

        SDL_SetRenderDrawColor(&renderer, selected ? 228 : 45, selected ? 183 : 52, selected ? 86 : 68, 255);
        SDL_RenderFillRect(&renderer, &item_rect);

        SDL_Rect inner { item_rect.x + 4, item_rect.y + 4, item_rect.w - 8, item_rect.h - 8 };
        SDL_SetRenderDrawColor(&renderer, selected ? 29 : 31, selected ? 34 : 38, selected ? 44 : 52, 255);
        SDL_RenderFillRect(&renderer, &inner);

        const int max_columns = (inner.w - 16) / ((TextRenderer::glyph_width + TextRenderer::glyph_spacing) * 2);
        const std::string title = TextRenderer::truncate_to_width(games_[item_index]->title, max_columns);
        TextRenderer::draw_text(renderer, title, inner.x + 8, inner.y + 6, 2, selected ? kHighlight : kTextPrimary);
    }

    const data::GameItem& selected_game = *games_[selected_index_];
    ensure_preview_texture(renderer);

    TextRenderer::draw_text(renderer, "PREVIEW", right_panel.x + 24, right_panel.y + 24, 3, kTextPrimary);

    SDL_Rect thumbnail_rect { right_panel.x + 24, right_panel.y + 74, right_panel.w - 48, 180 };
    SDL_SetRenderDrawColor(&renderer, 49, 58, 78, 255);
    SDL_RenderFillRect(&renderer, &thumbnail_rect);
    SDL_SetRenderDrawColor(&renderer, 74, 86, 114, 255);
    SDL_RenderDrawRect(&renderer, &thumbnail_rect);

    if (preview_texture_.is_loaded())
    {
        preview_texture_.render(renderer, SDL_Rect { thumbnail_rect.x + 6, thumbnail_rect.y + 6, thumbnail_rect.w - 12, thumbnail_rect.h - 12 });
    }
    else
    {
        TextRenderer::draw_text(
            renderer,
            selected_game.thumbnail.empty() ? "NO THUMBNAIL" : "THUMBNAIL MISSING",
            thumbnail_rect.x + 20,
            thumbnail_rect.y + 76,
            3,
            kTextMuted
        );
    }

    int text_y = thumbnail_rect.y + thumbnail_rect.h + 24;
    TextRenderer::draw_text_box(
        renderer,
        selected_game.title,
        SDL_Rect { right_panel.x + 24, text_y, right_panel.w - 48, 54 },
        3,
        kTextPrimary,
        2
    );
    text_y += 64;

    TextRenderer::draw_text(renderer, "GENRE: " + TextRenderer::truncate_to_width(selected_game.genre.empty() ? "N/A" : selected_game.genre, 24), right_panel.x + 24, text_y, 2, kTextMuted);
    text_y += 28;
    TextRenderer::draw_text(renderer, "PLAYERS: " + (selected_game.players.empty() ? std::string("N/A") : selected_game.players), right_panel.x + 24, text_y, 2, kTextMuted);
    text_y += 28;
    TextRenderer::draw_text(renderer, "DATE: " + (selected_game.release_date.empty() ? std::string("N/A") : TextRenderer::truncate_to_width(selected_game.release_date, 20)), right_panel.x + 24, text_y, 2, kTextMuted);
    text_y += 28;
    TextRenderer::draw_text(renderer, "SOURCE: " + selected_game.metadata_source, right_panel.x + 24, text_y, 2, kTextMuted);
    text_y += 36;

    TextRenderer::draw_text(renderer, "DESCRIPTION", right_panel.x + 24, text_y, 2, kTextPrimary);
    text_y += 28;
    TextRenderer::draw_text_box(
        renderer,
        selected_game.description.empty() ? "NO DESCRIPTION AVAILABLE" : selected_game.description,
        SDL_Rect { right_panel.x + 24, text_y, right_panel.w - 48, right_panel.h - (text_y - right_panel.y) - 20 },
        2,
        kTextMuted,
        8
    );
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


void GameBrowserScreen::ensure_preview_texture(SDL_Renderer& renderer)
{
    if (games_.empty())
    {
        preview_texture_.reset();
        loaded_thumbnail_path_.clear();
        return;
    }

    const std::string& thumbnail_path = games_[selected_index_]->thumbnail;
    if (thumbnail_path == loaded_thumbnail_path_)
    {
        return;
    }

    preview_texture_.reset();
    loaded_thumbnail_path_ = thumbnail_path;

    if (thumbnail_path.empty())
    {
        return;
    }

    const std::filesystem::path absolute_path = root_path_ / thumbnail_path;
    preview_texture_.load(renderer, absolute_path);
}
}
