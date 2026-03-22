#include "ui/fixed_ui_renderer.hpp"

#include "ui/text_renderer.hpp"

#include <algorithm>
#include <cctype>
#include <vector>

namespace bytedeck::ui
{
namespace
{
SDL_Color kDefaultTextColor { 245, 241, 230, 255 };

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

Uint8 apply_opacity(Uint8 alpha, int opacity_percent)
{
    return static_cast<Uint8>((static_cast<int>(alpha) * std::clamp(opacity_percent, 0, 100)) / 100);
}

std::string string_or_default(const nlohmann::json& value, const std::string& default_value = "")
{
    return value.is_string() ? value.get<std::string>() : default_value;
}

bool bool_or_default(const nlohmann::json& value, bool default_value = false)
{
    return value.is_boolean() ? value.get<bool>() : default_value;
}

int int_or_default(const nlohmann::json& value, int default_value = 0)
{
    return value.is_number_integer() ? value.get<int>() : default_value;
}

const nlohmann::json* binding_ptr(const UiBindings& bindings, const char* key)
{
    if (!bindings.is_object() || !bindings.contains(key))
    {
        return nullptr;
    }

    return &bindings[key];
}

std::string binding_string(const UiBindings& bindings, const char* key, const std::string& default_value = "")
{
    const nlohmann::json* value = binding_ptr(bindings, key);
    return value != nullptr && value->is_string() ? value->get<std::string>() : default_value;
}

bool binding_bool(const UiBindings& bindings, const char* key, bool default_value = false)
{
    const nlohmann::json* value = binding_ptr(bindings, key);
    return value != nullptr && value->is_boolean() ? value->get<bool>() : default_value;
}
}


FixedUiRenderer::FixedUiRenderer(const ThemeManager& theme_manager)
    : theme_manager_(theme_manager)
    , font_renderer_(theme_manager)
{
}


void FixedUiRenderer::render(SDL_Renderer& renderer, const std::string& screen_id, const UiBindings& screen_bindings, const UiBindings& shell_bindings)
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(&renderer, &width, &height);
    const SDL_Rect app_bounds { 0, 0, width, height };

    render_shell_background(renderer, app_bounds);
    const SDL_Rect header_rect = render_header(renderer, app_bounds, shell_bindings);
    const SDL_Rect footer_rect = render_footer(renderer, app_bounds, shell_bindings);
    const SDL_Rect content_bounds {
        app_bounds.x,
        header_rect.y + header_rect.h,
        app_bounds.w,
        std::max(0, footer_rect.y - (header_rect.y + header_rect.h))
    };

    if (screen_id == "main_menu")
    {
        render_main_menu(renderer, content_bounds, screen_bindings);
    }
    else if (screen_id == "games")
    {
        render_games(renderer, content_bounds, screen_bindings);
    }
    else if (screen_id == "game_browser")
    {
        render_game_browser(renderer, content_bounds, screen_bindings);
    }
    else if (screen_id == "apps")
    {
        render_apps(renderer, content_bounds, screen_bindings);
    }
    else if (screen_id == "settings")
    {
        render_settings(renderer, content_bounds, screen_bindings);
    }
    else if (screen_id == "placeholder")
    {
        render_placeholder(renderer, content_bounds, screen_bindings);
    }
    else
    {
        render_unknown(renderer, content_bounds, screen_id);
    }
}


void FixedUiRenderer::render_shell_background(SDL_Renderer& renderer, const SDL_Rect& bounds)
{
    draw_box(renderer, bounds, style_at("shell.root"));
}


SDL_Rect FixedUiRenderer::render_header(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& shell_bindings)
{
    const int header_height = theme_manager_.int_at("shell.metrics.header_height", 64);
    const SDL_Rect header_bounds = anchored_top(bounds, header_height);
    const nlohmann::json header_style = style_at("shell.header");
    draw_box(renderer, header_bounds, header_style);

    const SDL_Rect content = inset_rect(header_bounds, resolve_spacing(header_style.value("padding", nlohmann::json(0))));
    const std::string brand = binding_string(shell_bindings, "brand", "BYTEDECK");
    const std::string context = binding_string(shell_bindings, "context");
    const std::string time = binding_string(shell_bindings, "time", "--:--");
    const std::string battery = binding_string(shell_bindings, "battery", "--%");

    const nlohmann::json brand_style = style_at("shell.header_brand");
    const nlohmann::json context_style = style_at("shell.header_context");
    const nlohmann::json meta_style = style_at("shell.header_meta");
    const int gap = theme_manager_.int_at("shell.metrics.header_gap", 24);
    const int meta_gap = theme_manager_.int_at("shell.metrics.header_meta_gap", 24);

    const int battery_width = text_width(battery, meta_style);
    const int time_width = text_width(time, meta_style);
    const int meta_total_width = battery_width + time_width + meta_gap;
    const SDL_Rect meta_rect { content.x + std::max(0, content.w - meta_total_width), content.y, meta_total_width, content.h };

    draw_text(renderer, time, SDL_Rect { meta_rect.x, meta_rect.y, time_width, meta_rect.h }, meta_style, TextAlign::left, true);
    draw_text(renderer, battery, SDL_Rect { meta_rect.x + time_width + meta_gap, meta_rect.y, battery_width, meta_rect.h }, meta_style, TextAlign::left, true);

    const int brand_width = text_width(brand, brand_style);
    draw_text(renderer, brand, SDL_Rect { content.x, content.y, brand_width, content.h }, brand_style, TextAlign::left, true);

    const int context_x = content.x + brand_width + gap;
    const int context_width = std::max(0, meta_rect.x - context_x - gap);
    draw_text(renderer, context, SDL_Rect { context_x, content.y, context_width, content.h }, context_style, TextAlign::left, true);

    return header_bounds;
}


