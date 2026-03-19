#pragma once

#include <filesystem>
#include <string>

namespace bytedeck::platform
{
struct UserSettings
{
    std::string language = "en";
    bool confirm_before_shutdown = false;

    static UserSettings load(const std::filesystem::path& path);
};
}
