#pragma once

#include <SDL.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace bytedeck::ui
{
class TextRenderer
{
public:
    static constexpr int glyph_width = 5;
    static constexpr int glyph_height = 7;
    static constexpr int glyph_spacing = 1;

    static void draw_text(
        SDL_Renderer& renderer,
        std::string_view text,
        int x,
        int y,
        int scale,
        SDL_Color color
    );

    static void draw_text_box(
        SDL_Renderer& renderer,
        std::string_view text,
        const SDL_Rect& bounds,
        int scale,
        SDL_Color color,
        int max_lines
    );

    static int measure_text_width(std::string_view text, int scale);
    static std::string truncate_to_width(std::string_view text, int max_columns);

private:
    static char32_t normalize_codepoint(char32_t codepoint);
};
}
