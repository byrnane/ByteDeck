#pragma once

#include <SDL.h>

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

namespace bytedeck::platform
{
struct StatusSnapshot
{
    std::string time_text;
    std::string battery_text = "--%";
    bool battery_available = false;
    bool charging = false;
};


class StatusService
{
public:
    explicit StatusService(const std::filesystem::path& root_path);

    void update();
    const StatusSnapshot& snapshot() const;

private:
    StatusSnapshot build_snapshot();
    std::optional<std::filesystem::path> discover_battery_directory();
    std::string read_trimmed_file(const std::filesystem::path& path) const;

    std::filesystem::path root_path_;
    bool use_mock_battery_ = true;
    Uint32 last_time_tick_ = 0;
    Uint32 last_battery_tick_ = 0;
    StatusSnapshot snapshot_;
    std::optional<std::filesystem::path> battery_directory_;
};
}
