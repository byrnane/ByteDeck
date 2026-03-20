#pragma once

#include "ui/layout_registry.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace bytedeck::ui
{
class ThemeManager
{
public:
    ThemeManager(std::filesystem::path themes_root, std::string requested_theme);

    bool load();
    const std::string& active_theme_id() const;
    std::string screen_variant(const std::string& screen_id) const;
    nlohmann::json merge_style(const LayoutNode& node, const std::vector<std::string>& runtime_classes) const;
    nlohmann::json resolve_value(const nlohmann::json& value) const;

private:
    static void merge_object(nlohmann::json& target, const nlohmann::json& source);
    const nlohmann::json* find_style_bucket(const char* bucket_name, const std::string& key) const;
    const nlohmann::json* find_token(const std::string& token_path) const;
    bool load_theme_file(const std::string& theme_id);

    std::filesystem::path themes_root_;
    std::string requested_theme_;
    std::string active_theme_id_ = "default";
    nlohmann::json theme_json_ = nlohmann::json::object();
};
}