SDL_Rect FixedUiRenderer::render_footer(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& shell_bindings)
{
    const int footer_height = theme_manager_.int_at("shell.metrics.footer_height", 74);
    const SDL_Rect footer_bounds = anchored_bottom(bounds, footer_height);
    const nlohmann::json footer_style = style_at("shell.footer");
    draw_box(renderer, footer_bounds, footer_style);

    const SDL_Rect content = inset_rect(footer_bounds, resolve_spacing(footer_style.value("padding", nlohmann::json(0))));
    const nlohmann::json action_style = style_at("shell.footer_action");
    const nlohmann::json hint_style = style_at("shell.footer_hint");
    const int action_gap = theme_manager_.int_at("shell.metrics.footer_action_gap", 28);

    int current_x = content.x;
    const nlohmann::json* actions = binding_ptr(shell_bindings, "actions");
    if (actions != nullptr && actions->is_array())
    {
        for (const nlohmann::json& action : *actions)
        {
            const std::string label = string_or_default(action.value("label", nlohmann::json {}));
            const std::string text = string_or_default(action.value("text", nlohmann::json {}));
            const std::string composed = text.empty() ? label : label + "  " + text;
            const int width = text_width(composed, action_style);
            draw_text(renderer, composed, SDL_Rect { current_x, content.y, width, content.h }, action_style, TextAlign::left, true);
            current_x += width + action_gap;
        }
    }

    const std::string hint = binding_string(shell_bindings, "hint");
    if (!hint.empty())
    {
        const int hint_width = text_width(hint, hint_style);
        draw_text(renderer, hint, SDL_Rect { content.x + std::max(0, content.w - hint_width), content.y, hint_width, content.h }, hint_style, TextAlign::left, true);
    }

    return footer_bounds;
}


void FixedUiRenderer::render_main_menu(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings)
{
    const Spacing padding = resolve_spacing(style_at("screens.main_menu.content").value("padding", nlohmann::json(0)));
    const SDL_Rect content = inset_rect(bounds, padding);
    const nlohmann::json brand_style = style_at("screens.main_menu.brand");
    const nlohmann::json subtitle_style = style_at("screens.main_menu.subtitle");
    const nlohmann::json section_style = style_at("screens.main_menu.section");

    const std::string brand = binding_string(bindings, "brand", "BYTEDECK");
    const std::string subtitle = binding_string(bindings, "subtitle");
    draw_text(renderer, brand, SDL_Rect { content.x, content.y, content.w, line_height(brand_style) }, brand_style);
    draw_text(renderer, subtitle, SDL_Rect { content.x, content.y + 72, content.w, line_height(subtitle_style) }, subtitle_style);

    const int section_width = theme_manager_.int_at("screens.main_menu.metrics.section_width", 1040);
    const int section_height = theme_manager_.int_at("screens.main_menu.metrics.section_height", 426);
    const SDL_Rect section_bounds {
        content.x + std::max(0, (content.w - section_width) / 2),
        content.y + theme_manager_.int_at("screens.main_menu.metrics.section_top", 130),
        std::min(section_width, content.w),
        std::min(section_height, std::max(0, content.h - theme_manager_.int_at("screens.main_menu.metrics.section_top", 130)))
    };
    draw_box(renderer, section_bounds, section_style);

    const SDL_Rect section_content = inset_rect(section_bounds, resolve_spacing(section_style.value("padding", nlohmann::json(0))));
    const nlohmann::json* items = binding_ptr(bindings, "items");
    if (items == nullptr || !items->is_array())
    {
        return;
    }

    const int card_width = theme_manager_.int_at("screens.main_menu.metrics.card_width", 280);
    const int card_height = theme_manager_.int_at("screens.main_menu.metrics.card_height", 346);
    const int card_gap = theme_manager_.int_at("screens.main_menu.metrics.card_gap", 68);
    const int visible_cards = std::min(3, static_cast<int>(items->size()));
    const int total_width = visible_cards > 0 ? visible_cards * card_width + (visible_cards - 1) * card_gap : 0;
    const int start_x = section_content.x + std::max(0, (section_content.w - total_width) / 2);
    const int start_y = section_content.y + std::max(0, (section_content.h - card_height) / 2);

    for (int index = 0; index < visible_cards; ++index)
    {
        const nlohmann::json& item = (*items)[index];
        const bool selected = item.value("selected", false);
        const SDL_Rect frame_bounds { start_x + index * (card_width + card_gap), start_y, card_width, card_height };
        const nlohmann::json frame_style = selected ? style_at("screens.main_menu.card_frame_selected") : style_at("screens.main_menu.card_frame");
        const nlohmann::json inner_style = selected ? style_at("screens.main_menu.card_inner_selected") : style_at("screens.main_menu.card_inner");
        draw_box(renderer, frame_bounds, frame_style);
        const SDL_Rect inner_bounds = inset_rect(frame_bounds, resolve_spacing(frame_style.value("padding", nlohmann::json(0))));
        draw_box(renderer, inner_bounds, inner_style);
        const SDL_Rect inner = inset_rect(inner_bounds, resolve_spacing(inner_style.value("padding", nlohmann::json(0))));

        draw_text(renderer, string_or_default(item.value("title", nlohmann::json {})), SDL_Rect { inner.x, inner.y, inner.w, line_height(style_at("screens.main_menu.card_title")) }, style_at("screens.main_menu.card_title"));
        draw_text(renderer, string_or_default(item.value("status", nlohmann::json {})), SDL_Rect { inner.x, inner.y + 126, inner.w, line_height(style_at("screens.main_menu.card_info")) }, style_at("screens.main_menu.card_info"));
        const nlohmann::json hint_style = selected ? style_at("screens.main_menu.card_hint_selected") : style_at("screens.main_menu.card_hint");
        draw_text(renderer, string_or_default(item.value("hint", nlohmann::json {})), SDL_Rect { inner.x, inner.y + inner.h - 60, inner.w, line_height(hint_style) }, hint_style);
        draw_box(renderer, SDL_Rect { inner.x, inner.y + inner.h - 18, inner.w, 18 }, selected ? style_at("screens.main_menu.card_accent_selected") : style_at("screens.main_menu.card_accent"));
    }

    const int page_count = std::max(1, static_cast<int>((items->size() + 2) / 3));
    int selected_index = 0;
    for (std::size_t index = 0; index < items->size(); ++index)
    {
        if ((*items)[index].value("selected", false))
        {
            selected_index = static_cast<int>(index);
            break;
        }
    }
    const int active_page = selected_index / 3;
    const int dot_width = theme_manager_.int_at("screens.main_menu.metrics.pager_dot_width", 42);
    const int dot_height = theme_manager_.int_at("screens.main_menu.metrics.pager_dot_height", 10);
    const int dot_gap = theme_manager_.int_at("screens.main_menu.metrics.pager_dot_gap", 12);
    const int pager_width = page_count * dot_width + std::max(0, page_count - 1) * dot_gap;
    const int pager_x = content.x + std::max(0, (content.w - pager_width) / 2);
    const int pager_y = section_bounds.y + section_bounds.h + 24;
    for (int index = 0; index < page_count; ++index)
    {
        draw_box(renderer, SDL_Rect { pager_x + index * (dot_width + dot_gap), pager_y, dot_width, dot_height }, index == active_page ? style_at("screens.main_menu.pager_dot_active") : style_at("screens.main_menu.pager_dot"));
    }
}


