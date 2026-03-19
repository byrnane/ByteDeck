#include "data/models.hpp"

#include <nlohmann/json.hpp>

namespace bytedeck::data
{
void to_json(nlohmann::json& json, const GameItem& item)
{
    json = nlohmann::json {
        { "id", item.id },
        { "type", item.type },
        { "system_id", item.system_id },
        { "rom_path", item.rom_path },
        { "title", item.title },
        { "description", item.description },
        { "thumbnail", item.thumbnail },
        { "genre", item.genre },
        { "players", item.players },
        { "release_date", item.release_date },
        { "metadata_source", item.metadata_source }
    };
}


void to_json(nlohmann::json& json, const AppItem& item)
{
    json = nlohmann::json {
        { "id", item.id },
        { "type", item.type },
        { "system_id", item.system_id },
        { "launch_target", item.launch_target },
        { "title", item.title },
        { "description", item.description },
        { "icon", item.icon }
    };
}


void to_json(nlohmann::json& json, const Collection& collection)
{
    json = nlohmann::json {
        { "id", collection.id },
        { "name", collection.name },
        { "icon", collection.icon },
        { "description", collection.description },
        { "items", collection.items }
    };
}


void to_json(nlohmann::json& json, const SystemEntry& system)
{
    json = nlohmann::json {
        { "id", system.id },
        { "name", system.name },
        { "type", system.type },
        { "item_count", system.item_count },
        { "visible", system.visible }
    };
}


void to_json(nlohmann::json& json, const LibraryData& library)
{
    json = nlohmann::json {
        { "games", library.games },
        { "apps", library.apps },
        { "collections", library.collections },
        { "systems", library.systems }
    };
}
}
