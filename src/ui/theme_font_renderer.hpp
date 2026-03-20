#pragma once

#include "ui/theme_manager.hpp"

#include <SDL.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace bytedeck::ui
{
class ThemeFontRenderer
{
public:
    explicit ThemeFontRenderer(const ThemeManager& theme_manager);
    ~ThemeFontRenderer();

    ThemeFontRenderer(const ThemeFontRenderer&) = delete;
    ThemeFontRenderer& operator=(const ThemeFontRenderer&) = delete;

    bool can_render(const std::string& role);
    int measure_text_width(std::string_view text, const std::string& role, int size_override = 0);
    int line_height(const std::string& role, int size_override = 0);
    std::string truncate_to_width(std::string_view text, const std::string& role, int max_width, int size_override = 0);
    bool draw_text(
        SDL_Renderer& renderer,
        std::string_view text,
        int x,
        int y,
        const std::string& role,
        SDL_Color color,
        int size_override = 0);
    bool draw_text_box(
        SDL_Renderer& renderer,
        std::string_view text,
        const SDL_Rect& bounds,
        const std::string& role,
        SDL_Color color,
        int max_lines,
        int size_override = 0);

private:
    struct FontFace;
    struct CachedGlyph;

    struct GlyphKey
    {
        std::string font_key;
        int size = 0;
        char32_t codepoint = 0;

        bool operator==(const GlyphKey& other) const;
    };

    struct GlyphKeyHash
    {
        std::size_t operator()(const GlyphKey& key) const;
    };

    FontFace* load_face_for_role(const std::string& role);
    CachedGlyph* load_glyph(SDL_Renderer& renderer, FontFace& face, int pixel_size, char32_t codepoint);
    ThemeTypographyRole resolve_role(const std::string& role, int size_override) const;
    std::vector<std::string> wrap_lines(std::string_view text, const std::string& role, int max_width, int max_lines, int size_override);
    static std::vector<char32_t> decode_utf8(std::string_view text);
    static std::string utf8_prefix(std::string_view text, int max_codepoints);
    static std::string utf8_drop_prefix(std::string_view text, int codepoints_to_drop);
    static int utf8_length(std::string_view text);

    const ThemeManager& theme_manager_;
    std::unordered_map<std::string, std::unique_ptr<FontFace>> faces_;
    std::unordered_map<GlyphKey, std::unique_ptr<CachedGlyph>, GlyphKeyHash> glyphs_;
    std::unordered_map<std::string, bool> failed_faces_;
};
}
