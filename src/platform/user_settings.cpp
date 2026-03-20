#include "platform/user_settings.hpp"

#include "platform/logger.hpp"

#include <exception>
#include <fstream>

#include <nlohmann/json.hpp>

namespace bytedeck::platform
{
UserSettings UserSettings::load(const std::filesystem::path& path)
{
    UserSettings settings;

    if (!std::filesystem::exists(path))
    {
        Logger::instance().warn("User settings file not found, using defaults");
        return settings;
    }

    try
    {
        std::ifstream input(path);
        nlohmann::json json;
        input >> json;

        settings.language = json.value("language", settings.language);
        settings.theme = json.value("theme", settings.theme);
        settings.confirm_before_shutdown = json.value("confirm_before_shutdown", settings.confirm_before_shutdown);
    }
    catch (const std::exception& exception)
    {
        Logger::instance().warn(std::string("Failed to load user settings: ") + exception.what());
    }

    return settings;
}


bool UserSettings::save(const std::filesystem::path& path) const
{
    try
    {
        const std::filesystem::path parent = path.parent_path();
        if (!parent.empty())
        {
            std::filesystem::create_directories(parent);
        }

        nlohmann::json json = {
            { "language", language },
            { "theme", theme },
            { "confirm_before_shutdown", confirm_before_shutdown }
        };

        std::ofstream output(path, std::ios::out | std::ios::trunc);
        output << json.dump(2) << '\n';
        return true;
    }
    catch (const std::exception& exception)
    {
        Logger::instance().warn(std::string("Failed to save user settings: ") + exception.what());
        return false;
    }
}
}
