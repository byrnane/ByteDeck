#include "ui/theme_font_renderer.hpp"

#include "platform/logger.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

namespace bytedeck::ui
{
struct ThemeFontRenderer::FontFace
{
    std::filesystem::path path;
    std::vector<unsigned char> data;
    stbtt_fontinfo info {};
    int ascent = 0;
    int descent = 0;
    int line_gap = 0;
};


struct ThemeFontRenderer::CachedGlyph
{
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    int offset_x = 0;
    int offset_y = 0;
    int advance = 0;

    ~CachedGlyph()
    {
        if (texture != nullptr)
        {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
};


ThemeFontRenderer::ThemeFontRenderer(const ThemeManager& theme_manager)
    : theme_manager_(theme_manager)
{
}


ThemeFontRenderer::~ThemeFontRenderer() = default;


bool ThemeFontRenderer::GlyphKey::operator==(const GlyphKey& other) const
{
    return font_key == other.font_key && size == other.size && codepoint == other.codepoint;
}


std::size_t ThemeFontRenderer::GlyphKeyHash::operator()(const GlyphKey& key) const
{
    return std::hash<std::string>()(key.font_key) ^ (std::hash<int>()(key.size) << 1) ^ (std::hash<char32_t>()(key.codepoint) << 2);
}


bool ThemeFontRenderer::can_render(const std::string& role)
{
    return load_face_for_role(role) != nullptr;
}


int ThemeFontRenderer::measure_text_width(std::string_view text, const std::string& role, int size_override)
{
    FontFace* face = load_face_for_role(role);
    if (face == nullptr)
    {
        return 0;
    }

    const ThemeTypographyRole resolved = resolve_role(role, size_override);
    const float scale = stbtt_ScaleForPixelHeight(&face->info, static_cast<float>(resolved.size));
    int width = 0;
    for (char32_t codepoint : decode_utf8(text))
    {
        int advance = 0;
        int left_bearing = 0;
        stbtt_GetCodepointHMetrics(&face->info, static_cast<int>(codepoint), &advance, &left_bearing);
        (void)left_bearing;
        width += static_cast<int>(advance * scale);
    }

    return width;
}


int ThemeFontRenderer::line_height(const std::string& role, int size_override)
{
    FontFace* face = load_face_for_role(role);
    if (face == nullptr)
    {
        return 0;
    }

    const ThemeTypographyRole resolved = resolve_role(role, size_override);
    if (resolved.line_height > 0)
    {
        return resolved.line_height;
    }

    const float scale = stbtt_ScaleForPixelHeight(&face->info, static_cast<float>(resolved.size));
    return static_cast<int>((face->ascent - face->descent + face->line_gap) * scale);
}


std::string ThemeFontRenderer::truncate_to_width(std::string_view text, const std::string& role, int max_width, int size_override)
{
    FontFace* face = load_face_for_role(role);
    if (face == nullptr || max_width <= 0)
    {
        return {};
    }

    if (measure_text_width(text, role, size_override) <= max_width)
    {
        return std::string(text);
    }

    const std::string ellipsis = "...";
    const int ellipsis_width = measure_text_width(ellipsis, role, size_override);
    const std::vector<char32_t> codepoints = decode_utf8(text);

    std::string output;
    int count = 0;
    for (char32_t codepoint : codepoints)
    {
        (void)codepoint;
        const std::string candidate = utf8_prefix(text, count + 1) + ellipsis;
        if (measure_text_width(candidate, role, size_override) > max_width)
        {
            break;
        }

        output = candidate;
        ++count;
    }

    if (output.empty() && ellipsis_width <= max_width)
    {
        return ellipsis;
    }

    return output;
}


bool ThemeFontRenderer::draw_text(
    SDL_Renderer& renderer,
    std::string_view text,
    int x,
    int y,
    const std::string& role,
    SDL_Color color,
    int size_override)
{
    FontFace* face = load_face_for_role(role);
    if (face == nullptr)
    {
        return false;
    }

    const ThemeTypographyRole resolved = resolve_role(role, size_override);
    const float scale = stbtt_ScaleForPixelHeight(&face->info, static_cast<float>(resolved.size));
    const int baseline = y + static_cast<int>(face->ascent * scale);
    int cursor_x = x;

    for (char32_t codepoint : decode_utf8(text))
    {
        CachedGlyph* glyph = load_glyph(renderer, *face, resolved.size, codepoint);
        if (glyph == nullptr)
        {
            continue;
        }

        if (glyph->texture != nullptr && glyph->width > 0 && glyph->height > 0)
        {
            SDL_SetTextureColorMod(glyph->texture, color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(glyph->texture, color.a);

            const SDL_Rect destination {
                cursor_x + glyph->offset_x,
                baseline + glyph->offset_y,
                glyph->width,
                glyph->height
            };
            SDL_RenderCopy(&renderer, glyph->texture, nullptr, &destination);
        }

        cursor_x += glyph->advance;
    }

    return true;
}


bool ThemeFontRenderer::draw_text_box(
    SDL_Renderer& renderer,
    std::string_view text,
    const SDL_Rect& bounds,
    const std::string& role,
    SDL_Color color,
    int max_lines,
    int size_override)
{
    FontFace* face = load_face_for_role(role);
    if (face == nullptr)
    {
        return false;
    }

    const std::vector<std::string> lines = wrap_lines(text, role, bounds.w, max_lines, size_override);
    const int resolved_line_height = std::max(1, line_height(role, size_override));

    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        const int current_y = bounds.y + static_cast<int>(index) * resolved_line_height;
        if (current_y + resolved_line_height > bounds.y + bounds.h)
        {
            break;
        }

        draw_text(renderer, lines[index], bounds.x, current_y, role, color, size_override);
    }

    return true;
}


ThemeFontRenderer::FontFace* ThemeFontRenderer::load_face_for_role(const std::string& role)
{
    const ThemeTypographyRole resolved = resolve_role(role, 0);
    if (resolved.family.empty())
    {
        return nullptr;
    }

    const std::filesystem::path path = theme_manager_.font_path_for_family(resolved.family);
    if (path.empty())
    {
        return nullptr;
    }

    const std::string key = path.lexically_normal().string();
    if (failed_faces_.find(key) != failed_faces_.end())
    {
        return nullptr;
    }

    auto existing = faces_.find(key);
    if (existing != faces_.end())
    {
        return existing->second.get();
    }

    auto face = std::make_unique<FontFace>();
    face->path = path;

    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        failed_faces_[key] = true;
        return nullptr;
    }

    face->data.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (face->data.empty() || stbtt_InitFont(&face->info, face->data.data(), 0) == 0)
    {
        failed_faces_[key] = true;
        platform::Logger::instance().warn("Failed to initialize theme font: " + path.string());
        return nullptr;
    }

    stbtt_GetFontVMetrics(&face->info, &face->ascent, &face->descent, &face->line_gap);

    FontFace* result = face.get();
    faces_[key] = std::move(face);
    return result;
}


ThemeFontRenderer::CachedGlyph* ThemeFontRenderer::load_glyph(SDL_Renderer& renderer, FontFace& face, int pixel_size, char32_t codepoint)
{
    const GlyphKey key { face.path.lexically_normal().string(), pixel_size, codepoint };
    auto existing = glyphs_.find(key);
    if (existing != glyphs_.end())
    {
        return existing->second.get();
    }

    const float scale = stbtt_ScaleForPixelHeight(&face.info, static_cast<float>(pixel_size));
    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;
    stbtt_GetCodepointBitmapBox(&face.info, static_cast<int>(codepoint), scale, scale, &x0, &y0, &x1, &y1);

    int advance = 0;
    int left_bearing = 0;
    stbtt_GetCodepointHMetrics(&face.info, static_cast<int>(codepoint), &advance, &left_bearing);

    auto glyph = std::make_unique<CachedGlyph>();
    glyph->advance = static_cast<int>(advance * scale);
    glyph->offset_x = x0;
    glyph->offset_y = y0;
    glyph->width = std::max(0, x1 - x0);
    glyph->height = std::max(0, y1 - y0);

    if (glyph->width > 0 && glyph->height > 0)
    {
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&face.info, 0.0f, scale, static_cast<int>(codepoint), &glyph->width, &glyph->height, nullptr, nullptr);
        if (bitmap != nullptr)
        {
            std::vector<unsigned char> rgba(static_cast<std::size_t>(glyph->width * glyph->height * 4), 255);
            for (int index = 0; index < glyph->width * glyph->height; ++index)
            {
                rgba[index * 4 + 3] = bitmap[index];
            }

            glyph->texture = SDL_CreateTexture(&renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, glyph->width, glyph->height);
            if (glyph->texture != nullptr)
            {
                SDL_UpdateTexture(glyph->texture, nullptr, rgba.data(), glyph->width * 4);
                SDL_SetTextureBlendMode(glyph->texture, SDL_BLENDMODE_BLEND);
            }

            stbtt_FreeBitmap(bitmap, nullptr);
        }
    }

