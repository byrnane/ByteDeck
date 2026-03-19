#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bytedeck::core
{
struct ParsedGameMetadata
{
    std::filesystem::path rom_path;
    std::filesystem::path thumbnail_path;
    std::string title;
    std::string description;
    std::string genre;
    std::string players;
    std::string release_date;
};


class GamelistParser
{
public:
    std::vector<ParsedGameMetadata> parse(
        const std::filesystem::path& gamelist_path,
        const std::filesystem::path& system_root
    ) const;
};
}