void FixedUiRenderer::render_games(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings)
{
    const Spacing padding = resolve_spacing(style_at("screens.games.content").value("padding", nlohmann::json(0)));
    const SDL_Rect content = inset_rect(bounds, padding);
    draw_text(renderer, binding_string(bindings, "title"), SDL_Rect { content.x, content.y, content.w, line_height(style_at("screens.games.title")) }, style_at("screens.games.title"));
    draw_text(renderer, binding_string(bindings, "subtitle"), SDL_Rect { content.x, content.y + 74, content.w, line_height(style_at("screens.games.subtitle")) }, style_at("screens.games.subtitle"));

    if (binding_bool(bindings, "empty"))
    {
        const SDL_Rect empty_bounds { content.x, content.y + 150, content.w, 220 };
        draw_box(renderer, empty_bounds, style_at("screens.games.empty_panel"));
        const SDL_Rect inner = inset_rect(empty_bounds, resolve_spacing(style_at("screens.games.empty_panel").value("padding", nlohmann::json(0))));
        draw_text(renderer, binding_string(bindings, "empty_title"), SDL_Rect { inner.x, inner.y, inner.w, line_height(style_at("screens.games.empty_title")) }, style_at("screens.games.empty_title"));
        draw_text(renderer, binding_string(bindings, "empty_body"), SDL_Rect { inner.x, inner.y + 56, inner.w, inner.h - 56 }, style_at("screens.games.empty_body"));
        return;
    }

    const nlohmann::json* items = binding_ptr(bindings, "items");
    if (items == nullptr || !items->is_array())
    {
        return;
    }

    const int tile_width = theme_manager_.int_at("screens.games.metrics.tile_width", 430);
    const int tile_height = theme_manager_.int_at("screens.games.metrics.tile_height", 240);
    const int tile_gap = theme_manager_.int_at("screens.games.metrics.tile_gap", 44);
    const int icon_size = theme_manager_.int_at("screens.games.metrics.icon_size", 104);
    const int columns = 2;
    const int total_width = columns * tile_width + (columns - 1) * tile_gap;
    const int start_x = content.x + std::max(0, (content.w - total_width) / 2);
    const int start_y = content.y + 150;

    for (std::size_t index = 0; index < items->size(); ++index)
    {
        const int row = static_cast<int>(index) / columns;
        const int column = static_cast<int>(index) % columns;
        const nlohmann::json& item = (*items)[index];
        const bool selected = item.value("selected", false);
        const SDL_Rect frame_bounds { start_x + column * (tile_width + tile_gap), start_y + row * (tile_height + tile_gap), tile_width, tile_height };
        const nlohmann::json frame_style = selected ? style_at("screens.games.tile_frame_selected") : style_at("screens.games.tile_frame");
        const nlohmann::json inner_style = selected ? style_at("screens.games.tile_inner_selected") : style_at("screens.games.tile_inner");
        draw_box(renderer, frame_bounds, frame_style);
        const SDL_Rect inner_bounds = inset_rect(frame_bounds, resolve_spacing(frame_style.value("padding", nlohmann::json(0))));
        draw_box(renderer, inner_bounds, inner_style);
        const SDL_Rect inner = inset_rect(inner_bounds, resolve_spacing(inner_style.value("padding", nlohmann::json(0))));

        SDL_Rect icon_bounds { inner.x, inner.y + 8, icon_size, icon_size };
        draw_box(renderer, icon_bounds, style_at("screens.games.system_icon"));
        const std::filesystem::path icon_path = theme_manager_.system_icon_path(string_or_default(item.value("system_id", nlohmann::json {})));
        if (!icon_path.empty())
        {
            draw_image(renderer, inset_rect(icon_bounds, resolve_spacing(style_at("screens.games.system_icon").value("padding", nlohmann::json(0)))), style_at("screens.games.system_icon"), icon_path);
        }

        const int text_x = icon_bounds.x + icon_bounds.w + 28;
        const int text_w = std::max(0, inner.x + inner.w - text_x);
        draw_text(renderer, string_or_default(item.value("title", nlohmann::json {})), SDL_Rect { text_x, inner.y + 8, text_w, line_height(style_at("screens.games.tile_title")) }, style_at("screens.games.tile_title"));
        draw_text(renderer, string_or_default(item.value("meta", nlohmann::json {})), SDL_Rect { text_x, inner.y + 84, text_w, line_height(style_at("screens.games.tile_meta")) }, style_at("screens.games.tile_meta"));
        draw_text(renderer, string_or_default(item.value("hint", nlohmann::json {})), SDL_Rect { text_x, inner.y + inner.h - 56, text_w, line_height(selected ? style_at("screens.games.tile_hint_selected") : style_at("screens.games.tile_hint")) }, selected ? style_at("screens.games.tile_hint_selected") : style_at("screens.games.tile_hint"));
    }
}
void FixedUiRenderer::render_game_browser(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings)
{
    const Spacing padding = resolve_spacing(style_at("screens.browser.content").value("padding", nlohmann::json(0)));
    const SDL_Rect content = inset_rect(bounds, padding);
    const int split_gap = theme_manager_.int_at("screens.browser.metrics.split_gap", 42);
    const int left_width = theme_manager_.int_at("screens.browser.metrics.left_width", (content.w * 48) / 100);
    const SDL_Rect left_bounds = split_left(content, left_width, split_gap);
    const SDL_Rect right_bounds = split_right(content, left_width, split_gap);

    draw_box(renderer, left_bounds, style_at("screens.browser.left_panel"));
    draw_box(renderer, right_bounds, style_at("screens.browser.right_panel"));

    const SDL_Rect left = inset_rect(left_bounds, resolve_spacing(style_at("screens.browser.left_panel").value("padding", nlohmann::json(0))));
    const int icon_size = theme_manager_.int_at("screens.browser.metrics.system_icon_size", 96);
    SDL_Rect icon_bounds { left.x, left.y + 4, icon_size, icon_size };
    draw_box(renderer, icon_bounds, style_at("screens.browser.system_icon"));
    const std::filesystem::path system_icon = theme_manager_.system_icon_path(binding_string(bindings, "system_id"));
    if (!system_icon.empty())
    {
        draw_image(renderer, inset_rect(icon_bounds, resolve_spacing(style_at("screens.browser.system_icon").value("padding", nlohmann::json(0)))), style_at("screens.browser.system_icon"), system_icon);
    }

    draw_text(renderer, binding_string(bindings, "title"), SDL_Rect { icon_bounds.x + icon_bounds.w + 28, left.y, left.w - icon_bounds.w - 28, line_height(style_at("screens.browser.title")) }, style_at("screens.browser.title"));
    draw_text(renderer, binding_string(bindings, "subtitle"), SDL_Rect { icon_bounds.x + icon_bounds.w + 28, left.y + 56, left.w - icon_bounds.w - 28, line_height(style_at("screens.browser.subtitle")) }, style_at("screens.browser.subtitle"));

    if (binding_bool(bindings, "empty"))
    {
        const SDL_Rect empty_bounds { left.x, left.y + 128, left.w, 180 };
        draw_box(renderer, empty_bounds, style_at("screens.browser.empty_panel"));
        const SDL_Rect empty = inset_rect(empty_bounds, resolve_spacing(style_at("screens.browser.empty_panel").value("padding", nlohmann::json(0))));
        draw_text(renderer, binding_string(bindings, "empty_title"), SDL_Rect { empty.x, empty.y, empty.w, line_height(style_at("screens.browser.empty_title")) }, style_at("screens.browser.empty_title"));
        draw_text(renderer, binding_string(bindings, "empty_body"), SDL_Rect { empty.x, empty.y + 56, empty.w, empty.h - 56 }, style_at("screens.browser.empty_body"));
    }
    else
    {
        const nlohmann::json* items = binding_ptr(bindings, "items");
        const int row_height = theme_manager_.int_at("screens.browser.metrics.row_height", 68);
        const int row_gap = theme_manager_.int_at("screens.browser.metrics.row_gap", 10);
        int row_y = left.y + 132;
        if (items != nullptr && items->is_array())
        {
            for (const nlohmann::json& item : *items)
            {
                const bool selected = item.value("selected", false);
                const SDL_Rect frame_bounds { left.x, row_y, left.w, row_height };
                const nlohmann::json frame_style = selected ? style_at("screens.browser.list_item_frame_selected") : style_at("screens.browser.list_item_frame");
                const nlohmann::json inner_style = selected ? style_at("screens.browser.list_item_inner_selected") : style_at("screens.browser.list_item_inner");
                draw_box(renderer, frame_bounds, frame_style);
                const SDL_Rect inner_bounds = inset_rect(frame_bounds, resolve_spacing(frame_style.value("padding", nlohmann::json(0))));
                draw_box(renderer, inner_bounds, inner_style);
                draw_text(renderer, string_or_default(item.value("title", nlohmann::json {})), inset_rect(inner_bounds, resolve_spacing(inner_style.value("padding", nlohmann::json(0)))), selected ? style_at("screens.browser.list_item_title_selected") : style_at("screens.browser.list_item_title"), TextAlign::left, true);
                row_y += row_height + row_gap;
            }
        }
    }

    const SDL_Rect right = inset_rect(right_bounds, resolve_spacing(style_at("screens.browser.right_panel").value("padding", nlohmann::json(0))));
    draw_text(renderer, binding_string(bindings, "preview_title"), SDL_Rect { right.x, right.y, right.w, line_height(style_at("screens.browser.preview_label")) }, style_at("screens.browser.preview_label"));
    const SDL_Rect preview_bounds { right.x, right.y + 58, right.w, theme_manager_.int_at("screens.browser.metrics.preview_height", 260) };
    draw_box(renderer, preview_bounds, style_at("screens.browser.preview_frame"));
    const SDL_Rect preview_inner = inset_rect(preview_bounds, resolve_spacing(style_at("screens.browser.preview_frame").value("padding", nlohmann::json(0))));
    const std::string preview_path = binding_string(bindings, "preview_path");
    if (!preview_path.empty())
    {
        draw_image(renderer, preview_inner, style_at("screens.browser.preview_frame"), std::filesystem::path(preview_path));
    }
    else
    {
        draw_placeholder_image(renderer, preview_inner, style_at("screens.browser.preview_frame"), binding_string(bindings, "preview_placeholder"));
    }

    int current_y = preview_bounds.y + preview_bounds.h + 28;
    draw_text(renderer, binding_string(bindings, "game_title"), SDL_Rect { right.x, current_y, right.w, line_height(style_at("screens.browser.detail_title")) }, style_at("screens.browser.detail_title"));
    current_y += 72;
    const std::vector<std::string> metadata_lines = {
        binding_string(bindings, "genre_line"),
        binding_string(bindings, "players_line"),
        binding_string(bindings, "date_line"),
        binding_string(bindings, "source_line")
    };
    for (const std::string& line : metadata_lines)
    {
        draw_text(renderer, line, SDL_Rect { right.x, current_y, right.w, line_height(style_at("screens.browser.metadata")) }, style_at("screens.browser.metadata"));
        current_y += 34;
    }
    if (binding_bool(bindings, "launch_status_success_visible"))
    {
        draw_text(renderer, binding_string(bindings, "launch_status_text"), SDL_Rect { right.x, current_y + 4, right.w, line_height(style_at("screens.browser.status_success")) }, style_at("screens.browser.status_success"));
        current_y += 38;
    }
    else if (binding_bool(bindings, "launch_status_error_visible"))
    {
        draw_text(renderer, binding_string(bindings, "launch_status_text"), SDL_Rect { right.x, current_y + 4, right.w, line_height(style_at("screens.browser.status_error")) }, style_at("screens.browser.status_error"));
        current_y += 38;
    }
    draw_text(renderer, binding_string(bindings, "description_title"), SDL_Rect { right.x, current_y + 8, right.w, line_height(style_at("screens.browser.section_title")) }, style_at("screens.browser.section_title"));
    current_y += 54;
    draw_text(renderer, binding_string(bindings, "description_text"), SDL_Rect { right.x, current_y, right.w, std::max(0, right.y + right.h - current_y) }, style_at("screens.browser.description"));
}


