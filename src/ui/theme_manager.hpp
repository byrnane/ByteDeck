#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace bytedeck::ui
{
struct ThemeTypographyRole
{
    std::string family;
    int size = 16;
    int line_height = 0;
    int bitmap_scale = 2;
};


class ThemeManager
{
public:
    ThemeManager(std::filesystem::path themes_root, std::string requested_theme);

    bool load();
    const std::string& active_theme_id() const;
    const std::filesystem::path& active_theme_root() const;
    std::vector<std::string> available_theme_ids() const;
    nlohmann::json value_at(const std::string& path) const;
    int int_at(const std::string& path, int default_value) const;
    std::string string_at(const std::string& path, const std::string& default_value = "") const;
    bool bool_at(const std::string& path, bool default_value) const;
    nlohmann::json resolve_value(const nlohmann::json& value) const;
    std::filesystem::path resolve_asset_path(const std::string& path_value) const;
    std::filesystem::path system_icon_path(const std::string& system_id) const;
    std::filesystem::path font_path_for_family(const std::string& family) const;
    ThemeTypographyRole typography_role(const std::string& role) const;

private:
    const nlohmann::json* find_token(const std::string& token_path) const;
    const nlohmann::json* find_value_path(const std::string& path) const;
    bool load_theme_file(const std::string& theme_id);

    std::filesystem::path themes_root_;
    std::string requested_theme_;
    std::string active_theme_id_ = "default";
    std::filesystem::path active_theme_root_;
    nlohmann::json theme_json_ = nlohmann::json::object();
};
}
