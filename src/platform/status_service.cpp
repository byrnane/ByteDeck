#include "platform/status_service.hpp"

#include "platform/logger.hpp"

#include <SDL.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace bytedeck::platform
{
namespace
{
constexpr Uint32 kClockUpdateMs = 1000;
constexpr Uint32 kBatteryUpdateMs = 15000;
constexpr int kDesktopMockBatteryPercent = 76;
}


StatusService::StatusService(const std::filesystem::path& root_path)
    : root_path_(root_path)
{
    battery_directory_ = discover_battery_directory();
    use_mock_battery_ = !battery_directory_.has_value();
    snapshot_ = build_snapshot();
}


void StatusService::update()
{
    const Uint32 now = SDL_GetTicks();
    if (now - last_time_tick_ >= kClockUpdateMs)
    {
        snapshot_ = build_snapshot();
        last_time_tick_ = now;
        last_battery_tick_ = now;
        return;
    }

    if (!use_mock_battery_ && now - last_battery_tick_ >= kBatteryUpdateMs)
    {
        snapshot_ = build_snapshot();
        last_battery_tick_ = now;
    }
}


const StatusSnapshot& StatusService::snapshot() const
{
    return snapshot_;
}


StatusSnapshot StatusService::build_snapshot()
{
    StatusSnapshot status;

    const std::time_t now_time = std::time(nullptr);
    std::tm local_time {};
#if defined(_WIN32)
    localtime_s(&local_time, &now_time);
#else
    localtime_r(&now_time, &local_time);
#endif

    std::ostringstream time_stream;
    time_stream << std::put_time(&local_time, "%H:%M");
    status.time_text = time_stream.str();

    if (use_mock_battery_)
    {
        status.battery_available = true;
        status.battery_text = std::to_string(kDesktopMockBatteryPercent) + "%";
        status.charging = false;
        return status;
    }

    if (!battery_directory_.has_value())
    {
        return status;
    }

    const std::string capacity = read_trimmed_file(*battery_directory_ / "capacity");
    const std::string charging_status = read_trimmed_file(*battery_directory_ / "status");
    if (!capacity.empty())
    {
        status.battery_available = true;
        status.battery_text = capacity + "%";
    }

    status.charging = charging_status == "Charging" || charging_status == "Full";
    return status;
}


std::optional<std::filesystem::path> StatusService::discover_battery_directory()
{
    const std::filesystem::path power_supply_root("/sys/class/power_supply");
    if (!std::filesystem::exists(power_supply_root))
    {
        return std::nullopt;
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(power_supply_root))
        {
            if (!entry.is_directory())
            {
                continue;
            }

            const std::filesystem::path capacity_path = entry.path() / "capacity";
            if (std::filesystem::exists(capacity_path))
            {
                return entry.path();
            }
        }
    }
    catch (const std::exception& exception)
    {
        Logger::instance().warn(std::string("Failed to discover battery path: ") + exception.what());
    }

    return std::nullopt;
}


std::string StatusService::read_trimmed_file(const std::filesystem::path& path) const
{
    try
    {
        std::ifstream input(path);
        std::string value;
        std::getline(input, value);
        while (!value.empty() && (value.back() == '\n' || value.back() == '\r' || value.back() == ' '))
        {
            value.pop_back();
        }
        return value;
    }
    catch (const std::exception&)
    {
        return {};
    }
}
}
