#include "ui/screens/apps_screen.hpp"

#include "platform/logger.hpp"
#include "ui/navigation_input.hpp"
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


AppsScreen::AppsScreen(const data::LibraryData& library, std::filesystem::path root_path, LaunchAppCallback launch_app)
    : root_path_(std::move(root_path))
    , launch_app_(std::move(launch_app))
{
    for (const data::AppItem& app : library.apps)
    {
        apps_.push_back(&app);
    }
}


ScreenAction AppsScreen::handle_event(const SDL_Event& event)
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
        launch_selected_app();
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


void AppsScreen::render(SDL_Renderer& renderer)
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

    TextRenderer::draw_text(renderer, "APPS", left_panel.x + 24, left_panel.y + 20, 4, kTextPrimary);
    TextRenderer::draw_text(renderer, "TOOLS AND UTILITIES", left_panel.x + 24, left_panel.y + 64, 2, kTextMuted);

    if (apps_.empty())
    {
        TextRenderer::draw_text(renderer, "NO APPS FOUND", left_panel.x + 24, left_panel.y + 130, 3, kTextPrimary);
        TextRenderer::draw_text(renderer, "ADD APPS/*/MANIFEST.JSON", left_panel.x + 24, left_panel.y + 175, 2, kTextMuted);
        TextRenderer::draw_text(renderer, "APPS SCREEN IS READY FOR REAL DATA", right_panel.x + 24, right_panel.y + 64, 2, kTextMuted);
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
        if (item_index >= static_cast<int>(apps_.size()))
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
        const std::string title = TextRenderer::truncate_to_width(apps_[item_index]->title, max_columns);
        TextRenderer::draw_text(renderer, title, inner.x + 8, inner.y + 6, 2, selected ? kHighlight : kTextPrimary);
    }

    const data::AppItem& selected_app = *apps_[selected_index_];
    ensure_preview_texture(renderer);
    TextRenderer::draw_text(renderer, "APP PREVIEW", right_panel.x + 24, right_panel.y + 24, 3, kTextPrimary);

    SDL_Rect icon_rect { right_panel.x + 24, right_panel.y + 74, 180, 180 };
    SDL_SetRenderDrawColor(&renderer, 49, 58, 78, 255);
    SDL_RenderFillRect(&renderer, &icon_rect);
    SDL_SetRenderDrawColor(&renderer, 74, 86, 114, 255);
    SDL_RenderDrawRect(&renderer, &icon_rect);
    if (preview_texture_.is_loaded())
    {
        preview_texture_.render(renderer, SDL_Rect { icon_rect.x + 6, icon_rect.y + 6, icon_rect.w - 12, icon_rect.h - 12 });
    }
    else
    {
        TextRenderer::draw_text(renderer, selected_app.icon.empty() ? "NO ICON" : "ICON MISSING", icon_rect.x + 32, icon_rect.y + 76, 3, kTextMuted);
    }

    TextRenderer::draw_text_box(
        renderer,
        selected_app.title,
        SDL_Rect { right_panel.x + 230, right_panel.y + 78, right_panel.w - 254, 54 },
        3,
        kTextPrimary,
        2
    );
    TextRenderer::draw_text(renderer, "TARGET: " + TextRenderer::truncate_to_width(selected_app.launch_target, 28), right_panel.x + 230, right_panel.y + 154, 2, kTextMuted);
    if (!launch_status_.empty())
    {
        TextRenderer::draw_text(
            renderer,
            launch_status_,
            right_panel.x + 230,
            right_panel.y + 186,
            2,
            launch_status_ok_ ? kHighlight : SDL_Color { 220, 116, 116, 255 }
        );
    }

    TextRenderer::draw_text(renderer, "DESCRIPTION", right_panel.x + 24, right_panel.y + 282, 2, kTextPrimary);
    TextRenderer::draw_text_box(
        renderer,
        selected_app.description.empty() ? "NO DESCRIPTION AVAILABLE" : selected_app.description,
        SDL_Rect { right_panel.x + 24, right_panel.y + 318, right_panel.w - 48, right_panel.h - 342 },
        2,
        kTextMuted,
        10
    );
}


std::string AppsScreen::window_title() const
{
    if (apps_.empty())
    {
        return "Apps";
    }

    return "Apps - " + apps_[selected_index_]->title;
}


void AppsScreen::move_selection(int delta)
{
    if (apps_.empty())
    {
        return;
    }

    const int item_count = static_cast<int>(apps_.size());
    const int current = static_cast<int>(selected_index_);
    selected_index_ = static_cast<std::size_t>((current + delta + item_count * 4) % item_count);
}


void AppsScreen::ensure_preview_texture(SDL_Renderer& renderer)
{
    if (apps_.empty())
    {
        preview_texture_.reset();
        loaded_icon_path_.clear();
        return;
    }

    const std::string& icon_path = apps_[selected_index_]->icon;
    if (icon_path == loaded_icon_path_)
    {
        return;
    }

    preview_texture_.reset();
    loaded_icon_path_ = icon_path;

    if (icon_path.empty())
    {
        return;
    }

    const std::filesystem::path absolute_path = root_path_ / icon_path;
    preview_texture_.load(renderer, absolute_path);
}


void AppsScreen::launch_selected_app()
{
    if (apps_.empty())
    {
        return;
    }

    if (!launch_app_)
    {
        launch_status_ = "LAUNCH CALLBACK MISSING";
        launch_status_ok_ = false;
        platform::Logger::instance().error("Launch callback missing for apps screen");
        return;
    }

    const launch::LaunchResult result = launch_app_(*apps_[selected_index_]);
    launch_status_ = TextRenderer::truncate_to_width(result.message, 28);
    launch_status_ok_ = result.success;
}
}
