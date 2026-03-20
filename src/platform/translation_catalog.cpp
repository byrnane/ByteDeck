#include "platform/translation_catalog.hpp"

#include "platform/logger.hpp"

#include <fstream>
#include <vector>

namespace bytedeck::platform
{
bool TranslationCatalog::load(const std::filesystem::path& path)
{
    entries_.clear();

    if (!std::filesystem::exists(path))
    {
        Logger::instance().warn("Translations file not found: " + path.string());
        return false;
    }

    try
    {
        std::ifstream input(path);
        std::string line;
        std::vector<std::string> header;

        while (std::getline(input, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (line.empty())
            {
                continue;
            }

            const std::vector<std::string> columns = parse_csv_row(line);
            if (header.empty())
            {
                header = columns;
                continue;
            }

            if (columns.empty() || columns[0].empty())
            {
                continue;
            }

            Entry entry;
            for (std::size_t index = 1; index < columns.size() && index < header.size(); ++index)
            {
                entry[header[index]] = columns[index];
            }
            entries_[columns[0]] = std::move(entry);
        }

        return !entries_.empty();
    }
    catch (const std::exception& exception)
    {
        Logger::instance().error(std::string("Failed to load translations: ") + exception.what());
        entries_.clear();
        return false;
    }
}


void TranslationCatalog::set_language(std::string language)
{
    if (!language.empty())
    {
        active_language_ = std::move(language);
    }
}


const std::string& TranslationCatalog::language() const
{
    return active_language_;
}


std::string TranslationCatalog::translate(const std::string& key) const
{
    const auto it = entries_.find(key);
    if (it == entries_.end())
    {
        return key;
    }

    const Entry& entry = it->second;
    auto language_it = entry.find(active_language_);
    if (language_it != entry.end() && !language_it->second.empty())
    {
        return language_it->second;
    }

    language_it = entry.find("en");
    if (language_it != entry.end() && !language_it->second.empty())
    {
        return language_it->second;
    }

    return key;
}


std::vector<std::string> TranslationCatalog::parse_csv_row(const std::string& line)
{
    std::vector<std::string> columns;
    std::string current;
    bool in_quotes = false;

    for (std::size_t index = 0; index < line.size(); ++index)
    {
        const char character = line[index];
        if (in_quotes)
        {
            if (character == '"')
            {
                if (index + 1 < line.size() && line[index + 1] == '"')
                {
                    current.push_back('"');
                    ++index;
                }
                else
                {
                    in_quotes = false;
                }
            }
            else
            {
                current.push_back(character);
            }
            continue;
        }

        if (character == '"')
        {
            in_quotes = true;
        }
        else if (character == ',')
        {
            columns.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(character);
        }
    }

    columns.push_back(current);
    return columns;
}
}
