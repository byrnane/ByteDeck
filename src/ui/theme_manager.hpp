#pragma once

#include "ui/layout_registry.hpp"

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
    std::string screen_variant(const std::string& screen_id) const;
    nlohmann::json merge_style(const LayoutNode& node, const std::vector<std::string>& runtime_classes) const;
    nlohmann::json resolve_value(const nlohmann::json& value) const;
    std::filesystem::path resolve_asset_path(const std::string& path_value) const;
    std::filesystem::path system_icon_path(const std::string& system_id) const;
    std::filesystem::path font_path_for_family(const std::string& family) const;
    ThemeTypographyRole typography_role(const std::string& role) const;

private:
    static void merge_object(nlohmann::json& target, const nlohmann::json& source);
    const nlohmann::json* find_style_bucket(const char* bucket_name, const std::string& key) const;
    const nlohmann::json* find_token(const std::string& token_path) const;
    bool load_theme_file(const std::string& theme_id);

    std::filesystem::path themes_root_;
    std::string requested_theme_;
    std::string active_theme_id_ = "default";
    std::filesystem::path active_theme_root_;
    nlohmann::json theme_json_ = nlohmann::json::object();
};
}
