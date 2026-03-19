#include "ui/text_renderer.hpp"

#include <array>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace bytedeck::ui
{
namespace
{
using Glyph = std::array<const char*, TextRenderer::glyph_height>;


const Glyph& fallback_glyph()
{
    static const Glyph glyph {
        "01110",
        "10001",
        "00010",
        "00100",
        "00100",
        "00000",
        "00100"
    };
    return glyph;
}


const std::unordered_map<char32_t, Glyph>& glyphs()
{
    static const std::unordered_map<char32_t, Glyph> map {
        { U' ', { "00000", "00000", "00000", "00000", "00000", "00000", "00000" } },
        { U'-', { "00000", "00000", "00000", "11111", "00000", "00000", "00000" } },
        { U'.', { "00000", "00000", "00000", "00000", "00000", "00110", "00110" } },
        { U',', { "00000", "00000", "00000", "00000", "00110", "00100", "01000" } },
        { U':', { "00000", "00110", "00110", "00000", "00110", "00110", "00000" } },
        { U';', { "00000", "00110", "00110", "00000", "00110", "00100", "01000" } },
        { U'!', { "00100", "00100", "00100", "00100", "00100", "00000", "00100" } },
        { U'?', { "01110", "10001", "00001", "00010", "00100", "00000", "00100" } },
        { U'\'', { "00100", "00100", "00000", "00000", "00000", "00000", "00000" } },
        { U'"', { "01010", "01010", "00000", "00000", "00000", "00000", "00000" } },
        { U'/', { "00001", "00010", "00100", "01000", "10000", "00000", "00000" } },
        { U'\\', { "10000", "01000", "00100", "00010", "00001", "00000", "00000" } },
        { U'(', { "00010", "00100", "01000", "01000", "01000", "00100", "00010" } },
        { U')', { "01000", "00100", "00010", "00010", "00010", "00100", "01000" } },
        { U'[', { "01110", "01000", "01000", "01000", "01000", "01000", "01110" } },
        { U']', { "01110", "00010", "00010", "00010", "00010", "00010", "01110" } },
        { U'&', { "01100", "10010", "10100", "01000", "10101", "10010", "01101" } },
        { U'+', { "00000", "00100", "00100", "11111", "00100", "00100", "00000" } },
        { U'_', { "00000", "00000", "00000", "00000", "00000", "00000", "11111" } },
        { U'0', { "01110", "10001", "10011", "10101", "11001", "10001", "01110" } },
        { U'1', { "00100", "01100", "00100", "00100", "00100", "00100", "01110" } },
        { U'2', { "01110", "10001", "00001", "00010", "00100", "01000", "11111" } },
        { U'3', { "11110", "00001", "00001", "01110", "00001", "00001", "11110" } },
        { U'4', { "00010", "00110", "01010", "10010", "11111", "00010", "00010" } },
        { U'5', { "11111", "10000", "10000", "11110", "00001", "00001", "11110" } },
        { U'6', { "00110", "01000", "10000", "11110", "10001", "10001", "01110" } },
        { U'7', { "11111", "00001", "00010", "00100", "01000", "01000", "01000" } },
        { U'8', { "01110", "10001", "10001", "01110", "10001", "10001", "01110" } },
        { U'9', { "01110", "10001", "10001", "01111", "00001", "00010", "11100" } },
        { U'A', { "01110", "10001", "10001", "11111", "10001", "10001", "10001" } },
        { U'B', { "11110", "10001", "10001", "11110", "10001", "10001", "11110" } },
        { U'C', { "01110", "10001", "10000", "10000", "10000", "10001", "01110" } },
        { U'D', { "11110", "10001", "10001", "10001", "10001", "10001", "11110" } },
        { U'E', { "11111", "10000", "10000", "11110", "10000", "10000", "11111" } },
        { U'F', { "11111", "10000", "10000", "11110", "10000", "10000", "10000" } },
        { U'G', { "01110", "10001", "10000", "10111", "10001", "10001", "01110" } },
        { U'H', { "10001", "10001", "10001", "11111", "10001", "10001", "10001" } },
        { U'I', { "01110", "00100", "00100", "00100", "00100", "00100", "01110" } },
        { U'J', { "00001", "00001", "00001", "00001", "10001", "10001", "01110" } },
        { U'K', { "10001", "10010", "10100", "11000", "10100", "10010", "10001" } },
        { U'L', { "10000", "10000", "10000", "10000", "10000", "10000", "11111" } },
        { U'M', { "10001", "11011", "10101", "10101", "10001", "10001", "10001" } },
        { U'N', { "10001", "10001", "11001", "10101", "10011", "10001", "10001" } },
        { U'O', { "01110", "10001", "10001", "10001", "10001", "10001", "01110" } },
        { U'P', { "11110", "10001", "10001", "11110", "10000", "10000", "10000" } },
        { U'Q', { "01110", "10001", "10001", "10001", "10101", "10010", "01101" } },
        { U'R', { "11110", "10001", "10001", "11110", "10100", "10010", "10001" } },
        { U'S', { "01111", "10000", "10000", "01110", "00001", "00001", "11110" } },
        { U'T', { "11111", "00100", "00100", "00100", "00100", "00100", "00100" } },
        { U'U', { "10001", "10001", "10001", "10001", "10001", "10001", "01110" } },
        { U'V', { "10001", "10001", "10001", "10001", "10001", "01010", "00100" } },
        { U'W', { "10001", "10001", "10001", "10101", "10101", "11011", "10001" } },
        { U'X', { "10001", "10001", "01010", "00100", "01010", "10001", "10001" } },
        { U'Y', { "10001", "10001", "01010", "00100", "00100", "00100", "00100" } },
        { U'Z', { "11111", "00001", "00010", "00100", "01000", "10000", "11111" } },
        { U'\u0410', { "01110", "10001", "10001", "11111", "10001", "10001", "10001" } },
        { U'\u0411', { "11111", "10000", "10000", "11110", "10001", "10001", "11110" } },
        { U'\u0412', { "11110", "10001", "10001", "11110", "10001", "10001", "11110" } },
        { U'\u0413', { "11111", "10000", "10000", "10000", "10000", "10000", "10000" } },
        { U'\u0414', { "01111", "01001", "01001", "01001", "11111", "10001", "10001" } },
        { U'\u0415', { "11111", "10000", "10000", "11110", "10000", "10000", "11111" } },
        { U'\u0401', { "11111", "10000", "10000", "11110", "10000", "10000", "11111" } },
        { U'\u0416', { "10101", "10101", "01110", "00100", "01110", "10101", "10101" } },
        { U'\u0417', { "11110", "00001", "00001", "01110", "00001", "00001", "11110" } },
        { U'\u0418', { "10001", "10011", "10101", "11001", "10001", "10001", "10001" } },
        { U'\u0419', { "10001", "10011", "10101", "11001", "10001", "10001", "10001" } },
        { U'\u041A', { "10001", "10010", "10100", "11000", "10100", "10010", "10001" } },
        { U'\u041B', { "00111", "01001", "10001", "10001", "10001", "10001", "10001" } },
        { U'\u041C', { "10001", "11011", "10101", "10101", "10001", "10001", "10001" } },
        { U'\u041D', { "10001", "10001", "10001", "11111", "10001", "10001", "10001" } },
        { U'\u041E', { "01110", "10001", "10001", "10001", "10001", "10001", "01110" } },
        { U'\u041F', { "11111", "10001", "10001", "10001", "10001", "10001", "10001" } },
        { U'\u0420', { "11110", "10001", "10001", "11110", "10000", "10000", "10000" } },
        { U'\u0421', { "01110", "10001", "10000", "10000", "10000", "10001", "01110" } },
        { U'\u0422', { "11111", "00100", "00100", "00100", "00100", "00100", "00100" } },
        { U'\u0423', { "10001", "10001", "01010", "00100", "00100", "01000", "10000" } },
        { U'\u0424', { "00100", "01110", "10101", "10101", "01110", "00100", "00100" } },
        { U'\u0425', { "10001", "10001", "01010", "00100", "01010", "10001", "10001" } },
        { U'\u0426', { "10001", "10001", "10001", "10001", "10001", "11111", "00001" } },
        { U'\u0427', { "10001", "10001", "10001", "01111", "00001", "00001", "00001" } },
        { U'\u0428', { "10101", "10101", "10101", "10101", "10101", "10101", "11111" } },
        { U'\u0429', { "10101", "10101", "10101", "10101", "10101", "11111", "00001" } },
        { U'\u042A', { "11000", "01000", "01110", "01001", "01001", "01001", "01110" } },
        { U'\u042B', { "10001", "10001", "11101", "10011", "10001", "10001", "10001" } },
        { U'\u042C', { "10000", "10000", "11110", "10001", "10001", "10001", "11110" } },
        { U'\u042D', { "01110", "10001", "00001", "00111", "00001", "10001", "01110" } },
        { U'\u042E', { "10010", "10101", "10101", "11101", "10101", "10101", "10010" } },
        { U'\u042F', { "01111", "10001", "10001", "01111", "00101", "01001", "10001" } }
    };

    return map;
}


std::size_t next_codepoint_size(unsigned char lead_byte)
{
    if ((lead_byte & 0x80U) == 0U)
    {
        return 1;
    }
    if ((lead_byte & 0xE0U) == 0xC0U)
    {
        return 2;
    }
    if ((lead_byte & 0xF0U) == 0xE0U)
    {
        return 3;
    }
    if ((lead_byte & 0xF8U) == 0xF0U)
    {
        return 4;
    }
    return 1;
}


std::vector<char32_t> decode_utf8(std::string_view text)
{
    std::vector<char32_t> codepoints;
    std::size_t index = 0;

    while (index < text.size())
    {
        const unsigned char lead = static_cast<unsigned char>(text[index]);
        const std::size_t width = next_codepoint_size(lead);

        if (width == 1 || index + width > text.size())
        {
            codepoints.push_back(static_cast<char32_t>(lead));
            ++index;
            continue;
        }

        char32_t codepoint = 0;
        if (width == 2)
        {
            codepoint =
                (static_cast<char32_t>(lead & 0x1FU) << 6) |
                static_cast<char32_t>(static_cast<unsigned char>(text[index + 1]) & 0x3FU);
        }
        else if (width == 3)
        {
            codepoint =
                (static_cast<char32_t>(lead & 0x0FU) << 12) |
                (static_cast<char32_t>(static_cast<unsigned char>(text[index + 1]) & 0x3FU) << 6) |
                static_cast<char32_t>(static_cast<unsigned char>(text[index + 2]) & 0x3FU);
        }
        else
        {
            codepoint =
                (static_cast<char32_t>(lead & 0x07U) << 18) |
                (static_cast<char32_t>(static_cast<unsigned char>(text[index + 1]) & 0x3FU) << 12) |
                (static_cast<char32_t>(static_cast<unsigned char>(text[index + 2]) & 0x3FU) << 6) |
                static_cast<char32_t>(static_cast<unsigned char>(text[index + 3]) & 0x3FU);
        }

        codepoints.push_back(codepoint);
        index += width;
    }

    return codepoints;
}


int utf8_length(std::string_view text)
{
    return static_cast<int>(decode_utf8(text).size());
}


std::string utf8_prefix(std::string_view text, int max_codepoints)
{
    if (max_codepoints <= 0)
    {
        return {};
    }

    std::size_t index = 0;
    int count = 0;
    while (index < text.size() && count < max_codepoints)
    {
        index += next_codepoint_size(static_cast<unsigned char>(text[index]));
        ++count;
    }

    return std::string(text.substr(0, index));
}


std::string utf8_drop_prefix(std::string_view text, int codepoints_to_drop)
{
    std::size_t index = 0;
    int count = 0;
    while (index < text.size() && count < codepoints_to_drop)
    {
        index += next_codepoint_size(static_cast<unsigned char>(text[index]));
        ++count;
    }

    return std::string(text.substr(index));
}


std::vector<std::string> wrap_lines(std::string_view text, int max_columns, int max_lines)
{
    std::vector<std::string> lines;
    if (max_columns <= 0 || max_lines <= 0)
    {
        return lines;
    }

    std::istringstream stream { std::string(text) };
    std::string raw_line;
    while (std::getline(stream, raw_line))
    {
        std::istringstream words { raw_line };
        std::string word;
        std::string current;

        while (words >> word)
        {
            const std::string candidate = current.empty() ? word : current + " " + word;
            if (utf8_length(candidate) <= max_columns)
            {
                current = candidate;
            }
            else
            {
                if (!current.empty())
                {
                    lines.push_back(current);
                    if (static_cast<int>(lines.size()) >= max_lines)
                    {
                        lines.back() = TextRenderer::truncate_to_width(lines.back(), max_columns);
                        return lines;
                    }
                }

                current = word;
                while (utf8_length(current) > max_columns)
                {
                    lines.push_back(utf8_prefix(current, max_columns));
                    current = utf8_drop_prefix(current, max_columns);
                    if (static_cast<int>(lines.size()) >= max_lines)
                    {
                        lines.back() = TextRenderer::truncate_to_width(lines.back(), max_columns);
                        return lines;
                    }
                }
            }
        }

        if (!current.empty())
        {
            lines.push_back(current);
            if (static_cast<int>(lines.size()) >= max_lines)
            {
                lines.back() = TextRenderer::truncate_to_width(lines.back(), max_columns);
                return lines;
            }
        }
        else if (raw_line.empty())
        {
            lines.emplace_back();
            if (static_cast<int>(lines.size()) >= max_lines)
            {
                return lines;
            }
        }
    }

    return lines;
}
}


void TextRenderer::draw_text(
    SDL_Renderer& renderer,
    std::string_view text,
    int x,
    int y,
    int scale,
    SDL_Color color
)
{
    if (scale <= 0)
    {
        return;
    }

    SDL_SetRenderDrawColor(&renderer, color.r, color.g, color.b, color.a);

    int cursor_x = x;
    const std::vector<char32_t> codepoints = decode_utf8(text);
    for (char32_t codepoint : codepoints)
    {
        const char32_t normalized = normalize_codepoint(codepoint);
        const auto it = glyphs().find(normalized);
        const Glyph& glyph = it != glyphs().end() ? it->second : fallback_glyph();

        for (int row = 0; row < glyph_height; ++row)
        {
            for (int column = 0; column < glyph_width; ++column)
            {
                if (glyph[row][column] != '1')
                {
                    continue;
                }

                SDL_Rect pixel {
                    cursor_x + column * scale,
                    y + row * scale,
                    scale,
                    scale
                };
                SDL_RenderFillRect(&renderer, &pixel);
            }
        }

        cursor_x += (glyph_width + glyph_spacing) * scale;
    }
}


void TextRenderer::draw_text_box(
    SDL_Renderer& renderer,
    std::string_view text,
    const SDL_Rect& bounds,
    int scale,
    SDL_Color color,
    int max_lines
)
{
    if (scale <= 0)
    {
        return;
    }

    const int max_columns = bounds.w / ((glyph_width + glyph_spacing) * scale);
    const std::vector<std::string> lines = wrap_lines(text, max_columns, max_lines);
    const int line_height = (glyph_height + 2) * scale;

    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        const int current_y = bounds.y + static_cast<int>(index) * line_height;
        if (current_y + glyph_height * scale > bounds.y + bounds.h)
        {
            break;
        }

        draw_text(renderer, lines[index], bounds.x, current_y, scale, color);
    }
}


