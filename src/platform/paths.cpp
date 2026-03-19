#include "platform/paths.hpp"

#include <filesystem>
#include <stdexcept>

namespace bytedeck::platform
{
namespace
{
std::filesystem::path find_root_candidate()
{
    std::filesystem::path current = std::filesystem::current_path();

    while (!current.empty())
    {
        const bool has_config = std::filesystem::exists(current / "config");
        const bool has_spec = std::filesystem::exists(current / "trimui_launcher_spec.md");
        if (has_config && has_spec)
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
}


Paths Paths::discover()
{
    Paths paths;
    paths.root_ = find_root_candidate();
    paths.roms_root_ = paths.root_ / "roms";
    paths.bios_root_ = paths.root_ / "bios";
    paths.apps_root_ = paths.root_ / "Apps";
    paths.collections_root_ = paths.root_ / "collections";
    paths.cache_root_ = paths.root_ / "cache";
    paths.scripts_root_ = paths.root_ / "scripts";
    paths.config_root_ = paths.root_ / "config";
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


std::filesystem::path Paths::log_file_path() const
{
    return cache_root_ / "logs" / "bytedeck.log";
}
}
