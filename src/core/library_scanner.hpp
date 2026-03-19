#pragma once

#include "data/models.hpp"
#include "platform/paths.hpp"

#include <filesystem>

namespace bytedeck::core
{
class LibraryScanner
{
public:
    explicit LibraryScanner(const platform::Paths& paths);

    data::LibraryData scan() const;
    bool save_cache(const data::LibraryData& library) const;

private:
    const platform::Paths& paths_;
};
}
