#include "ui/ui_renderer.hpp"

#include "platform/logger.hpp"
#include "ui/text_renderer.hpp"

#include <algorithm>
#include <cctype>

namespace bytedeck::ui
{
namespace
{
SDL_Color kDefaultTextColor { 245, 241, 230, 255 };
SDL_Color kDefaultMutedColor { 147, 157, 176, 255 };
SDL_Color kDefaultErrorColor { 220, 116, 116, 255 };
nlohmann::json kRuntimeDefaults = {
    { "screen", {
        { "background_color", "#0F1218" },
        { "padding", 0 }
    } },
    { "panel", {
        { "padding", 0 },
        { "border_width", 0 }
    } },
    { "stack", {
        { "direction", "vertical" },
        { "gap", 0 },
        { "padding", 0 },
        { "align", "stretch" }
    } },
    { "text", {
        { "scale", 2 },
        { "font_role", "body" },
        { "text_color", "#F5F1E6" },
        { "wrap", false },
        { "truncate", false },
        { "max_lines", 1 }
    } },
    { "list", {
        { "direction", "vertical" },
        { "gap", 0 },
        { "padding", 0 }
    } },
    { "rect", {
        { "border_width", 0 }
    } },
    { "image", {
        { "padding", 0 },
        { "border_width", 0 }
    } }
};

int text_height_for_scale(int scale)
{
    return TextRenderer::glyph_height * scale;
}

Uint8 percent_to_alpha(int alpha_percent, Uint8 /*default_alpha*/)
{
    return static_cast<Uint8>(std::clamp(alpha_percent, 0, 100) * 255 / 100);
}

bool is_hex_digit(char value)
{
    return std::isxdigit(static_cast<unsigned char>(value)) != 0;
}

bool parse_hex_byte(std::string_view value, std::size_t offset, Uint8& out)
{
    if (offset + 2 > value.size() || !is_hex_digit(value[offset]) || !is_hex_digit(value[offset + 1]))
    {
        return false;
    }

    out = static_cast<Uint8>(std::stoi(std::string(value.substr(offset, 2)), nullptr, 16));
    return true;
}
}


UiRenderer::UiRenderer(const platform::Paths& paths, const LayoutRegistry& layout_registry, const ThemeManager& theme_manager)
    : paths_(paths)
    , layout_registry_(layout_registry)
    , theme_manager_(theme_manager)
    , font_renderer_(theme_manager)
{
}


void UiRenderer::render_screen(SDL_Renderer& renderer, const std::string& screen_id, const UiBindings& bindings)
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(&renderer, &width, &height);
    render_screen(renderer, screen_id, bindings, SDL_Rect { 0, 0, width, height });
}


void UiRenderer::render_screen(SDL_Renderer& renderer, const std::string& screen_id, const UiBindings& bindings, const SDL_Rect& bounds)
{

    std::string error_message;
    const std::string variant = theme_manager_.screen_variant(screen_id);
    const LayoutNode* layout = layout_registry_.find_layout(screen_id, variant, &error_message);
    if (layout == nullptr)
    {
        draw_fallback(renderer, screen_id, error_message, bounds);
        return;
    }

    render_node(renderer, *layout, bindings, bounds);
}


void UiRenderer::render_node(SDL_Renderer& renderer, const LayoutNode& node, const UiBindings& bindings, const SDL_Rect& bounds)
{
    const nlohmann::json style = resolve_style(node, bindings);
    if (!is_visible(style, bindings))
    {
        return;
    }

    if (node.type == "screen" || node.type == "panel")
    {
        render_container(renderer, node, bindings, bounds, style);
    }
    else if (node.type == "stack")
    {
        render_stack(renderer, node, bindings, bounds, style);
    }
    else if (node.type == "text")
    {
        render_text(renderer, bindings, bounds, style);
    }
    else if (node.type == "image")
    {
        render_image(renderer, bindings, bounds, style);
    }
    else if (node.type == "list")
    {
        render_list(renderer, node, bindings, bounds, style);
    }
    else if (node.type == "rect")
    {
        render_rect(renderer, bounds, style);
    }
}


