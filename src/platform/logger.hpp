#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace bytedeck::platform
{
class Logger
{
public:
    static Logger& instance();

    void initialize(const std::filesystem::path& log_file_path);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

private:
    Logger() = default;

    void write_line(const std::string& level, const std::string& message);

    std::mutex mutex_;
    std::ofstream stream_;
};
}