int TextRenderer::measure_text_width(std::string_view text, int scale)
{
    if (text.empty())
    {
        return 0;
    }

    return utf8_length(text) * (glyph_width + glyph_spacing) * scale - glyph_spacing * scale;
}


char32_t TextRenderer::normalize_codepoint(char32_t codepoint)
{
    if (codepoint >= U'a' && codepoint <= U'z')
    {
        return codepoint - (U'a' - U'A');
    }

    if (codepoint >= U'\u0430' && codepoint <= U'\u044F')
    {
        return codepoint - (U'\u0430' - U'\u0410');
    }

    if (codepoint == U'\u0451')
    {
        return U'\u0401';
    }

    if ((codepoint >= 32 && codepoint <= 126) || (codepoint >= U'\u0410' && codepoint <= U'\u042F') || codepoint == U'\u0401')
    {
        return codepoint;
    }

    return U'?';
}


std::string TextRenderer::truncate_to_width(std::string_view text, int max_columns)
{
    if (max_columns <= 0)
    {
        return {};
    }

    if (utf8_length(text) <= max_columns)
    {
        return std::string(text);
    }

    if (max_columns <= 3)
    {
        return std::string(static_cast<std::size_t>(max_columns), '.');
    }

    return utf8_prefix(text, max_columns - 3) + "...";
}
}