void UiRenderer::render_container(
    SDL_Renderer& renderer,
    const LayoutNode& node,
    const UiBindings& bindings,
    const SDL_Rect& bounds,
    const nlohmann::json& style)
{
    render_rect(renderer, bounds, style);
    const SDL_Rect content = apply_padding(bounds, style);

    for (const LayoutNode& child : node.children)
    {
        render_node(renderer, child, bindings, content);
    }
}


void UiRenderer::render_stack(
    SDL_Renderer& renderer,
    const LayoutNode& node,
    const UiBindings& bindings,
    const SDL_Rect& bounds,
    const nlohmann::json& style)
{
    render_rect(renderer, bounds, style);
    const SDL_Rect content = apply_padding(bounds, style);
    const bool horizontal = resolve_string(style, "direction", bindings, "vertical") == "horizontal";
    const int gap = resolve_int(style, "gap", bindings, 0);
    const std::string align = resolve_string(style, "align", bindings, "stretch");

    struct ChildLayout
    {
        const LayoutNode* node = nullptr;
        nlohmann::json style = nlohmann::json::object();
        int main_size = 0;
        int cross_size = 0;
        bool fill = false;
    };

    std::vector<ChildLayout> children;
    int fixed_main = 0;
    int fill_count = 0;

    for (const LayoutNode& child : node.children)
    {
        const nlohmann::json child_style = resolve_style(child, bindings);
        if (!is_visible(child_style, bindings))
        {
            continue;
        }

        const int main_size = resolve_main_size(child, child_style, bindings, horizontal, content);
        const int cross_size = resolve_cross_size(child, child_style, bindings, horizontal, content);
        const bool fill = main_size < 0;

        children.push_back({ &child, child_style, fill ? 0 : main_size, cross_size, fill });
        if (fill)
        {
            ++fill_count;
        }
        else
        {
            fixed_main += main_size;
        }
    }

    if (children.empty())
    {
        return;
    }

    const int total_gaps = gap * static_cast<int>(children.size() - 1);
    const int available_main = (horizontal ? content.w : content.h) - fixed_main - total_gaps;
    const int fill_main = fill_count > 0 ? std::max(0, available_main) / fill_count : 0;
    int current_main = horizontal ? content.x : content.y;

    for (ChildLayout& child : children)
    {
        const int main_size = child.fill ? fill_main : child.main_size;
        SDL_Rect child_rect = resolve_child_rect(child.style, content, horizontal, current_main, main_size);

        if (align == "center")
        {
            if (horizontal && child.cross_size > 0 && child.cross_size < content.h)
            {
                child_rect.y = content.y + (content.h - child.cross_size) / 2;
                child_rect.h = child.cross_size;
            }
            else if (!horizontal && child.cross_size > 0 && child.cross_size < content.w)
            {
                child_rect.x = content.x + (content.w - child.cross_size) / 2;
                child_rect.w = child.cross_size;
            }
        }
        else if (align == "start")
        {
            if (horizontal && child.cross_size > 0 && child.cross_size < content.h)
            {
                child_rect.h = child.cross_size;
            }
            else if (!horizontal && child.cross_size > 0 && child.cross_size < content.w)
            {
                child_rect.w = child.cross_size;
            }
        }

        render_node(renderer, *child.node, bindings, child_rect);
        current_main += main_size + gap;
    }
}


