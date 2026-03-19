#include "core/library_scanner.hpp"

#include "core/gamelist_parser.hpp"
#include "platform/logger.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <exception>
#include <fstream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace bytedeck::core
{
namespace
{
struct SupportedSystem
{
    const char* id;
    const char* name;
    std::vector<std::string> extensions;
};


const std::vector<SupportedSystem>& supported_systems()
{
    static const std::vector<SupportedSystem> systems {
        { "nes", "NES", { ".nes", ".zip" } },
        { "snes", "SNES", { ".sfc", ".smc", ".zip" } },
        { "megadrive", "Mega Drive", { ".md", ".bin", ".gen", ".zip" } },
        { "psp", "PSP", { ".iso", ".cso" } }
    };

    return systems;
}


std::string lowercase(std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); }
    );
    return value;
}


std::string slugify(const std::string& value)
{
    std::string slug;
    bool previous_underscore = false;

    for (unsigned char character : value)
    {
        if (std::isalnum(character) != 0)
        {
            slug.push_back(static_cast<char>(std::tolower(character)));
            previous_underscore = false;
        }
        else if (!previous_underscore)
        {
            slug.push_back('_');
            previous_underscore = true;
        }
    }

    while (!slug.empty() && slug.front() == '_')
    {
        slug.erase(slug.begin());
    }
    while (!slug.empty() && slug.back() == '_')
    {
        slug.pop_back();
    }

    if (slug.empty())
    {
        slug = "item";
    }

    return slug;
}


std::string make_portable_path(const platform::Paths& paths, const std::filesystem::path& input_path)
{
    if (input_path.empty())
    {
        return {};
    }

    const std::filesystem::path normalized = input_path.lexically_normal();
    const std::filesystem::path root = paths.root().lexically_normal();

    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(normalized, root, error);
    const std::string relative_string = relative.generic_string();
    if (!error && !relative.empty() && relative_string.rfind("..", 0) != 0)
    {
        return relative_string;
    }

    return normalized.generic_string();
}


std::vector<std::filesystem::path> sorted_directory_entries(const std::filesystem::path& directory)
{
    std::vector<std::filesystem::path> entries;

    std::error_code error;
    if (!std::filesystem::exists(directory, error) || !std::filesystem::is_directory(directory, error))
    {
        return entries;
    }

    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory, error))
    {
        if (error)
        {
            break;
        }
        entries.push_back(entry.path());
    }

    std::sort(entries.begin(), entries.end());
    return entries;
}


bool has_supported_extension(const std::filesystem::path& path, const SupportedSystem& system)
{
    const std::string extension = lowercase(path.extension().string());
    return std::find(system.extensions.begin(), system.extensions.end(), extension) != system.extensions.end();
}


std::string build_unique_id(
    const std::string& prefix,
    const std::string& seed,
    std::unordered_set<std::string>& used_ids
)
{
    std::string candidate = prefix + ":" + slugify(seed);
    if (used_ids.insert(candidate).second)
    {
        return candidate;
    }

    int suffix = 2;
    for (;; ++suffix)
    {
        std::string with_suffix = candidate + "_" + std::to_string(suffix);
        if (used_ids.insert(with_suffix).second)
        {
            return with_suffix;
        }
    }
}


std::string coalesce_title(const ParsedGameMetadata* metadata, const std::filesystem::path& rom_path)
{
    if (metadata != nullptr && !metadata->title.empty())
    {
        return metadata->title;
    }

    return rom_path.stem().string();
}


std::string safe_portable_media_path(
    const platform::Paths& paths,
    const std::filesystem::path& media_path
)
{
    std::error_code error;
    if (media_path.empty() || !std::filesystem::exists(media_path, error))
    {
        return {};
    }

    return make_portable_path(paths, media_path);
}


data::GameItem build_game_item(
    const platform::Paths& paths,
    const SupportedSystem& system,
    const std::filesystem::path& rom_path,
    const ParsedGameMetadata* metadata,
    std::unordered_set<std::string>& used_ids
)
{
    data::GameItem item;
    item.id = build_unique_id(system.id, rom_path.stem().string(), used_ids);
    item.system_id = system.id;
    item.rom_path = make_portable_path(paths, rom_path);
    item.title = coalesce_title(metadata, rom_path);
    item.description = metadata != nullptr ? metadata->description : "";
    item.thumbnail = metadata != nullptr ? safe_portable_media_path(paths, metadata->thumbnail_path) : "";
    item.genre = metadata != nullptr ? metadata->genre : "";
    item.players = metadata != nullptr ? metadata->players : "";
    item.release_date = metadata != nullptr ? metadata->release_date : "";
    item.metadata_source = metadata != nullptr ? "gamelist" : "fallback";
    return item;
}