void FixedUiRenderer::render_apps(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings)
{
    const Spacing padding = resolve_spacing(style_at("screens.apps.content").value("padding", nlohmann::json(0)));
    const SDL_Rect content = inset_rect(bounds, padding);
    const int split_gap = theme_manager_.int_at("screens.apps.metrics.split_gap", 42);
    const int left_width = theme_manager_.int_at("screens.apps.metrics.left_width", (content.w * 48) / 100);
    const SDL_Rect left_bounds = split_left(content, left_width, split_gap);
    const SDL_Rect right_bounds = split_right(content, left_width, split_gap);

    draw_box(renderer, left_bounds, style_at("screens.apps.left_panel"));
    draw_box(renderer, right_bounds, style_at("screens.apps.right_panel"));

    const SDL_Rect left = inset_rect(left_bounds, resolve_spacing(style_at("screens.apps.left_panel").value("padding", nlohmann::json(0))));
    draw_text(renderer, binding_string(bindings, "title"), SDL_Rect { left.x, left.y, left.w, line_height(style_at("screens.apps.title")) }, style_at("screens.apps.title"));
    draw_text(renderer, binding_string(bindings, "subtitle"), SDL_Rect { left.x, left.y + 72, left.w, line_height(style_at("screens.apps.subtitle")) }, style_at("screens.apps.subtitle"));

    if (binding_bool(bindings, "empty"))
    {
        const SDL_Rect empty_bounds { left.x, left.y + 132, left.w, 220 };
        draw_box(renderer, empty_bounds, style_at("screens.apps.empty_panel"));
        const SDL_Rect empty = inset_rect(empty_bounds, resolve_spacing(style_at("screens.apps.empty_panel").value("padding", nlohmann::json(0))));
        draw_text(renderer, binding_string(bindings, "empty_title"), SDL_Rect { empty.x, empty.y, empty.w, line_height(style_at("screens.apps.empty_title")) }, style_at("screens.apps.empty_title"));
        draw_text(renderer, binding_string(bindings, "empty_body"), SDL_Rect { empty.x, empty.y + 56, empty.w, empty.h - 56 }, style_at("screens.apps.empty_body"));
        draw_text(renderer, binding_string(bindings, "empty_hint"), SDL_Rect { right_bounds.x + 32, right_bounds.y + 32, right_bounds.w - 64, line_height(style_at("screens.apps.empty_body")) }, style_at("screens.apps.empty_body"));
        return;
    }

    const nlohmann::json* items = binding_ptr(bindings, "items");
    const int row_height = theme_manager_.int_at("screens.apps.metrics.row_height", 68);
    const int row_gap = theme_manager_.int_at("screens.apps.metrics.row_gap", 10);
    int row_y = left.y + 132;
    if (items != nullptr && items->is_array())
    {
        for (const nlohmann::json& item : *items)
        {
            const bool selected = item.value("selected", false);
            const SDL_Rect frame_bounds { left.x, row_y, left.w, row_height };
            const nlohmann::json frame_style = selected ? style_at("screens.apps.list_item_frame_selected") : style_at("screens.apps.list_item_frame");
            const nlohmann::json inner_style = selected ? style_at("screens.apps.list_item_inner_selected") : style_at("screens.apps.list_item_inner");
            draw_box(renderer, frame_bounds, frame_style);
            const SDL_Rect inner_bounds = inset_rect(frame_bounds, resolve_spacing(frame_style.value("padding", nlohmann::json(0))));
            draw_box(renderer, inner_bounds, inner_style);
            draw_text(renderer, string_or_default(item.value("title", nlohmann::json {})), inset_rect(inner_bounds, resolve_spacing(inner_style.value("padding", nlohmann::json(0)))), selected ? style_at("screens.apps.list_item_title_selected") : style_at("screens.apps.list_item_title"), TextAlign::left, true);
            row_y += row_height + row_gap;
        }
    }

    const SDL_Rect right = inset_rect(right_bounds, resolve_spacing(style_at("screens.apps.right_panel").value("padding", nlohmann::json(0))));
    draw_text(renderer, binding_string(bindings, "preview_title"), SDL_Rect { right.x, right.y, right.w, line_height(style_at("screens.apps.preview_label")) }, style_at("screens.apps.preview_label"));
    const int preview_size = theme_manager_.int_at("screens.apps.metrics.preview_size", 240);
    const SDL_Rect preview_bounds { right.x, right.y + 58, preview_size, preview_size };
    draw_box(renderer, preview_bounds, style_at("screens.apps.preview_frame"));
    const SDL_Rect preview_inner = inset_rect(preview_bounds, resolve_spacing(style_at("screens.apps.preview_frame").value("padding", nlohmann::json(0))));
    const std::string preview_path = binding_string(bindings, "preview_path");
    if (!preview_path.empty())
    {
        draw_image(renderer, preview_inner, style_at("screens.apps.preview_frame"), std::filesystem::path(preview_path));
    }
    else
    {
        draw_placeholder_image(renderer, preview_inner, style_at("screens.apps.preview_frame"), binding_string(bindings, "preview_placeholder"));
    }

    const int text_x = preview_bounds.x + preview_bounds.w + 28;
    const int text_w = std::max(0, right.x + right.w - text_x);
    draw_text(renderer, binding_string(bindings, "app_title"), SDL_Rect { text_x, preview_bounds.y, text_w, line_height(style_at("screens.apps.detail_title")) }, style_at("screens.apps.detail_title"));
    draw_text(renderer, binding_string(bindings, "target_line"), SDL_Rect { text_x, preview_bounds.y + 74, text_w, line_height(style_at("screens.apps.metadata")) }, style_at("screens.apps.metadata"));
    if (binding_bool(bindings, "launch_status_success_visible"))
    {
        draw_text(renderer, binding_string(bindings, "launch_status_text"), SDL_Rect { text_x, preview_bounds.y + 118, text_w, line_height(style_at("screens.apps.status_success")) }, style_at("screens.apps.status_success"));
    }
    else if (binding_bool(bindings, "launch_status_error_visible"))
    {
        draw_text(renderer, binding_string(bindings, "launch_status_text"), SDL_Rect { text_x, preview_bounds.y + 118, text_w, line_height(style_at("screens.apps.status_error")) }, style_at("screens.apps.status_error"));
    }
    draw_text(renderer, binding_string(bindings, "description_title"), SDL_Rect { right.x, preview_bounds.y + preview_bounds.h + 34, right.w, line_height(style_at("screens.apps.section_title")) }, style_at("screens.apps.section_title"));
    draw_text(renderer, binding_string(bindings, "description_text"), SDL_Rect { right.x, preview_bounds.y + preview_bounds.h + 82, right.w, std::max(0, right.y + right.h - (preview_bounds.y + preview_bounds.h + 82)) }, style_at("screens.apps.description"));
}