void UiRenderer::render_text(SDL_Renderer& renderer, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style)
{
    const std::string text = resolve_string(style, "text", bindings);
    if (text.empty())
    {
        return;
    }

    const std::string font_role = resolve_string(style, "font_role", bindings, "body");
    const int font_size = std::max(0, resolve_int(style, "font_size", bindings, 0));
    const ThemeTypographyRole typography = theme_manager_.typography_role(font_role);
    const int scale = std::max(1, resolve_int(style, "scale", bindings, std::max(1, typography.bitmap_scale)));
    const SDL_Color color = resolve_color(style, "text_color", bindings, kDefaultTextColor);
    const bool wrap = resolve_bool(style, "wrap", bindings, false);
    const bool truncate = resolve_bool(style, "truncate", bindings, false);
    const int max_lines = std::max(1, resolve_int(style, "max_lines", bindings, 1));
    const bool use_theme_font = font_renderer_.can_render(font_role);
    const int max_columns = std::max(1, bounds.w / ((TextRenderer::glyph_width + TextRenderer::glyph_spacing) * scale));
    std::string output = text;

    if (!wrap && truncate)
    {
        if (use_theme_font)
        {
            output = font_renderer_.truncate_to_width(text, font_role, bounds.w, font_size);
        }
        else
        {
            output = TextRenderer::truncate_to_width(text, max_columns);
        }
    }

    if (use_theme_font)
    {
        if (wrap)
        {
            font_renderer_.draw_text_box(renderer, output, bounds, font_role, color, max_lines, font_size);
        }
        else
        {
            font_renderer_.draw_text(renderer, output, bounds.x, bounds.y, font_role, color, font_size);
        }
        return;
    }

    if (wrap)
    {
        TextRenderer::draw_text_box(renderer, output, bounds, scale, color, max_lines);
    }
    else
    {
        TextRenderer::draw_text(renderer, output, bounds.x, bounds.y, scale, color);
    }
}


void UiRenderer::render_image(SDL_Renderer& renderer, const UiBindings& bindings, const SDL_Rect& bounds, const nlohmann::json& style)
{
    render_rect(renderer, bounds, style);
    const SDL_Rect content = apply_padding(bounds, style);
    std::filesystem::path resolved_path;
    const std::string path_value = resolve_string(style, "path", bindings);
    if (!path_value.empty())
    {
        const std::filesystem::path candidate(path_value);
        if (candidate.is_absolute())
        {
            resolved_path = candidate;
        }
        else
        {
            const std::filesystem::path theme_path = theme_manager_.resolve_asset_path(path_value);
            if (!theme_path.empty() && std::filesystem::exists(theme_path))
            {
                resolved_path = theme_path;
            }
            else
            {
                resolved_path = paths_.root() / candidate;
            }
        }
    }

    if (resolved_path.empty() && style.contains("system_icon_bind") && style["system_icon_bind"].is_string())
    {
        const nlohmann::json* value = find_binding_value(bindings, style["system_icon_bind"].get<std::string>());
        if (value != nullptr && value->is_string())
        {
            resolved_path = theme_manager_.system_icon_path(value->get<std::string>());
        }
    }

    if (!resolved_path.empty())
    {
        if (ImageTexture* texture = load_image(renderer, resolved_path))
        {
            texture->render(renderer, content);
            return;
        }
    }

    const std::string placeholder = resolve_string(style, "placeholder_text", bindings);
    if (!placeholder.empty())
    {
        const SDL_Color color = resolve_color(style, "placeholder_color", bindings, kDefaultMutedColor);
        TextRenderer::draw_text_box(renderer, placeholder, content, 3, color, 2);
    }
}


void UiRenderer::render_list(
    SDL_Renderer& renderer,
    const LayoutNode& node,
    const UiBindings& bindings,
    const SDL_Rect& bounds,
    const nlohmann::json& style)
{
    render_rect(renderer, bounds, style);
    const SDL_Rect content = apply_padding(bounds, style);
    const std::string bind = resolve_string(style, "bind", bindings);
    const nlohmann::json* items = find_binding_value(bindings, bind);
    if (items == nullptr || !items->is_array() || node.children.empty())
    {
        return;
    }

    const LayoutNode& item_template = node.children.front();
    const std::string direction = resolve_string(style, "direction", bindings, "vertical");
    const bool horizontal = direction == "horizontal";
    const int gap = resolve_int(style, "gap", bindings, 0);
    const int columns = std::max(1, resolve_int(style, "columns", bindings, horizontal ? static_cast<int>(items->size()) : 1));
    const int item_width = resolve_int(style, "item_width", bindings, columns > 1 ? std::max(0, (content.w - gap * (columns - 1)) / columns) : content.w);
    const int item_height = resolve_int(style, "item_height", bindings, horizontal ? content.h : 40);

    for (std::size_t index = 0; index < items->size(); ++index)
    {
        const int row = static_cast<int>(index) / columns;
        const int column = static_cast<int>(index) % columns;
        SDL_Rect item_bounds {};

        if (horizontal || columns > 1)
        {
            item_bounds = SDL_Rect {
                content.x + column * (item_width + gap),
                content.y + row * (item_height + gap),
                item_width,
                item_height
            };
        }
        else
        {
            item_bounds = SDL_Rect {
                content.x,
                content.y + static_cast<int>(index) * (item_height + gap),
                item_width,
                item_height
            };
        }

        render_node(renderer, item_template, (*items)[index], item_bounds);
    }
}


