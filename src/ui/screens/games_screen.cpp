#include "ui/screens/games_screen.hpp"

#include "ui/text_renderer.hpp"

#include <SDL.h>

namespace bytedeck::ui
{
namespace
{
constexpr int kTileWidth = 260;
constexpr int kTileHeight = 150;
constexpr int kTileSpacing = 24;
constexpr int kColumns = 2;

SDL_Color kTextPrimary { 245, 241, 230, 255 };
SDL_Color kTextMuted { 147, 157, 176, 255 };
}


GamesScreen::GamesScreen(const data::LibraryData& library)
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
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
            move_selection(-1);
            return {};
        case SDLK_RIGHT:
            move_selection(1);
            return {};
        case SDLK_UP:
            move_selection(-kColumns);
            return {};
        case SDLK_DOWN:
            move_selection(kColumns);
            return {};
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!entries_.empty() && entries_[selected_index_].type == "game_system")
            {
                return { ScreenActionType::open_game_browser, entries_[selected_index_].id };
            }
            return { ScreenActionType::open_placeholder, "Collections" };
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
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            move_selection(-1);
            return {};
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            move_selection(1);
            return {};
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            move_selection(-kColumns);
            return {};
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            move_selection(kColumns);
            return {};
        case SDL_CONTROLLER_BUTTON_A:
            if (!entries_.empty() && entries_[selected_index_].type == "game_system")
            {
                return { ScreenActionType::open_game_browser, entries_[selected_index_].id };
            }
            return { ScreenActionType::open_placeholder, "Collections" };
        case SDL_CONTROLLER_BUTTON_B:
            return { ScreenActionType::pop };
        default:
            return {};
        }
    }

    return {};
}


void GamesScreen::render(SDL_Renderer& renderer)
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(&renderer, &width, &height);

    TextRenderer::draw_text(renderer, "GAMES", 48, 40, 4, kTextPrimary);
    TextRenderer::draw_text(renderer, "VISIBLE SYSTEMS AND COLLECTIONS", 48, 86, 2, kTextMuted);

    if (entries_.empty())
    {
        TextRenderer::draw_text(renderer, "NO SYSTEMS FOUND", 48, 170, 3, kTextPrimary);
        TextRenderer::draw_text(renderer, "ADD ROMS TO ROMS/NES OR ROMS/MEGADRIVE", 48, 215, 2, kTextMuted);
        return;
    }

    const int total_width = kColumns * kTileWidth + (kColumns - 1) * kTileSpacing;
    const int start_x = (width - total_width) / 2;
    const int start_y = 150;

    for (std::size_t index = 0; index < entries_.size(); ++index)
    {
        const int row = static_cast<int>(index) / kColumns;
        const int column = static_cast<int>(index) % kColumns;
        const int x = start_x + column * (kTileWidth + kTileSpacing);
        const int y = start_y + row * (kTileHeight + kTileSpacing);
        const bool selected = index == selected_index_;

        SDL_Rect tile { x, y, kTileWidth, kTileHeight };
        SDL_SetRenderDrawColor(&renderer, selected ? 228 : 59, selected ? 183 : 70, selected ? 86 : 97, 255);
        SDL_RenderFillRect(&renderer, &tile);

        SDL_Rect inner { x + 8, y + 8, kTileWidth - 16, kTileHeight - 16 };
        SDL_SetRenderDrawColor(&renderer, selected ? 31 : 33, selected ? 37 : 40, selected ? 48 : 55, 255);
        SDL_RenderFillRect(&renderer, &inner);

        TextRenderer::draw_text(renderer, entries_[index].name, x + 24, y + 24, 3, kTextPrimary);
        TextRenderer::draw_text(renderer, std::to_string(entries_[index].item_count) + " ITEMS", x + 24, y + 76, 2, kTextMuted);
        TextRenderer::draw_text(renderer, selected ? "PRESS A TO OPEN" : "READY", x + 24, y + 108, 2, selected ? SDL_Color { 228, 183, 86, 255 } : kTextMuted);
    }
}


std::string GamesScreen::window_title() const
{
    if (entries_.empty())
    {
        return "Games";
    }

    return "Games - " + entries_[selected_index_].name;
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