void FixedUiRenderer::render_settings(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings)
{
    const Spacing padding = resolve_spacing(style_at("screens.settings.content").value("padding", nlohmann::json(0)));
    const SDL_Rect content = inset_rect(bounds, padding);
    const int split_gap = theme_manager_.int_at("screens.settings.metrics.split_gap", 42);
    const int left_width = theme_manager_.int_at("screens.settings.metrics.left_width", (content.w * 42) / 100);
    const SDL_Rect left_bounds = split_left(content, left_width, split_gap);
    const SDL_Rect right_bounds = split_right(content, left_width, split_gap);
    const bool focus_left = binding_bool(bindings, "focus_left");
    const bool focus_right = binding_bool(bindings, "focus_right");

    draw_box(renderer, left_bounds, focus_left ? style_at("screens.settings.left_panel_active") : style_at("screens.settings.left_panel"));
    draw_box(renderer, right_bounds, focus_right ? style_at("screens.settings.right_panel_active") : style_at("screens.settings.right_panel"));

    const SDL_Rect left = inset_rect(left_bounds, resolve_spacing((focus_left ? style_at("screens.settings.left_panel_active") : style_at("screens.settings.left_panel")).value("padding", nlohmann::json(0))));
    draw_text(renderer, binding_string(bindings, "title"), SDL_Rect { left.x, left.y, left.w, line_height(style_at("screens.settings.title")) }, style_at("screens.settings.title"));
    draw_text(renderer, binding_string(bindings, "subtitle"), SDL_Rect { left.x, left.y + 72, left.w, line_height(style_at("screens.settings.subtitle")) }, style_at("screens.settings.subtitle"));

    const nlohmann::json* items = binding_ptr(bindings, "items");
    const int item_height = theme_manager_.int_at("screens.settings.metrics.item_height", 126);
    const int item_gap = theme_manager_.int_at("screens.settings.metrics.item_gap", 12);
    int item_y = left.y + 136;
    if (items != nullptr && items->is_array())
    {
        for (const nlohmann::json& item : *items)
        {
            const bool selected = item.value("selected", false);
            const bool active = item.value("active", false);
            const nlohmann::json frame_style = active ? style_at("screens.settings.item_frame_active") : (selected ? style_at("screens.settings.item_frame_selected") : style_at("screens.settings.item_frame"));
            const nlohmann::json inner_style = active ? style_at("screens.settings.item_inner_active") : (selected ? style_at("screens.settings.item_inner_selected") : style_at("screens.settings.item_inner"));
            const SDL_Rect frame_bounds { left.x, item_y, left.w, item_height };
            draw_box(renderer, frame_bounds, frame_style);
            const SDL_Rect inner_bounds = inset_rect(frame_bounds, resolve_spacing(frame_style.value("padding", nlohmann::json(0))));
            draw_box(renderer, inner_bounds, inner_style);
            const SDL_Rect inner = inset_rect(inner_bounds, resolve_spacing(inner_style.value("padding", nlohmann::json(0))));
            draw_text(renderer, string_or_default(item.value("title", nlohmann::json {})), SDL_Rect { inner.x, inner.y, inner.w, line_height(style_at("screens.settings.item_title")) }, style_at("screens.settings.item_title"));
            draw_text(renderer, string_or_default(item.value("value", nlohmann::json {})), SDL_Rect { inner.x, inner.y + 56, inner.w, line_height(style_at("screens.settings.item_value")) }, style_at("screens.settings.item_value"));
            item_y += item_height + item_gap;
        }
    }

    const SDL_Rect right = inset_rect(right_bounds, resolve_spacing((focus_right ? style_at("screens.settings.right_panel_active") : style_at("screens.settings.right_panel")).value("padding", nlohmann::json(0))));
    draw_text(renderer, binding_string(bindings, "editor_title"), SDL_Rect { right.x, right.y, right.w, line_height(style_at("screens.settings.editor_title")) }, style_at("screens.settings.editor_title"));
    draw_text(renderer, binding_string(bindings, "editor_description"), SDL_Rect { right.x, right.y + 70, right.w, 96 }, style_at("screens.settings.editor_body"));
    draw_text(renderer, binding_string(bindings, "editor_hint"), SDL_Rect { right.x, right.y + 172, right.w, line_height(style_at("screens.settings.editor_hint")) }, style_at("screens.settings.editor_hint"));

    const nlohmann::json* editor_items = binding_ptr(bindings, "editor_items");
    const int editor_height = theme_manager_.int_at("screens.settings.metrics.editor_item_height", 112);
    const int editor_gap = theme_manager_.int_at("screens.settings.metrics.editor_item_gap", 12);
    int editor_y = right.y + 220;
    if (editor_items != nullptr && editor_items->is_array())
    {
        for (const nlohmann::json& item : *editor_items)
        {
            const bool selected = item.value("selected", false);
            const bool active = item.value("active", false);
            const nlohmann::json frame_style = active ? style_at("screens.settings.editor_item_frame_active") : (selected ? style_at("screens.settings.editor_item_frame_selected") : style_at("screens.settings.editor_item_frame"));
            const nlohmann::json inner_style = active ? style_at("screens.settings.editor_item_inner_active") : (selected ? style_at("screens.settings.editor_item_inner_selected") : style_at("screens.settings.editor_item_inner"));
            const SDL_Rect frame_bounds { right.x, editor_y, right.w, editor_height };
            draw_box(renderer, frame_bounds, frame_style);
            const SDL_Rect inner_bounds = inset_rect(frame_bounds, resolve_spacing(frame_style.value("padding", nlohmann::json(0))));
            draw_box(renderer, inner_bounds, inner_style);
            const SDL_Rect inner = inset_rect(inner_bounds, resolve_spacing(inner_style.value("padding", nlohmann::json(0))));
            draw_text(renderer, string_or_default(item.value("title", nlohmann::json {})), SDL_Rect { inner.x, inner.y, inner.w, line_height(selected ? style_at("screens.settings.editor_item_title_selected") : style_at("screens.settings.editor_item_title")) }, selected ? style_at("screens.settings.editor_item_title_selected") : style_at("screens.settings.editor_item_title"));
            draw_text(renderer, string_or_default(item.value("description", nlohmann::json {})), SDL_Rect { inner.x, inner.y + 44, inner.w, inner.h - 44 }, style_at("screens.settings.editor_item_description"));
            editor_y += editor_height + editor_gap;
        }
    }
    if (binding_bool(bindings, "editor_status_success"))
    {
        draw_text(renderer, binding_string(bindings, "editor_status_text"), SDL_Rect { right.x, right.y + right.h - 42, right.w, line_height(style_at("screens.settings.status_success")) }, style_at("screens.settings.status_success"));
    }
    else if (binding_bool(bindings, "editor_status_error"))
    {
        draw_text(renderer, binding_string(bindings, "editor_status_text"), SDL_Rect { right.x, right.y + right.h - 42, right.w, line_height(style_at("screens.settings.status_error")) }, style_at("screens.settings.status_error"));
    }
}


