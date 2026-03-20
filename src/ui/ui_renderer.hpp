#pragma once

#include "platform/paths.hpp"
#include "ui/image_texture.hpp"
#include "ui/layout_registry.hpp"
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
class UiRenderer
{
public:
    UiRenderer(const platform::Paths& paths, const LayoutRegistry& layout_registry, const ThemeManager& theme_manager);

    void render_screen(SDL_Renderer& renderer, const std::string& screen_id, const UiBindings& bindings);

private:
    struct Spacing
    {
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;
    };

    void render_node(SDL_Renderer& renderer, const LayoutNode& node, const UiBindings& bindings, const SDL_Rect& bounds);
    void render_container(SDL_Renderer& renderer, const LayoutNode& node, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style);
    void render_stack(SDL_Renderer& renderer, const LayoutNode& node, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style);
    void render_text(SDL_Renderer& renderer, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style);
    void render_image(SDL_Renderer& renderer, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style);
    void render_list(SDL_Renderer& renderer, const LayoutNode& node, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style);
    void render_rect(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style);
    void draw_fallback(SDL_Renderer& renderer, const std::string& screen_id, const std::string& message);

    nlohmann::json resolve_style(const LayoutNode& node, const UiBindings& bindings) const;
    std::vector<std::string> collect_runtime_classes(const LayoutNode& node, const UiBindings& bindings) const;
    bool is_visible(const nlohmann::json& style, const UiBindings& bindings) const;
    SDL_Rect apply_padding(const SDL_Rect& bounds, const nlohmann::json& style) const;
    Spacing resolve_spacing(const nlohmann::json& value) const;
    int resolve_int(const nlohmann::json& style, const char* key, const UiBindings& bindings, int default_value) const;
    std::string resolve_string(const nlohmann::json& style, const char* key, const UiBindings& bindings, const std::string& default_value = "") const;
    bool resolve_bool(const nlohmann::json& style, const char* key, const UiBindings& bindings, bool default_value) const;
    SDL_Color resolve_color(const nlohmann::json& style, const char* key, const UiBindings& bindings, SDL_Color default_value) const;
    const nlohmann::json* find_binding_value(const UiBindings& bindings, const std::string& path) const;
    SDL_Rect resolve_child_rect(const nlohmann::json& child_style, const SDL_Rect& parent_bounds, bool horizontal, int main_offset, int main_size) const;
    int resolve_main_size(const LayoutNode& node, const nlohmann::json& style, const UiBindings& bindings, bool horizontal, const SDL_Rect& bounds) const;
    int resolve_cross_size(const LayoutNode& node, const nlohmann::json& style, const UiBindings& bindings, bool horizontal, const SDL_Rect& bounds) const;
    int resolve_dimension_value(const nlohmann::json& value, int parent_size, int default_value) const;
    SDL_Color color_from_json(const nlohmann::json& value, SDL_Color default_value) const;
    ImageTexture* load_image(SDL_Renderer& renderer, const std::filesystem::path& path);

    const platform::Paths& paths_;
    const LayoutRegistry& layout_registry_;
    const ThemeManager& theme_manager_;
    std::unordered_map<std::string, std::unique_ptr<ImageTexture>> image_cache_;
    std::unordered_set<std::string> failed_images_;
    std::unordered_set<std::string> logged_fallbacks_;
};
}
