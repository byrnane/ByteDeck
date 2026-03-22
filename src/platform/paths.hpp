#pragma once

#include <filesystem>

namespace bytedeck::platform
{
class Paths
{
public:
    static Paths discover();

    const std::filesystem::path& root() const;
    const std::filesystem::path& roms_root() const;
    const std::filesystem::path& bios_root() const;
    const std::filesystem::path& apps_root() const;
    const std::filesystem::path& collections_root() const;
    const std::filesystem::path& cache_root() const;
    const std::filesystem::path& scripts_root() const;
    const std::filesystem::path& config_root() const;

    std::filesystem::path user_settings_path() const;
    std::filesystem::path translations_path() const;
    std::filesystem::path themes_root() const;
    std::filesystem::path log_file_path() const;

private:
    std::filesystem::path root_;
    std::filesystem::path roms_root_;
    std::filesystem::path bios_root_;
    std::filesystem::path apps_root_;
    std::filesystem::path collections_root_;
    std::filesystem::path cache_root_;
    std::filesystem::path scripts_root_;
    std::filesystem::path config_root_;
    std::filesystem::path themes_root_;
};
}
