#pragma once

#include <filesystem>
#include <string>

namespace bytedeck::platform
{
struct UserSettings
{
    std::string language = "en";
    std::string theme = "default";
    bool confirm_before_shutdown = false;

    static UserSettings load(const std::filesystem::path& path);
};
}
