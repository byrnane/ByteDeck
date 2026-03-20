#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace bytedeck::platform
{
class TranslationCatalog
{
public:
    bool load(const std::filesystem::path& path);

    void set_language(std::string language);
    const std::string& language() const;

    std::string translate(const std::string& key) const;

private:
    using Entry = std::unordered_map<std::string, std::string>;

    static std::vector<std::string> parse_csv_row(const std::string& line);

    std::string active_language_ = "en";
    std::unordered_map<std::string, Entry> entries_;
};
}
