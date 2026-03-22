#include "platform/paths.hpp"

#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace bytedeck::platform
{
namespace
{
std::filesystem::path path_from_env(const char* variable_name)
{
    const char* value = std::getenv(variable_name);
    if (value == nullptr || *value == '\0')
    {
        return {};
    }

    return std::filesystem::path(value);
}


bool directory_exists(const std::filesystem::path& path)
{
    return !path.empty() && std::filesystem::exists(path) && std::filesystem::is_directory(path);
}


std::filesystem::path first_existing_child(
    const std::filesystem::path& root,
    std::initializer_list<const char*> names)
{
    for (const char* name : names)
    {
        const auto candidate = root / name;
        if (directory_exists(candidate))
        {
            return candidate;
        }
    }

    return {};
}


bool is_root_candidate(const std::filesystem::path& path)
{
    const bool has_config = directory_exists(path / "config");
    const bool has_scripts = directory_exists(path / "scripts");
    return has_config && has_scripts;
}


bool is_repository_root(const std::filesystem::path& path)
{
    return directory_exists(path / "src") || directory_exists(path / "cmake") || directory_exists(path / ".git");
}


std::filesystem::path find_root_candidate()
{
    const auto env_root = path_from_env("BYTEDECK_ROOT");
    if (!env_root.empty())
    {
        if (!directory_exists(env_root))
        {
            throw std::runtime_error("BYTEDECK_ROOT does not exist or is not a directory");
        }
        return env_root;
    }

    std::filesystem::path current = std::filesystem::current_path();

    while (!current.empty())
    {
        if (is_root_candidate(current))
        {
            return current;
        }

        const auto parent = current.parent_path();
        if (parent == current)
        {
            break;
        }
        current = parent;
    }

    throw std::runtime_error("Unable to resolve ByteDeck root from current working directory");
}


std::filesystem::path resolve_rooted_path(
    const std::filesystem::path& root,
    const char* env_name,
    std::initializer_list<const char*> preferred_names,
    const char* default_name)
{
    const auto env_path = path_from_env(env_name);
    if (!env_path.empty())
    {
        return env_path;
    }

    const auto existing_path = first_existing_child(root, preferred_names);
    if (!existing_path.empty())
    {
        return existing_path;
    }

    return root / default_name;
}


std::filesystem::path resolve_cache_root(const std::filesystem::path& root)
{
    const auto env_path = path_from_env("BYTEDECK_CACHE_ROOT");
    if (!env_path.empty())
    {
        return env_path;
    }

    if (is_repository_root(root))
    {
        return root / "out" / "runtime";
    }

    if (directory_exists(root / "cache"))
    {
        return root / "cache";
    }

    return root / "cache";
}
}


Paths Paths::discover()
{
    Paths paths;
    paths.root_ = find_root_candidate();
    paths.roms_root_ = resolve_rooted_path(paths.root_, "BYTEDECK_ROMS_ROOT", {"ROMS", "roms", "Roms"}, "ROMS");
    paths.bios_root_ = resolve_rooted_path(paths.root_, "BYTEDECK_BIOS_ROOT", {"BIOS", "bios", "Bios"}, "BIOS");
    paths.apps_root_ = resolve_rooted_path(paths.root_, "BYTEDECK_APPS_ROOT", {"Apps", "apps", "App"}, "Apps");
    paths.collections_root_ = resolve_rooted_path(
        paths.root_,
        "BYTEDECK_COLLECTIONS_ROOT",
        {"collections", "Collections"},
        "collections");
    paths.cache_root_ = resolve_cache_root(paths.root_);
    paths.scripts_root_ = resolve_rooted_path(paths.root_, "BYTEDECK_SCRIPTS_ROOT", {"scripts", "Scripts"}, "scripts");
    paths.config_root_ = resolve_rooted_path(paths.root_, "BYTEDECK_CONFIG_ROOT", {"config", "Config"}, "config");
    const auto explicit_themes = path_from_env("BYTEDECK_THEMES_ROOT");
    if (!explicit_themes.empty())
    {
        paths.themes_root_ = explicit_themes;
    }
    else
    {
        const auto root_themes = paths.root_ / "themes";
        const auto legacy_themes = paths.config_root_ / "themes";
        paths.themes_root_ = directory_exists(root_themes) ? root_themes : legacy_themes;
    }
    return paths;
}


const std::filesystem::path& Paths::root() const
{
    return root_;
}


const std::filesystem::path& Paths::roms_root() const
{
    return roms_root_;
}


const std::filesystem::path& Paths::bios_root() const
{
    return bios_root_;
}


const std::filesystem::path& Paths::apps_root() const
{
    return apps_root_;
}


const std::filesystem::path& Paths::collections_root() const
{
    return collections_root_;
}


const std::filesystem::path& Paths::cache_root() const
{
    return cache_root_;
}


const std::filesystem::path& Paths::scripts_root() const
{
    return scripts_root_;
}


const std::filesystem::path& Paths::config_root() const
{
    return config_root_;
}


std::filesystem::path Paths::user_settings_path() const
{
    return config_root_ / "user_settings.json";
}


std::filesystem::path Paths::translations_path() const
{
    return config_root_ / "i18n" / "translations.csv";
}


std::filesystem::path Paths::themes_root() const
{
    return themes_root_;
}


std::filesystem::path Paths::log_file_path() const
{
    return cache_root_ / "logs" / "bytedeck.log";
}
}
