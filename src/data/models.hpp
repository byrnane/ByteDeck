#pragma once

#include <nlohmann/json_fwd.hpp>

#include <string>
#include <vector>

namespace bytedeck::data
{
struct GameItem
{
    std::string id;
    std::string type = "game";
    std::string system_id;
    std::string rom_path;
    std::string title;
    std::string description;
    std::string thumbnail;
    std::string genre;
    std::string players;
    std::string release_date;
    std::string metadata_source;
};


struct AppItem
{
    std::string id;
    std::string type = "app";
    std::string system_id = "apps";
    std::string launch_target;
    std::string title;
    std::string description;
    std::string icon;
};


struct Collection
{
    std::string id;
    std::string name;
    std::string icon;
    std::string description;
    std::vector<std::string> items;
};


struct SystemEntry
{
    std::string id;
    std::string name;
    std::string type;
    int item_count = 0;
    bool visible = false;
};


struct LibraryData
{
    std::vector<GameItem> games;
    std::vector<AppItem> apps;
    std::vector<Collection> collections;
    std::vector<SystemEntry> systems;
};


void to_json(nlohmann::json& json, const GameItem& item);
void to_json(nlohmann::json& json, const AppItem& item);
void to_json(nlohmann::json& json, const Collection& collection);
void to_json(nlohmann::json& json, const SystemEntry& system);
void to_json(nlohmann::json& json, const LibraryData& library);
}
