#pragma once

#include "data/models.hpp"
#include "platform/paths.hpp"

#include <filesystem>
#include <string>
#include <utility>

namespace bytedeck::launch
{
struct LaunchRequest
{
    std::string system_id;
    std::filesystem::path item_path;
};


struct LaunchResult
{
    bool success = false;
    std::string message;
    bool should_exit_launcher = false;
    LaunchRequest request;

    LaunchResult() = default;

    LaunchResult(bool launch_success, std::string launch_message)
        : success(launch_success)
        , message(std::move(launch_message))
    {
    }

    LaunchResult(bool launch_success, std::string launch_message, bool exit_launcher, LaunchRequest launch_request)
        : success(launch_success)
        , message(std::move(launch_message))
        , should_exit_launcher(exit_launcher)
        , request(std::move(launch_request))
    {
    }
};


class LaunchService
{
public:
    explicit LaunchService(platform::Paths paths);

    LaunchResult launch_game(const data::GameItem& item) const;
    LaunchResult launch_app(const data::AppItem& item) const;
    bool execute_prepared(const LaunchRequest& request) const;

private:
    LaunchResult launch_item(const std::string& system_id, const std::string& item_path) const;

    platform::Paths paths_;
};
}