    CachedGlyph* result = glyph.get();
    glyphs_[key] = std::move(glyph);
    return result;
}


ThemeTypographyRole ThemeFontRenderer::resolve_role(const std::string& role, int size_override) const
{
    ThemeTypographyRole resolved = theme_manager_.typography_role(role);
    if (size_override > 0)
    {
        resolved.size = size_override;
    }
    return resolved;
}


std::vector<std::string> ThemeFontRenderer::wrap_lines(std::string_view text, const std::string& role, int max_width, int max_lines, int size_override)
{
    std::vector<std::string> lines;
    if (max_width <= 0 || max_lines <= 0)
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
            if (measure_text_width(candidate, role, size_override) <= max_width)
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
                        lines.back() = truncate_to_width(lines.back(), role, max_width, size_override);
                        return lines;
                    }
                }

                current = word;
                while (measure_text_width(current, role, size_override) > max_width)
                {
                    std::string best_fit;
                    for (int codepoints = 1; codepoints <= utf8_length(current); ++codepoints)
                    {
                        const std::string candidate_part = utf8_prefix(current, codepoints);
                        if (measure_text_width(candidate_part, role, size_override) > max_width)
                        {
                            break;
                        }
                        best_fit = candidate_part;
                    }

                    if (best_fit.empty())
                    {
                        break;
                    }

                    lines.push_back(best_fit);
                    current = utf8_drop_prefix(current, utf8_length(best_fit));
                    if (static_cast<int>(lines.size()) >= max_lines)
                    {
                        lines.back() = truncate_to_width(lines.back(), role, max_width, size_override);
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
                lines.back() = truncate_to_width(lines.back(), role, max_width, size_override);
                return lines;
            }
        }
        else if (raw_line.empty())
        {
            lines.emplace_back();
        }
    }

    return lines;
}


