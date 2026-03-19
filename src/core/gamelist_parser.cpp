#include "core/gamelist_parser.hpp"

#include "platform/logger.hpp"

#include <tinyxml2.h>

#include <utility>

namespace bytedeck::core
{
namespace
{
std::string read_text(const tinyxml2::XMLElement* parent, const char* child_name)
{
    if (parent == nullptr)
    {
        return {};
    }

    const tinyxml2::XMLElement* child = parent->FirstChildElement(child_name);
    if (child == nullptr || child->GetText() == nullptr)
    {
        return {};
    }

    return child->GetText();
}


std::filesystem::path resolve_relative_path(const std::filesystem::path& system_root, const std::string& raw_path)
{
    if (raw_path.empty())
    {
        return {};
    }

    const std::filesystem::path input_path(raw_path);
    if (input_path.is_absolute())
    {
        return input_path.lexically_normal();
    }

    return (system_root / input_path).lexically_normal();
}
}


std::vector<ParsedGameMetadata> GamelistParser::parse(
    const std::filesystem::path& gamelist_path,
    const std::filesystem::path& system_root
) const
{
    std::vector<ParsedGameMetadata> result;

    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError load_result = document.LoadFile(gamelist_path.string().c_str());
    if (load_result != tinyxml2::XML_SUCCESS)
    {
        platform::Logger::instance().warn(
            "Failed to parse gamelist.xml: " + gamelist_path.generic_string() + " error=" + document.ErrorStr()
        );
        return result;
    }

    const tinyxml2::XMLElement* game_list = document.FirstChildElement("gameList");
    if (game_list == nullptr)
    {
        platform::Logger::instance().warn("gamelist.xml missing <gameList>: " + gamelist_path.generic_string());
        return result;
    }

    for (const tinyxml2::XMLElement* game = game_list->FirstChildElement("game");
         game != nullptr;
         game = game->NextSiblingElement("game"))
    {
        const std::string raw_rom_path = read_text(game, "path");
        if (raw_rom_path.empty())
        {
            platform::Logger::instance().warn("Skipping gamelist entry without path in " + gamelist_path.generic_string());
            continue;
        }

        ParsedGameMetadata metadata;
        metadata.rom_path = resolve_relative_path(system_root, raw_rom_path);
        metadata.thumbnail_path = resolve_relative_path(system_root, read_text(game, "thumbnail"));
        metadata.title = read_text(game, "name");
        metadata.description = read_text(game, "desc");
        metadata.genre = read_text(game, "genre");
        metadata.players = read_text(game, "players");
        metadata.release_date = read_text(game, "releasedate");
        result.push_back(std::move(metadata));
    }

    return result;
}
}
