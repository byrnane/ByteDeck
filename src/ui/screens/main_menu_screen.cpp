#include "ui/screens/main_menu_screen.hpp"

#include "ui/text_renderer.hpp"

#include <SDL.h>

namespace bytedeck::ui
{
namespace
{
constexpr int kTileWidth = 240;
constexpr int kTileHeight = 280;
constexpr int kTileSpacing = 36;

SDL_Color kTextPrimary { 245, 241, 230, 255 };
SDL_Color kTextMuted { 147, 157, 176, 255 };
}


MainMenuScreen::MainMenuScreen(const data::LibraryData& library)
    : items_ { "Games", "Settings", "Apps" }
{
    item_counts_.push_back(static_cast<int>(library.games.size()));
    item_counts_.push_back(0);
    item_counts_.push_back(static_cast<int>(library.apps.size()));
}


ScreenAction MainMenuScreen::handle_event(const SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
        case SDLK_UP:
            move_selection(-1);
            return {};
        case SDLK_RIGHT:
        case SDLK_DOWN:
            move_selection(1);
            return {};
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (selected_index_ == 0)
            {
                return { ScreenActionType::open_games };
            }
            if (selected_index_ == 1)
            {
                return { ScreenActionType::open_placeholder, "Settings" };
            }
            return { ScreenActionType::open_apps };
        case SDLK_ESCAPE:
            return { ScreenActionType::quit };
        default:
            return {};
        }
    }

    if (event.type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch (event.cbutton.button)
        {
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            move_selection(-1);
            return {};
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            move_selection(1);
            return {};
        case SDL_CONTROLLER_BUTTON_A:
            if (selected_index_ == 0)
            {
                return { ScreenActionType::open_games };
            }
            if (selected_index_ == 1)
            {
                return { ScreenActionType::open_placeholder, "Settings" };
            }
            return { ScreenActionType::open_apps };
        case SDL_CONTROLLER_BUTTON_B:
            return { ScreenActionType::quit };
        default:
            return {};
        }
    }

    return {};
}


void MainMenuScreen::render(SDL_Renderer& renderer)
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(&renderer, &width, &height);

    TextRenderer::draw_text(renderer, "BYTEDECK", 48, 40, 5, kTextPrimary);
    TextRenderer::draw_text(renderer, "MAIN MENU", 48, 94, 2, kTextMuted);

    const int total_width = static_cast<int>(items_.size()) * kTileWidth + static_cast<int>(items_.size() - 1) * kTileSpacing;
    const int start_x = (width - total_width) / 2;
    const int start_y = 180;

    SDL_Rect backdrop { start_x - 48, start_y - 48, total_width + 96, kTileHeight + 96 };
    SDL_SetRenderDrawColor(&renderer, 24, 30, 42, 255);
    SDL_RenderFillRect(&renderer, &backdrop);

    for (std::size_t index = 0; index < items_.size(); ++index)
    {
        const int x = start_x + static_cast<int>(index) * (kTileWidth + kTileSpacing);
        SDL_Rect tile { x, start_y, kTileWidth, kTileHeight };

        const bool selected = index == selected_index_;
        if (selected)
        {
            SDL_SetRenderDrawColor(&renderer, 232, 179, 82, 255);
            SDL_RenderFillRect(&renderer, &tile);

            SDL_Rect inner { x + 8, start_y + 8, kTileWidth - 16, kTileHeight - 16 };
            SDL_SetRenderDrawColor(&renderer, 33, 38, 52, 255);
            SDL_RenderFillRect(&renderer, &inner);
        }
        else
        {
            SDL_SetRenderDrawColor(&renderer, 58, 69, 94, 255);
            SDL_RenderFillRect(&renderer, &tile);
        }

        TextRenderer::draw_text(renderer, items_[index], x + 24, start_y + 28, 3, kTextPrimary);
        if (item_counts_[index] > 0)
        {
            TextRenderer::draw_text(renderer, std::to_string(item_counts_[index]) + " ITEMS", x + 24, start_y + 82, 2, kTextMuted);
        }
        else if (index == 1)
        {
            TextRenderer::draw_text(renderer, "STUB", x + 24, start_y + 82, 2, kTextMuted);
        }
        else
        {
            TextRenderer::draw_text(renderer, "EMPTY", x + 24, start_y + 82, 2, kTextMuted);
        }

        SDL_Rect accent {
            x + 18,
            start_y + kTileHeight - 48,
            kTileWidth - 36,
            18
        };
        if (selected)
        {
            SDL_SetRenderDrawColor(&renderer, 232, 179, 82, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(&renderer, 106, 122, 160, 255);
        }
        SDL_RenderFillRect(&renderer, &accent);

        TextRenderer::draw_text(renderer, selected ? "PRESS A" : "READY", x + 24, start_y + kTileHeight - 86, 2, selected ? SDL_Color { 228, 183, 86, 255 } : kTextMuted);
    }
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