void UiRenderer::render_rect(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style)
{
    const SDL_Color fill = resolve_color(style, "background_color", nlohmann::json::object(),
        resolve_color(style, "fill_color", nlohmann::json::object(), SDL_Color { 0, 0, 0, 0 }));
    const SDL_Color border = resolve_color(style, "border_color", nlohmann::json::object(), SDL_Color { 0, 0, 0, 0 });
    const int border_width = std::max(0, resolve_int(style, "border_width", nlohmann::json::object(), 0));
    const std::string background_image = resolve_string(style, "background_image", nlohmann::json::object());

    if (fill.a > 0)
    {
        SDL_SetRenderDrawColor(&renderer, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect(&renderer, &bounds);
    }

    if (!background_image.empty())
    {
        const std::filesystem::path image_path = theme_manager_.resolve_asset_path(background_image);
        if (ImageTexture* texture = load_image(renderer, image_path))
        {
            texture->render(renderer, bounds);
        }
    }

    if (border_width > 0 && border.a > 0)
    {
        SDL_SetRenderDrawColor(&renderer, border.r, border.g, border.b, border.a);
        for (int index = 0; index < border_width; ++index)
        {
            SDL_Rect rect { bounds.x + index, bounds.y + index, bounds.w - index * 2, bounds.h - index * 2 };
            if (rect.w > 0 && rect.h > 0)
            {
                SDL_RenderDrawRect(&renderer, &rect);
            }
        }
    }
}


void UiRenderer::draw_fallback(SDL_Renderer& renderer, const std::string& screen_id, const std::string& message, const SDL_Rect& bounds)
{
    if (logged_fallbacks_.insert(screen_id + "|" + message).second)
    {
        platform::Logger::instance().error("UI fallback for screen '" + screen_id + "': " + message);
    }

    SDL_Rect panel {
        bounds.x + std::max(0, (bounds.w - 640) / 2),
        bounds.y + std::max(0, (bounds.h - 240) / 2),
        std::min(640, bounds.w),
        std::min(240, bounds.h)
    };
    SDL_SetRenderDrawColor(&renderer, 26, 31, 42, 255);
    SDL_RenderFillRect(&renderer, &panel);
    SDL_SetRenderDrawColor(&renderer, 74, 86, 114, 255);
    SDL_RenderDrawRect(&renderer, &panel);

    TextRenderer::draw_text(renderer, "UI LAYOUT ERROR", panel.x + 28, panel.y + 28, 4, kDefaultTextColor);
    TextRenderer::draw_text(renderer, screen_id, panel.x + 28, panel.y + 84, 2, kDefaultMutedColor);
    TextRenderer::draw_text_box(renderer, message.empty() ? "Unknown UI layout error" : message, SDL_Rect { panel.x + 28, panel.y + 118, panel.w - 56, panel.h - 144 }, 2, kDefaultErrorColor, 4);
}


nlohmann::json UiRenderer::resolve_style(const LayoutNode& node, const UiBindings& bindings) const
{
    nlohmann::json style = nlohmann::json::object();
    if (kRuntimeDefaults.contains(node.type))
    {
        style = kRuntimeDefaults[node.type];
    }

    const std::vector<std::string> runtime_classes = collect_runtime_classes(node, bindings);
    const nlohmann::json themed = theme_manager_.merge_style(node, runtime_classes);
    for (auto it = themed.begin(); it != themed.end(); ++it)
    {
        style[it.key()] = theme_manager_.resolve_value(it.value());
    }
    return style;
}


std::vector<std::string> UiRenderer::collect_runtime_classes(const LayoutNode& node, const UiBindings& bindings) const
{
    std::vector<std::string> classes;
    for (const ConditionalClass& entry : node.conditional_classes)
    {
        const nlohmann::json* value = find_binding_value(bindings, entry.bind);
        if (value != nullptr && value->is_boolean() && value->get<bool>())
        {
            classes.push_back(entry.class_name);
        }
    }

    return classes;
}


bool UiRenderer::is_visible(const nlohmann::json& style, const UiBindings& bindings) const
{
    if (style.contains("visible_bind") && style["visible_bind"].is_string())
    {
        const nlohmann::json* value = find_binding_value(bindings, style["visible_bind"].get<std::string>());
        return value != nullptr && value->is_boolean() && value->get<bool>();
    }

    if (style.contains("visible") && style["visible"].is_boolean())
    {
        return style["visible"].get<bool>();
    }

    return true;
}


SDL_Rect UiRenderer::apply_padding(const SDL_Rect& bounds, const nlohmann::json& style) const
{
    const Spacing padding = resolve_spacing(style.contains("padding") ? style["padding"] : nlohmann::json {});
    return SDL_Rect {
        bounds.x + padding.left,
        bounds.y + padding.top,
        std::max(0, bounds.w - padding.left - padding.right),
        std::max(0, bounds.h - padding.top - padding.bottom)
    };
}


UiRenderer::Spacing UiRenderer::resolve_spacing(const nlohmann::json& value) const
{
    const nlohmann::json resolved = theme_manager_.resolve_value(value);
    if (resolved.is_number_integer())
    {
        const int uniform = resolved.get<int>();
        return { uniform, uniform, uniform, uniform };
    }

    if (!resolved.is_object())
    {
        return {};
    }

    Spacing spacing;
    spacing.left = resolved.value("left", resolved.value("x", resolved.value("all", 0)));
    spacing.right = resolved.value("right", resolved.value("x", resolved.value("all", 0)));
    spacing.top = resolved.value("top", resolved.value("y", resolved.value("all", 0)));
    spacing.bottom = resolved.value("bottom", resolved.value("y", resolved.value("all", 0)));
    return spacing;
}


int UiRenderer::resolve_int(const nlohmann::json& style, const char* key, const UiBindings& bindings, int default_value) const
{
    const std::string bind_key = std::string(key) + "_bind";
    if (style.contains(bind_key) && style[bind_key].is_string())
    {
        const nlohmann::json* value = find_binding_value(bindings, style[bind_key].get<std::string>());
        if (value != nullptr && value->is_number_integer())
        {
            return value->get<int>();
        }
    }

    if (!style.contains(key))
    {
        return default_value;
    }

    const nlohmann::json value = theme_manager_.resolve_value(style[key]);
    if (value.is_number_integer())
    {
        return value.get<int>();
    }

    return default_value;
}


std::string UiRenderer::resolve_string(const nlohmann::json& style, const char* key, const UiBindings& bindings, const std::string& default_value) const
{
    const std::string bind_key = std::string(key) + "_bind";
    if (style.contains(bind_key) && style[bind_key].is_string())
    {
        const nlohmann::json* value = find_binding_value(bindings, style[bind_key].get<std::string>());
        if (value != nullptr)
        {
            if (value->is_string())
            {
                return value->get<std::string>();
            }
            if (value->is_number_integer())
            {
                return std::to_string(value->get<int>());
            }
            if (value->is_boolean())
            {
                return value->get<bool>() ? "true" : "false";
            }
        }

        return default_value;
    }

    if (!style.contains(key))
    {
        return default_value;
    }

    const nlohmann::json value = theme_manager_.resolve_value(style[key]);
    if (value.is_string())
    {
        return value.get<std::string>();
    }

    return default_value;
}


bool UiRenderer::resolve_bool(const nlohmann::json& style, const char* key, const UiBindings& bindings, bool default_value) const
{
    const std::string bind_key = std::string(key) + "_bind";
    if (style.contains(bind_key) && style[bind_key].is_string())
    {
        const nlohmann::json* value = find_binding_value(bindings, style[bind_key].get<std::string>());
        if (value != nullptr && value->is_boolean())
        {
            return value->get<bool>();
        }
    }

    if (!style.contains(key))
    {
        return default_value;
    }

    const nlohmann::json value = theme_manager_.resolve_value(style[key]);
    if (value.is_boolean())
    {
        return value.get<bool>();
    }

    return default_value;
}


SDL_Color UiRenderer::resolve_color(const nlohmann::json& style, const char* key, const UiBindings& bindings, SDL_Color default_value) const
{
    const std::string bind_key = std::string(key) + "_bind";
    if (style.contains(bind_key) && style[bind_key].is_string())
    {
        const nlohmann::json* value = find_binding_value(bindings, style[bind_key].get<std::string>());
        if (value != nullptr)
        {
            return color_from_json(*value, default_value);
        }
    }

    if (!style.contains(key))
    {
        return default_value;
    }

    return color_from_json(theme_manager_.resolve_value(style[key]), default_value);
}


const nlohmann::json* UiRenderer::find_binding_value(const UiBindings& bindings, const std::string& path) const
{
    if (path.empty() || path == ".")
    {
        return &bindings;
    }

    const nlohmann::json* current = &bindings;
    std::size_t start = 0;
    while (start < path.size())
    {
        const std::size_t dot = path.find('.', start);
        const std::string segment = path.substr(start, dot == std::string::npos ? std::string::npos : dot - start);
        if (!current->is_object() || !current->contains(segment))
        {
            return nullptr;
        }

        current = &(*current)[segment];
        if (dot == std::string::npos)
        {
            break;
        }

        start = dot + 1;
    }

    return current;
}


SDL_Rect UiRenderer::resolve_child_rect(const nlohmann::json& child_style, const SDL_Rect& parent_bounds, bool horizontal, int main_offset, int main_size) const
{
    SDL_Rect rect = parent_bounds;
    if (horizontal)
    {
        rect.x = main_offset;
        rect.w = main_size;
        rect.h = resolve_dimension_value(child_style.value("height", nlohmann::json {}), parent_bounds.h, parent_bounds.h);
    }
    else
    {
        rect.y = main_offset;
        rect.h = main_size;
        rect.w = resolve_dimension_value(child_style.value("width", nlohmann::json {}), parent_bounds.w, parent_bounds.w);
    }

    return rect;
}


int UiRenderer::resolve_main_size(const LayoutNode& node, const nlohmann::json& style, const UiBindings& bindings, bool horizontal, const SDL_Rect& bounds) const
{
    const char* key = horizontal ? "width" : "height";
    if (style.contains(key))
    {
        const nlohmann::json value = style[key];
        if (value.is_string() && value.get<std::string>() == "fill")
        {
            return -1;
        }

        return resolve_dimension_value(value, horizontal ? bounds.w : bounds.h, -1);
    }

    if (node.type == "text")
    {
        const std::string font_role = resolve_string(style, "font_role", bindings, "body");
        const int font_size = std::max(0, resolve_int(style, "font_size", bindings, 0));
        const ThemeTypographyRole typography = theme_manager_.typography_role(font_role);
        const int scale = std::max(1, resolve_int(style, "scale", bindings, std::max(1, typography.bitmap_scale)));
        if (font_renderer_.can_render(font_role))
        {
            return horizontal
                ? font_renderer_.measure_text_width(resolve_string(style, "text", bindings), font_role, font_size)
                : std::max(1, font_renderer_.line_height(font_role, font_size));
        }

        return horizontal ? TextRenderer::measure_text_width(resolve_string(style, "text", bindings), scale) : text_height_for_scale(scale);
    }

    if (node.type == "rect" || node.type == "image" || node.type == "panel" || node.type == "stack" || node.type == "list" || node.type == "spacer")
    {
        return -1;
    }

    return -1;
}


int UiRenderer::resolve_cross_size(const LayoutNode& node, const nlohmann::json& style, const UiBindings& bindings, bool horizontal, const SDL_Rect& bounds) const
{
    const char* key = horizontal ? "height" : "width";
    if (style.contains(key))
    {
        return resolve_dimension_value(style[key], horizontal ? bounds.h : bounds.w, horizontal ? bounds.h : bounds.w);
    }

    if (node.type == "text")
    {
        const std::string font_role = resolve_string(style, "font_role", bindings, "body");
        const int font_size = std::max(0, resolve_int(style, "font_size", bindings, 0));
        const ThemeTypographyRole typography = theme_manager_.typography_role(font_role);
        const int scale = std::max(1, resolve_int(style, "scale", bindings, std::max(1, typography.bitmap_scale)));
        if (font_renderer_.can_render(font_role))
        {
            return std::max(1, font_renderer_.line_height(font_role, font_size));
        }

        return text_height_for_scale(scale);
    }

    return horizontal ? bounds.h : bounds.w;
}


int UiRenderer::resolve_dimension_value(const nlohmann::json& value, int parent_size, int default_value) const
{
    const nlohmann::json resolved = theme_manager_.resolve_value(value);
    if (resolved.is_number_integer())
    {
        return resolved.get<int>();
    }

    if (resolved.is_string())
    {
        const std::string string_value = resolved.get<std::string>();
        if (string_value == "fill")
        {
            return default_value;
        }

        if (!string_value.empty() && string_value.back() == '%')
        {
            const int percent = std::stoi(string_value.substr(0, string_value.size() - 1));
            return parent_size * percent / 100;
        }
    }

    return default_value;
}


SDL_Color UiRenderer::color_from_json(const nlohmann::json& value, SDL_Color default_value) const
{
    if (value.is_string())
    {
        std::string string_value = value.get<std::string>();
        if (!string_value.empty() && string_value.front() == '#')
        {
            string_value.erase(0, 1);
        }

        if (string_value.size() == 6 || string_value.size() == 8)
        {
            Uint8 red = default_value.r;
            Uint8 green = default_value.g;
            Uint8 blue = default_value.b;
            Uint8 alpha = 255;
            if (parse_hex_byte(string_value, 0, red) &&
                parse_hex_byte(string_value, 2, green) &&
                parse_hex_byte(string_value, 4, blue))
            {
                if (string_value.size() == 8)
                {
                    parse_hex_byte(string_value, 6, alpha);
                }
                return SDL_Color { red, green, blue, alpha };
            }
        }
    }

    if (value.is_array() && value.size() >= 3)
    {
        return SDL_Color {
            static_cast<Uint8>(value[0].get<int>()),
            static_cast<Uint8>(value[1].get<int>()),
            static_cast<Uint8>(value[2].get<int>()),
            static_cast<Uint8>(value.size() > 3 ? value[3].get<int>() : 255)
        };
    }

    if (value.is_object())
    {
        if (value.contains("color"))
        {
            SDL_Color resolved = color_from_json(value["color"], default_value);
            if (value.contains("alpha") && value["alpha"].is_number_integer())
            {
                resolved.a = percent_to_alpha(value["alpha"].get<int>(), resolved.a);
            }
            return resolved;
        }

        return SDL_Color {
            static_cast<Uint8>(value.value("r", default_value.r)),
            static_cast<Uint8>(value.value("g", default_value.g)),
            static_cast<Uint8>(value.value("b", default_value.b)),
            static_cast<Uint8>(value.value("a", value.contains("r") || value.contains("g") || value.contains("b") ? 255 : default_value.a))
        };
    }

    return default_value;
}


ImageTexture* UiRenderer::load_image(SDL_Renderer& renderer, const std::filesystem::path& path)
{
    const std::string key = path.lexically_normal().string();
    if (failed_images_.count(key) > 0)
    {
        return nullptr;
    }

    auto existing = image_cache_.find(key);
    if (existing != image_cache_.end())
    {
        return existing->second.get();
    }

    auto texture = std::make_unique<ImageTexture>();
    if (!texture->load(renderer, path))
    {
        failed_images_.insert(key);
        return nullptr;
    }

    ImageTexture* result = texture.get();
    image_cache_[key] = std::move(texture);
    return result;
}
}
