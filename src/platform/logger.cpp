#include "platform/logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace bytedeck::platform
{
Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}


void Logger::initialize(const std::filesystem::path& log_file_path)
{
    std::scoped_lock lock(mutex_);

    const auto parent = log_file_path.parent_path();
    if (!parent.empty())
    {
        std::filesystem::create_directories(parent);
    }

    stream_.open(log_file_path, std::ios::out | std::ios::trunc);
}


void Logger::info(const std::string& message)
{
    write_line("INFO", message);
}


void Logger::warn(const std::string& message)
{
    write_line("WARN", message);
}


void Logger::error(const std::string& message)
{
    write_line("ERROR", message);
}


void Logger::write_line(const std::string& level, const std::string& message)
{
    std::scoped_lock lock(mutex_);

    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time {};
#if defined(_WIN32)
    localtime_s(&local_time, &now_time);
#else
    localtime_r(&now_time, &local_time);
#endif

    std::ostringstream line;
    line << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << " [" << level << "] " << message;

    std::cout << line.str() << '\n';
    if (stream_.is_open())
    {
        stream_ << line.str() << '\n';
        stream_.flush();
    }
}
}