void FixedUiRenderer::render_placeholder(SDL_Renderer& renderer, const SDL_Rect& bounds, const UiBindings& bindings)
{
    const Spacing padding = resolve_spacing(style_at("screens.placeholder.content").value("padding", nlohmann::json(0)));
    const SDL_Rect content = inset_rect(bounds, padding);
    const SDL_Rect panel_bounds { content.x + std::max(0, (content.w - 760) / 2), content.y + 80, std::min(760, content.w), 320 };
    draw_box(renderer, panel_bounds, style_at("screens.placeholder.panel"));
    const SDL_Rect inner = inset_rect(panel_bounds, resolve_spacing(style_at("screens.placeholder.panel").value("padding", nlohmann::json(0))));
    draw_text(renderer, binding_string(bindings, "title"), SDL_Rect { inner.x, inner.y, inner.w, line_height(style_at("screens.placeholder.title")) }, style_at("screens.placeholder.title"));
    draw_text(renderer, binding_string(bindings, "subtitle"), SDL_Rect { inner.x, inner.y + 64, inner.w, line_height(style_at("screens.placeholder.subtitle")) }, style_at("screens.placeholder.subtitle"));
    draw_text(renderer, binding_string(bindings, "body"), SDL_Rect { inner.x, inner.y + 116, inner.w, inner.h - 116 }, style_at("screens.placeholder.body"));
}


