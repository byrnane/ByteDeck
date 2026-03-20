#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace bytedeck::ui
{
struct ConditionalClass
{
    std::string bind;
    std::string class_name;
};


struct LayoutNode
{
    std::string type;
    std::string id;
    std::vector<std::string> classes;
    std::vector<ConditionalClass> conditional_classes;
    nlohmann::json props = nlohmann::json::object();
    std::vector<LayoutNode> children;
};


struct LayoutDocument
{
    std::string screen_id;
    std::unordered_map<std::string, LayoutNode> variants;
};


class LayoutRegistry
{
public:
    explicit LayoutRegistry(std::filesystem::path screens_root);

    bool load();
    const LayoutDocument* find_document(const std::string& screen_id) const;
    const LayoutNode* find_layout(const std::string& screen_id, const std::string& variant, std::string* error_message = nullptr) const;

private:
    LayoutNode parse_node(const nlohmann::json& json, const std::filesystem::path& source_path) const;
    void load_file(const std::filesystem::path& path);

    std::filesystem::path screens_root_;
    std::unordered_map<std::string, LayoutDocument> documents_;
    std::unordered_map<std::string, std::string> errors_;
};
}