std::set<std::string> build_valid_item_id_set(const data::LibraryData& library)
{
    std::set<std::string> valid_ids;
    for (const data::GameItem& game : library.games)
    {
        valid_ids.insert(game.id);
    }
    for (const data::AppItem& app : library.apps)
    {
        valid_ids.insert(app.id);
    }
    return valid_ids;
}
}


LibraryScanner::LibraryScanner(const platform::Paths& paths)
    : paths_(paths)
{
}


data::LibraryData LibraryScanner::scan() const
{
    data::LibraryData library;
    GamelistParser parser;
    std::unordered_set<std::string> used_ids;

    for (const SupportedSystem& system : supported_systems())
    {
        const std::filesystem::path system_root = paths_.roms_root() / system.id;
        const std::vector<std::filesystem::path> entries = sorted_directory_entries(system_root);
        std::vector<std::filesystem::path> rom_files;
        std::unordered_set<std::string> rom_file_keys;

        for (const std::filesystem::path& entry_path : entries)
        {
            std::error_code error;
            if (!std::filesystem::is_regular_file(entry_path, error) || error)
            {
                continue;
            }
            if (has_supported_extension(entry_path, system))
            {
                const std::filesystem::path normalized = entry_path.lexically_normal();
                rom_files.push_back(normalized);
                rom_file_keys.insert(normalized.generic_string());
            }
        }

        std::unordered_map<std::string, ParsedGameMetadata> metadata_by_rom_path;
        const std::filesystem::path gamelist_path = system_root / "gamelist.xml";
        std::error_code error;
        if (std::filesystem::exists(gamelist_path, error))
        {
            for (const ParsedGameMetadata& metadata : parser.parse(gamelist_path, system_root))
            {
                const std::string key = metadata.rom_path.generic_string();
                if (metadata_by_rom_path.find(key) == metadata_by_rom_path.end())
                {
                    metadata_by_rom_path.emplace(key, metadata);
                }
            }
        }

        for (const auto& [metadata_path, metadata] : metadata_by_rom_path)
        {
            if (rom_file_keys.find(metadata_path) == rom_file_keys.end())
            {
                platform::Logger::instance().warn(
                    "Ignoring gamelist entry that points to a missing ROM: " + metadata_path
                );
            }
        }

        for (const std::filesystem::path& rom_path : rom_files)
        {
            const std::string key = rom_path.generic_string();
            const auto metadata_it = metadata_by_rom_path.find(key);
            const ParsedGameMetadata* metadata = metadata_it != metadata_by_rom_path.end()
                ? &metadata_it->second
                : nullptr;

            library.games.push_back(build_game_item(paths_, system, rom_path, metadata, used_ids));
        }
    }

    {
        const std::vector<std::filesystem::path> app_entries = sorted_directory_entries(paths_.apps_root());
        for (const std::filesystem::path& app_directory : app_entries)
        {
            std::error_code error;
            if (!std::filesystem::is_directory(app_directory, error) || error)
            {
                continue;
            }

            const std::filesystem::path manifest_path = app_directory / "manifest.json";
            if (!std::filesystem::exists(manifest_path, error))
            {
                continue;
            }

            try
            {
                std::ifstream input(manifest_path);
                nlohmann::json manifest;
                input >> manifest;

                const std::string manifest_id = manifest.value("id", "");
                const std::string manifest_type = manifest.value("type", "");
                const std::string name = manifest.value("name", "");
                const std::string description = manifest.value("description", "");
                const std::string icon = manifest.value("icon", "");

                if (manifest_id.empty() || name.empty() || manifest_type != "app")
                {
                    platform::Logger::instance().warn("Skipping invalid app manifest: " + manifest_path.generic_string());
                    continue;
                }

                const nlohmann::json launch = manifest.value("launch", nlohmann::json::object());
                const std::string launch_type = launch.value("type", "");
                const std::string launch_path = launch.value("path", "");
                if (launch_type != "script" || launch_path.empty())
                {
                    platform::Logger::instance().warn("Skipping app manifest with invalid launch target: " + manifest_path.generic_string());
                    continue;
                }

                data::AppItem item;
                item.id = build_unique_id("apps", manifest_id, used_ids);
                item.launch_target = make_portable_path(paths_, app_directory / launch_path);
                item.title = name;
                item.description = description;
                item.icon = safe_portable_media_path(paths_, app_directory / icon);
                library.apps.push_back(std::move(item));
            }
            catch (const std::exception& exception)
            {
                platform::Logger::instance().warn(
                    "Skipping broken app manifest " + manifest_path.generic_string() + ": " + exception.what()
                );
            }
        }
    }

    {
        const std::set<std::string> valid_item_ids = build_valid_item_id_set(library);
        const std::vector<std::filesystem::path> collection_files = sorted_directory_entries(paths_.collections_root());

        for (const std::filesystem::path& collection_file : collection_files)
        {
            std::error_code error;
            if (!std::filesystem::is_regular_file(collection_file, error) || error || collection_file.extension() != ".json")
            {
                continue;
            }

            try
            {
                std::ifstream input(collection_file);
                nlohmann::json json;
                input >> json;

                data::Collection collection;
                collection.id = json.value("id", collection_file.stem().string());
                collection.name = json.value("name", collection.id);
                collection.description = json.value("description", "");

                const std::string icon = json.value("icon", "");
                collection.icon = safe_portable_media_path(paths_, paths_.collections_root() / icon);

                const nlohmann::json items = json.value("items", nlohmann::json::array());
                if (items.is_array())
                {
                    for (const nlohmann::json& item_ref : items)
                    {
                        if (!item_ref.is_string())
                        {
                            continue;
                        }

                        const std::string id = item_ref.get<std::string>();
                        if (valid_item_ids.find(id) != valid_item_ids.end())
                        {
                            collection.items.push_back(id);
                        }
                        else
                        {
                            platform::Logger::instance().warn(
                                "Skipping missing collection item reference: " + id + " in " + collection_file.generic_string()
                            );
                        }
                    }
                }

                library.collections.push_back(std::move(collection));
            }
            catch (const std::exception& exception)
            {
                platform::Logger::instance().warn(
                    "Skipping broken collection file " + collection_file.generic_string() + ": " + exception.what()
                );
            }
        }
    }

    for (const SupportedSystem& system : supported_systems())
    {
        const int game_count = static_cast<int>(std::count_if(
            library.games.begin(),
            library.games.end(),
            [&system](const data::GameItem& item)
            {
                return item.system_id == system.id;
            }
        ));

        library.systems.push_back({
            system.id,
            system.name,
            "game_system",
            game_count,
            game_count > 0
        });
    }

    const int visible_collection_count = static_cast<int>(std::count_if(
        library.collections.begin(),
        library.collections.end(),
        [](const data::Collection& collection)
        {
            return !collection.items.empty();
        }
    ));
    library.systems.push_back({
        "collections",
        "Collections",
        "collection_group",
        visible_collection_count,
        visible_collection_count > 0
    });

    const int app_count = static_cast<int>(library.apps.size());
    library.systems.push_back({
        "apps",
        "Apps",
        "app_group",
        app_count,
        app_count > 0
    });

    platform::Logger::instance().info(
        "Library scan completed: games=" + std::to_string(library.games.size()) +
        " apps=" + std::to_string(library.apps.size()) +
        " collections=" + std::to_string(library.collections.size())
    );

    return library;
}


bool LibraryScanner::save_cache(const data::LibraryData& library) const
{
    try
    {
        std::filesystem::create_directories(paths_.cache_root());
        const std::filesystem::path cache_path = paths_.cache_root() / "library.json";

        std::ofstream output(cache_path);
        if (!output.is_open())
        {
            platform::Logger::instance().error("Failed to open library cache for writing: " + cache_path.generic_string());
            return false;
        }

        nlohmann::json json = library;
        output << json.dump(2);
        output << '\n';

        platform::Logger::instance().info("Saved library cache to " + cache_path.generic_string());
        return true;
    }
    catch (const std::exception& exception)
    {
        platform::Logger::instance().error(std::string("Failed to save library cache: ") + exception.what());
        return false;
    }
}
}
