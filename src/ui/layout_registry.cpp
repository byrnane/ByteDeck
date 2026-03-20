#include "ui/layout_registry.hpp"

#include "platform/logger.hpp"

#include <fstream>
#include <stdexcept>

namespace bytedeck::ui
{
namespace
{
std::vector<std::string> parse_string_array(const nlohmann::json& json)
{
    std::vector<std::string> values;
    if (!json.is_array())
    {
        return values;
    }

    for (const nlohmann::json& entry : json)
    {
        if (entry.is_string())
        {
            values.push_back(entry.get<std::string>());
        }
    }

    return values;
}
}


LayoutRegistry::LayoutRegistry(std::filesystem::path screens_root)
    : screens_root_(std::move(screens_root))
{
}


bool LayoutRegistry::load()
{
    documents_.clear();
    errors_.clear();

    if (!std::filesystem::exists(screens_root_))
    {
        platform::Logger::instance().warn("UI screens directory not found: " + screens_root_.string());
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(screens_root_))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
        {
            continue;
        }

        load_file(entry.path());
    }

    return !documents_.empty();
}


const LayoutDocument* LayoutRegistry::find_document(const std::string& screen_id) const
{
    const auto it = documents_.find(screen_id);
    if (it == documents_.end())
    {
        return nullptr;
    }

    return &it->second;
}


const LayoutNode* LayoutRegistry::find_layout(const std::string& screen_id, const std::string& variant, std::string* error_message) const
{
    const auto error_it = errors_.find(screen_id);
    if (error_it != errors_.end())
    {
        if (error_message != nullptr)
        {
            *error_message = error_it->second;
        }
        return nullptr;
    }

    const LayoutDocument* document = find_document(screen_id);
    if (document == nullptr)
    {
        if (error_message != nullptr)
        {
            *error_message = "Layout document not found for screen: " + screen_id;
        }
        return nullptr;
    }

    auto variant_it = document->variants.find(variant);
    if (variant_it != document->variants.end())
    {
        return &variant_it->second;
    }

    variant_it = document->variants.find("default");
    if (variant_it != document->variants.end())
    {
        return &variant_it->second;
    }

    if (error_message != nullptr)
    {
        *error_message = "No layout variant found for screen: " + screen_id;
    }
    return nullptr;
}


LayoutNode LayoutRegistry::parse_node(const nlohmann::json& json, const std::filesystem::path& source_path) const
{
    if (!json.is_object())
    {
        throw std::runtime_error("Layout node must be an object in " + source_path.string());
    }

    if (!json.contains("type") || !json["type"].is_string())
    {
        throw std::runtime_error("Layout node is missing string field 'type' in " + source_path.string());
    }

    LayoutNode node;
    node.type = json["type"].get<std::string>();
    if (json.contains("id") && json["id"].is_string())
    {
        node.id = json["id"].get<std::string>();
    }

    if (json.contains("classes"))
    {
        node.classes = parse_string_array(json["classes"]);
    }

    if (json.contains("conditional_classes") && json["conditional_classes"].is_array())
    {
        for (const nlohmann::json& entry : json["conditional_classes"])
        {
            if (!entry.is_object() || !entry.contains("bind") || !entry.contains("class"))
            {
                continue;
            }

            if (!entry["bind"].is_string() || !entry["class"].is_string())
            {
                continue;
            }

            node.conditional_classes.push_back({
                entry["bind"].get<std::string>(),
                entry["class"].get<std::string>()
            });
        }
    }

    for (auto it = json.begin(); it != json.end(); ++it)
    {
        if (it.key() == "type" || it.key() == "id" || it.key() == "classes" || it.key() == "children" || it.key() == "conditional_classes")
        {
            continue;
        }

        node.props[it.key()] = it.value();
    }

    if (json.contains("children"))
    {
        if (!json["children"].is_array())
        {
            throw std::runtime_error("Layout node field 'children' must be an array in " + source_path.string());
        }

        for (const nlohmann::json& child : json["children"])
        {
            node.children.push_back(parse_node(child, source_path));
        }
    }

    return node;
}


void LayoutRegistry::load_file(const std::filesystem::path& path)
{
    const std::string screen_id = path.stem().string();

    try
    {
        std::ifstream input(path);
        nlohmann::json json;
        input >> json;

        LayoutDocument document;
        document.screen_id = json.value("screen_id", screen_id);

        if (!json.contains("variants") || !json["variants"].is_object())
        {
            throw std::runtime_error("Layout document must contain object field 'variants'");
        }

        for (auto it = json["variants"].begin(); it != json["variants"].end(); ++it)
        {
            document.variants.emplace(it.key(), parse_node(it.value(), path));
        }

        documents_[document.screen_id] = std::move(document);
    }
    catch (const std::exception& exception)
    {
        const std::string message = std::string("Failed to load UI layout '") + screen_id + "': " + exception.what();
        errors_[screen_id] = message;
        platform::Logger::instance().error(message);
    }
}
}