std::vector<char32_t> ThemeFontRenderer::decode_utf8(std::string_view text)
{
    std::vector<char32_t> codepoints;
    std::size_t index = 0;

    while (index < text.size())
    {
        const unsigned char lead = static_cast<unsigned char>(text[index]);
        std::size_t width = 1;
        if ((lead & 0x80U) == 0U)
        {
            width = 1;
        }
        else if ((lead & 0xE0U) == 0xC0U)
        {
            width = 2;
        }
        else if ((lead & 0xF0U) == 0xE0U)
        {
            width = 3;
        }
        else if ((lead & 0xF8U) == 0xF0U)
        {
            width = 4;
        }

        if (index + width > text.size() || width == 1)
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


std::string ThemeFontRenderer::utf8_prefix(std::string_view text, int max_codepoints)
{
    if (max_codepoints <= 0)
    {
        return {};
    }

    std::size_t index = 0;
    int count = 0;
    while (index < text.size() && count < max_codepoints)
    {
        const unsigned char lead = static_cast<unsigned char>(text[index]);
        if ((lead & 0x80U) == 0U)
        {
            index += 1;
        }
        else if ((lead & 0xE0U) == 0xC0U)
        {
            index += 2;
        }
        else if ((lead & 0xF0U) == 0xE0U)
        {
            index += 3;
        }
        else if ((lead & 0xF8U) == 0xF0U)
        {
            index += 4;
        }
        else
        {
            index += 1;
        }
        ++count;
    }

    return std::string(text.substr(0, index));
}


std::string ThemeFontRenderer::utf8_drop_prefix(std::string_view text, int codepoints_to_drop)
{
    std::size_t index = 0;
    int count = 0;
    while (index < text.size() && count < codepoints_to_drop)
    {
        const unsigned char lead = static_cast<unsigned char>(text[index]);
        if ((lead & 0x80U) == 0U)
        {
            index += 1;
        }
        else if ((lead & 0xE0U) == 0xC0U)
        {
            index += 2;
        }
        else if ((lead & 0xF0U) == 0xE0U)
        {
            index += 3;
        }
        else if ((lead & 0xF8U) == 0xF0U)
        {
            index += 4;
        }
        else
        {
            index += 1;
        }
        ++count;
    }

    return std::string(text.substr(index));
}


int ThemeFontRenderer::utf8_length(std::string_view text)
{
    return static_cast<int>(decode_utf8(text).size());
}
}
