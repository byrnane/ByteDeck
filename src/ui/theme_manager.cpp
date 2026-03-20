#include "ui/theme_manager.hpp"

#include "platform/logger.hpp"

#include <algorithm>
#include <fstream>

namespace bytedeck::ui
{
ThemeManager::ThemeManager(std::filesystem::path themes_root, std::string requested_theme)
    : themes_root_(std::move(themes_root))
    , requested_theme_(std::move(requested_theme))
{
}


bool ThemeManager::load()
{
    if (!requested_theme_.empty() && load_theme_file(requested_theme_))
    {
        active_theme_id_ = requested_theme_;
        return true;
    }

    if (requested_theme_ != "default")
    {
        platform::Logger::instance().warn("Falling back to default theme from '" + requested_theme_ + "'");
    }

    if (load_theme_file("default"))
    {
        active_theme_id_ = "default";
        return true;
    }

    platform::Logger::instance().error("Failed to load default UI theme");
    theme_json_ = nlohmann::json::object();
    active_theme_id_ = "default";
    return false;
}


const std::string& ThemeManager::active_theme_id() const
{
    return active_theme_id_;
}


const std::filesystem::path& ThemeManager::active_theme_root() const
{
    return active_theme_root_;
}


std::vector<std::string> ThemeManager::available_theme_ids() const
{
    std::vector<std::string> theme_ids;
    if (!std::filesystem::exists(themes_root_))
    {
        return theme_ids;
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(themes_root_))
        {
            if (!entry.is_directory())
            {
                continue;
            }

            const std::filesystem::path theme_file = entry.path() / "theme.json";
            if (std::filesystem::exists(theme_file))
            {
                theme_ids.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const std::exception& exception)
    {
        platform::Logger::instance().warn(std::string("Failed to enumerate themes: ") + exception.what());
    }

    std::sort(theme_ids.begin(), theme_ids.end());
    return theme_ids;
}


std::string ThemeManager::screen_variant(const std::string& screen_id) const
{
    if (theme_json_.contains("screen_variants") && theme_json_["screen_variants"].contains(screen_id) && theme_json_["screen_variants"][screen_id].is_string())
    {
        return theme_json_["screen_variants"][screen_id].get<std::string>();
    }

    return "default";
}


nlohmann::json ThemeManager::merge_style(const LayoutNode& node, const std::vector<std::string>& runtime_classes) const
{
    nlohmann::json style = nlohmann::json::object();

    if (const nlohmann::json* by_type = find_style_bucket("types", node.type))
    {
        merge_object(style, *by_type);
    }

    for (const std::string& class_name : node.classes)
    {
        if (const nlohmann::json* by_class = find_style_bucket("classes", class_name))
        {
            merge_object(style, *by_class);
        }
    }

    for (const std::string& class_name : runtime_classes)
    {
        if (const nlohmann::json* by_class = find_style_bucket("classes", class_name))
        {
            merge_object(style, *by_class);
        }
    }

    if (!node.id.empty())
    {
        if (const nlohmann::json* by_id = find_style_bucket("ids", node.id))
        {
            merge_object(style, *by_id);
        }
    }

    merge_object(style, node.props);
    return style;
}


nlohmann::json ThemeManager::resolve_value(const nlohmann::json& value) const
{
    if (value.is_string())
    {
        const std::string string_value = value.get<std::string>();
        if (!string_value.empty() && string_value.front() == '$')
        {
            const nlohmann::json* token_value = find_token(string_value.substr(1));
            if (token_value != nullptr)
            {
                return resolve_value(*token_value);
            }
        }

        return value;
    }

    if (value.is_array())
    {
        nlohmann::json resolved = nlohmann::json::array();
        for (const nlohmann::json& entry : value)
        {
            resolved.push_back(resolve_value(entry));
        }
        return resolved;
    }

    if (value.is_object())
    {
        nlohmann::json resolved = nlohmann::json::object();
        for (auto it = value.begin(); it != value.end(); ++it)
        {
            resolved[it.key()] = resolve_value(it.value());
        }
        return resolved;
    }

    return value;
}


std::filesystem::path ThemeManager::resolve_asset_path(const std::string& path_value) const
{
    if (path_value.empty())
    {
        return {};
    }

    const std::filesystem::path path(path_value);
    if (path.is_absolute())
    {
        return path;
    }

    return active_theme_root_ / path;
}


std::filesystem::path ThemeManager::system_icon_path(const std::string& system_id) const
{
    if (system_id.empty() || !theme_json_.contains("system_icons"))
    {
        return {};
    }

    const nlohmann::json& system_icons = theme_json_["system_icons"];
    if (!system_icons.is_object() || !system_icons.contains(system_id) || !system_icons[system_id].is_string())
    {
        return {};
    }

    return resolve_asset_path(system_icons[system_id].get<std::string>());
}


std::filesystem::path ThemeManager::font_path_for_family(const std::string& family) const
{
    if (family.empty() || !theme_json_.contains("fonts"))
    {
        return {};
    }

    const nlohmann::json& fonts = theme_json_["fonts"];
    if (!fonts.is_object() || !fonts.contains(family))
    {
        return {};
    }

    const nlohmann::json& definition = fonts[family];
    if (definition.is_string())
    {
        return resolve_asset_path(definition.get<std::string>());
    }

    if (!definition.is_object() || !definition.contains("path") || !definition["path"].is_string())
    {
        return {};
    }

    return resolve_asset_path(definition["path"].get<std::string>());
}


ThemeTypographyRole ThemeManager::typography_role(const std::string& role) const
{
    ThemeTypographyRole result;
    if (role.empty() || !theme_json_.contains("typography"))
    {
        return result;
    }

    const nlohmann::json& typography = theme_json_["typography"];
    if (!typography.is_object() || !typography.contains(role) || !typography[role].is_object())
    {
        return result;
    }

    const nlohmann::json& definition = typography[role];
    result.family = definition.value("family", result.family);
    result.size = definition.value("size", result.size);
    result.line_height = definition.value("line_height", result.line_height);
    result.bitmap_scale = definition.value("bitmap_scale", result.bitmap_scale);
    return result;
}


void ThemeManager::merge_object(nlohmann::json& target, const nlohmann::json& source)
{
    if (!source.is_object())
    {
        return;
    }

    for (auto it = source.begin(); it != source.end(); ++it)
    {
        target[it.key()] = it.value();
    }
}


const nlohmann::json* ThemeManager::find_style_bucket(const char* bucket_name, const std::string& key) const
{
    if (!theme_json_.contains("styles") || !theme_json_["styles"].contains(bucket_name))
    {
        return nullptr;
    }

    const nlohmann::json& bucket = theme_json_["styles"][bucket_name];
    if (!bucket.is_object() || !bucket.contains(key))
    {
        return nullptr;
    }

    return &bucket[key];
}


const nlohmann::json* ThemeManager::find_token(const std::string& token_path) const
{
    if (!theme_json_.contains("tokens"))
    {
        return nullptr;
    }

    const nlohmann::json* current = &theme_json_["tokens"];
    std::size_t start = 0;
    while (start < token_path.size())
    {
        const std::size_t dot = token_path.find('.', start);
        const std::string segment = token_path.substr(start, dot == std::string::npos ? std::string::npos : dot - start);
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


bool ThemeManager::load_theme_file(const std::string& theme_id)
{
    const std::filesystem::path path = themes_root_ / theme_id / "theme.json";
    if (!std::filesystem::exists(path))
    {
        return false;
    }

    try
    {
        std::ifstream input(path);
        input >> theme_json_;
        active_theme_root_ = path.parent_path();
        return true;
    }
    catch (const std::exception& exception)
    {
        platform::Logger::instance().error(
            std::string("Failed to load theme '") + theme_id + "': " + exception.what());
        return false;
    }
}
}
