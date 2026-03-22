#pragma once

#include "ui/image_texture.hpp"
#include "ui/theme_font_renderer.hpp"
#include "ui/theme_manager.hpp"
#include "ui/ui_bindings.hpp"

#include <SDL.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace bytedeck::ui
{
class FixedUiRenderer
{
public:
    explicit FixedUiRenderer(const ThemeManager& theme_manager);

    void render(SDL_Renderer& renderer, const std::string& screen_id, const UiBindings& screen_bindings, const UiBindings& shell_bindings);

private:
    struct Spacing
    {
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;
    };

    enum class TextAlign
    {
        left,
        center,
        right
    };

    void render_shell_background(SDL_Renderer& renderer, const SDL_Rect& bounds);
    SDL_Rect render_header(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& shell_bindings);
    SDL_Rect render_footer(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& shell_bindings);

    void render_main_menu(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings);
    void render_games(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings);
    void render_game_browser(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings);
    void render_apps(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings);
    void render_settings(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings);
    void render_placeholder(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings);
    void render_unknown(SDL_Renderer& renderer, const SDL_Rect& bounds, const std::string& screen_id);

    void draw_box(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style);
    void draw_text(
        SDL_Renderer& renderer,
        std::string_view text,
        const SDL_Rect& bounds,
        const nlohmann::json& style,
        TextAlign align = TextAlign::left,
        bool vertically_center = false);
    void draw_image(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style, const std::filesystem::path& path);
    void draw_placeholder_image(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style, const std::string& placeholder);

    SDL_Rect inset_rect(const SDL_Rect& bounds, const Spacing& spacing) const;
    SDL_Rect split_left(const SDL_Rect& bounds, int width, int gap) const;
    SDL_Rect split_right(const SDL_Rect& bounds, int left_width, int gap) const;
    SDL_Rect anchored_bottom(const SDL_Rect& bounds, int height) const;
    SDL_Rect anchored_top(const SDL_Rect& bounds, int height) const;
    int text_width(std::string_view text, const nlohmann::json& style) const;
    int line_height(const nlohmann::json& style) const;
    Spacing resolve_spacing(const nlohmann::json& value) const;
    SDL_Color color_from_json(const nlohmann::json& value, SDL_Color default_value) const;
    SDL_Color color_prop(const nlohmann::json& style, const char* key, SDL_Color default_value) const;
    int int_prop(const nlohmann::json& style, const char* key, int default_value) const;
    bool bool_prop(const nlohmann::json& style, const char* key, bool default_value) const;
    std::string string_prop(const nlohmann::json& style, const char* key, const std::string& default_value = "") const;
    nlohmann::json style_at(const std::string& path) const;
    std::filesystem::path asset_path_from_style(const nlohmann::json& style, const char* key) const;
    ImageTexture* load_image(SDL_Renderer& renderer, const std::filesystem::path& path);

    const ThemeManager& theme_manager_;
    mutable ThemeFontRenderer font_renderer_;
    std::unordered_map<std::string, std::unique_ptr<ImageTexture>> image_cache_;
    std::unordered_set<std::string> failed_images_;
};
}
