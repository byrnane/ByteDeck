#include "ui/screens/placeholder_screen.hpp"

#include "ui/text_renderer.hpp"

#include <SDL.h>

#include <utility>

namespace bytedeck::ui
{
namespace
{
SDL_Color kTextPrimary { 245, 241, 230, 255 };
SDL_Color kTextMuted { 147, 157, 176, 255 };
}


PlaceholderScreen::PlaceholderScreen(std::string title)
    : title_(std::move(title))
{
}


ScreenAction PlaceholderScreen::handle_event(const SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
    {
        return { ScreenActionType::pop };
    }

    if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_B)
    {
        return { ScreenActionType::pop };
    }

    return {};
}


void PlaceholderScreen::render(SDL_Renderer& renderer)
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(&renderer, &width, &height);

    SDL_Rect panel { width / 2 - 280, height / 2 - 120, 560, 240 };
    SDL_SetRenderDrawColor(&renderer, 26, 31, 42, 255);
    SDL_RenderFillRect(&renderer, &panel);

    TextRenderer::draw_text(renderer, title_, panel.x + 40, panel.y + 44, 4, kTextPrimary);
    TextRenderer::draw_text(renderer, "SCREEN STUB", panel.x + 40, panel.y + 96, 2, kTextMuted);
    TextRenderer::draw_text(renderer, "PRESS B OR ESC TO GO BACK", panel.x + 40, panel.y + 146, 2, kTextMuted);
}


std::string PlaceholderScreen::window_title() const
{
    return title_;
}
}
