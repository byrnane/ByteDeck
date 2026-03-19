#include "launch/launch_service.hpp"

#include "platform/logger.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>

#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace bytedeck::launch
{
namespace
{
std::string shell_quote(const std::string& value)
{
    std::string quoted = "'";
    for (char character : value)
    {
        if (character == '\'')
        {
            quoted += "'\\''";
        }
        else
        {
            quoted.push_back(character);
        }
    }
    quoted.push_back('\'');
    return quoted;
}


std::filesystem::path resolve_item_path(const platform::Paths& paths, const std::string& item_path)
{
    const std::filesystem::path path(item_path);
    if (path.is_absolute())
    {
        return path.lexically_normal();
    }

    return (paths.root() / path).lexically_normal();
}


std::string launch_mode()
{
    const char* raw_mode = std::getenv("BYTEDECK_LAUNCH_MODE");
    if (raw_mode == nullptr || *raw_mode == '\0')
    {
        return "mock";
    }

    return raw_mode;
}


bool should_defer_until_shutdown()
{
#if defined(_WIN32)
    return false;
#else
    return launch_mode() == "execute";
#endif
}
}


LaunchService::LaunchService(platform::Paths paths)
    : paths_(std::move(paths))
{
}


LaunchResult LaunchService::launch_game(const data::GameItem& item) const
{
    return launch_item(item.system_id, item.rom_path);
}


LaunchResult LaunchService::launch_app(const data::AppItem& item) const
{
    return launch_item(item.system_id, item.launch_target);
}


LaunchResult LaunchService::launch_item(const std::string& system_id, const std::string& item_path) const
{
    const std::filesystem::path script_path = paths_.scripts_root() / "launch_item.sh";
    if (!std::filesystem::exists(script_path))
    {
        const std::string message = "Launch script not found: " + script_path.generic_string();
        platform::Logger::instance().error(message);
        return { false, message };
    }

    const std::filesystem::path resolved_item_path = resolve_item_path(paths_, item_path);
    const std::string mode = launch_mode();
    const std::string display_message = system_id + " -> " + resolved_item_path.generic_string();

    if (mode != "execute")
    {
        platform::Logger::instance().info("[mock] Launch " + display_message);
        return { true, "MOCK: " + system_id + " -> " + resolved_item_path.filename().string() };
    }

    if (should_defer_until_shutdown())
    {
        platform::Logger::instance().info("Prepared launch command: " + display_message);
        LaunchResult result;
        result.success = true;
        result.message = "EXITING TO LAUNCH: " + system_id;
        result.should_exit_launcher = true;
        result.request = { system_id, resolved_item_path };
        return result;
    }

#if defined(_WIN32)
    const std::string command =
        "sh " +
        shell_quote(script_path.generic_string()) + " " +
        shell_quote(system_id) + " " +
        shell_quote(resolved_item_path.generic_string());
#else
    const std::string command =
        "/bin/sh " +
        shell_quote(script_path.generic_string()) + " " +
        shell_quote(system_id) + " " +
        shell_quote(resolved_item_path.generic_string());
#endif

    platform::Logger::instance().info("Executing launch command: " + command);
    const int exit_code = std::system(command.c_str());
    if (exit_code != 0)
    {
        const std::string message = "Launch failed for " + system_id + " (exit " + std::to_string(exit_code) + ")";
        platform::Logger::instance().error(message);
        return { false, message };
    }

    return { true, "LAUNCHED: " + system_id };
}


bool LaunchService::execute_prepared(const LaunchRequest& request) const
{
    const std::filesystem::path script_path = paths_.scripts_root() / "launch_item.sh";
    if (!std::filesystem::exists(script_path))
    {
        platform::Logger::instance().error("Launch script not found: " + script_path.generic_string());
        return false;
    }

#if defined(_WIN32)
    const std::string command =
        "sh " +
        shell_quote(script_path.generic_string()) + " " +
        shell_quote(request.system_id) + " " +
        shell_quote(request.item_path.generic_string());
    return std::system(command.c_str()) == 0;
#else
    platform::Logger::instance().info(
        "Executing prepared launch: " + request.system_id + " -> " + request.item_path.generic_string()
    );
    execl(
        "/bin/sh",
        "sh",
        script_path.c_str(),
        request.system_id.c_str(),
        request.item_path.c_str(),
        static_cast<char*>(nullptr)
    );
    platform::Logger::instance().error("Failed to exec launch script: " + std::string(std::strerror(errno)));
    return false;
#endif
}
}