void FixedUiRenderer::render_unknown(SDL_Renderer& renderer, const SDL_Rect& bounds, const std::string& screen_id)
{
    const SDL_Rect panel_bounds { bounds.x + std::max(0, (bounds.w - 640) / 2), bounds.y + std::max(0, (bounds.h - 240) / 2), std::min(640, bounds.w), std::min(240, bounds.h) };
    draw_box(renderer, panel_bounds, style_at("screens.unknown.panel"));
    const SDL_Rect inner = inset_rect(panel_bounds, resolve_spacing(style_at("screens.unknown.panel").value("padding", nlohmann::json(0))));
    draw_text(renderer, "UI SCREEN ERROR", SDL_Rect { inner.x, inner.y, inner.w, line_height(style_at("screens.unknown.title")) }, style_at("screens.unknown.title"));
    draw_text(renderer, screen_id, SDL_Rect { inner.x, inner.y + 64, inner.w, line_height(style_at("screens.unknown.subtitle")) }, style_at("screens.unknown.subtitle"));
}


void FixedUiRenderer::draw_box(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style)
{
    SDL_Color fill = color_prop(style, "background_color", SDL_Color { 0, 0, 0, 0 });
    SDL_Color border = color_prop(style, "border_color", SDL_Color { 0, 0, 0, 0 });
    const int border_width = std::max(0, int_prop(style, "border_width", 0));
    const int opacity = int_prop(style, "opacity", 100);
    fill.a = apply_opacity(fill.a, opacity);
    border.a = apply_opacity(border.a, opacity);

    if (fill.a > 0)
    {
        SDL_SetRenderDrawBlendMode(&renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(&renderer, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect(&renderer, &bounds);
    }

    const std::filesystem::path background_image = asset_path_from_style(style, "background_image");
    if (!background_image.empty())
    {
        draw_image(renderer, bounds, style, background_image);
    }

    if (border_width > 0 && border.a > 0)
    {
        SDL_SetRenderDrawBlendMode(&renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(&renderer, border.r, border.g, border.b, border.a);
        for (int index = 0; index < border_width; ++index)
        {
            SDL_Rect border_rect { bounds.x + index, bounds.y + index, bounds.w - index * 2, bounds.h - index * 2 };
            if (border_rect.w > 0 && border_rect.h > 0)
            {
                SDL_RenderDrawRect(&renderer, &border_rect);
            }
        }
    }
}


void FixedUiRenderer::draw_image(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style, const std::filesystem::path& path)
{
    if (ImageTexture* texture = load_image(renderer, path))
    {
        texture->render(renderer, bounds);
        return;
    }

    draw_placeholder_image(renderer, bounds, style, "");
}


void FixedUiRenderer::draw_placeholder_image(SDL_Renderer& renderer, const SDL_Rect& bounds, const nlohmann::json& style, const std::string& placeholder)
{
    (void)style;
    const std::string placeholder_text = placeholder.empty() ? "NO IMAGE" : placeholder;
    draw_text(renderer, placeholder_text, bounds, style_at("components.image_placeholder"), TextAlign::center, true);
}


void FixedUiRenderer::draw_text(
    SDL_Renderer& renderer,
    std::string_view text,
    const SDL_Rect& bounds,
    const nlohmann::json& style,
    TextAlign align,
    bool vertically_center)
{
    if (text.empty() || bounds.w <= 0 || bounds.h <= 0)
    {
        return;
    }

    const std::string font_role = string_prop(style, "font_role", "body");
    const int font_size = std::max(0, int_prop(style, "font_size", 0));
    const ThemeTypographyRole typography = theme_manager_.typography_role(font_role);
    const int scale = std::max(1, int_prop(style, "scale", std::max(1, typography.bitmap_scale)));
    const SDL_Color color = color_prop(style, "text_color", kDefaultTextColor);
    const bool wrap = bool_prop(style, "wrap", false);
    const bool truncate = bool_prop(style, "truncate", false);
    const int max_lines = std::max(1, int_prop(style, "max_lines", 1));
    const int resolved_line_height = line_height(style);
    const bool use_theme_font = font_renderer_.can_render(font_role);
    std::string output(text);

    if (!wrap && truncate)
    {
        output = use_theme_font
            ? font_renderer_.truncate_to_width(text, font_role, bounds.w, font_size)
            : TextRenderer::truncate_to_width(text, std::max(1, bounds.w / ((TextRenderer::glyph_width + TextRenderer::glyph_spacing) * scale)));
    }

    int x = bounds.x;
    if (!wrap)
    {
        const int measured_width = use_theme_font
            ? font_renderer_.measure_text_width(output, font_role, font_size)
            : TextRenderer::measure_text_width(output, scale);
        if (align == TextAlign::center)
        {
            x = bounds.x + std::max(0, (bounds.w - measured_width) / 2);
        }
        else if (align == TextAlign::right)
        {
            x = bounds.x + std::max(0, bounds.w - measured_width);
        }
    }

    int y = bounds.y;
    if (vertically_center)
    {
        y = bounds.y + std::max(0, (bounds.h - resolved_line_height) / 2);
    }

    if (use_theme_font)
    {
        if (wrap)
        {
            font_renderer_.draw_text_box(renderer, output, bounds, font_role, color, max_lines, font_size);
        }
        else
        {
            font_renderer_.draw_text(renderer, output, x, y, font_role, color, font_size);
        }
        return;
    }

    if (wrap)
    {
        TextRenderer::draw_text_box(renderer, output, bounds, scale, color, max_lines);
    }
    else
    {
        TextRenderer::draw_text(renderer, output, x, y, scale, color);
    }
}


SDL_Rect FixedUiRenderer::inset_rect(const SDL_Rect& bounds, const Spacing& spacing) const
{
    return SDL_Rect {
        bounds.x + spacing.left,
        bounds.y + spacing.top,
        std::max(0, bounds.w - spacing.left - spacing.right),
        std::max(0, bounds.h - spacing.top - spacing.bottom)
    };
}


SDL_Rect FixedUiRenderer::split_left(const SDL_Rect& bounds, int width, int gap) const
{
    (void)gap;
    return SDL_Rect { bounds.x, bounds.y, std::min(width, bounds.w), bounds.h };
}


SDL_Rect FixedUiRenderer::split_right(const SDL_Rect& bounds, int left_width, int gap) const
{
    const int x = bounds.x + left_width + gap;
    return SDL_Rect { x, bounds.y, std::max(0, bounds.w - left_width - gap), bounds.h };
}


SDL_Rect FixedUiRenderer::anchored_bottom(const SDL_Rect& bounds, int height) const
{
    return SDL_Rect { bounds.x, bounds.y + std::max(0, bounds.h - height), bounds.w, std::min(height, bounds.h) };
}


SDL_Rect FixedUiRenderer::anchored_top(const SDL_Rect& bounds, int height) const
{
    return SDL_Rect { bounds.x, bounds.y, bounds.w, std::min(height, bounds.h) };
}


int FixedUiRenderer::text_width(std::string_view text, const nlohmann::json& style) const
{
    const std::string font_role = string_prop(style, "font_role", "body");
    const int font_size = std::max(0, int_prop(style, "font_size", 0));
    const ThemeTypographyRole typography = theme_manager_.typography_role(font_role);
    const int scale = std::max(1, int_prop(style, "scale", std::max(1, typography.bitmap_scale)));
    return font_renderer_.can_render(font_role)
        ? font_renderer_.measure_text_width(text, font_role, font_size)
        : TextRenderer::measure_text_width(text, scale);
}


int FixedUiRenderer::line_height(const nlohmann::json& style) const
{
    const std::string font_role = string_prop(style, "font_role", "body");
    const int font_size = std::max(0, int_prop(style, "font_size", 0));
    const ThemeTypographyRole typography = theme_manager_.typography_role(font_role);
    const int scale = std::max(1, int_prop(style, "scale", std::max(1, typography.bitmap_scale)));
    return font_renderer_.can_render(font_role)
        ? std::max(1, font_renderer_.line_height(font_role, font_size))
        : TextRenderer::glyph_height * scale;
}


FixedUiRenderer::Spacing FixedUiRenderer::resolve_spacing(const nlohmann::json& value) const
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


SDL_Color FixedUiRenderer::color_from_json(const nlohmann::json& value, SDL_Color default_value) const
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
                resolved.a = apply_opacity(255, value["alpha"].get<int>());
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


SDL_Color FixedUiRenderer::color_prop(const nlohmann::json& style, const char* key, SDL_Color default_value) const
{
    if (!style.is_object() || !style.contains(key))
    {
        return default_value;
    }

    return color_from_json(style[key], default_value);
}


int FixedUiRenderer::int_prop(const nlohmann::json& style, const char* key, int default_value) const
{
    if (!style.is_object() || !style.contains(key))
    {
        return default_value;
    }

    return int_or_default(theme_manager_.resolve_value(style[key]), default_value);
}


bool FixedUiRenderer::bool_prop(const nlohmann::json& style, const char* key, bool default_value) const
{
    if (!style.is_object() || !style.contains(key))
    {
        return default_value;
    }

    return bool_or_default(theme_manager_.resolve_value(style[key]), default_value);
}


std::string FixedUiRenderer::string_prop(const nlohmann::json& style, const char* key, const std::string& default_value) const
{
    if (!style.is_object() || !style.contains(key))
    {
        return default_value;
    }

    return string_or_default(theme_manager_.resolve_value(style[key]), default_value);
}


nlohmann::json FixedUiRenderer::style_at(const std::string& path) const
{
    const nlohmann::json value = theme_manager_.value_at(path);
    return value.is_object() ? value : nlohmann::json::object();
}


std::filesystem::path FixedUiRenderer::asset_path_from_style(const nlohmann::json& style, const char* key) const
{
    if (!style.is_object() || !style.contains(key))
    {
        return {};
    }

    const nlohmann::json resolved = theme_manager_.resolve_value(style[key]);
    return resolved.is_string() ? theme_manager_.resolve_asset_path(resolved.get<std::string>()) : std::filesystem::path {};
}


ImageTexture* FixedUiRenderer::load_image(SDL_Renderer& renderer, const std::filesystem::path& path)
{
    if (path.empty())
    {
        return nullptr;
    }

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
